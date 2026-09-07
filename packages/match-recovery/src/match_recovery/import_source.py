"""Source-owned COFF import identities; original slot addresses are derived.

An import declaration binds an emitted pointer symbol to a DLL export name or
ordinal. It never aliases a recovered body to a system function or establishes
the candidate link's selected DLL/version. Those require separate link/ABI proof.
"""

from dataclasses import dataclass
from pathlib import Path
import re

from .components import relative_path
from .source_config import SourceConfiguration


PREFIX = "// RECOVERY_IMPORT:"
PATTERN = re.compile(r"// RECOVERY_IMPORT: v1 (\S+) (\S+) (name:\S+|ordinal:[0-9]+)\s*")


@dataclass(frozen=True)
class ImportBinding:
    symbol: str
    module: str
    name: str | None
    ordinal: int | None
    path: str
    line: int


def parse_imports(text: str, path: str) -> tuple[ImportBinding, ...]:
    relative_path(path)
    bindings = []
    for line, source_line in enumerate(text.splitlines(), 1):
        if not source_line.lstrip().startswith(PREFIX):
            continue
        match = PATTERN.fullmatch(source_line.strip())
        if match is None or any(not 33 <= ord(char) <= 126 for token in match.groups() for char in token):
            raise ValueError(f"invalid import annotation at {path}:{line}")
        symbol, module, export = match.groups()
        name = export[5:] if export.startswith("name:") else None
        ordinal = None if name is not None else int(export[8:])
        if ordinal is not None and ordinal > 0xFFFF:
            raise ValueError(f"import ordinal exceeds 16 bits at {path}:{line}")
        bindings.append(ImportBinding(symbol, module, name, ordinal, path, line))
    return tuple(bindings)


def read_imports(root: Path) -> tuple[ImportBinding, ...]:
    root = root.resolve()
    source = SourceConfiguration.read(root)
    bindings = []
    for path in source.files(root):
        relative = path.relative_to(root).as_posix()
        if not source.is_seed(relative):
            bindings.extend(parse_imports(path.read_text(encoding="utf-8"), relative))
    if len({item.symbol for item in bindings}) != len(bindings):
        raise ValueError("duplicate source import symbol binding")
    return tuple(bindings)


def import_targets(bindings, objects, image):
    """Resolve only referenced, ordinary undefined externals; never definitions.

    Case-sensitive export identity and explicit module select a unique original
    IAT slot. Duplicate imports stay ambiguous rather than consulting the code
    operand to pick one. Weak/common/defined symbols need link-selection proof.
    """
    if len({item.symbol for item in bindings}) != len(bindings):
        raise ValueError("duplicate source import symbol binding")
    referenced = {obj.symbols[index].name for obj in objects.values() for section in obj.sections
                  for _, index, kind in section.relocations if kind in (6, 20)}
    active = [item for item in bindings if item.symbol in referenced]
    if not active:
        return {}, []
    try:
        imports, error = image.imports(), None
    except ValueError as problem:
        imports, error = (), str(problem)
    targets, reports = {}, []
    for binding in active:
        matches = [item for item in imports if item.module.lower() == binding.module.lower()
                   and item.name == binding.name and item.ordinal == binding.ordinal]
        symbols = [symbol for obj in objects.values() for symbol in obj.symbols.values()
                   if symbol.name == binding.symbol]
        # VC6 emits type 0x20 even for __imp__ pointer symbols. COFF's type
        # hint is not pointer-role evidence; the explicit source import identity
        # selects that role. Linkage/definition state must still be unambiguous.
        ordinary = all(s.storage == 2 and s.section == 0 and s.value == 0 for s in symbols)
        row = {"symbol": binding.symbol, "module": binding.module, "name": binding.name,
               "ordinal": binding.ordinal, "source": binding.path, "line": binding.line,
               "identity": "unknown", "address": None, "link_selection": "unknown"}
        if error:
            row["reason"] = error
        elif not ordinary:
            row["reason"] = "import requires an ordinary undefined external; definition or linker selection remains"
        elif len(matches) != 1:
            row["reason"] = "requires exactly one original DLL/export import slot"
        else:
            targets[binding.symbol] = matches[0].address
            row.update(identity="pass", address=f"0x{matches[0].address:08X}")
        reports.append(row)
    return targets, reports
