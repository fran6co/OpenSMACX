"""Controlled same-TU layout probes for the PE/COFF i386 backend.

Append only a uniquely named pointer-parameter function, never an include or a
class definition. A witness describes the declaration visible at the end of
this TU, not product code preservation, ABI independence or object lifetime.
The compiler's ordinary overlay protocol owns all toolchain interaction.
"""

import re
from pathlib import Path

from .codeview import ClassLayouts, ClassNotEmitted
from .coff_replica import function_signature, object_signature
from .components import local_path
from .model import Result, digest
from .pe_coff import Coff


def missing_layout_witnesses(root, unit, names, baseline, compiler, command, flags):
    """Probe missing definitions only; never replace a malformed/observed one.

    One unchanged replica controls the TU's private include context. Each class
    then needs an emitted probe and a changed-body control under identical
    flags. Unsupported overlays, invisible/incomplete classes and failed
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
        try:
            if identifier.encode() in original or any(s.name == symbol for s in baseline.symbols.values()):
                raise ValueError("layout witness name collides with product input")
            # Do not let a macro turn the requested class into a different type.
            guards = "\n".join("#ifdef " + part + "\n#error layout witness class is a macro\n#endif"
                               for part in name.split("::"))
            prefix = ("\n\n" + guards + '\nextern "C" int ' + identifier +
                      "(" + name + "* object) { return ").encode("ascii")
            probe = compile(original + prefix + b"0; }\n")
            layout = ClassLayouts(probe).layout(name)
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
        rows.append(row)
    return {"compile_attempts": attempts, "witnesses": rows}
