"""COFF/import-library declarations, never selected storage or code proof.

Microsoft PE/COFF Import Library Format:
https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#import-library-format
The record reader is intentionally separate from Coff's COMDAT verifier. No
linker search, archive-index selection, dependency closure or target admission
is inferred from a successfully parsed inventory.
"""

from collections import Counter
from dataclasses import asdict
import hashlib
import struct

from .coff_archive import CoffArchive
from .model import Result
from .pe_coff import CoffRecords, weak_external


def short_import(data: bytes) -> dict:
    """Decode the documented version-zero i386 short import record."""
    if len(data) < 20:
        raise ValueError("truncated short import header")
    sig1, sig2, version, machine, timestamp, size, hint, flags = struct.unpack_from("<HHHHIIHH", data)
    if (sig1, sig2, version, machine) != (0, 0xFFFF, 0, 0x14C):
        raise ValueError("unsupported short import signature, version or machine")
    kind, naming = flags & 3, flags >> 2 & 7
    if flags >> 5 or kind > 2 or naming > 3:
        raise ValueError("unsupported short import type/name mode or reserved bits")
    if size != len(data) - 20:
        raise ValueError("short import size does not exactly cover its strings")
    parts = data[20:].split(b"\0")
    if len(parts) != 3 or parts[-1] or not all(parts[:2]):
        raise ValueError("short import requires exactly two nonempty terminated strings")
    try:
        symbol, module = (part.decode("ascii") for part in parts[:2])
    except UnicodeDecodeError as error:
        raise ValueError("unsupported non-ASCII short import identity") from error
    if any(not 32 <= ord(char) < 127 for char in symbol + module):
        raise ValueError("invalid short import identity character")
    exported = symbol
    if naming in (2, 3) and exported.startswith(("?", "@", "_")):
        exported = exported[1:]
    if naming == 3:
        exported = exported.split("@", 1)[0]
    if naming and not exported:
        raise ValueError("short import name transformation is empty")
    return {"symbol": symbol, "module": module, "type": ("code", "data", "const")[kind],
            "name_type": ("ordinal", "name", "noprefix", "undecorate")[naming],
            "name": exported if naming else None, "ordinal": hint if not naming else None,
            "hint": hint if naming else None, "timestamp": timestamp,
            "selection": Result.UNKNOWN}


def inventory_object(data: bytes) -> dict:
    """All declared regular records or the complete supported short import.

    Common values are allocation requests, not selected extents. Weak fallback
    edges and raw linker directives remain visible without being interpreted as
    decisions. Parsing pass is not a link or allocation verification pass.
    """
    report = {"sha256": hashlib.sha256(data).hexdigest(), "result": Result.UNKNOWN,
              "symbols": [], "imports": [], "directives": [], "format": "unsupported"}
    try:
        if data.startswith(b"\0\0\xff\xff"):
            report.update(format="short_import", imports=[short_import(data)])
        else:
            obj = CoffRecords(data)
            report.update(format="coff", machine=obj.machine, optional_header_size=obj.optional_header_size)
            for symbol in obj.symbols.values():
                row = {**asdict(symbol), "declaration": "local_or_metadata"}
                if symbol.section < -2 or not symbol.name or "\0" in symbol.name:
                    raise ValueError("unsupported COFF symbol identity or section")
                if symbol.storage == 2:
                    if symbol.section > 0:
                        row["declaration"] = "defined"
                    elif symbol.section == 0:
                        row["declaration"] = "common" if symbol.value else "undefined"
                    elif symbol.section == -1:
                        row["declaration"] = "absolute"
                    else:
                        raise ValueError("external COFF symbol has a debug-only section")
                elif symbol.storage == 105:
                    target, mode = weak_external(obj, symbol)
                    row.update(declaration="weak", fallback=target.name, search_mode=mode)
                report["symbols"].append(row)
            for index, section in enumerate(obj.sections, 1):
                if section.name == ".drectve":
                    if section.relocations:
                        raise ValueError("relocated linker directives are unsupported")
                    report["directives"].append({"section": index, "text": section.data.decode("ascii"),
                                                  "interpretation": Result.UNKNOWN})
        report["result"] = Result.PASS
    except ValueError as error:
        report["reason"] = str(error)
    return report


def inventory_archive(data: bytes) -> dict:
    """Enumerate every archive record, including repeated import member names."""
    archive = CoffArchive(data)
    members = [{"member": name, "occurrence": occurrence, **inventory_object(raw)}
               for name, bodies in archive.members.items() for occurrence, raw in enumerate(bodies)]
    complete = bool(members) and all(row["result"] == Result.PASS for row in members)
    return {"check": "coff_archive_inventory", "sha256": hashlib.sha256(data).hexdigest(),
            "result": Result.PASS if complete else Result.UNKNOWN, "members": members,
            "member_count": len(members),
            "format_counts": dict(sorted(Counter(row["format"] for row in members).items())),
            "unsupported_members": [{key: row[key] for key in ("member", "occurrence", "reason")}
                                    for row in members if row["result"] != Result.PASS],
            "common_requests": [{"member": row["member"], "occurrence": row["occurrence"],
                                 "symbol": symbol["name"], "size": symbol["value"]}
                                for row in members if row["result"] == Result.PASS
                                for symbol in row["symbols"] if symbol["declaration"] == "common"],
            "link_selection": Result.UNKNOWN, "allocation_verification": Result.UNKNOWN,
            "persisted": False, "ready_for_collection": False,
            "limitations": ["inventory of the supplied archive, not all linker inputs or selected members",
                            "successful parsing is not code, data, ABI or runtime verification",
                            "common requests, weak fallbacks and directives require separate link-selection proof",
                            "unsupported records prevent complete inventory; partial declarations are not absence evidence"]}
