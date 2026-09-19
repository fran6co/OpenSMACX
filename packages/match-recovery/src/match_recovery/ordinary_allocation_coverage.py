"""Native ordinary retention and bounded static-allocation isolation.

Physical inputs, PDB contributions and verified whole-section extents must close
the competitor domain together. No missing placement means discard, and no saved
report or source-scoped body permission supplies an allocation exemption.
"""

from collections import Counter, defaultdict
from contextlib import ExitStack

from .graph_limits import MAX_NODES
from .linked_code_graph import observe_code_graph
from .linked_code_region import select_code_region
from .linked_contributions import ordinary_contributions
from .linked_data_contents import data_contents_batch
from .linked_interval_index import IntervalIndex
from .model import Result, digest
from .pe_coff import Coff


def _contribution_groups(sections):
    """Index every assigned claimant, including failed and ungrouped rows."""
    groups = defaultdict(set)
    for row in sections:
        if row.get("contribution") is not None:
            groups[row["contribution"]].add(row.get("fold_group_id"))
    return groups


def observe_ordinary_allocations(native, *, folded_groups=()):
    """Recompute supported static input allocations in the enclosing live epoch.

    The enclosing LinkedBuild guards all parsed state and artifacts. This private
    operation additionally brackets physical population and context freshness.
    Certified ICF fold groups may compose same-key code coalescence; everything
    outside them keeps its own obligations. It establishes neither whole-image
    attribution nor task/runtime acceptance.
    """
    from .linked_context import LinkedBuild
    from .application_code_roots import combine_roots
    if type(native) is not LinkedBuild:
        raise ValueError("ordinary allocations require the live native linked build")
    if not isinstance(folded_groups, (list, tuple)):
        raise ValueError("folded groups must be certified group certificates")
    fold_members = {}
    for group in folded_groups:
        for member in group.get("members", ()):
            key = (digest(member["input"]), member["object_sha256"], member["root"]["section"])
            if fold_members.get(key, group)["group_id"] != group["group_id"]:
                raise ValueError("one folded member claims two certified groups")
            fold_members[key] = group
    native.check_current()
    context, inputs = native.context, native.inputs
    before = digest(context.report)
    _, fresh = inputs.objects._constant_population()
    observation_stack = ExitStack()
    try:
        context.check_current()
        joined = ordinary_contributions(context, inputs)
        census = joined["census"]
        gaps, sections, roots = [], [], []
        modules = defaultdict(list)
        for row in joined["modules"]["modules"]:
            if row["result"] == Result.PASS:
                modules[digest(row["input"])].append(row)
        contributions = joined["contributions"]
        from .linker_generated_storage import assign_linker_contributions
        generated = assign_linker_contributions(contributions, joined["modules"]["modules"], inputs.image,
                                               pdb=inputs.artifacts.get("msvc-pdb"))
        assigned_contributions = {record["index"] for record in generated["records"]
                                  if record["result"] == Result.PASS}
        contribution_ranges = IntervalIndex()
        for row in contributions:
            if row["size"] > 0:
                contribution_ranges.insert(row["index"], row["address"], row["address"] + row["size"])

        def gap(reason, item=None):
            gaps.append({"reason": reason, **({"input": item["input"],
                **({"object_section": item["object_section"]} if "object_section" in item else {})}
                if item is not None else {})})

        if census["enumeration"] != Result.PASS:
            gap("physical allocation population is incomplete")
        # These observations contain unsupported policy and unbound participation,
        # not merely an empty intersection list. None is an allocation exemption.
        # Common requests and exact positively removed sections are reconciled
        # below against live native proofs. Other census obligations stay.
        ordinary = {(digest(row["input"]), row["object_section"]): row for row in joined["sections"]}
        verify_data = observation_stack.enter_context(data_contents_batch(inputs))
        choices, removed = None, {}
        for candidate in census["inputs"]:
            if (candidate["participation"] != "final_pass" or len(candidate["owners"]) != 1):
                gap("candidate lacks independent retained or non-admitted allocation evidence", candidate)
            if candidate["imports"] or candidate["directives"]:
                gap("import or directive allocation effects remain unbounded", candidate)
            if candidate["format"] != "coff":
                gap("non-COFF input requires a linker-generated allocation bound", candidate)
            owner = candidate["owners"][0] if len(candidate["owners"]) == 1 else None
            obj = None
            for item in candidate["sections"]:
                row = {"input": dict(candidate["input"]), "object_sha256": candidate["object_sha256"],
                       **{key: item[key] for key in ("object_section", "size", "flags", "classification")},
                       "retention": Result.UNKNOWN, "input_consistency": Result.UNKNOWN,
                       "characteristics_agreement": Result.UNKNOWN,
                       "allocation_isolation": Result.UNKNOWN}
                sections.append(row)
                category = item["classification"]
                if category == "empty":
                    # The native classifier checks both zero extent and absence
                    # of relocation writes/nonsection symbol-bearing allocations.
                    row["nonoccupying"] = Result.PASS
                    continue
                if item["flags"] & 0x1000 and owner is not None and candidate["participation"] == "final_pass":
                    try:
                        # The census and native query must name the same exact
                        # physical object, not just an equal basename or body.
                        selected = inputs.objects.resolve(owner)
                        if (selected["identity"] != row["input"] or selected["sha256"] != row["object_sha256"]):
                            raise ValueError("removal query differs from its physical census object")
                        if choices is None:
                            from .linked_comdat_selection import comdat_selection
                            choices = observation_stack.enter_context(comdat_selection(native))
                        removal = choices.discarded(owner, row["object_section"])
                        if removal is None:
                            removal = choices.loser(owner, row["object_section"])
                        if removal is not None:
                            if (removal["queried_section"] != row["object_section"]
                                    or row["object_section"] not in removal["sections"]
                                    or removal["disposition"] not in (
                                        "discarded_any_duplicate", "discarded_unreferenced_comdat")):
                                raise ValueError("removal certificate does not cover this physical section")
                            # Preserve positive source size, flags and identity.
                            # Only its final allocation is absent; this is not
                            # an empty COFF section or an incoming-edge waiver.
                            row.update(nonoccupying=Result.PASS, retention=Result.PASS,
                                       disposition=removal["disposition"],
                                       selection_certificate=removal["certificate_id"])
                            removed[digest(row["input"]), row["object_section"]] = category
                            continue
                    except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
                        # Unsupported removal leaves the original root and all
                        # of its placement/content obligations in the graph.
                        row["removal_refusal"] = str(error)
                if category not in ("ordinary_data", "code"):
                    gap("section has no supported independent allocation interpretation", row)
                    continue
                if owner is None or candidate["participation"] != "final_pass":
                    gap("section has no unique selected physical owner", row)
                    continue
                try:
                    if category == "ordinary_data":
                        placed = ordinary[digest(row["input"]), row["object_section"]]
                        if "envelope" in placed:
                            row["envelope"] = placed["envelope"]
                        proof = verify_data(owner, row["object_section"])
                        row.update(proof=proof, input_consistency=proof["input_consistency"])
                        if (proof["input"] != row["input"] or proof["object_sha256"] != row["object_sha256"]
                                or proof["envelope"] != row.get("envelope")):
                            raise ValueError("contents and contribution observations disagree on their physical section")
                    else:
                        if obj is None:
                            selected = inputs.objects.resolve(owner)
                            if selected["identity"] != row["input"] or selected["sha256"] != row["object_sha256"]:
                                raise ValueError("code allocation differs from its selected physical candidate")
                            obj = Coff(selected["raw"])
                        entries = []
                        for symbol in item["symbols"]:
                            if symbol["storage"] in (2, 3) and symbol["type"] & 0x20:
                                try:
                                    entry = select_code_region(obj, symbol["name"])
                                except ValueError:
                                    continue
                                if entry.section == row["object_section"]:
                                    entries.append(entry.name)
                        if not entries:
                            raise ValueError("code allocation lacks a supported independently bounded entry")
                        roots.append({"owner": owner, "symbol": min(entries)})
                except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
                    gap(str(error), row)
        certificates = choices.certificates() if choices is not None else []
        # Failed final freshness is not one section's missing evidence. Both
        # placement and removal batches close before any result is consumed.
        observation_stack.close()
        for item in census["gaps"]:
            if item["reason"] == "common_requests require separate linked-effect verification":
                continue  # Reconciled below by independent common storage.
            key = digest(item.get("input")), item.get("object_section")
            category = removed.get(key)
            if category is not None and item["reason"] == "unplaced or separately verified section: " + category:
                continue  # Only this exact certified family's placement gap.
            gaps.append(item)
        try:
            roots = combine_roots(native, roots)
            code = observe_code_graph(inputs.objects, inputs.mapping, inputs.image, roots,
                                      max_functions=MAX_NODES, fixups=inputs.fixups, folding=inputs.folding)
        except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
            # Missing code evidence must not erase already measured data damage.
            code = {"result": Result.UNKNOWN, "nodes": [], "reason": str(error)}
        by_section = defaultdict(list)
        for node in code["nodes"]:
            if "input" in node and "section" in node:
                by_section[digest(node["input"]), node["section"]].append(node)
        if code["result"] != Result.PASS:
            gap("native code allocation observation is incomplete")
        allocations = IntervalIndex()
        for index, row in enumerate(sections):
            if row.get("disposition") in ("discarded_any_duplicate", "discarded_unreferenced_comdat"):
                continue  # No linked extent; retain the complete original row.
            if row["classification"] == "code":
                nodes = by_section[digest(row["input"]), row["object_section"]]
                if len(nodes) == 1:
                    node, = nodes
                    proof = node.get("proof", {})
                    if (node["object_sha256"] == row["object_sha256"] and node["result"] == Result.PASS
                            and ("regions" not in proof or len(proof["regions"]) == 1)
                            and proof["size"] == row["size"]):
                        row.update(input_consistency=Result.PASS, proof=proof,
                                   envelope={"address": proof["address"], "size": proof["size"]})
            envelope = row.get("envelope")
            if envelope is None:
                if not row.get("nonoccupying"):
                    gap("positive section lacks an independent whole allocation envelope", row)
                continue
            begin, end = envelope["address"], envelope["address"] + row["size"]
            allocations.insert(index, begin, end)
            physical = modules[digest(row["input"])]
            matches = contribution_ranges.overlaps(begin, end)
            row["intersections"] = [item[0] for item in matches]
            group = (fold_members.get((digest(row["input"]), row["object_sha256"], row["object_section"]))
                     if row["classification"] == "code" else None)
            storage = group["final_storage"] if group is not None else None
            if storage is not None and (storage["address"] != begin or storage["size"] != row["size"]):
                gap("certified fold storage disagrees with the allocation envelope", row)
                continue
            if storage is not None:
                # A certified ICF member shares the selected member's storage;
                # the group proof already bound its exact physical identity.
                row["fold_group_id"] = group["group_id"]
                row["contribution"] = storage["contribution"]
                row["pdb_flags"] = storage["pdb_flags"]
                if storage["object_flags"] == row["flags"]:
                    row["characteristics_agreement"] = Result.PASS
                else:
                    # Same retention rule as an own-module join: producer flag
                    # transformations remain visible and unexplained.
                    gap("allocation contribution characteristics remain unexplained", row)
                continue
            if len(physical) != 1 or physical[0]["object_sha256"] != row["object_sha256"] or len(matches) != 1:
                gap("allocation lacks a unique physical module/contribution join", row)
                continue
            contribution = contributions[matches[0][0]]
            row["pdb_flags"] = contribution["flags"]
            if (contribution["module"] != physical[0]["index"] or contribution["address"] != begin
                    or contribution["size"] != row["size"]):
                gap("allocation and PDB contribution disagree on exact module or extent", row)
                continue
            if row["classification"] == "ordinary_data":
                placed = ordinary[digest(row["input"]), row["object_section"]]
                if (placed["correspondence"] != Result.PASS
                        or placed["contribution"] != contribution["index"]):
                    gap("ordinary allocation lacks supported physical contribution correspondence", row)
                    continue
            if contribution["flags"] == row["flags"]:
                row["characteristics_agreement"] = Result.PASS
            else:
                # Whole-section presence is not producer flag equivalence. The
                # ordinary join checks supported form/kind/access/alignment;
                # retain unexplained bits as an isolation/acceptance obligation.
                gap("allocation contribution characteristics remain unexplained", row)
                if row["classification"] != "ordinary_data":
                    continue
            row["contribution"] = contribution["index"]
        from .common_allocation_coverage import observe_common_allocations
        commons = observe_common_allocations(inputs, sections)
        gaps.extend(commons["gaps"])
        retained_names = {row["symbol"] for row in commons["allocations"] if row["retention"] == Result.PASS}
        for candidate in census["inputs"]:
            if any(symbol["name"] not in retained_names for symbol in candidate["common_requests"]):
                gap("common requests lack independently retained storage", candidate)
        # The relation must be injective in both directions. Neither a failed
        # join nor a physically placed competitor may disappear from this check.
        users = Counter(index for row in sections for index in row.get("intersections", ()))
        contribution_groups = _contribution_groups(sections)
        used = set()
        overlap = commons["allocation_isolation"] == Result.FAIL
        for index, row in enumerate(sections):
            contribution = row.get("contribution")
            same_group = None
            if contribution is not None:
                identities = contribution_groups[contribution]
                same_group = (row.get("fold_group_id") is not None and len(identities) == 1
                              and None not in identities)
            if (contribution is not None and row["input_consistency"] == Result.PASS
                    and (users[contribution] == 1 or same_group)):
                # One certified group jointly retains its shared contribution;
                # every member keeps its own physical/code obligations.
                row["retention"] = Result.PASS
                used.add(contribution)
            elif not row.get("nonoccupying"):
                gap("section retention lacks complete unique physical correspondence", row)
            if "envelope" in row:
                begin = row["envelope"]["address"]
                row["overlaps"] = [other for other, _, _ in allocations.overlaps(begin, begin + row["size"])
                                   if other != index]
                if row["overlaps"]:
                    # Native code verification permits proven ICF/entry aliases.
                    # Their retained allocation identity still needs composition;
                    # do not reclassify supported coalescence as data corruption.
                    distinct = [other for other in row["overlaps"] if row["classification"] != "code"
                                or sections[other]["classification"] != "code"]
                    if distinct:
                        row["allocation_isolation"] = Result.FAIL
                        overlap = True
                    elif "fold_group_id" in row and all(
                            sections[other].get("fold_group_id") == row["fold_group_id"]
                            for other in row["overlaps"]):
                        row["allocation_sharing"] = "certified_icf_coalescence"
                    else:
                        gap("shared native code needs retained allocation correspondence", row)
        # Verified linker-generated storage is assigned, not physically joined;
        # it no longer keeps the contribution domain open.
        common_contributions = set(commons["retained_contributions"])
        unmatched = [row["index"] for row in contributions
                     if row["index"] not in used and row["index"] not in assigned_contributions
                     and row["index"] not in common_contributions]
        if unmatched:
            gap("PDB contributions remain without unique verified physical allocations")
        closed = not gaps and not unmatched
        for row in sections:
            if row["retention"] == Result.PASS and row["allocation_isolation"] != Result.FAIL and closed:
                row["allocation_isolation"] = Result.PASS
        for row in commons["allocations"]:
            if row["retention"] == Result.PASS and closed:
                row["allocation_isolation"] = Result.PASS
        damaged = (code["result"] == Result.FAIL or commons["input_consistency"] == Result.FAIL
                   or any(row["input_consistency"] == Result.FAIL for row in sections))
        return {"check": "ordinary_allocation_coverage", "sections": sections, "code": code,
                "contributions": contributions, "unmatched_contributions": unmatched, "gaps": gaps,
                "generated_storage": generated,
                "comdat_removal_certificates": certificates,
                "common_storage": commons,
                "empty_contributions": [row["index"] for row in contributions if row["size"] == 0],
                "input_consistency": Result.FAIL if damaged else Result.PASS if closed else Result.UNKNOWN,
                "allocation_domain": Result.PASS if closed else Result.UNKNOWN,
                "allocation_isolation": Result.FAIL if overlap else Result.PASS if closed else Result.UNKNOWN,
                "result": Result.FAIL if damaged or overlap else Result.PASS if closed else Result.UNKNOWN,
                "remaining_linked_effects": Result.UNKNOWN, "retained_storage_coverage": Result.UNKNOWN,
                "scope": "supported_static_physical_allocations", "ready_for_collection": False, "persisted": False}
    finally:
        try:
            observation_stack.close()
        finally:
            try:
                fresh()
                if digest(context.report) != before:
                    raise ValueError("ordinary allocation context changed during observation")
            finally:
                native.check_current()
