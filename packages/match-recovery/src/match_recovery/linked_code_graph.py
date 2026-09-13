"""Relocation-reachable linked function code, not memory or startup semantics.

Roots and every discovered code target are bound to selected physical inputs.
Following an address operand includes callbacks and tail-call helpers; it does
not imply that the operand executes, or discover indirect targets through data.
"""

from collections import Counter
import hashlib

from .linked_code_region import select_code_region, verify_code_region
from .linked_imports import import_slots, resolve_import_slot
from .binary import RelocationKind
from .model import Result, digest
from .pe_coff import Coff


MAX_FUNCTIONS = 4096


def observe_code_graph(objects, mapping, image, roots, *, max_functions=MAX_FUNCTIONS, fixups=None, folding=None):
    if folding is None:
        return _observe_code_graph(objects, mapping, image, roots, max_functions=max_functions, fixups=fixups)
    from .linked_folding import Folding
    if (not isinstance(folding, Folding) or folding._objects is not objects
            or folding._mapping != mapping or folding._image is not image or folding._fixups is not fixups):
        raise ValueError("folded graph requires this exact live input/image binding")
    folding.check_current()
    try:
        return _observe_code_graph(objects, mapping, image, roots, max_functions=max_functions,
                                   fixups=fixups, folding=folding)
    finally:
        folding.check_current()


def _observe_code_graph(objects, mapping, image, roots, *, max_functions=MAX_FUNCTIONS, fixups=None, folding=None):
    """Observe a deterministic rooted graph inside the caller's live epoch.

Node numbering follows root/relocation order, not local compiler spellings.
Shared targets and exact linked-address aliases remain shared; identical bytes
never merge distinct definitions. External symbol identity remains explicit.
"""
    if type(max_functions) is not int or not 1 <= max_functions <= MAX_FUNCTIONS:
        raise ValueError("linked code graph requires a bounded positive function limit")
    mapping.validate(image)
    if fixups is not None:
        from .linked_fixups import check_image
        check_image(fixups, image)
    slots, slot_ids, imports = import_slots(image), {}, []
    nodes, keys, parsed, canonical, gaps = [], {}, {}, [], []
    storage, code_targets = [], []
    from .linked_objects import SelectedObjects
    from .linked_import_thunks import import_thunk_candidates, verify_import_thunk
    thunk_candidates = import_thunk_candidates(objects) if isinstance(objects, SelectedObjects) else frozenset()

    def add(owner, symbol):
        if not isinstance(owner, str) or not owner or not isinstance(symbol, str) or not symbol:
            raise ValueError("linked code graph requires owner-qualified function roots")
        key = owner.casefold(), symbol
        if key not in keys:
            if len(nodes) >= max_functions:
                raise ValueError("linked code graph function resource bound exceeded")
            keys[key] = len(nodes)
            nodes.append({"id": len(nodes), "owner": owner, "symbol": symbol, "result": Result.UNKNOWN})
            canonical.append(None)
        return keys[key]

    root_ids = []
    for root in roots:
        root_ids.append(add(root["owner"], root["symbol"]))
    public = {}
    for row in mapping.publics:
        public.setdefault(row.symbol, []).append(row)
    spans, family_spans = [], []
    fold_views, fold_ids = {}, {}
    fold_groups = []
    fold_candidates = folding.candidate_keys() if folding is not None else frozenset()
    cursor = 0

    def register_fold(group):
        from .linked_fold_frames import verify_fold_frames, original_owner_view
        member_keys = [{(group.selected.owner.casefold(), member.symbol),
                        (member.owner.casefold(), member.symbol)} for member in group.members]
        enrolled = set()
        for keys_for_member in member_keys:
            if enrolled & keys_for_member:
                raise ValueError("folded members have ambiguous original/map lookup identities")
            enrolled.update(keys_for_member)
            if len({keys[key] for key in keys_for_member if key in keys}) > 1:
                raise ValueError("folded member has duplicate prequeued original/map identities")
        old = [fold_views[key] for keys_for_member in member_keys for key in keys_for_member if key in fold_views]
        if old:
            if any(value is None or value[1].group != group for value in old):
                raise ValueError("folded group conflicts with an earlier entry interpretation")
            return old[0][1]
        frames = verify_fold_frames(group, image, fixups)
        view = original_owner_view(mapping, group)
        group_id = len(fold_groups)
        fold_groups.append(group)
        for member, keys_for_member in zip(group.members, member_keys):
            for key in keys_for_member:
                fold_views[key] = member, frames, view, group_id
            # Preserve an existing request's identity, then alias other lookup
            # spellings to that same original entry rather than duplicate it.
            existing = [keys[key] for key in keys_for_member if key in keys]
            entry_id = min(existing) if existing else add(group.selected.owner, member.symbol)
            for key in keys_for_member:
                keys.setdefault(key, entry_id)
        return frames

    while cursor < len(nodes):
        node = nodes[cursor]
        try:
            if node["symbol"] in thunk_candidates:
                proof = verify_import_thunk(objects, mapping, image, node["owner"], node["symbol"])
                node.update(input=proof["input"], object_sha256=proof["object_sha256"],
                            proof=proof, result=proof["result"])
                begin, end = proof["address"], proof["address"] + proof["size"]
                if any(left < end and begin < right for _, left, right in spans):
                    raise ValueError("import thunk overlaps another observed code owner")
                spans.append((cursor, begin, end))
                # A linker recipe is not an ordinary COFF body. Keep this span
                # exclusive so a later ordinary entry cannot silently alias it.
                family_spans.append((cursor, begin, end))
                imported = proof["import"]
                slot_id = slot_ids.setdefault(imported["address"], len(slot_ids))
                identity = {key: imported[key] for key in ("module", "name", "ordinal", "symbol")}
                edge = {"offset": 2, "kind": RelocationKind.ABSOLUTE32,
                        "target": {"loader_import": identity, "slot": slot_id}}
                imports.append({"node": cursor, **edge, "address": imported["address"],
                                "implementation": Result.UNKNOWN, "selection": proof["selection"]})
                node["edges"] = [edge]
                canonical[cursor] = {"extent_kind": proof["extent_kind"], "size": proof["size"],
                                     "body_fingerprint": proof["body_fingerprint"], "edges": [edge]}
                cursor += 1
                continue
            owner, local_mapping, frames = node["owner"], mapping, None
            if folding is not None:
                key = owner.casefold(), node["symbol"]
                if key not in fold_views:
                    group = None
                    if key in fold_candidates:
                        group = folding.for_map(owner, node["symbol"], optional=True)
                        if group is None:
                            group = folding.group(owner, node["symbol"], optional=True)
                    if group is None:
                        fold_views[key] = None
                    else:
                        register_fold(group)
                if fold_views[key] is not None:
                    member, frames, local_mapping, group_id = fold_views[key]
                    owner = member.owner
                    fold_ids[cursor] = group_id
                    node.update(source_owner=owner, folded_group=group_id)
            item = objects.resolve(owner)
            if (item["owner"].casefold() != owner.casefold()
                    or hashlib.sha256(item["raw"]).hexdigest() != item["sha256"]):
                raise ValueError("linked code object differs from its selected physical identity")
            if item["sha256"] not in parsed:
                parsed[item["sha256"]] = Coff(item["raw"])
            obj = parsed[item["sha256"]]
            subject = select_code_region(obj, node["symbol"])
            if folding is not None:
                # Private code references retain their original COFF owner even
                # when the map attributes their address to an ICF winner in a
                # different object. Never disguise such code as private data.
                owned = {subject.section}
                while True:
                    added = {child for child, parent in obj.associations.items() if parent in owned} - owned
                    if not added:
                        break
                    owned.update(added)
                targets = {index for number in owned for _, index, kind in obj.sections[number - 1].relocations
                           if kind and obj.symbols[index].storage == 3 and obj.symbols[index].type & 0x20
                           and obj.symbols[index].section > 0 and obj.symbols[index].section not in owned}
                for index in sorted(targets):
                    target = obj.symbols[index]
                    if (owner.casefold(), target.name) not in fold_candidates:
                        continue
                    if any(row.symbol == target.name and row.owner.casefold() == owner.casefold()
                           for row in local_mapping.statics or ()):
                        continue
                    group = folding.group(owner, target.name, optional=True)
                    if group is not None:
                        from .linked_fold_frames import original_owner_view
                        register_fold(group)
                        local_mapping = original_owner_view(local_mapping, group)
            node_static = {}
            for row in local_mapping.statics or ():
                node_static.setdefault((row.owner.casefold(), row.symbol), []).append(row)
            proof = verify_code_region(obj, subject, item["owner"], local_mapping, image,
                                       objects=objects, fixups=fixups, folded_frames=frames)
            node.update(input=item["identity"], object_sha256=item["sha256"],
                        section=subject.section, proof=proof, result=proof["result"])
            if "regions" in proof:
                region_edges = []
                for region in proof["regions"]:
                    begin, end = region["address"], region["address"] + region["size"]
                    overlaps = [(index, left, right) for index, left, right in spans if left < end and begin < right]
                    if any(not (region["ordinal"] == 0 and len(proof["regions"]) == 1
                                and (left, right) == (begin, end) and cursor in fold_ids
                                and fold_ids.get(index) == fold_ids[cursor]
                                and len(nodes[index].get("proof", {}).get("regions", [None])) == 1)
                           for index, left, right in overlaps):
                        # No partial/shared family attribution is inferred from
                        # equal addresses. Ordinary single-region folding below
                        # retains its separate established behavior.
                        raise ValueError("associated family overlaps another observed owner")
                    spans.append((cursor, begin, end))
                    family_spans.append((cursor, begin, end))
                    body = obj.sections[region["section"] - 1]
                    by_offset = {offset: obj.symbols[index] for offset, index, kind in body.relocations if kind}
                    edges = []
                    for reference in region["references"]:
                        target = by_offset[reference["offset"]]
                        addend = int.from_bytes(body.data[reference["offset"]:reference["offset"] + 4], "little", signed=True)
                        edge = {"offset": reference["offset"], "kind": reference["kind"]}
                        identity = reference["identity"]
                        if "absolute_constant" in identity:
                            if reference["kind"] != RelocationKind.ABSOLUTE32:
                                raise ValueError("relative linker constants require independent rebase semantics")
                            edge["target"] = identity
                        elif "owned_region" in identity:
                            target_region = proof["regions"][identity["owned_region"]]
                            if not 0 <= identity["offset"] + addend < target_region["size"]:
                                raise ValueError("effective associated reference is outside its owned region")
                            edge["target"] = identity
                        elif "private_binding" in reference:
                            edge["target"] = {"data": identity}
                            storage.append({"node": cursor, "region": region["ordinal"], **edge,
                                            "address": reference["address"],
                                            "placement": reference["private_binding"], "contents": Result.UNKNOWN})
                        else:
                            matches = [row for row in (public.get(target.name, ()) if target.storage == 2 else
                                       node_static.get((item["owner"].casefold(), target.name), ())) if row.segment]
                            if len(matches) != 1 or matches[0].address != reference["address"]:
                                raise ValueError("associated outward graph edge lacks its verified placement")
                            placed, = matches
                            imported = resolve_import_slot(target, placed, addend, slots)
                            if imported is not None:
                                if reference["kind"] != RelocationKind.ABSOLUTE32:
                                    raise ValueError("loader slot requires an absolute pointer relocation")
                                slot_id = slot_ids.setdefault(imported["address"], len(slot_ids))
                                imported_identity = {key: imported[key] for key in ("module", "name", "ordinal", "symbol")}
                                edge["target"] = {"loader_import": imported_identity, "slot": slot_id}
                                imports.append({"node": cursor, "region": region["ordinal"], **edge,
                                    "address": imported["address"], "implementation": Result.UNKNOWN, "selection": Result.UNKNOWN})
                            elif placed.function:
                                edge["target"] = {"node": add(placed.owner, placed.symbol)}
                                code_targets.append((cursor, edge["target"]["node"], addend))
                                if addend:
                                    edge["target"]["addend"] = addend
                                if target.storage == 2:
                                    edge["target"]["external"] = target.name
                            else:
                                if target.type & 0x20:
                                    raise ValueError("declared code target has a non-function linked placement")
                                edge["target"] = {"data": identity}
                                storage.append({"node": cursor, "region": region["ordinal"], **edge,
                                                "address": placed.address, "contents": Result.UNKNOWN})
                        edges.append(edge)
                    region_edges.append({key: region[key] for key in
                        ("ordinal", "parent", "flags", "code_sha256", "symbols")} | {"edges": edges})
                node["edges"] = region_edges
                canonical[cursor] = {"extent_kind": "associated_coff_regions", "regions": region_edges,
                                     "debug_regions": proof["debug_regions"],
                                     "folded_group": fold_ids.get(cursor),
                                     "folded_frames": frames.canonical() if frames else None}
                cursor += 1
                continue
            code, relocations, _ = obj.subject(subject)
            begin, end = proof["address"], proof["address"] + len(code)
            overlaps = [(index, left, right) for index, left, right in spans if left < end and begin < right]
            if any(left < end and begin < right and not (
                    cursor in fold_ids and fold_ids.get(index) == fold_ids[cursor]
                    and (left, right) == (begin, end)
                    and len(nodes[index].get("proof", {}).get("regions", ())) == 1)
                   for index, left, right in family_spans):
                raise ValueError("ordinary code overlaps an observed associated family")
            if any((left, right) != (begin, end) for _, left, right in overlaps):
                raise ValueError("linked code graph has overlapping, differently bounded functions")
            if any((cursor in fold_ids or index in fold_ids) and fold_ids.get(cursor) != fold_ids.get(index)
                   for index, _, _ in overlaps):
                raise ValueError("linked code overlaps a different or unproved folding group")
            alias = overlaps[0][0] if overlaps else cursor
            spans.append((cursor, begin, end))
            edges = []
            constants = {row["offset"]: row for row in proof["references"] if "constant_binding" in row}
            private = {row["offset"]: row for row in proof["references"] if "private_binding" in row}
            for relocation in sorted(relocations, key=lambda row: row.offset):
                target = obj.symbols[int(relocation.target)]
                addend = int.from_bytes(code[relocation.offset:relocation.offset + 4], "little", signed=True)
                edge = {"offset": relocation.offset, "kind": relocation.kind}
                if relocation.offset in constants:
                    if relocation.kind != RelocationKind.ABSOLUTE32:
                        raise ValueError("relative linker constants require independent rebase semantics")
                    edge["target"] = constants[relocation.offset]["identity"]
                elif relocation.offset in private:
                    reference = private[relocation.offset]
                    edge["target"] = {"data": reference["identity"]}
                    storage.append({"node": cursor, **edge, "address": reference["address"],
                                    "placement": reference["private_binding"], "contents": Result.UNKNOWN})
                elif target.section == subject.section and 0 <= target.value < len(code):
                    if not 0 <= target.value + addend < len(code):
                        raise ValueError("effective self code reference is outside its observed function span")
                    edge["target"] = {"self_offset": target.value}
                    if target.storage == 2 and proof.get("extent_kind") == "selected_code_section":
                        edge["target"]["external"] = target.name
                else:
                    if target.storage not in (2, 3) or target.section < 0:
                        raise ValueError("unsupported linked code target identity")
                    matches = [row for row in (public.get(target.name, ()) if target.storage == 2 else
                               node_static.get((item["owner"].casefold(), target.name), ())) if row.segment]
                    if len(matches) != 1:
                        raise ValueError("linked code target has no unique independent placement")
                    placed, = matches
                    imported = resolve_import_slot(target, placed, addend, slots)
                    if imported is not None:
                        if relocation.kind != RelocationKind.ABSOLUTE32:
                            raise ValueError("loader slot requires an absolute pointer relocation")
                        slot_id = slot_ids.setdefault(imported["address"], len(slot_ids))
                        identity = {key: imported[key] for key in ("module", "name", "ordinal", "symbol")}
                        edge["target"] = {"loader_import": identity, "slot": slot_id}
                        imports.append({"node": cursor, **edge, "address": imported["address"],
                                        "implementation": Result.UNKNOWN, "selection": Result.UNKNOWN})
                    elif placed.function:
                        edge["target"] = {"node": add(placed.owner, placed.symbol)}
                        code_targets.append((cursor, edge["target"]["node"], addend))
                        if addend:
                            edge["target"]["addend"] = addend
                        if target.storage == 2:
                            edge["target"]["external"] = target.name
                    else:
                        if target.type & 0x20:
                            raise ValueError("declared code target has a non-function linked placement")
                        identity = ({"external": target.name} if target.storage == 2 else
                                    {"local_owner": item["owner"], "symbol": target.name})
                        edge["target"] = {"data": identity}
                        storage.append({"node": cursor, **edge, "address": placed.address,
                                        "contents": Result.UNKNOWN})
                edges.append(edge)
            node["edges"] = edges
            canonical[cursor] = {"code_sha256": hashlib.sha256(code).hexdigest(), "size": len(code),
                                 "flags": obj.sections[subject.section - 1].flags,
                                 "extent_kind": proof.get("extent_kind", "function_comdat"),
                                 "section_symbols": proof.get("section_symbols", []),
                                 "entry_offset": proof.get("entry_offset", 0),
                                 "entry_symbol": node["symbol"] if proof.get("extent_kind") == "selected_code_section" else None,
                                 "folded_group": fold_ids.get(cursor),
                                 "folded_frames": frames.canonical() if frames else None,
                                 "address_alias": alias, "edges": edges}
        except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
            # An observed code counterexample is not erased by a later gap in
            # the same function's target closure.
            node.update(result=Result.FAIL if node["result"] == Result.FAIL else Result.UNKNOWN, reason=str(error))
            gaps.append({"node": cursor, "owner": node["owner"], "symbol": node["symbol"], "reason": str(error)})
        cursor += 1
    # A relocation can name a real function while its addend actually lands
    # somewhere else. Observe the effective destination, not just the symbol.
    for source, target, addend in code_targets:
        proof = nodes[target].get("proof")
        if proof is not None and not 0 <= proof.get("entry_offset", 0) + addend < proof["size"]:
            node = nodes[source]
            reason = "effective code reference is outside its observed target function span"
            node.update(result=Result.FAIL if node["result"] == Result.FAIL else Result.UNKNOWN, reason=reason)
            gaps.append({"node": source, "owner": node["owner"], "symbol": node["symbol"], "reason": reason})
    counts = Counter(row["result"] for row in nodes)
    result = Result.FAIL if counts[Result.FAIL] else Result.UNKNOWN if counts[Result.UNKNOWN] else Result.PASS
    return {"check": "linked_relocation_code_graph", "result": result, "roots": root_ids,
            "nodes": nodes, "counts": dict(counts), "gaps": gaps, "data_references": storage,
            "import_references": imports,
            "graph_fingerprint": digest({"roots": root_ids, "nodes": canonical}) if result == Result.PASS else None,
            "remaining_linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
            "limitations": ["code reached by explicit COFF relocations only; data contents and indirect targets are unproved",
                            "code agreement is not guard, callback registration, storage or runtime effect equivalence",
                            "loader slots bind map symbols to actual PE import identities, not selected import-library intent or loaded DLL behavior",
                            "one bounded code COMDAT, a completely verified associated family, or an entire ordinary section with jointly placed external entries; other definitions refuse",
                            "shared definitions, relocation order and linked address aliasing are retained"]}


def compare_code_graphs(baseline, current):
    """Compose in-epoch observations, never accept this report as a waiver."""
    results = [baseline["result"], current["result"]]
    complete = all(result == Result.PASS for result in results)
    equal = complete and baseline["graph_fingerprint"] == current["graph_fingerprint"]
    result = (Result.FAIL if Result.FAIL in results else Result.UNKNOWN if not complete else
              Result.PASS if equal else Result.FAIL)
    return {"check": "application_initializer_code_dependencies", "result": result,
            "baseline": baseline, "current": current,
            "graph_equal": equal if complete else None,
            "remaining_linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False}
