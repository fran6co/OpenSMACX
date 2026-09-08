"""Explicit mapped-subject selection shared by diagnostics and publication."""

import re

from .model import require_text
from .source.mangled import identifier, qualified_name


def select_records(records, targets, host, symbol_format):
    if not isinstance(targets, (list, tuple)) or not targets:
        raise ValueError("source selection requires explicit nonempty targets")
    candidates = [row for row in records if host is None or row.path == host]
    selected = []
    for target in targets:
        require_text(target, "source target")
        matches = [row for row in candidates if target in (row.name, row.symbol)]
        if not matches and symbol_format == "msvc":
            matches = [row for row in candidates if any(
                target in (qualified_name(name), identifier(name)) for name in (row.name, row.symbol) if name)]
        # C++ names can be hex strings too. Explicit 0x means address; otherwise
        # source names take precedence. No implicit product-over-seed preference.
        if (re.fullmatch(r"0[xX][0-9a-fA-F]+", target)
                or not matches and re.fullmatch(r"[0-9a-fA-F]+", target)):
            matches = [row for row in candidates if row.address == int(target, 16)]
        if len(matches) != 1:
            locations = ", ".join(f"{row.path}:{row.line} ({row.address_hex} {row.symbol or row.name})" for row in matches)
            raise ValueError(f"source target {target!r} requires one source mapping; found {len(matches)}" +
                             (f": {locations}; select an exact --in path or original address/symbol" if matches else ""))
        selected.append(matches[0])
    if len({(row.path, row.line) for row in selected}) != len(selected):
        raise ValueError("the same source mapping was selected more than once")
    return selected
