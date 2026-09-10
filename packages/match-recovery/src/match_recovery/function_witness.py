"""Controlled, never-linked observations of otherwise un-emitted functions.

Witness objects are transient diagnostic evidence, not product objects or
permission to waive exports, callers, data, layout or preservation checks.
The caller owns whole-project input freshness; this helper additionally pins
the canonical TU and explicit body header across every compiler operation.
"""

from pathlib import Path
import re

from .coff_replica import function_signature, object_signature
from .components import local_path
from .inline_body import _name, damage_body
from .inline_ownership import definition_signature
from .model import Result, digest
from .pe_coff import Coff
from .source.mangled import qualified_name


def _expression(symbol, symbol_format):
    name = _name(symbol, symbol_format)
    if symbol_format == "none":
        return name
    if symbol.startswith("?"):
        # _name already refused generated names, templates and unsupported
        # encodings. Only ordinary free/public nonvirtual members are eligible.
        access = symbol.split("@@", 1)[1][0]
        if access not in ("Y", "Q", "S"):
            raise ValueError("retention requires a free or public nonvirtual function")
    expression = qualified_name(symbol)
    if not re.fullmatch(r"[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*", expression, re.ASCII):
        raise ValueError("unsupported qualified function address")
    return expression


def _absent(obj, name):
    matches = [symbol for symbol in obj.symbols.values() if symbol.name == name]
    if len(matches) > 1 or any(symbol.section != 0 or symbol.storage != 2
                              or symbol.value != 0 or obj.auxiliary.get(symbol.index)
                              or symbol.type not in (0, 0x20) for symbol in matches):
        raise ValueError("retention requires an absent exact function, not a defined or unsupported symbol")


def _escape(obj, probe, target, sink):
    entry = obj.function(probe)
    if entry.storage != 2 or sum(item.name == probe for item in obj.symbols.values()) != 1:
        raise ValueError("retention probe is not externally emitted")
    definition = obj.function(target)
    if definition.storage != 2 or sum(item.name == target for item in obj.symbols.values()) != 1:
        raise ValueError("retention target has an ambiguous exact symbol identity")
    references = []
    for number in obj.code_sections(entry):
        section, occupied = obj.sections[number - 1], set()
        for offset, index, kind in section.relocations:
            extent = set(range(offset, offset + 4))
            if (kind not in (6, 20) or not 0 <= offset <= len(section.data) - 4
                    or section.data[offset:offset + 4] != bytes(4) or occupied & extent):
                raise ValueError("retention probe has unsupported address/sink relocation evidence")
            occupied.update(extent)
            references.append((obj.symbols[index], kind))
    # The deliberately bounded i386 witness passes a real function address and
    # calls an unresolved external sink. A no-op relocation, same-named data,
    # thunk call or unsupported indirection is not that evidence.
    if not any(item.index == definition.index and kind == 6 for item, kind in references):
        raise ValueError("retention probe does not reference the exact target definition")
    def sink_name(name):
        # VC6 can instantiate this free template with an ordinary function
        # name and a P6/P8 function/member-pointer parameter. This recognizes
        # only that lexical form (not arbitrary namespaces or overloads).
        # It is not a type/ABI proof; fresh compiler/body controls and the exact
        # target definition remain necessary independently of this spelling.
        ordinary = re.fullmatch(r"\?" + re.escape(sink) + r"@@YAXP[68][A-Za-z_0-9@]+Z@Z", name)
        return name in (sink, "_" + sink) or name.startswith("??$" + sink + "@") or ordinary is not None

    sinks = [item for item in obj.symbols.values() if sink_name(item.name)]
    if (len(sinks) != 1 or sinks[0].section != 0 or sinks[0].storage != 2 or sinks[0].value != 0
            or obj.auxiliary.get(sinks[0].index)
            or not any(item.index == sinks[0].index and kind == 20 for item, kind in references)):
        raise ValueError("retention probe lacks its external address sink")


def retained_definition(root, binding, body_path, baseline, compiler, command, flags, symbol_format):
    """Return ``(Coff | None, report)`` after five controlled private compiles.

    Only established non-emission is eligible. No source decoration is removed,
    no product object is modified, and no claim/admission status is granted.
    Unsupported inputs and failed controls return explicit unknown evidence.
    """
    report = {"basis": "same_tu_retained_definition", "result": Result.UNKNOWN,
              "symbol": binding.symbol, "unit": binding.unit, "body_path": body_path,
              "compile_attempts": 0, "replica_control": Result.UNKNOWN,
              "repeat_control": Result.UNKNOWN, "target_body_control": Result.UNKNOWN,
              "probe_control": Result.UNKNOWN, "product_emitted": None,
              "diagnostic_only": True, "never_link": True,
              "ready_for_collection": False, "persisted": False}
    try:
        root = Path(root).resolve()
        source, header = local_path(root, binding.unit), local_path(root, body_path)
        if source == header:
            raise ValueError("retention requires a canonical TU and an explicit source header")
        for path in (source, header):
            if path.resolve() != path or not path.is_file():
                raise ValueError("retention source paths must be existing canonical regular files")
        original = {binding.unit: source.read_bytes(), body_path: header.read_bytes()}
        if (not isinstance(command, (list, tuple)) or not isinstance(flags, (list, tuple))
                or any(not isinstance(word, str) or not word or "\0" in word for word in (*command, *flags))):
            raise ValueError("retention requires compiler argument tokens")
        command, flags = tuple(command), tuple(flags)
        if any(word.lower().startswith(("/yu", "/yc", "/fp", "-yu", "-yc", "-fp", "@"))
               for word in (*command, *flags)):
            raise ValueError("retention requires standalone compilation without PCH or response files")
        if not isinstance(baseline, Coff):
            raise ValueError("retention requires a successfully parsed product object")
        _absent(baseline, binding.symbol)
        report["product_emission"] = "not_emitted"
        report["product_emitted"] = False
        expression = _expression(binding.symbol, symbol_format)
        damaged_header = damage_body(original[body_path], binding.symbol, symbol_format)
        if not callable(getattr(compiler, "overlay", None)):
            raise ValueError("compiler does not support private source overlays")
        identity = "match_recovery_retention_" + digest({
            "unit": binding.unit, "header": body_path, "symbol": binding.symbol,
            "inputs": {name: raw.hex() for name, raw in original.items()},
            "command": command, "flags": flags})[:24]
        sink, probe = identity + "_keep", identity + "_probe"
        probe_symbol = "_" + probe  # Explicit i386 C __cdecl linkage.
        if any(identity.encode() in raw for raw in original.values()) or any(
                identity in symbol.name for symbol in baseline.symbols.values()):
            raise ValueError("retention witness name collides with product inputs")
        guards = "\n".join("#ifdef " + name + "\n#error retention witness name is a macro\n#endif"
                           for name in (*expression.split("::"), sink, probe))

        def snippet(result):
            return ("\n\n" + guards + "\n" +
                    "template<class T> void " + sink + "(T);\n" +
                    'extern "C" int __cdecl ' + probe + "() { " + sink + "(&" + expression +
                    "); return " + str(result) + "; }\n").encode("ascii")

        def unchanged():
            for name, contents in original.items():
                path = root / name
                if path.resolve() != path or not path.is_file() or path.read_bytes() != contents:
                    raise ValueError("retention source changed during compilation")

        def compile(replacements):
            unchanged()
            report["compile_attempts"] += 1
            try:
                return Coff(compiler.overlay(source, replacements, command, flags))
            finally:
                unchanged()

        replica = compile(original)
        if object_signature(replica) != object_signature(baseline):
            raise ValueError("unchanged retention replica differs from product compilation")
        report["replica_control"] = Result.PASS
        probe_inputs = {**original, binding.unit: original[binding.unit] + snippet(0)}
        retained = compile(probe_inputs)
        fingerprint = definition_signature(retained, binding.symbol)
        _escape(retained, probe_symbol, binding.symbol, sink)
        repeated = compile(probe_inputs)
        if object_signature(repeated) != object_signature(retained):
            raise ValueError("repeated retention witness compilation differs")
        report["repeat_control"] = Result.PASS
        damaged = compile({**probe_inputs, body_path: damaged_header})
        definition_signature(damaged, binding.symbol)
        _escape(damaged, probe_symbol, binding.symbol, sink)
        if function_signature(damaged, binding.symbol) == function_signature(retained, binding.symbol):
            raise ValueError("retained target body damage was not observed")
        report["target_body_control"] = Result.PASS
        control = compile({**original, binding.unit: original[binding.unit] + snippet(1)})
        _escape(control, probe_symbol, binding.symbol, sink)
        if function_signature(control, probe_symbol) == function_signature(retained, probe_symbol):
            raise ValueError("retention probe body damage was not observed")
        if definition_signature(control, binding.symbol) != fingerprint:
            raise ValueError("retained definition changed with unrelated probe control")
        unchanged()
        report.update(result=Result.PASS, probe_control=Result.PASS,
                      probe_symbol=probe_symbol, address_expression=expression,
                      definition_fingerprint=fingerprint,
                      limitations=["private retained definition, not a product emission or linked object",
                                   "caller owns whole-project freshness and independent strict comparison",
                                   "no export, caller, data, layout, preservation or acceptance waiver"])
        return retained, report
    except (OSError, ValueError) as error:
        report["reason"] = str(error)
        return None, report
