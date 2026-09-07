"""Controlled same-TU layout probes for the PE/COFF i386 backend.

Pointer-parameter probes observe existing declarations. Counterfactual class
definitions can separately establish no prior complete definition at a namespace
scope; their diagnostic types are never recovered layouts. Neither result proves
product code preservation, ABI independence or object lifetime.
The compiler's ordinary overlay protocol owns all toolchain interaction.
"""

import re
from pathlib import Path

from .codeview import ClassLayouts, ClassNotEmitted
from .coff_replica import function_signature, object_signature
from .components import local_path
from .model import Result, digest
from .pe_coff import Coff


def declaration_absent(witness):
    """Recognize this backend's complete absence-control contract, not a verdict alone."""
    return (isinstance(witness, dict) and witness.get("basis") == "same_tu_definition_absence"
            and witness.get("result") == witness.get("replica_control") == Result.PASS
            and "compiler_layout" in witness and witness["compiler_layout"] is None
            and witness.get("definition_controls") == [{"extent": 17, "result": Result.PASS},
                                                       {"extent": 29, "result": Result.PASS}]
            and isinstance(witness.get("probe_symbol"), str) and witness["probe_symbol"].startswith("_match_recovery_"))


def _absence_witnesses(compile, original, probes):
    """Positive counterfactuals, not inference from a failed compiler command.

    C++ forbids a second complete definition in the same namespace. Accepting
    two different diagnostic definitions establishes only that no conflicting
    complete definition was present there. It does not establish ABI independence
    or the absence of raw/untyped consumers. Diagnostic types never enter the
    recovered layout or product-object cache.
    """
    signatures = {name: [] for name, _, _ in probes}
    for width in (17, 29):
        snippets = []
        for name, identifier, guards in probes:
            parts = name.split("::")
            opening = "".join("namespace " + part + " {\n" for part in parts[:-1])
            closing = "}\n" * (len(parts) - 1)
            snippets.append("\n\n" + guards + "\n" + opening + "struct " + parts[-1] +
                            " { char " + identifier + "_sentinel[" + str(width) + "]; };\n" + closing +
                            'extern "C" int ' + identifier + "(" + name + "* object) { return sizeof(" + name + "); }\n")
        obj = compile(original + "".join(snippets).encode("ascii"))
        types = ClassLayouts(obj)
        for name, identifier, _ in probes:
            member = identifier + "_sentinel"
            layout = types.layout(name)
            if (layout["size"] != width or layout["members"] != {member: 0} or layout["bases"]
                    or layout["member_type_kinds"] != {member: "array"}):
                raise ValueError("class absence control did not emit its exact diagnostic definition")
            signatures[name].append(function_signature(obj, "_" + identifier))
    if any(first == second for first, second in signatures.values()):
        raise ValueError("class absence changed-definition control was not observed")
    return [{"basis": "same_tu_definition_absence", "result": Result.PASS,
             "compiler_layout": None, "replica_control": Result.PASS,
             "definition_controls": [{"extent": width, "result": Result.PASS} for width in (17, 29)],
             "probe_symbol": "_" + identifier,
             "reason": "two distinct complete diagnostic definitions accepted at the requested namespace scope; no prior complete declaration there; ABI independence unproved"}
            for _, identifier, _ in probes]


def missing_layout_witnesses(root, unit, names, baseline, compiler, command, flags):
    """Probe missing definitions only; never replace a malformed/observed one.

    One unchanged replica controls the TU's private include context. Each class
    then needs an emitted probe and a changed-body control under identical
    flags. Missing definitions need independent counterfactual controls; failed
    controls remain unknown. Reports are transient, not source claims.
    """
    root = Path(root).resolve()
    source = local_path(root, unit)
    if source.resolve() != source or not source.is_file():
        raise ValueError("layout witness requires a canonical existing TU")
    if not isinstance(names, (list, tuple)) or not names or any(
            not isinstance(name, str) or not re.fullmatch(r"[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*", name, re.ASCII)
            for name in names) or len(set(names)) != len(names):
        raise ValueError("layout witnesses require unique qualified C++ class names")
    if any(word.lower().startswith(("/yu", "/yc", "/fp", "-yu", "-yc", "-fp", "@"))
           for word in (*command, *flags)):
        raise ValueError("layout witnesses require standalone compilation")
    types = ClassLayouts(baseline)
    pending_absences = []
    for name in names:
        try:
            types.layout(name)
        except ClassNotEmitted:
            continue
        raise ValueError("layout witness cannot replace an observed definition")
    original = source.read_bytes()
    attempts, rows = 0, []

    def compile(contents):
        nonlocal attempts
        attempts += 1
        try:
            return Coff(compiler.overlay(source, {unit: contents}, command, flags))
        finally:
            if source.resolve() != source or not source.is_file() or source.read_bytes() != original:
                raise ValueError("layout witness source changed during compilation")

    try:
        if not callable(getattr(compiler, "overlay", None)):
            raise ValueError("compiler does not support private source overlays")
        replica = compile(original)
        if object_signature(replica) != object_signature(baseline):
            raise ValueError("unchanged layout witness replica differs from product compilation")
    except (OSError, ValueError) as error:
        return {"compile_attempts": attempts, "witnesses": [
            {"class_name": name, "result": Result.UNKNOWN, "reason": str(error),
             "replica_control": Result.UNKNOWN, "compiler_layout": None} for name in names]}

    for name in names:
        identifier = "match_recovery_layout_" + digest([unit, name, original.hex()])[:24]
        symbol = "_" + identifier  # Explicit i386 C linkage, not a guessed class method.
        row = {"class_name": name, "result": Result.UNKNOWN, "compiler_layout": None,
               "replica_control": Result.PASS, "probe_control": Result.UNKNOWN,
               "probe_symbol": symbol, "basis": "same_tu_pointer_parameter_probe"}
        absence_eligible = False
        try:
            if identifier.encode() in original or any(s.name == symbol for s in baseline.symbols.values()):
                raise ValueError("layout witness name collides with product input")
            # Do not let a macro turn the requested class into a different type.
            guards = "\n".join("#ifdef " + part + "\n#error layout witness class is a macro\n#endif"
                               for part in name.split("::"))
            prefix = ("\n\n" + guards + '\nextern "C" int ' + identifier +
                      "(" + name + "* object) { return ").encode("ascii")
            absence_eligible = True
            probe = compile(original + prefix + b"0; }\n")
            try:
                layout = ClassLayouts(probe).layout(name)
            except ClassNotEmitted:
                raise
            except ValueError:
                absence_eligible = False  # Unsupported definitions are not absent.
                raise
            absence_eligible = False  # Never replace an observed class or failed body control.
            probe_signature = function_signature(probe, symbol)
            control = compile(original + prefix + b"1; }\n")
            if function_signature(control, symbol) == probe_signature:
                raise ValueError("layout witness changed-body control was not observed")
            if ClassLayouts(control).layout(name) != layout:
                raise ValueError("layout witness declaration differs between probe and control")
            row.update(result=Result.PASS, compiler_layout=layout, probe_control=Result.PASS,
                       reason="same-TU declaration emitted with replica and changed-body controls")
        except (OSError, ValueError) as error:
            row["reason"] = str(error)
            if absence_eligible:
                pending_absences.append((row, identifier, guards))
        rows.append(row)

    def check_absences(pending):
        if not pending:
            return
        try:
            results = _absence_witnesses(compile, original,
                                         [(row["class_name"], identifier, guards) for row, identifier, guards in pending])
        except (OSError, ValueError) as error:
            # A collision or unsupported definition must not hide observable
            # siblings. Re-run smaller independent groups; never retain a
            # partial result from a failed batch or mix widths across groups.
            if len(pending) == 1:
                pending[0][0]["absence_reason"] = str(error)
            else:
                middle = len(pending) // 2
                check_absences(pending[:middle])
                check_absences(pending[middle:])
        else:
            for (row, _, _), result in zip(pending, results, strict=True):
                row.update(result)
                row.pop("probe_control")

    check_absences(pending_absences)
    return {"compile_attempts": attempts, "witnesses": rows}
