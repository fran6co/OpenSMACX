"""Selected i386 short-import thunks, not imported DLL implementations.

The six-byte extent is the supported linker recipe, never a next-symbol guess:
https://github.com/llvm/llvm-project/blob/main/lld/COFF/Chunks.h
https://github.com/llvm/llvm-project/blob/main/lld/COFF/Chunks.cpp
Short-header identity follows Microsoft's PE/COFF Import Library Format.
The enclosing native build observer owns map/image receipt and final guards.
"""

from pathlib import PurePosixPath, PureWindowsPath

from .binary import Relocation, RelocationKind, compare_bound_code
from .common_allocation import _policy
from .linked_imports import import_slots
from .model import Result, Span, digest, require_text


def import_thunk_candidates(objects):
    """One guarded advisory dispatch snapshot, not selection or code proof.

    Consumers still call ``verify_import_thunk`` for each actual use; that
    verifies the live population again. This set cannot authorize a thunk.
    """
    records, fresh = objects._constant_population()
    try:
        return frozenset(item["symbol"] for row in records for item in row["imports"])
    finally:
        fresh()


def verify_import_thunk(objects, mapping, image, owner, symbol):
    """Bind a unique physical producer and named use, then verify its recipe.

    Repeated archive member basenames are permitted only because the complete
    physical population supplies exactly one producer of this symbol pair.
    Final-pass observations establish the physical library/member family, not
    an invented Found/Loaded record or a unique owner-only archive selection.
    Byte/reference failure is FAIL; unsupported selection raises ValueError.
    """
    for value, label in ((owner, "import owner"), (symbol, "import thunk symbol")):
        require_text(value, label)
        if any(ord(char) < 33 or ord(char) >= 127 for char in value):
            raise ValueError("unsupported import owner or symbol spelling")
    records, fresh = objects._constant_population()
    try:
        if _policy(objects._constant_evidence, records):
            raise ValueError("import thunk has unsupported linker policy or directives")
        pointer = "__imp_" + symbol
        namespace = {symbol, pointer}
        candidates = [(row, item) for row in records for item in row["imports"]
                      if namespace & {item["symbol"], "__imp_" + item["symbol"]}]
        if len(candidates) != 1:
            raise ValueError("import thunk requires exactly one complete-population producer")
        record, imported = candidates[0]
        if (record["format"] != "short_import" or "member" not in record
                or imported["symbol"] != symbol or imported["type"] != "code"):
            raise ValueError("import thunk requires an exact archived short code import")
        for row in records:
            for declared in row["symbols"]:
                if (declared["declaration"] == "weak" and declared.get("fallback") in namespace):
                    raise ValueError("import thunk namespace has a weak fallback")
                if declared["name"] in namespace and not (
                        declared["declaration"] == "undefined" and declared["storage"] == 2
                        and declared["section"] == 0 and declared["value"] == 0
                        and declared["type"] in (0, 0x20) and not declared["auxiliary_size"]
                        and row["machine"] == 0x14C and row["optional_header_size"] == 0):
                    raise ValueError("import thunk namespace has a competing definition or alias")
        path = record["path"]
        member = PurePosixPath(record["member"]).name.casefold()
        observations = objects._owners.get(owner.casefold(), ())
        family = [row for row in records if row["path"] == path and "member" in row
                  and PurePosixPath(row["member"]).name.casefold() == member]
        if not observations:
            raise ValueError("import thunk has no final-pass library family")
        for observed in observations:
            if (observed["library"] is None or observed["paths"] != (path,)
                    or observed["candidate_coverage"] != Result.PASS
                    or type(observed["candidate_count"]) is not int
                    or observed["candidate_count"] != len(family)
                    or PureWindowsPath(observed["member"]).name.casefold() != member
                    or objects._library_names[PureWindowsPath(observed["library"]).name.casefold()] != {path}):
                raise ValueError("import thunk lacks an exact final-pass physical library family")
        mapping.validate(image)
        def placed(name, function):
            rows = [row for row in (*mapping.publics, *(mapping.statics or ())) if row.symbol == name]
            if (len(rows) != 1 or rows[0] not in mapping.publics or not rows[0].segment
                    or rows[0].function != function or rows[0].owner.casefold() != owner.casefold()):
                raise ValueError("import thunk requires exact named public thunk and IAT placements")
            return rows[0]
        thunk, iat = placed(symbol, True), placed(pointer, False)
        slot = import_slots(image).get(iat.address)
        identity = {"module": imported["module"].casefold(), "name": imported["name"],
                    "ordinal": imported["ordinal"]}
        if slot is None or slot["segment"] != iat.segment or any(slot[key] != value for key, value in identity.items()):
            raise ValueError("selected import header disagrees with the independent PE loader slot")
        span = Span(thunk.address, thunk.address + 6)
        region = image.sections[thunk.segment - 1][0]
        flags = image.section_flags[thunk.segment - 1]
        if span.end > region.end or flags & 0x20000020 != 0x20000020:
            raise ValueError("import thunk recipe requires wholly contained executable code")
        if any(row != thunk and span.start <= row.address < span.end
               for row in (*mapping.publics, *(mapping.statics or ()))):
            raise ValueError("import thunk recipe overlaps another named placement")
        recipe = b"\xff\x25\0\0\0\0"
        proof = compare_bound_code(span, image.read(span), recipe,
            (Relocation(2, RelocationKind.ABSOLUTE32, pointer),), {pointer: iat.address},
            original_absolute_sites=image.absolute_sites(span))
        source = {key: record[key] for key in ("path", "member", "occurrence")}
        return {"check": "linked_short_import_thunk", "result": proof["result"],
            "owner": owner, "symbol": symbol, "address": thunk.address, "size": 6,
            "extent_kind": "i386_short_import_thunk", "recipe": "i386_jmp_indirect_abs32",
            "input": source, "object_sha256": record["sha256"],
            "import_header": dict(imported), "import": {**identity, "address": iat.address, "symbol": pointer},
            "selection_basis": "unique_complete_physical_producer_and_named_linked_use",
            "selection": Result.PASS, "proof": proof,
            "body_fingerprint": digest({"recipe": "i386_jmp_indirect_abs32", "symbol": symbol, "import": identity}),
            "dll_implementation": Result.UNKNOWN, "runtime": Result.UNKNOWN,
            "original_verification": Result.UNKNOWN, "ready_for_collection": False}
    finally:
        fresh()
