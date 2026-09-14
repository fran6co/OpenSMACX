"""Retained physical compiled common requests, not storage or trial coverage.

The native producer separately proves both selected common allocations. This
helper establishes only a stable, independently retained duplicate request;
neither a report nor an archive basename can supply that witness.
"""

from hashlib import sha256

from .build_graph_commands import _context
from .components import relative_path
from .link_file_evidence import _stamp
from .linked_objects import _read
from .pe_coff import Coff


def retained_witnesses(baseline, current, *, excluded_units, requests):
    """Find an exact selected compiled witness outside all generated-change TUs.

    Addresses, bytes from another object, and archive members are not alternate
    source identities. Whole object hashes may differ across builds; each must
    independently match its own native compilation receipt and selected input.
    Returned dictionaries are diagnostics, never capabilities.
    """
    def fresh():
        try:
            baseline.check_current()
        finally:
            current.check_current()

    try:
        fresh()
        if (type(excluded_units) is not frozenset
                or any(not isinstance(unit, str) or relative_path(unit) != unit for unit in excluded_units)):
            raise ValueError("common witnesses require immutable canonical excluded source units")
        if (type(requests) is not dict or not 0 < len(requests) <= 4096
                or any(not isinstance(name, str) or not name
                       or any(ord(char) < 33 or ord(char) >= 127 for char in name)
                       or type(size) is not int or size <= 0 for name, size in requests.items())):
            raise ValueError("common witnesses require exact positive native common requests")
        requests = dict(requests)
        found = {}
        for version, linked in (("baseline", baseline), ("current", current)):
            fresh()
            context = linked.context
            root, _, rows, command = _context(context)
            if (list(command.arguments) != linked.inputs.evidence["arguments"]
                    or str(command.cwd) != linked.inputs.evidence["cwd"]):
                raise ValueError("common witness command differs from actual native link input evidence")
            receipts = {row["source"]: row["compilation"]["output"]["after"]
                        for row in context.report["compile_link"]["compilations"]}
            candidates = {}
            for unit in sorted(rows.keys() - excluded_units):
                output = rows[unit]["output"]
                receipt = receipts[str(root / unit)]
                _stamp(receipt)
                if receipt is None:
                    raise ValueError("retained common witness lacks its native output receipt")
                raw = _read(output, receipt)
                obj = Coff(raw)
                named = {name: [symbol for symbol in obj.symbols.values() if symbol.name == name]
                         for name in requests}
                # Ordinary external references are neither allocation requests
                # nor overrides. They cannot witness redundancy, but must not
                # disqualify an independent retained request in another TU.
                for name, matches in named.items():
                    if len(matches) == 1:
                        symbol, = matches
                        if (symbol.section == 0 and symbol.storage == 2 and symbol.value == 0
                                and symbol.type in (0, 0x20) and not obj.auxiliary.get(symbol.index)):
                            named[name] = []
                if not any(named.values()):
                    continue
                if obj.machine != 0x14C or obj.optional_header_size != 0:
                    raise ValueError("retained common witness requires native i386 object records")
                selected = linked.inputs.objects.resolve(output.name)
                identity = {"path": str(output)}
                if (selected["identity"] != identity or selected["raw"] != raw
                        or selected["sha256"] != sha256(raw).hexdigest()):
                    raise ValueError("retained common witness differs from its exact selected compilation")
                symbols = {}
                for name, matches in named.items():
                    if not matches:
                        continue
                    if len(matches) != 1:
                        raise ValueError("retained common witness has ambiguous symbol records")
                    symbol, = matches
                    if (symbol.section != 0 or symbol.storage != 2 or symbol.type != 0
                            or symbol.value != requests[name] or obj.auxiliary.get(symbol.index)):
                        raise ValueError("retained common witness is not an exact uniform common request")
                    symbols[name] = {"input": identity, "object_sha256": selected["sha256"],
                                     "owner": selected["owner"], "object_index": symbol.index,
                                     "symbol": name, "size": symbol.value}
                candidates[unit] = symbols
            found[version] = candidates
        result = {}
        units = sorted(found["baseline"].keys() & found["current"].keys())
        for name in requests:
            eligible = [unit for unit in units
                        if name in found["baseline"][unit] and name in found["current"][unit]]
            if not eligible:
                raise ValueError("common request lacks a retained selected compiled witness: " + name)
            unit = eligible[0]
            result[name] = {"unit": unit, **{version: found[version][unit][name]
                                            for version in ("baseline", "current")}}
        return result
    except (KeyError, TypeError, AttributeError) as error:
        raise ValueError("incomplete native retained common witness evidence") from error
    finally:
        fresh()
