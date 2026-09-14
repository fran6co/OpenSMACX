"""Observed private function COMDAT form, not a global key or selection claim.

The section definition uses the PE/COFF COMDAT metadata format. VC6 also emits
an independent NODUP section whose second symbol is a static function; that
name is local to its original object and must never enter global key matching.
https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
"""

from .coff_comdat import _definition, _function_auxiliary
from .pe_coff import _slice, _unpack


def private_comdat_root(obj, section):
    """Return a supported private root, or None for a nonprivate key form.

    Malformed private forms refuse, including function auxiliaries that fail
    the shared function-definition auxiliary qualification. The native
    caller still owns physical/parsed freshness and all placement evidence.
    """
    symbols, policy = _definition(obj, section)
    if len(symbols) < 2 or symbols[1].storage != 3:
        return None
    root = symbols[1]
    if (policy != 1 or root.type != 0x20 or root.value != 0
            or not root.name or root.name == obj.sections[section - 1].name
            or not obj.sections[section - 1].flags & 0x20
            or obj.function(root.name) != root):
        raise ValueError("unsupported private independent NODUP function form")
    if [symbol for symbol in obj.symbols.values() if symbol.name == root.name] != [root]:
        raise ValueError("private function has ambiguous same-object symbol names")
    raw = getattr(obj, "raw_data", None)
    if type(raw) is not bytes:
        raise ValueError("private function lacks original physical COFF bytes")
    table, count = _unpack("<II", raw, 8)
    if not 0 <= root.index < count:
        raise ValueError("private function index is outside physical symbol records")
    if obj.auxiliary.get(root.index) or _slice(raw, table + root.index * 18 + 17, 1)[0]:
        _function_auxiliary(obj, root)
    return root
