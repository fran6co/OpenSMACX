"""Private data anchors from completely verified selected caller regions.

A PDB target or mapped caller alone is not proof. Verify the actual emitted
region/family before using its exact private-symbol binding. This is placement,
not target contents, caller-closure coverage, storage isolation or acceptance.
"""

from collections import defaultdict

from .linked_code_region import verify_code_region
from .linked_fixups import check_image, check_selected
from .model import Result, digest


def collect_anchors(inputs, selected, obj):
    """Internal caller-owned native batch; no supplied code reports or bounds."""
    owner, image, mapping = selected["owner"], inputs.image, inputs.mapping
    check_image(inputs.fixups, image)
    actual = check_selected(obj, owner, inputs.objects)
    if any(actual[key] != selected[key] for key in ("identity", "sha256", "raw")):
        raise ValueError("data anchor collector differs from selected physical object")
    mapped_private = {row.symbol for row in mapping.statics or () if row.owner.casefold() == owner.casefold()}
    retained_names = {storage: {row.symbol for row in pool if row.owner.casefold() == owner.casefold()
                               and row.function and row.segment}
                      for storage, pool in ((2, mapping.publics), (3, mapping.statics or ()))}
    code_symbols = defaultdict(list)
    for symbol in obj.symbols.values():
        if symbol.section > 0 and symbol.type & 0x20 and symbol.storage in (2, 3):
            code_symbols[symbol.section].append(symbol)
    # An associated child's referring sites require its full owning family.
    roots = {}
    for number in range(1, len(obj.sections) + 1):
        root, seen = number, set()
        while root in obj.associations:
            if root in seen:
                raise ValueError("data anchor caller has cyclic section ownership")
            seen.add(root)
            root = obj.associations[root]
        roots[number] = root
    needed = defaultdict(set)
    for number, section in enumerate(obj.sections, 1):
        for _, index, kind in section.relocations:
            target = obj.symbols[index]
            if (not kind or target.storage != 3 or target.type != 0 or target.section <= 0
                    or obj.auxiliary.get(target.index) or target.section in obj.associations
                    or target.section in obj.associations.values()
                    or obj.sections[target.section - 1].flags & (0x20 | 0x20000000)
                    or target.name in mapped_private):
                continue
            needed[roots[number]].add(target.section)
    candidates, gaps, callers, anchors = [], [], [], defaultdict(list)
    for number, targets in sorted(needed.items()):
        retained = [symbol for symbol in code_symbols[number]
                    if obj.sections[number - 1].flags & 0x20
                    and symbol.name in retained_names[symbol.storage]]
        if not retained:
            gaps.append({"code_section": number, "target_sections": sorted(targets),
                         "reason": "no retained same-owner caller placement for private data references"})
        candidates.extend((symbol, targets) for symbol in retained)
    if len(candidates) > 4096:
        raise ValueError("private data anchor callers exceed the native region resource bound")
    folding = getattr(inputs, "folding", None)
    fold_candidates = folding.candidate_keys() if folding is not None and candidates else frozenset()
    for symbol, targets in candidates:
        row = {"symbol": symbol.name, "object_index": symbol.index, "code_section": symbol.section,
               "target_sections": sorted(targets), "result": Result.UNKNOWN}
        callers.append(row)
        try:
            # ICF map labels can name a different original member under the
            # winner's owner. Even identical code and valid fixups cannot bind
            # that member's private data to this object's same-named COMDAT.
            # This observer does not yet compose complete original-owner groups.
            if (owner.casefold(), symbol.name) in fold_candidates:
                raise ValueError("folded caller requires complete original-owner group evidence")
            proof = verify_code_region(obj, symbol, owner, mapping, image,
                                       objects=inputs.objects, fixups=inputs.fixups)
            if proof["result"] != Result.PASS:
                row.update(result=proof["result"], reason="whole linked caller region/family is unverified")
                gaps.append(dict(row))
                continue
            pending = []
            fingerprint = digest(proof)
            regions = proof.get("regions", [{"section": symbol.section, "address": proof["address"],
                                              "references": proof["references"]}])
            for region in regions:
                for reference in region["references"]:
                    binding = reference.get("private_binding")
                    if binding is None:
                        continue
                    target = obj.symbols.get(binding["object_index"])
                    if (target is None or binding["input"] != selected["identity"]
                            or binding["object_sha256"] != selected["sha256"]
                            or binding["owner"].casefold() != owner.casefold()
                            or target.name != binding["symbol"] or target.section != binding["object_section"]
                            or target.value != binding["object_offset"] or target.storage != 3 or target.type != 0
                            or obj.auxiliary.get(target.index) or binding["placement"] != "pdb_fixup"
                            or reference["address"] != binding["address"]):
                        raise ValueError("verified caller private binding differs from selected target identity")
                    kind = {"absolute32": 6, "relative32": 20}.get(reference["kind"])
                    edge = (reference["offset"], target.index, kind)
                    if (not kind or obj.sections[region["section"] - 1].relocations.count(edge) != 1
                            or roots[region["section"]] != symbol.section):
                        raise ValueError("verified private binding lacks its exact owning caller relocation")
                    if target.section not in targets:
                        continue  # Other independently verified references are not requested data anchors.
                    pending.append({**binding, "placement": "verified_caller_pdb_fixup",
                        "referring_symbol": symbol.name, "referring_object_index": symbol.index,
                        "referring_section": region["section"],
                        "referring_site": region["address"] + reference["offset"],
                        "relocation_kind": reference["kind"], "caller_fingerprint": fingerprint})
            for anchor in pending:
                anchors[anchor["object_section"]].append(anchor)
            row.update(result=Result.PASS, address=proof["address"], size=proof["size"],
                       anchor_count=len(pending), caller_fingerprint=fingerprint)
        except (OSError, ValueError) as error:
            row["reason"] = str(error)
            gaps.append(dict(row))
    return {"anchors": dict(anchors), "callers": callers, "gaps": gaps,
            "caller_coverage": Result.UNKNOWN, "target_contents": Result.UNKNOWN,
            "allocation_isolation": Result.UNKNOWN}
