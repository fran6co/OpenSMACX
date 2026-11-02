"""Archive-qualified external data points, never object or allocation proofs."""

from .coff_inventory import inventory_archive
from .components import ToolchainBssDataStart
from .model import Result


def verify_data_starts(manifest, objects, image, archive_data, errors):
    """Freshly inventory every physical member; unsupported competitors refuse.

    A declared source identity supplies the original point. No instruction under
    test, neighboring name, section placement or byte search supplies it. COFF
    section/request sizes qualify current records only and never become original
    spans. Even successful point qualification retains an UNKNOWN overall row.
    """
    if not manifest.data_starts:
        return [], None
    inventory = inventory_archive(archive_data) if archive_data is not None else None
    summary = ({key: inventory[key] for key in
                ("result", "sha256", "member_count", "format_counts", "unsupported_members")}
               if inventory is not None else None)
    rows = []
    for item in manifest.data_starts:
        row = {"unit": item.unit, "symbol": item.symbol,
               "kind": "bss" if isinstance(item, ToolchainBssDataStart) else "common",
               "declared_address": f"0x{item.origin.address:08X}", "address": None,
               "result": Result.UNKNOWN, "start_identity": Result.UNKNOWN,
               "scoped_reference_target": False, "callable": False, "link_target": False,
               "original_extent": Result.UNKNOWN, "original_content": Result.UNKNOWN,
               "allocation": Result.UNKNOWN, "selection": Result.UNKNOWN,
               "lifetime": Result.UNKNOWN}
        try:
            if inventory is None:
                raise ValueError("archive pin did not pass")
            if inventory["result"] != Result.PASS:
                raise ValueError("incomplete archive symbol inventory cannot prove provider uniqueness")
            declarations, blockers = [], []
            for member in inventory["members"]:
                identity = {key: member[key] for key in ("member", "occurrence", "sha256")}
                for symbol in member["symbols"]:
                    if symbol["name"] == item.symbol:
                        if symbol["declaration"] == "undefined":
                            if symbol["type"] != 0 or symbol["auxiliary_size"]:
                                blockers.append({**identity, "reason": "nonordinary data declaration"})
                        elif symbol["storage"] in (2, 105):
                            declarations.append({**identity, **symbol})
                    if symbol["declaration"] == "weak" and symbol.get("fallback") == item.symbol:
                        blockers.append({**identity, "reason": "weak fallback names the data start"})
                for imported in member["imports"]:
                    if item.symbol in (imported["symbol"], "__imp_" + imported["symbol"]):
                        blockers.append({**identity, "reason": "import conflicts with the data start"})
                # Linker aliases are not ordinary undefined-data routing. Keep
                # them visible even in otherwise neutral machine-zero members.
                from .common_allocation import _tokens
                for directive in member["directives"]:
                    for token in _tokens(directive["text"]):
                        key, _, value = token.lstrip("/-").partition(":")
                        if key.lower() == "alternatename" and item.symbol in value.split("="):
                            blockers.append({**identity, "reason": "linker alias names the data start"})
            row["declarations"] = declarations
            row["blockers"] = blockers
            if blockers or len(declarations) != 1:
                raise ValueError("data start requires one unambiguous ordinary archive provider")
            declared = declarations[0]
            if declared["member"] != item.unit or declared["occurrence"] != 0:
                raise ValueError("data-start provider differs from the exact declared member")
            obj = objects.get(item.unit)
            if obj is None:
                raise ValueError(errors.get(item.unit, "archive member was not read"))
            named = [symbol for symbol in obj.symbols.values() if symbol.name == item.symbol]
            if (len(named) != 1 or named[0].storage != 2 or named[0].type != 0
                    or named[0].index in obj.auxiliary):
                raise ValueError("data start is not one ordinary external non-function record")
            symbol = named[0]
            if isinstance(item, ToolchainBssDataStart):
                if not 1 <= item.section <= len(obj.sections) or symbol.section != item.section:
                    raise ValueError("BSS data-start section ordinal differs")
                section = obj.sections[item.section - 1]
                size = obj.section_sizes[item.section - 1]
                if (section.name != item.section_name or symbol.value != item.offset
                        or size != item.section_size or not 0 <= symbol.value < size):
                    raise ValueError("BSS data-start name, offset or physical section size differs")
                if (section.flags & 0xC0000080 != 0xC0000080
                        or section.flags & (0x20 | 0x40 | 0x200 | 0x800 | 0x1000
                                            | 0x02000000 | 0x20000000)
                        or section.name.startswith(".tls") or section.data or section.relocations
                        or item.section in obj.associations):
                    raise ValueError("data start requires ordinary writable uninitialized COFF storage")
                row["physical_definition"] = {"section": item.section, "section_name": section.name,
                                              "offset": symbol.value, "section_size": size,
                                              "flags": section.flags}
            else:
                if symbol.section != 0 or symbol.value != item.common_size:
                    raise ValueError("COMMON data-start allocation request differs")
                row["physical_definition"] = {"section": 0, "common_size": symbol.value}
            # Check only the point's load-time storage class, not a guessed
            # original allocation length (including the COMMON request size).
            mapped = [(span, raw_size, flags) for (span, _, raw_size), flags in
                      zip(image.sections, image.section_flags, strict=True)
                      if span.start <= item.origin.address < span.end]
            if (len(mapped) != 1 or mapped[0][2] & 0xC0000000 != 0xC0000000
                    or mapped[0][2] & 0x20000020
                    or item.origin.address < mapped[0][0].start + mapped[0][1]):
                raise ValueError("original data start is not a unique writable virtual-zero point")
            row.update(address=f"0x{item.origin.address:08X}", start_identity=Result.PASS,
                       scoped_reference_target=True)
        except ValueError as error:
            row["reason"] = str(error)
        rows.append(row)
    return rows, summary
