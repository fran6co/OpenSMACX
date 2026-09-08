"""Derived executable starting points, never inferred function/TU boundaries.

Recursive disassembly is conditional on its roots and instruction interpretations.
In particular a call's fallthrough is explored without claiming that it returns.
Unknown indirect control flow is retained, not patched with guessed targets.
"""

from collections import defaultdict, deque
from dataclasses import asdict
import hashlib
from pathlib import Path

from .model import Origin, Span
from .pe_coff import PeImage
from .source_inventory import assert_snapshot_current
from .ownership_source import project_view


def survey(image: PeImage, pin: str, *, starts=(), max_instructions=50000) -> dict:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_GRP_CALL, CS_GRP_JUMP, CS_GRP_RET, CS_GRP_IRET, CS_GRP_INT
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_INS_JMP, X86_INS_LJMP, X86_INS_LOOP, X86_INS_LOOPE, X86_INS_LOOPNE

    if type(max_instructions) is not int or not 1 <= max_instructions <= 1000000:
        raise ValueError("instruction budget must be between 1 and 1000000")
    if len(starts) > 4096 or len(set(starts)) != len(starts):
        raise ValueError("survey starting hints must be unique and at most 4096")
    if any(type(address) is not int or not 0 <= address <= 0xFFFFFFFF for address in starts):
        raise ValueError("survey starting hints require PE32 virtual addresses")
    if hashlib.sha256(image.data).hexdigest() != pin:
        raise ValueError("survey image identity disagrees")
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    sections = [{"name": name, "mapped": asdict(region), "file_offset": offset, "file_size": size,
                 "executable": bool(flags & 0x20000000), "writable": bool(flags & 0x80000000)}
                for (region, offset, size), flags, name in zip(image.sections, image.section_flags, image.section_names)]

    def backing(address):
        return next((row for row in sections if row["executable"]
                     and row["mapped"]["start"] <= address < row["mapped"]["start"] + row["file_size"]
                     and 0 <= address <= 0xFFFFFFFF), None)

    metadata_gaps, imports = [], []
    try:
        imports = [asdict(item) for item in image.imports()]
    except ValueError as error:
        metadata_gaps.append({"kind": "ordinary_imports", "reason": str(error)})
    import_slots = {row["address"]: row for row in imports}
    for index, name in ((0, "exports"), (3, "exception_handlers"), (9, "tls_callbacks"), (13, "delay_imports")):
        if len(image.directories) > index and any(image.directories[index]):
            metadata_gaps.append({"kind": name, "reason": "additional control-flow roots/edges not decoded by this survey"})
    roots = ([{"address": image.entrypoint, "kind": "pe_entrypoint"}] if image.entrypoint is not None else [])
    roots += [{"address": address, "kind": "supplied_start_hint"} for address in starts]
    if not roots:
        metadata_gaps.append({"kind": "roots", "reason": "no PE entry point or supplied starting hint"})
    evidence = defaultdict(list)
    pending, scheduled = deque(), set()
    instructions, byte_owner, edges, unresolved, conflicts = {}, {}, [], [], set()

    def queue(address):
        if address not in scheduled:
            pending.append(address)
            scheduled.add(address)

    for root in roots:
        evidence[root["address"]].append({"kind": root["kind"]})
        queue(root["address"])
    while pending and len(instructions) < max_instructions:
        address = pending.popleft()
        region = backing(address)
        if region is None:
            unresolved.append({"address": address, "reason": "not executable file-backed image storage"})
            continue
        end = min(region["mapped"]["start"] + region["file_size"], address + 15, 0x100000000)
        instruction = next(decoder.disasm(image.read(Span(address, end)), address, count=1), None)
        if instruction is None:
            unresolved.append({"address": address, "reason": "instruction could not be decoded within executable backing"})
            continue
        occupied = {byte_owner[at] for at in range(address, address + instruction.size) if at in byte_owner}
        if occupied:
            conflicts.update({address, *occupied})
            unresolved.append({"address": address, "reason": "overlapping instruction interpretations", "other_starts": sorted(occupied)})
            continue
        for at in range(address, address + instruction.size):
            byte_owner[at] = address
        instructions[address] = {"address": address, "size": instruction.size,
                                 "mnemonic": instruction.mnemonic, "operands": instruction.op_str}
        call = instruction.group(CS_GRP_CALL)
        jump = instruction.group(CS_GRP_JUMP) or instruction.id in (X86_INS_LOOP, X86_INS_LOOPE, X86_INS_LOOPNE)
        if call or jump:
            kind = "call" if call else "jump" if instruction.id in (X86_INS_JMP, X86_INS_LJMP) else "conditional"
            operands = instruction.operands
            target = operands[0].imm & 0xFFFFFFFF if len(operands) == 1 and operands[0].type == X86_OP_IMM else None
            edge = {"source": address, "kind": kind, "target": target}
            if target is not None:
                if backing(target) is not None:
                    queue(target)
                    if call:
                        evidence[target].append({"kind": "decoded_direct_call", "source": address})
                else:
                    unresolved.append({"address": address, "target": target, "reason": "direct target is not executable file-backed storage"})
            else:
                # A fixed IAT operand names a loader slot, never a loaded code
                # address. Segment overrides and indexed addressing stay unknown.
                operand = operands[0] if len(operands) == 1 else None
                if (operand is not None and operand.type == X86_OP_MEM and not operand.mem.base
                        and not operand.mem.index and not operand.mem.segment):
                    edge["slot"] = operand.mem.disp & 0xFFFFFFFF
                    if edge["slot"] in import_slots:
                        edge["import"] = import_slots[edge["slot"]]
                unresolved.append({"address": address, "reason": "indirect control flow not followed",
                                   **({"import": edge["import"]} if "import" in edge else {})})
            edges.append(edge)
            if kind == "jump":
                continue
        if (instruction.group(CS_GRP_RET) or instruction.group(CS_GRP_IRET) or instruction.group(CS_GRP_INT)
                or instruction.mnemonic in ("hlt", "ud2")):
            continue
        queue(address + instruction.size)
    candidates = []
    for address, reasons in sorted(evidence.items()):
        origin = Origin(pin, "pe:va", address)
        candidates.append({"origin": asdict(origin), "entity_id": origin.entity_id, "evidence": reasons,
                           "decode_status": "conflict" if address in conflicts else "decoded" if address in instructions else "not_decoded",
                           "original_extent": "unknown", "ownership": "unknown", "source_bindings": []})
    return {"schema_version": 1, "check": "image_survey", "image_sha256": pin, "image_base": image.base,
            "backend": "pe-coff-i386", "entrypoint": image.entrypoint, "sections": sections, "roots": roots,
            "imports": imports, "metadata_gaps": metadata_gaps, "candidates": candidates,
            "instructions": [instructions[key] for key in sorted(instructions)], "edges": edges, "unresolved": unresolved,
            "instruction_count": len(instructions), "edge_count": len(edges), "candidate_count": len(candidates),
            "max_instructions": max_instructions, "budget_exhausted": bool(pending), "pending_addresses": list(pending),
            "verification": "not_measured", "coverage": "partial_static_reachability",
            "historical_tu_boundaries": "not_inferred", "ready_for_collection": False, "persisted": False,
            "limitations": ["starting points and decoded calls are hints, not function identities or extents",
                            "call fallthrough is explored without proof of return; data may decode as instructions",
                            "indirect flow, exception paths, exports and TLS roots need additional evidence",
                            "jumps are block edges, not automatically tail calls or new functions",
                            "source correlations retain alternatives and seed provenance; no C++ or annotations are generated"]}


def survey_image(root: Path, image_path: Path, *, starts=(), max_instructions=50000, summary=False) -> dict:
    root = root.resolve()
    image_path = image_path if image_path.is_absolute() else root / image_path
    view, snapshot = project_view(root, image_path, allow_empty=True)
    if snapshot["source_configuration"]["address_space"] != "pe:va":
        raise ValueError("native image survey requires the pe:va source address space")
    pin = snapshot["image_sha256"]
    image = PeImage(image_path.read_bytes(), pin)
    report = survey(image, pin, starts=starts, max_instructions=max_instructions)
    bindings = defaultdict(list)
    for row in snapshot["records"]:
        bindings[row["origin"]["address"]].append({key: row[key] for key in
                                                  ("path", "line", "candidate_kind", "catalogued_symbol", "emitted_symbol_hint")})
    owners = {entity["entity_id"]: (group, entity) for group in view["groups"] for entity in group["entities"]}
    for candidate in report["candidates"]:
        candidate["source_bindings"] = bindings[candidate["origin"]["address"]]
        group, entity = owners.get(candidate["entity_id"], ({}, {}))
        candidate["component_bindings"] = [{"component": row["component"], "path": row["path"], "symbol": row["symbol"]}
                                           for row in entity.get("candidates", []) if row["component"]]
        candidate["ownership_hints"] = entity.get("ownership", {})
        candidate["next_action"] = group.get("next_action", "establish_original_extent_and_ownership")
    report["unmapped_candidate_count"] = sum(not row["source_bindings"] and not row["component_bindings"]
                                             for row in report["candidates"])
    report["declared_dependency_candidate_count"] = sum(bool(row["component_bindings"]) for row in report["candidates"])
    assert_snapshot_current(root, snapshot)
    with image_path.open("rb") as stream:
        if hashlib.file_digest(stream, "sha256").hexdigest() != pin:
            raise ValueError("original image changed during survey")
    if summary:
        report.pop("instructions")
        report.pop("edges")
    return report
