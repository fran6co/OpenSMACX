"""Live, per-key COFF selection certificates, not scope or acceptance.

An independently verified native winner and a complete physical same-key
candidate inventory establish only that the other ANY definitions (and their
associative descendants) were removed. Different loser bytes are permitted.
Missing MAP entries alone never establish REF removal or winner identity.
NODUP code removal requires a unique positive native REF event independently
bound to its selected physical input, with no contradictory original evidence.
Neither operation transfers source ownership or erases incoming references.

PDB module/address correspondence is distinct from characteristics equivalence:
https://github.com/microsoft/microsoft-pdb/blob/master/PDB/include/dbicommon.h
https://github.com/microsoft/microsoft-pdb/blob/master/PDB/dbi/mod.cpp
"""

from contextlib import contextmanager
from copy import deepcopy
from dataclasses import asdict
from weakref import WeakKeyDictionary

from .coff_comdat import comdat_key, comdat_selection as section_selection
from .common_allocation import _policy, _tokens
from .linked_code_ownership import _contents
from .linked_code_region import verify_code_region
from .linked_codeview import LinkedProcedures
from .linked_constant_batch import constant_lookups
from .linked_context import LinkedBuild
from .parsed_state import same
from .linked_interval_index import IntervalIndex
from .linked_input_epoch import input_epoch, _borrow_epoch
from .linked_map_index import index_for
from .linked_objects import _owner
from .linked_storage_census import _candidates
from .model import Result, digest
from .pdb_contributions import bound_contributions
from .pe_coff import Coff, CoffRecords, weak_external


MAX_FOLD_OVERLAPS = 4000000


def _identity(record):
    return {key: record[key] for key in ("path", "member", "occurrence", "generated") if key in record}


def _family(obj, root):
    sections = {root}
    for number in range(1, len(obj.sections) + 1):
        parent = number
        while parent in obj.associations:
            parent = obj.associations[parent]
        if parent == root:
            sections.add(number)
    # A weak alias may name a private child label rather than the external
    # root. The global key index cannot rule that edge out by root spelling.
    # Keep the entire family unresolved; removing it must not erase a forced
    # or retained alias, including the final edge of a weak-alias chain.
    for symbol in obj.symbols.values():
        if symbol.storage == 105 and weak_external(obj, symbol)[0].section in sections:
            raise ValueError("COMDAT family has a weak fallback into an original section")
    # Every child keeps its own original section identity and parent relation.
    rows = []
    for number in sorted(sections):
        if number != root and section_selection(obj, number) != 5:
            raise ValueError("COMDAT descendant lacks exact associative selection metadata")
        section = obj.sections[number - 1]
        rows.append({"object_section": number, "name": section.name,
                     "parent": obj.associations.get(number), "flags": section.flags,
                     "size": obj.section_sizes[number - 1],
                     "symbols": [asdict(item) for item in obj.symbols.values() if item.section == number]})
    return rows


class ComdatSelection:
    """Expiring native query handle. Serialized reports cannot recreate it."""

    __slots__ = ("__weakref__",)

    def __new__(cls, *args, **kwargs):
        raise TypeError("COMDAT selection requires its live context manager")

    def __reduce__(self):
        raise TypeError("COMDAT selection handles are not serializable")

    def loser(self, owner, section):
        """Return a certified original loser family, or None for a non-loser.

        Unsupported or contradictory relevant evidence raises ValueError. The
        consumer must retain that original root as an unresolved obligation.
        """
        # Atomic batch: the enclosing context authenticates every physical
        # input before entry and again before any result may escape its exit.
        # Re-hashing the whole compiler population per loser is unnecessary.
        return _loser(_entry(self), owner, section)

    def certificates(self):
        """One defensive complete inventory per key, not per loser section."""
        state = _entry(self)
        return deepcopy([*state["certificates"].values(), *state["ref_certificates"].values()])

    def discarded(self, owner, section):
        """Query positive native REF removal of an exact NODUP code root.

        No matching event returns None, not an inferred retention/discard.
        Original incoming-reference obligations are never discharged here.
        """
        return _discarded(_entry(self), owner, section)

    def discard_observations(self):
        """Return the parsed native log observations, if a query needed them."""
        return deepcopy(_entry(self)["discard_observation"])

    def placed(self, owner, section):
        """Query positive retention of an exact placed COMDAT code root.

        An independently placed NODUP root or the certified ANY winner earns
        its disposition; anything else returns None, never an inferred
        selection. The root and its descendants stay physical/code
        obligations; this is derived retention evidence, not a census
        exclusion.
        """
        return _placed(_entry(self), owner, section)

    def folded_groups(self):
        """One complete native group proof, separate from same-key removals."""
        return deepcopy(list(_entry(self)["fold_groups"].values()))

    def unique_placements(self):
        """Independent exact NODUP root placements, never census exclusions."""
        return deepcopy(list(_entry(self)["unique_placements"].values()))

    def folded(self, owner, section):
        """Query the certified fold group of one exact original member.

        No certified membership returns None; ambiguous membership raises.
        This reuses complete group proofs; it verifies nothing new and never
        discharges the member's own physical obligations.
        """
        return _folded_member(_entry(self), owner, section)


_live = WeakKeyDictionary()


def _entry(handle):
    if type(handle) is not ComdatSelection or handle not in _live:
        raise ValueError("COMDAT selection handle is unrecognized or expired")
    return _live[handle]


def _object(state, record):
    identity = _identity(record)
    token = digest(identity), record["sha256"]
    if token not in state["objects"]:
        raw = _contents(identity, record["sha256"], state["inputs"])
        obj = Coff(raw)
        state["objects"][token] = obj
        state["object_witnesses"][token] = obj, deepcopy(obj.__dict__)
    return state["objects"][token]


def _metadata(state):
    if state["metadata"] is not None:
        return state["metadata"]
    native = state["native"]
    inputs = native.inputs
    pdb = inputs.artifacts.get("msvc-pdb")
    if pdb is None:
        raise ValueError("ANY selection requires its actual image-bound PDB")
    contributions = bound_contributions(inputs.image, pdb)
    modules = native.module_bindings()
    if modules["image_sha256"] != contributions.image_sha256 or modules["pdb_sha256"] != contributions.pdb_sha256:
        raise ValueError("ANY module/contribution image binding differs")
    module_index = {}
    for row in modules["modules"]:
        if row["result"] == Result.PASS:
            module_index.setdefault((digest(row["input"]), row["object_sha256"]), []).append(row["index"])
    spans = IntervalIndex(max_intervals=1000000)
    for index, row in enumerate(contributions.contributions):
        if row.size:
            start = inputs.image.sections[row.section - 1][0].start + row.offset
            spans.insert(index, start, start + row.size)
    state["metadata"] = contributions, module_index, spans
    state["metadata_witness"] = contributions, deepcopy(contributions), deepcopy(module_index), deepcopy(spans.__dict__)
    return state["metadata"]


def _code_candidates(state):
    """Complete possible code population, not only mapped or reached roots."""
    if state["code_candidates"] is not None:
        return state["code_candidates"]
    rows = []
    for records in state["records"].values():
        if len(records) != 1:
            raise ValueError("no-fold population has duplicate physical input identities")
        record, = records
        if record["format"] == "short_import":
            # Import recipes have no original COMDAT section. They are not
            # application sections eligible for this whole-section ICF model.
            continue
        identity = _identity(record)
        obj = CoffRecords(_contents(identity, record["sha256"], state["inputs"]))
        for number, section in enumerate(obj.sections, 1):
            if not section.flags & 0x20000020:
                continue
            size = obj.section_sizes[number - 1]
            if not size and not section.data and not section.relocations:
                continue  # No original code bytes can supply a nonempty winner.
            if obj.machine != 0x14C or obj.optional_header_size:
                raise ValueError("no-fold competitor has an unsupported code object form")
            rows.append((digest(identity), record["sha256"], number, section, size))
            if len(rows) > 1000000:
                raise ValueError("no-fold physical code population exceeds its bound")
    state["code_candidates"] = tuple(rows)
    state["code_candidates_witness"] = deepcopy(state["code_candidates"])
    return state["code_candidates"]


def _no_other_fold(state, obj, root, candidates, address):
    from .linked_comdat_nofold import sections_distinct

    # The complete same-key ANY inventory has already qualified these precise
    # independent roots. Associated code children are NOT excluded with them.
    excluded = {(digest(row["input"]), row["object_sha256"], row["root"]["section"])
                for row in candidates}
    code = obj.sections[root.section - 1]
    size = obj.section_sizes[root.section - 1]
    checked = 0
    placements = []
    removed = []
    for identity, stamp, number, other, other_size in _code_candidates(state):
        if (identity, stamp, number) in excluded:
            continue
        if not sections_distinct(code, size, other, other_size):
            try:
                removal = _discarded_candidate(state, identity, stamp, number)
                if removal is not None:
                    removed.append(removal["certificate_id"])
                    checked += 1
                    continue
                peer = _unique_placement(state, identity, stamp, number)
                if address < peer["address"] + peer["size"] and peer["address"] < address + size:
                    raise ValueError("unique NODUP root is not separately placed from this ANY candidate")
            except ValueError as error:
                if type(error) is not ValueError:
                    raise
                raise ValueError("ANY winner has a possible distinct-section ICF competitor: " + str(error)) from error
            placements.append(peer["placement_id"])
        checked += 1
    method = ("immutable_prefix_or_native_ref_or_independent_unique_root_placement" if removed
              else "immutable_prefix_or_independent_unique_root_placement" if placements
              else "immutable_code_prefix_mismatch")
    return {"result": Result.PASS, "method": method,
            "other_physical_code_sections": checked,
            "unique_placements": placements,
            "ref_discard_certificates": list(dict.fromkeys(removed)),
            "same_key_roots": len(excluded), "source_authority": "not_transferred"}


def _certificate(state, name):
    if name in state["certificates"]:
        return state["certificates"][name]
    if name in state["refusals"]:
        raise ValueError(state["refusals"][name])
    try:
        return _certificate_uncached(state, name)
    except ValueError as error:
        # Do not turn I/O, unexpected errors or specialized ValueError
        # subclasses into a cached semantic refusal.
        if type(error) is not ValueError:
            raise
        reason = str(error)
        state["refusals"][name] = reason
        state["refusal_witnesses"][name] = reason
        raise


def _noncode_merges(state):
    """Bound code-independent merges; not data or initialization equivalence."""
    if not state["merges"]:
        return []
    image = state["native"].inputs.image
    names = {row[3].name.split("$", 1)[0].casefold() for row in _code_candidates(state)}
    if any(name.startswith("/") for name in names):
        raise ValueError("code merge policy has unresolved physical section names")
    result = []
    for source, target in state["merges"]:
        if "$" in source or "$" in target:
            raise ValueError("subsection-specific merge semantics remain unqualified")
        if source == target or source in names or target in names:
            raise ValueError("section merge can affect original physical code")
        destinations = [index for index, name in enumerate(image.section_names)
                        if name.casefold() == target]
        if len(destinations) != 1 or image.section_flags[destinations[0]] & 0x20000020:
            raise ValueError("section merge lacks one independently nonexecutable output destination")
        if any(flags & 0x20000020 for name, flags in zip(image.section_names, image.section_flags)
               if name.casefold() == source):
            raise ValueError("section merge has a contradictory executable output source")
        result.append({"source": source, "target": target, "target_image_section": destinations[0] + 1,
                       "physical_code_sections_affected": 0, "data_preservation": Result.UNKNOWN})
    return result


def _event_selected(state, label):
    """Bind all present original/final-pass owner spelling, never basename alone."""
    inputs = state["native"].inputs
    owner = _owner(label)
    selected = inputs.objects.resolve(owner)
    families = [row for row in inputs.coverage["inputs"] if _owner(row).casefold() == owner.casefold()]
    if len(families) != 1:
        raise ValueError("optimization event owner lacks one native final-pass family")
    family, = families
    for row in (label, family):
        if any(value is not None and any(char in value for char in "/\\:")
               for value in (row["library"], row["member"])):
            raise ValueError("qualified optimization/final-pass owner needs independent command-path binding")
    normalize = lambda value: None if value is None else value.casefold()
    if any(normalize(label[key]) != normalize(family[key]) for key in ("library", "member")):
        raise ValueError("optimization event owner contradicts exact final-pass family")
    candidates = _candidates(family, inputs.records)
    if (family["selection"] != Result.PASS or family["candidate_coverage"] != Result.PASS
            or type(family["candidate_count"]) is not int or family["candidate_count"] != 1
            or len(candidates) != 1):
        raise ValueError("optimization event owner has ambiguous physical input candidates")
    candidate = inputs.records[candidates[0]]
    if _identity(candidate) != selected["identity"] or candidate["sha256"] != selected["sha256"]:
        raise ValueError("optimization event differs from selected original physical input")
    return selected, _object(state, candidate)


def _fold_event_group(state, name):
    # Parsing the exact native transcript also retains positive REF conflicts.
    # Named K must occur exactly once globally, not just in a convenient group.
    _discard_events(state)
    matches = []
    for index, group in enumerate(state["discard_observation"]["icf_groups"]):
        for row in (group["selected"], *group["replaced"]):
            relevant = row["symbol"] == name
            if row["symbol"].startswith("."):
                _, obj = _event_selected(state, row["owner"])
                relevant = any(symbol.name == name and symbol.section > 0
                               and obj.sections[symbol.section - 1].name == row["symbol"]
                               for symbol in obj.symbols.values())
            if relevant:
                matches.append((index, group))
    if len(matches) != 1:
        raise ValueError("ANY key lacks one globally unique positive ICF member observation")
    return matches[0]


def _placed_storage(state, identity, stamp, owner, root, obj, address, mapping):
    inputs = state["native"].inputs
    inventory, module_index, spans = _metadata(state)
    size = obj.section_sizes[root.section - 1]
    begin, end = address, address + size
    containing = [flags for (span, _, _), flags in zip(inputs.image.sections, inputs.image.section_flags)
                  if span.start <= begin and end <= span.end]
    if len(containing) != 1 or not containing[0] & 0x20000000 or not containing[0] & 0x20:
        raise ValueError("native root placement is not wholly within executable PE code storage")
    modules = module_index.get((digest(identity), stamp), ())
    rows = [(index, inventory.contributions[index], start) for index, start, _ in spans.overlaps(begin, end)]
    labels = [row for row in (*mapping.publics, *(mapping.statics or ()))
              if row.symbol == root.name and row.owner.casefold() == owner.casefold()]
    if len(modules) != 1 or len(rows) != 1 or len(labels) != 1:
        raise ValueError("native root placement lacks one exact module/MAP/contribution")
    index, contribution, start = rows[0]
    flags = obj.sections[root.section - 1].flags
    if (start != begin or contribution.size != size or contribution.module != modules[0]
            or contribution.section != labels[0].segment or labels[0].address != begin
            or (contribution.flags ^ flags) & ~0x2000):
        raise ValueError("native root contribution contradicts the original physical section")
    for module in inventory.modules:
        first = module.first
        if first is None or not first.size:
            continue
        address = inputs.image.sections[first.section - 1][0].start + first.offset
        if address < end and begin < address + first.size and first != contribution:
            raise ValueError("native root placement has contradictory module-first contribution")
    return {"input": dict(identity), "object_sha256": stamp,
            "owner": owner, "root": asdict(root), "address": begin, "size": size,
            "module": contribution.module, "contribution": index, "object_flags": flags,
            "pdb_flags": contribution.flags,
            "characteristics_agreement": Result.PASS if contribution.flags == flags else Result.UNKNOWN}


def _unique_placement(state, identity_digest, stamp, number):
    """Prove one original NODUP root elsewhere; no recursive ANY inference."""
    token = digest([identity_digest, stamp, number])
    if token in state["unique_placements"]:
        return state["unique_placements"][token]
    if token in state["unique_refusals"]:
        raise ValueError(state["unique_refusals"][token])
    try:
        return _unique_placement_uncached(state, identity_digest, stamp, number, token)
    except ValueError as error:
        if type(error) is not ValueError:
            raise
        state["unique_refusals"][token] = str(error)
        state["unique_refusal_witnesses"][token] = str(error)
        raise


def _unique_placement_uncached(state, identity_digest, stamp, number, token):
    from .coff_private_function import private_comdat_root

    native = state["native"]
    inputs = native.inputs
    records = state["records"].get((identity_digest, stamp), ())
    if len(records) != 1:
        raise ValueError("unique placement lacks an exact physical candidate identity")
    record, = records
    obj = _object(state, record)
    private = private_comdat_root(obj, number)
    root = private if private is not None else comdat_key(obj, number)
    if (section_selection(obj, number) != 1 or root.type != 32
            or obj.function(root.name) != root or root.value != 0):
        raise ValueError("indistinguishable peer is not a unique independent NODUP code root")
    forest = _family(obj, number)
    sections = {row["object_section"] for row in forest}
    external = [symbol for symbol in obj.symbols.values() if symbol.section in sections and symbol.storage == 2]
    if external != ([] if private is not None else [root]):
        raise ValueError("unique NODUP root or descendants have extra external aliases")
    if private is not None:
        definitions = {min(symbol.index for symbol in obj.symbols.values() if symbol.section == number)
                       for number in sections}
        if any(symbol != root and symbol.section in sections and symbol.storage in (3, 6)
                and symbol.index not in definitions for symbol in obj.symbols.values()):
            raise ValueError("private NODUP family has additional static or local definitions")
    definitions = []
    for candidate, declarations, imports in (() if private is not None else state["interest"].get(root.name, ())):
        if imports:
            raise ValueError("unique NODUP key conflicts with an import declaration")
        for symbol in declarations:
            if (symbol["name"] == root.name and symbol["storage"] == 2 and symbol["section"] == 0
                    and symbol["value"] == 0 and symbol["type"] in (0, 32) and not symbol["auxiliary_size"]):
                continue
            if (candidate is not record or symbol["index"] != root.index or symbol["name"] != root.name
                    or symbol["storage"] != 2 or symbol["section"] != number or symbol["type"] != 32):
                raise ValueError("NODUP peer key is not globally unique with undef-only other declarations")
            definitions.append(symbol)
    if private is None and len(definitions) != 1:
        raise ValueError("NODUP peer lacks one complete original key definition")
    labels = [row for row in (*inputs.mapping.publics, *(inputs.mapping.statics or ())) if row.symbol == root.name]
    if private is not None:
        labels = _private_own_labels(state, record, labels)
    pool = inputs.mapping.publics if private is None else (inputs.mapping.statics or ())
    if len(labels) != 1 or labels[0] not in pool or not labels[0].function or not labels[0].segment:
        raise ValueError("unique NODUP peer lacks one exact " + ("public" if private is None else "private") + " MAP placement")
    label, = labels
    families = [row for row in inputs.coverage["inputs"] if _owner(row).casefold() == label.owner.casefold()]
    if len(families) != 1:
        raise ValueError("unique NODUP peer lacks one final physical input family")
    selected, original = _event_selected(state, families[0])
    if (_identity(record) != selected["identity"] or stamp != selected["sha256"] or original is not obj
            or selected["owner"].casefold() != label.owner.casefold()):
        raise ValueError("NODUP peer MAP owner differs from its original physical input")
    _discard_events(state)
    names = {symbol["name"] for part in forest for symbol in part["symbols"] if symbol["name"] != part["name"]}
    section_names = {part["name"] for part in forest}
    if private is not None:
        _private_icf_aliases(state, names)
    events = [(row, False) for row in state["discard_observation"]["rows"]]
    events += [(row, True) for group in state["discard_observation"]["icf_groups"]
               for row in (group["selected"], *group["replaced"])]
    for event, icf in events:
        relevant = event["symbol"] in names or event["symbol"] in section_names
        if private is not None and relevant and not icf:
            _private_ref_conflict(state, selected, event)
        elif private is None and (event["symbol"] in names or event["symbol"] in section_names
                and (event["owner"] is None
                     or _owner(event["owner"]).casefold() == selected["owner"].casefold())):
            raise ValueError("unique NODUP peer has contradictory or unsupported REF/ICF observations")
    proof = verify_code_region(obj, root, selected["owner"], inputs.mapping, inputs.image,
        objects=inputs.objects, fixups=inputs.fixups, _index=state["map_index"],
        _constants=state["constants"], codeview=state["codeview"])
    if proof["result"] != Result.PASS or proof["address"] != label.address:
        raise ValueError("unique NODUP peer complete code/references/metadata did not verify")
    placement = _placed_storage(state, selected["identity"], stamp, selected["owner"], root,
                                obj, label.address, inputs.mapping)
    inventory = _metadata(state)[0]
    result = {"check": "native_unique_nodup_placement", "result": Result.PASS,
              "root_identity_kind": "private" if private is not None else "external",
              **placement, "sections": forest, "proof": proof,
              "image_sha256": inventory.image_sha256, "pdb_sha256": inventory.pdb_sha256,
              "log_sha256": state["discard_observation"]["sha256"],
              "remaining_linked_effects": Result.UNKNOWN, "allocation_isolation": Result.UNKNOWN,
              "outside_scope_verification": Result.UNKNOWN, "source_authority": "not_transferred",
              "ready_for_collection": False, "persisted": False}
    result["placement_id"] = digest(result)
    state["unique_placements"][token] = result
    state["unique_placement_witnesses"][token] = deepcopy(result)
    return result


def _private_own_labels(state, record, labels):
    """Same private spelling in another physical object is not this root."""
    owned = []
    for label in labels:
        families = [row for row in state["native"].inputs.coverage["inputs"]
                    if _owner(row).casefold() == label.owner.casefold()]
        if len(families) != 1:
            raise ValueError("private MAP label lacks one physical final-pass family")
        selected, _ = _event_selected(state, families[0])
        if selected["identity"] == _identity(record) and selected["sha256"] == record["sha256"]:
            owned.append(label)
    return owned


def _private_ref_conflict(state, selected, event):
    """REF can remove another private identity; it cannot rewrite a MAP owner."""
    from .coff_private_function import private_comdat_root

    if event["owner"] is None:
        raise ValueError("private NODUP peer has ownerless REF evidence")
    other, obj = _event_selected(state, event["owner"])
    if other["identity"] == selected["identity"]:
        raise ValueError("private NODUP peer has contradictory own REF evidence")
    if event["symbol"].startswith("."):
        sections = {number for number, section in enumerate(obj.sections, 1) if section.name == event["symbol"]}
        if not sections:
            raise ValueError("other-owner section event lacks an exact original section")
    else:
        root = obj.function(event["symbol"])
        if private_comdat_root(obj, root.section) != root:
            raise ValueError("same-name event is not a distinct physical private root")


def _private_icf_aliases(state, names):
    """Printed ICF keys need not name every label that the linker rewrites.

    Inspect exact original event forests, including local labels/descendants.
    Different original owners cannot disambiguate final MAP alias provenance.
    """
    for group in state["discard_observation"]["icf_groups"]:
        for event in (group["selected"], *group["replaced"]):
            _, obj = _event_selected(state, event["owner"])
            if not any(symbol.name in names for symbol in obj.symbols.values()):
                continue
            if event["symbol"].startswith("."):
                roots = {number for number, section in enumerate(obj.sections, 1)
                         if section.name == event["symbol"]}
                if not roots:
                    raise ValueError("private ICF alias event lacks its exact original section")
            else:
                roots = {obj.function(event["symbol"]).section}
            for number in roots:
                while number in obj.associations:
                    number = obj.associations[number]
                forest = _family(obj, number)
                if any(symbol["name"] in names for part in forest for symbol in part["symbols"]):
                    raise ValueError("private original-family ICF provenance remains unsupported")


def _winner_ref_conflicts(state, winner, obj, root):
    """A positive own-root discard cannot coexist with a retained ANY proof."""
    _discard_events(state)
    names = {symbol["name"] for part in winner["sections"] for symbol in part["symbols"]
             if symbol["name"] != part["name"]}
    section_names = {part["name"] for part in winner["sections"]}
    selected = state["native"].inputs.objects.resolve(winner["owner"])
    for row in state["discard_observation"]["rows"]:
        if row["symbol"] not in names | section_names:
            continue
        if row["owner"] is None or _owner(row["owner"]).casefold() == selected["owner"].casefold():
            raise ValueError("ANY winner has contradictory positive native REF evidence")
        if row["symbol"] == root.name:
            # A different original same-key ANY copy may legitimately have its
            # own discard event. Authenticate that identity, not just its name.
            other, other_obj = _event_selected(state, row["owner"])
            symbol = other_obj.function(root.name)
            if (other["identity"] == selected["identity"] or comdat_key(other_obj, symbol.section) != symbol
                    or section_selection(other_obj, symbol.section) != 2):
                raise ValueError("ANY winner discard label lacks a distinct original duplicate")


def _folded_winner(state, name, candidates, label):
    from .linked_fold_frames import original_owner_view, verify_fold_frames
    from .linked_folding import Folding

    native = state["native"]
    inputs = native.inputs
    folding = inputs.folding
    if (type(folding) is not Folding or folding._objects is not inputs.objects
            or folding._mapping is not inputs.mapping or folding._image is not inputs.image
            or folding._fixups is not inputs.fixups):
        raise ValueError("ANY ICF composition requires exact live native folding inputs")
    words = state["command_words"]
    if (not any(word in ("/verbose", "-verbose") for word in words)
            or not any(word in ("/opt:icf", "-opt:icf") for word in words)
            or any(word.lower() in ("/opt:noicf", "-opt:noicf") for word in state["all_words"])):
        raise ValueError("ANY ICF composition requires explicit supported native ICF/VERBOSE policy")
    event_index, event = _fold_event_group(state, name)
    if event_index in state["fold_index"]:
        result = state["fold_groups"][state["fold_index"][event_index]]
    else:
        group = folding.for_map(label.owner, name)
        old_group = deepcopy(group)
        rows = (event["selected"], *event["replaced"])
        if len(rows) != len(group.members):
            raise ValueError("ICF group differs from complete original event multiplicity")
        originals = []
        for row, member in zip(rows, group.members, strict=True):
            selected, obj = _event_selected(state, row["owner"])
            if (dict(member.input) != selected["identity"] or member.object_sha256 != selected["sha256"]
                    or member.raw != selected["raw"] or member.owner != selected["owner"]
                    or row["symbol"] != member.reported_symbol
                    or obj.symbols.get(member.root.index) != member.root):
                raise ValueError("ICF event member differs from exact original physical producer")
            root = comdat_key(obj, member.root.section)
            if root != member.root or section_selection(obj, root.section) not in (1, 2):
                raise ValueError("ICF peer lacks a supported exact independent COMDAT key")
            peer_group, _ = _fold_event_group(state, root.name)
            if peer_group != event_index:
                raise ValueError("ICF original member overlaps another reported fold group")
            forest = _family(obj, root.section)
            numbers = {part["object_section"] for part in forest}
            if [item for item in obj.symbols.values() if item.section in numbers and item.storage == 2] != [root]:
                raise ValueError("ICF peer or descendants have additional external definitions")
            names = {symbol["name"] for part in forest for symbol in part["symbols"]
                     if symbol["name"] != part["name"]}
            sections = {part["name"] for part in forest}
            for discarded in state["discard_observation"]["rows"]:
                if (discarded["symbol"] in names
                        or discarded["symbol"] in sections and discarded["owner"] is not None
                        and _owner(discarded["owner"]).casefold() == member.owner.casefold()):
                    raise ValueError("ICF retained member has contradictory positive REF evidence")
            for record, declarations, imports in state["interest"].get(root.name, ()):
                if imports:
                    raise ValueError("ICF peer key conflicts with an import declaration")
                for declaration in declarations:
                    if (declaration["name"] == root.name and declaration["storage"] == 2
                            and declaration["section"] == 0 and declaration["value"] == 0
                            and declaration["type"] in (0, 32) and not declaration["auxiliary_size"]):
                        continue
                    peer = _object(state, record)
                    defined = comdat_key(peer, declaration["section"])
                    if (defined.name != root.name or defined.index != declaration["index"]
                            or defined.type != 32 or section_selection(peer, defined.section) != section_selection(obj, root.section)
                            or section_selection(obj, root.section) == 1
                            and (_identity(record) != selected["identity"] or defined.index != root.index)):
                        raise ValueError("ICF peer has conflicting strong/weak/common/key declarations")
            originals.append((obj, root, forest))
        frames = verify_fold_frames(group, inputs.image, inputs.fixups)
        view = original_owner_view(inputs.mapping, group)
        old_frames, old_view = deepcopy(frames), deepcopy(view)
        index = index_for(view)
        old_index = deepcopy(index.__dict__)
        proofs = []
        regions_seen = IntervalIndex(max_intervals=1000000)
        overlap_work = 0
        for member, (obj, root, forest) in zip(group.members, originals, strict=True):
            proof = verify_code_region(obj, root, member.owner, view, inputs.image,
                objects=inputs.objects, fixups=inputs.fixups, folded_frames=frames,
                _index=index, _constants=state["constants"], codeview=state["codeview"])
            if proof["result"] != Result.PASS or proof["address"] != group.address:
                raise ValueError("ICF original peer code/references/companions did not verify")
            regions = proof.get("regions", [dict(proof, ordinal=0)])
            if not regions:
                raise ValueError("ICF member proof has no complete runtime regions")
            for region in regions:
                begin, end = region["address"], region["address"] + region["size"]
                overlaps = regions_seen.overlaps(begin, end)
                overlap_work += len(overlaps)
                if overlap_work > min(MAX_FOLD_OVERLAPS, len(group.members) * 1024):
                    raise ValueError("ICF member overlap work exceeds its bounded native budget")
                for previous, left, right in overlaps:
                    prior = proof if previous == len(proofs) else proofs[previous]["proof"]
                    if not (region["ordinal"] == 0 and len(regions) == 1
                            and previous != len(proofs)
                            and len(prior.get("regions", [None])) == 1 and (begin, end) == (left, right)):
                        raise ValueError("ICF members have unequal or shared companion runtime extents")
                regions_seen.insert(len(proofs), begin, end)
            proofs.append({"input": dict(member.input), "object_sha256": member.object_sha256,
                           "owner": member.owner, "root": asdict(root), "sections": forest, "proof": proof})
        selected_member = group.selected
        storage = _placed_storage(state, dict(selected_member.input), selected_member.object_sha256,
            selected_member.owner, selected_member.root, originals[0][0], selected_member.address, inputs.mapping)
        if (not same(group, old_group) or not same(frames, old_frames)
                or not same(view, old_view) or not same(index.__dict__, old_index)):
            raise ValueError("ICF derived group proof changed during native composition")
        result = {"check": "native_comdat_folded_members", "result": Result.PASS,
                  "event": deepcopy(event), "members": proofs, "final_storage": storage,
                  "frames": frames.canonical(), "image_sha256": group.image_sha256,
                  "pdb_sha256": inputs.fixups.pdb_sha256, "log_sha256": group.log_sha256,
                  "remaining_linked_effects": Result.UNKNOWN, "outside_scope_verification": Result.UNKNOWN,
                  "source_authority": "not_transferred", "ready_for_collection": False, "persisted": False}
        result["group_id"] = digest(result)
        state["fold_groups"][result["group_id"]] = result
        state["fold_group_witnesses"][result["group_id"]] = deepcopy(result)
        state["fold_index"][event_index] = result["group_id"]
        state["fold_index_witnesses"][event_index] = result["group_id"]
    original = [row for row in result["members"] if row["root"]["name"] == name]
    if len(original) != 1:
        raise ValueError("ICF group does not retain one original ANY key producer")
    original, = original
    matches = [row for row in candidates if row["input"] == original["input"]
               and row["object_sha256"] == original["object_sha256"] and row["root"] == original["root"]]
    if len(matches) != 1 or result["final_storage"]["address"] != label.address:
        raise ValueError("ICF original ANY winner differs from complete candidate family")
    return matches[0], {"original_owner": original["owner"], "group_id": result["group_id"],
                        "storage": result["final_storage"], "image_sha256": result["image_sha256"],
                        "pdb_sha256": result["pdb_sha256"]}


def _certificate_uncached(state, name):
    native = state["native"]
    inputs = native.inputs
    if state["policy_gaps"] or not state["nonincremental"]:
        raise ValueError("ANY selection has unsupported linker/directive selection policy")
    merges = _noncode_merges(state)
    candidates = []
    for record, symbols, imports in state["interest"].get(name, ()):
        if imports:
            raise ValueError("ANY key conflicts with an import-library declaration")
        definitions = []
        for symbol in symbols:
            if (symbol["storage"] == 2 and symbol["section"] == 0 and symbol["value"] == 0
                    and symbol["type"] in (0, 0x20) and not symbol["auxiliary_size"]):
                continue  # Ordinary unresolved references do not choose a winner.
            if (symbol["name"] != name or symbol["storage"] != 2 or symbol["section"] <= 0
                    or symbol["value"] or symbol["type"] != 0x20):
                raise ValueError("ANY key has a weak, common, absolute or unsupported conflicting declaration")
            definitions.append(symbol)
        if not definitions:
            continue
        if len(definitions) != 1:
            raise ValueError("ANY key has duplicate physical definitions in one input")
        symbol, = definitions
        obj = _object(state, record)
        root = comdat_key(obj, symbol["section"])
        if (root.index != symbol["index"] or root.name != name
                or section_selection(obj, root.section) != 2
                or not obj.sections[root.section - 1].flags & 0x20):
            raise ValueError("same-key candidate is not an independent code COMDAT ANY key")
        forest = _family(obj, root.section)
        numbers = {row["object_section"] for row in forest}
        external = [item for item in obj.symbols.values() if item.section in numbers and item.storage == 2]
        if external != [root]:
            raise ValueError("ANY root or associated descendants have additional external definitions")
        candidates.append({"input": _identity(record), "object_sha256": record["sha256"],
                           "root": asdict(root), "sections": forest})
    if not candidates:
        raise ValueError("ANY key has no complete physical definition family")
    labels = [row for row in (*inputs.mapping.publics, *(inputs.mapping.statics or ())) if row.symbol == name]
    if len(labels) != 1 or labels[0] not in inputs.mapping.publics or not labels[0].function or not labels[0].segment:
        raise ValueError("ANY winner lacks one independent public MAP placement")
    label, = labels
    folded = _folded_winner(state, name, candidates, label) if name in state["folding_names"] else None
    if folded is not None:
        winner, fold = folded
        result = {"check": "native_comdat_any_selection", "result": Result.PASS,
                  "key": name, "policy": "IMAGE_COMDAT_SELECT_ANY", "candidates": candidates,
                  "winner": {**deepcopy(winner), "owner": fold["original_owner"], "address": label.address},
                  "fold_group_id": fold["group_id"], "final_storage": deepcopy(fold["storage"]),
                  "rewritten_key": asdict(label), "noncode_merges": merges,
                  "image_sha256": fold["image_sha256"], "pdb_sha256": fold["pdb_sha256"],
                  "remaining_linked_effects": Result.UNKNOWN, "outside_scope_verification": Result.UNKNOWN,
                  "source_authority": "not_transferred", "ready_for_collection": False, "persisted": False}
        result["certificate_id"] = digest(result)
        state["certificates"][name] = result
        state["certificate_witnesses"][name] = deepcopy(result)
        return result
    selected = inputs.objects.resolve(label.owner)
    winners = [row for row in candidates if row["input"] == selected["identity"]
               and row["object_sha256"] == selected["sha256"]]
    if len(winners) != 1:
        raise ValueError("ANY MAP winner differs from the complete physical candidate family")
    winner = winners[0]
    obj = state["objects"][(digest(winner["input"]), winner["object_sha256"])]
    root = obj.symbols[winner["root"]["index"]]
    # Absence from reported ICF groups is not a no-fold proof. Every other
    # possible physical code section needs an immutable mismatch or its own
    # independently verified unique NODUP root placement at a disjoint range.
    no_fold = _no_other_fold(state, obj, root, candidates, label.address)
    _winner_ref_conflicts(state, {**winner, "owner": selected["owner"]}, obj, root)
    proof = verify_code_region(obj, root, label.owner, inputs.mapping, inputs.image,
                               objects=inputs.objects, fixups=inputs.fixups,
                               _index=state["map_index"], _constants=state["constants"],
                               codeview=state["codeview"])
    if proof["result"] != Result.PASS or proof["address"] != label.address:
        raise ValueError("ANY winner code, references or associated metadata did not verify")
    # Exact root region, not a guessed padded symbol extent or an image search.
    size = obj.section_sizes[root.section - 1]
    begin, end = label.address, label.address + size
    containing = [flags for (span, _, _), flags in zip(inputs.image.sections, inputs.image.section_flags)
                  if span.start <= begin and end <= span.end]
    if len(containing) != 1 or not containing[0] & 0x20000000 or not containing[0] & 0x20:
        raise ValueError("ANY code winner is not wholly within executable PE storage")
    inventory, module_index, spans = _metadata(state)
    modules = module_index.get((digest(winner["input"]), winner["object_sha256"]), ())
    if len(modules) != 1:
        raise ValueError("ANY winner lacks a unique physical PDB module")
    rows = [(index, inventory.contributions[index], start) for index, start, stop in spans.overlaps(begin, end)]
    if len(rows) != 1:
        raise ValueError("ANY winner has missing, duplicate or competing PDB contributions")
    index, contribution, start = rows[0]
    flags = obj.sections[root.section - 1].flags
    if (start != begin or contribution.size != size or contribution.module != modules[0]
            or contribution.section != label.segment):
        raise ValueError("ANY winner PDB contribution contradicts its physical section")
    # Module/address/extent correspondence is separate from characteristics
    # equivalence. Preserve reserved 0x2000 verbatim and leave its interpretation
    # unknown; it appears on ordinary as well as COMDAT PDB contributions.
    # No other difference is supported here. This cannot waive allocation flags.
    if (contribution.flags ^ flags) & ~0x2000:
        raise ValueError("ANY winner PDB contribution has contradictory characteristics")
    for module in inventory.modules:
        first = module.first
        if first is None or not first.size:
            continue
        start = inputs.image.sections[first.section - 1][0].start + first.offset
        if start < end and begin < start + first.size and first != contribution:
            raise ValueError("ANY winner has contradictory module-first contribution evidence")
    result = {"check": "native_comdat_any_selection", "result": Result.PASS,
              "key": name, "policy": "IMAGE_COMDAT_SELECT_ANY", "candidates": candidates,
              "winner": {**deepcopy(winner), "owner": selected["owner"], "address": begin,
                         "module": contribution.module, "contribution": index,
                         "object_flags": flags, "pdb_flags": contribution.flags,
                         "characteristics_agreement": Result.PASS if flags == contribution.flags else Result.UNKNOWN,
                         "body_fingerprint": proof["body_fingerprint"]},
              "no_other_fold": no_fold,
              "noncode_merges": merges,
              "image_sha256": inventory.image_sha256,
              "pdb_sha256": inventory.pdb_sha256,
              "remaining_linked_effects": Result.UNKNOWN, "outside_scope_verification": Result.UNKNOWN,
              "source_authority": "not_transferred", "ready_for_collection": False, "persisted": False}
    result["certificate_id"] = digest(result)
    state["certificates"][name] = result
    state["certificate_witnesses"][name] = deepcopy(result)
    return result


def _query(state, owner, number):
    selected = state["native"].inputs.objects.resolve(owner)
    if type(number) is not int:
        raise ValueError("COMDAT query requires an exact original section number")
    records = state["records"].get((digest(selected["identity"]), selected["sha256"]), ())
    if len(records) != 1:
        raise ValueError("COMDAT query differs from its unique physical input inventory")
    obj = _object(state, records[0])
    if not 1 <= number <= len(obj.sections):
        raise ValueError("COMDAT query section is outside its selected physical input")
    root_number = number
    while root_number in obj.associations:
        root_number = obj.associations[root_number]
    return selected, obj, root_number


def _loser(state, owner, number):
    selected, obj, root_number = _query(state, owner, number)
    if not obj.sections[root_number - 1].flags & 0x1000 or section_selection(obj, root_number) != 2:
        return None
    root = comdat_key(obj, root_number)
    certificate = _certificate(state, root.name)
    candidates = [row for row in certificate["candidates"] if row["input"] == selected["identity"]
                  and row["object_sha256"] == selected["sha256"] and row["root"]["section"] == root_number]
    if len(candidates) != 1:
        raise ValueError("ANY query is absent from its complete certified family")
    if certificate["winner"]["input"] == selected["identity"]:
        return None
    names = {row["name"] for section in candidates[0]["sections"] for row in section["symbols"]
             if row["name"] != section["name"]}
    labels = (*state["native"].inputs.mapping.publics, *(state["native"].inputs.mapping.statics or ()))
    if any(row.owner.casefold() == selected["owner"].casefold() and row.symbol in names
           and not (certificate.get("fold_group_id") and row in state["native"].inputs.mapping.publics
                    and asdict(row) == certificate["rewritten_key"])
           for row in labels):
        raise ValueError("ANY loser has contradictory independently placed original labels")
    return {"certificate_id": certificate["certificate_id"], "key": root.name,
            "queried_section": number, "sections": [row["object_section"] for row in candidates[0]["sections"]],
            "disposition": "discarded_any_duplicate"}


def _placed(state, owner, number):
    selected, obj, root_number = _query(state, owner, number)
    if not obj.sections[root_number - 1].flags & 0x1000:
        return None
    policy = section_selection(obj, root_number)
    if policy == 2:
        # The certified ANY winner is positively selected; a loser copy is a
        # removal question, never a placement.
        root = comdat_key(obj, root_number)
        certificate = _certificate(state, root.name)
        if certificate["winner"]["input"] != selected["identity"]:
            return None
        winner, = [row for row in certificate["candidates"]
                   if row["input"] == selected["identity"]
                   and row["object_sha256"] == selected["sha256"]
                   and row["root"]["section"] == root_number]
        return {"certificate_id": certificate["certificate_id"], "key": root.name,
                "queried_section": number,
                "sections": [row["object_section"] for row in winner["sections"]],
                "root_identity_kind": "external", "disposition": "selected_any_winner"}
    if policy != 1:
        return None
    placement = _unique_placement(state, digest(selected["identity"]), selected["sha256"], root_number)
    return {"placement_id": placement["placement_id"], "key": placement["root"]["name"],
            "queried_section": number,
            "sections": [row["object_section"] for row in placement["sections"]],
            "root_identity_kind": placement["root_identity_kind"],
            "disposition": "independently_placed_nodup_root"}


def _folded_member(state, owner, number):
    selected, obj, root_number = _query(state, owner, number)
    groups = []
    for group in state["fold_groups"].values():
        if any(member["input"] == selected["identity"]
               and member["object_sha256"] == selected["sha256"]
               and member["root"]["section"] == root_number
               for member in group["members"]):
            groups.append(group)
    if not groups:
        return None
    if len({group["group_id"] for group in groups}) != 1:
        raise ValueError("folded member belongs to ambiguous certified groups")
    group, = groups
    return {"group_id": group["group_id"], "key": group["members"][0]["root"]["name"],
            "final_storage": deepcopy(group["final_storage"]),
            "disposition": "certified_icf_coalescence"}


def _discard_events(state):
    if state["discard_failure"] is not None:
        raise ValueError(state["discard_failure"])
    if state["discard_observation"] is not None:
        return state["discard_events"]
    from .msvc_discard import parse_discard_log
    try:
        observation = parse_discard_log(state["native"].inputs.artifacts["msvc-link-log"])
        events = {}
        named = {}
        for row in observation["rows"]:
            named.setdefault(row["symbol"], []).append(row)
            if row["owner"] is None:
                continue
            try:
                owner = _owner(row["owner"]).casefold()
            except ValueError:
                # An unsupported final-pass owner cannot become a query owner;
                # keep the entire event in the diagnostic rather than erase it.
                continue
            events.setdefault((owner, row["symbol"]), []).append(row)
        state["discard_observation"] = observation
        state["discard_events"] = events
        state["discard_named_events"] = named
        state["discard_witness"] = deepcopy((observation, events, named))
        return events
    except ValueError as error:
        # A consumer may preserve the query as UNKNOWN, but a malformed whole
        # transcript must not let a different partial certificate escape exit.
        state["discard_failure"] = str(error)
        state["discard_failure_witness"] = str(error)
        raise


def _discarded_candidate(state, identity_digest, stamp, number):
    """Positive REF removes only this exact physical root/association member.

    This is a native query, independent of earlier census traversal order.
    No event means no conclusion: a separate-placement proof may still apply.
    A present unsupported/contradictory event is never treated as absence.
    """
    records = state["records"].get((identity_digest, stamp), ())
    if len(records) != 1:
        raise ValueError("REF competitor lacks an exact physical candidate identity")
    record, = records
    obj = _object(state, record)
    if type(number) is not int or not 1 <= number <= len(obj.sections):
        raise ValueError("REF competitor section is outside its original input")
    root_number = number
    while root_number in obj.associations:
        root_number = obj.associations[root_number]
    if not obj.sections[root_number - 1].flags & 0x1000 or section_selection(obj, root_number) != 1:
        return None
    from .coff_private_function import private_comdat_root
    if private_comdat_root(obj, root_number) is not None:
        return None  # No private REF authority; require independent placement.
    root = comdat_key(obj, root_number)
    _discard_events(state)
    events = state["discard_named_events"].get(root.name, ())
    if not events:
        return None
    if len(events) != 1 or events[0]["owner"] is None:
        raise ValueError("REF competitor lacks one owner-qualified positive event")
    event, = events
    selected, selected_obj = _event_selected(state, event["owner"])
    if (selected["identity"] != _identity(record) or selected["sha256"] != stamp
            or digest(selected["identity"]) != identity_digest or selected_obj is not obj):
        raise ValueError("REF competitor event names a different physical input")
    removal = _discarded(state, selected["owner"], number)
    if removal is None or removal["queried_section"] != number or number not in removal["sections"]:
        raise ValueError("REF competitor is not in its exact certified original forest")
    return removal


def _ref_certificate(state, selected, obj, root, event):
    token = digest(selected["identity"]), selected["sha256"], root.index
    if token in state["ref_certificates"]:
        return state["ref_certificates"][token]
    if token in state["ref_refusals"]:
        raise ValueError(state["ref_refusals"][token])
    try:
        return _ref_certificate_uncached(state, selected, obj, root, event, token)
    except ValueError as error:
        if type(error) is not ValueError:
            raise
        state["ref_refusals"][token] = str(error)
        state["ref_refusal_witnesses"][token] = str(error)
        raise


def _ref_certificate_uncached(state, selected, obj, root, event, token):
    native = state["native"]
    inputs = native.inputs
    commands = state["command_words"]
    words = state["all_words"]
    if (state["policy_gaps"] or not state["nonincremental"]
            or not any(word in ("/opt:ref", "-opt:ref") for word in commands)
            or not any(word in ("/verbose", "-verbose") for word in commands)
            or any(word.lower() in ("/opt:noref", "-opt:noref") for word in words)):
        raise ValueError("REF selection requires supported explicit native REF, nonincremental and VERBOSE options")
    merges = _noncode_merges(state)
    # The reported owner is not resolved by basename alone. Its full spelling
    # must agree with the one native final-pass family that independently binds
    # this physical selected producer. Different archive occurrences refuse.
    families = [row for row in inputs.coverage["inputs"] if _owner(row).casefold() == selected["owner"].casefold()]
    if len(families) != 1:
        raise ValueError("REF event owner lacks one native final-pass candidate family")
    family, = families
    for label in (event["owner"], family):
        if any(value is not None and any(char in value for char in "/\\:")
               for value in (label["library"], label["member"])):
            raise ValueError("qualified REF/final-pass owner needs independent command-path binding")
    normalize = lambda value: None if value is None else value.replace("\\", "/").casefold()
    if any(normalize(event["owner"][key]) != normalize(family[key]) for key in ("library", "member")):
        raise ValueError("REF reported owner spelling contradicts its exact final-pass family")
    candidates = _candidates(family, inputs.records)
    if (family["selection"] != Result.PASS or family["candidate_coverage"] != Result.PASS
            or type(family["candidate_count"]) is not int or family["candidate_count"] != 1
            or len(candidates) != 1):
        raise ValueError("REF event owner has ambiguous selected physical candidates")
    candidate = inputs.records[candidates[0]]
    if _identity(candidate) != selected["identity"] or candidate["sha256"] != selected["sha256"]:
        raise ValueError("REF event candidate differs from its queried original input")
    if (root.type != 0x20 or obj.function(root.name) != root
            or not obj.sections[root.section - 1].flags & 0x20
            or section_selection(obj, root.section) != 1):
        raise ValueError("REF event does not name a supported independent NODUP code root")
    forest = _family(obj, root.section)
    numbers = {row["object_section"] for row in forest}
    if [item for item in obj.symbols.values() if item.section in numbers and item.storage == 2] != [root]:
        raise ValueError("REF root or descendants have additional external definitions")
    # All possible linker declarations remain in scope. An ordinary undefined
    # use remains a separate reference obligation, not a second definition.
    for record, symbols, imports in state["interest"].get(root.name, ()):
        if imports:
            raise ValueError("REF key conflicts with an import declaration")
        for symbol in symbols:
            if (_identity(record) == selected["identity"] and record["sha256"] == selected["sha256"]
                    and symbol["index"] == root.index and symbol["name"] == root.name):
                continue
            if (symbol["name"] == root.name and symbol["storage"] == 2 and symbol["section"] == 0
                    and symbol["value"] == 0 and symbol["type"] in (0, 0x20) and not symbol["auxiliary_size"]):
                continue
            raise ValueError("REF key has a conflicting strong, weak, common or unsupported declaration")
    names = {row["name"] for section in forest for row in section["symbols"] if row["name"] != section["name"]}
    if any(word.lower().startswith(("/include:", "-include:", "/entry:", "-entry:", "/export:", "-export:"))
           and word.split(":", 1)[1].split(",", 1)[0] in names for word in words):
        raise ValueError("REF root has a contradictory forced-retention directive")
    labels = (*inputs.mapping.publics, *(inputs.mapping.statics or ()))
    if any(row.symbol == root.name or row.owner.casefold() == selected["owner"].casefold() and row.symbol in names
           for row in labels):
        raise ValueError("REF root or associated labels have contradictory native MAP placements")
    if names & state["folding_names"]:
        raise ValueError("REF root or descendants have contradictory ICF event identities")
    section_names = {section["name"] for section in forest}
    for group in state["discard_observation"]["icf_groups"]:
        for row in (group["selected"], *group["replaced"]):
            # Native folding capabilities may be absent without PDB fixups.
            # Positive log contradictions still count, including section-only
            # labels in this owner. Basename comparison here only REFUSES;
            # it never authenticates an event or transfers source authority.
            if row["symbol"] in names or (row["symbol"] in section_names
                    and _owner(row["owner"]).casefold() == selected["owner"].casefold()):
                raise ValueError("REF family has a contradictory reported ICF event")
    result = {"check": "native_comdat_ref_selection", "result": Result.PASS,
              "key": root.name, "policy": "IMAGE_COMDAT_SELECT_NODUPLICATES_OPT_REF",
              "original": {"input": deepcopy(selected["identity"]), "object_sha256": selected["sha256"],
                           "owner": selected["owner"], "root": asdict(root), "sections": forest},
              "event": deepcopy(event), "log_sha256": state["discard_observation"]["sha256"],
              "noncode_merges": merges, "source_authority": "not_transferred",
              "incoming_references": Result.UNKNOWN, "remaining_linked_effects": Result.UNKNOWN,
              "outside_scope_verification": Result.UNKNOWN, "ready_for_collection": False, "persisted": False}
    result["certificate_id"] = digest(result)
    state["ref_certificates"][token] = result
    state["ref_certificate_witnesses"][token] = deepcopy(result)
    return result


def _discarded(state, owner, number):
    selected, obj, root_number = _query(state, owner, number)
    if not obj.sections[root_number - 1].flags & 0x1000 or section_selection(obj, root_number) != 1:
        return None
    root = comdat_key(obj, root_number)
    events = _discard_events(state).get((selected["owner"].casefold(), root.name), ())
    if not events:
        return None
    if len(events) != 1:
        raise ValueError("REF root has repeated or ambiguous positive discard events")
    certificate = _ref_certificate(state, selected, obj, root, events[0])
    return {"certificate_id": certificate["certificate_id"], "key": root.name,
            "queried_section": number, "sections": [row["object_section"] for row in certificate["original"]["sections"]],
            "disposition": "discarded_unreferenced_comdat"}


@contextmanager
def comdat_selection(native, *, epoch=None):
    """Issue per-key queries only inside the original native input lifetime."""
    if type(native) is not LinkedBuild:
        raise ValueError("COMDAT selection requires its original live native linked build")
    if epoch is None:
        with input_epoch(native.context, native.inputs.objects) as owned:
            with comdat_selection(native, epoch=owned) as handle:
                yield handle
        return
    native.check_current()
    try:
        with _borrow_epoch(epoch, native.context, native.inputs.objects) as observed:
            inputs = native.inputs
            records, interest = {}, {}
            for record in observed.records:
                if record["result"] != Result.PASS:
                    raise ValueError("COMDAT selection requires complete physical candidate declarations")
                token = digest(_identity(record)), record["sha256"]
                records.setdefault(token, []).append(record)
                names = {}
                for symbol in record["symbols"]:
                    if symbol["storage"] in (2, 105):
                        names.setdefault(symbol["name"], []).append(symbol)
                        if symbol.get("fallback") != symbol["name"] and symbol.get("fallback"):
                            names.setdefault(symbol["fallback"], []).append(symbol)
                imports = {name for item in record["imports"] for name in (item["symbol"], "__imp_" + item["symbol"])}
                for name in names.keys() | imports:
                    interest.setdefault(name, []).append((record, names.get(name, ()), name in imports))
            words = [word.lower() for word in inputs.evidence["arguments"]]
            words += [word.lower() for row in observed.records for item in row["directives"] for word in _tokens(item["text"])]
            with constant_lookups(inputs.objects) as constants:
                handle = object.__new__(ComdatSelection)
                state = {"native": native, "epoch": epoch, "inputs": observed.inputs,
                    "interest": interest, "records": records, "objects": {}, "certificates": {},
                    "object_witnesses": {}, "certificate_witnesses": {},
                    "fold_groups": {}, "fold_group_witnesses": {},
                    "fold_index": {}, "fold_index_witnesses": {},
                    "unique_placements": {}, "unique_placement_witnesses": {},
                    "unique_refusals": {}, "unique_refusal_witnesses": {},
                    "refusals": {}, "refusal_witnesses": {},
                    "ref_certificates": {}, "ref_certificate_witnesses": {},
                    "ref_refusals": {}, "ref_refusal_witnesses": {},
                    "discard_observation": None, "discard_events": None, "discard_named_events": None,
                    "discard_witness": None,
                    "discard_failure": None, "discard_failure_witness": None,
                    "metadata": None, "metadata_witness": None,
                    "code_candidates": None, "code_candidates_witness": None,
                    "policy_gaps": _policy(inputs.evidence, observed.records),
                    "merges": tuple(tuple(word.split(":", 1)[1].split("=")) for word in words
                                    if word.startswith(("/merge:", "-merge:"))),
                    "nonincremental": any(word.lower() in ("/incremental:no", "-incremental:no")
                                          for word in inputs.evidence["arguments"]),
                    "command_words": tuple(word.lower() for word in inputs.evidence["arguments"]),
                    "all_words": tuple(inputs.evidence["arguments"]) + tuple(word for row in observed.records
                        for item in row["directives"] for word in _tokens(item["text"])),
                    "folding_names": frozenset(name for _, name in inputs.folding.candidate_keys())
                        if inputs.folding is not None else frozenset(),
                    "map_index": index_for(inputs.mapping), "constants": constants, "codeview": LinkedProcedures(native)}
                _live[handle] = state
                fixed = ("native", "epoch", "inputs", "map_index", "constants", "codeview")
                pointers = {key: state[key] for key in fixed}
                indexed = ("interest", "records", "policy_gaps", "merges", "nonincremental", "folding_names",
                           "command_words", "all_words")
                before = deepcopy({key: state[key] for key in indexed})
                map_before = deepcopy(state["map_index"].__dict__)
                try:
                    yield handle
                finally:
                    del _live[handle]
                    if (any(state[key] is not pointer for key, pointer in pointers.items())
                            or not same({key: state[key] for key in indexed}, before)
                            or not same(state["map_index"].__dict__, map_before)
                            or not same(state["certificates"], state["certificate_witnesses"])
                            or not same(state["fold_groups"], state["fold_group_witnesses"])
                            or not same(state["fold_index"], state["fold_index_witnesses"])
                            or not same(state["unique_placements"], state["unique_placement_witnesses"])
                            or not same(state["unique_refusals"], state["unique_refusal_witnesses"])
                            or not same(state["refusals"], state["refusal_witnesses"])
                            or not same(state["ref_certificates"], state["ref_certificate_witnesses"])
                            or not same(state["ref_refusals"], state["ref_refusal_witnesses"])
                            or not same(state["discard_failure"], state["discard_failure_witness"])
                            or not same(state["code_candidates"], state["code_candidates_witness"])
                            or state["objects"].keys() != state["object_witnesses"].keys()):
                        raise ValueError("COMDAT native derived selection indices changed during the batch")
                    if state["discard_failure"] is not None:
                        raise ValueError("native REF transcript remains malformed: " + state["discard_failure"])
                    if any(state[key] is not None for key in ("discard_observation", "discard_events", "discard_named_events", "discard_witness")) and not same(
                            (state["discard_observation"], state["discard_events"], state["discard_named_events"]), state["discard_witness"]):
                        raise ValueError("native REF event observations changed during the batch")
                    for key, (obj, before_obj) in state["object_witnesses"].items():
                        if state["objects"][key] is not obj or not same(obj.__dict__, before_obj):
                            raise ValueError("COMDAT parsed physical COFF changed during the batch")
                    if state["metadata"] is not None:
                        inventory, index, spans = state["metadata"]
                        original, old_inventory, old_index, old_spans = state["metadata_witness"]
                        if (inventory is not original or not same(inventory, old_inventory) or not same(index, old_index)
                                or not same(spans.__dict__, old_spans)):
                            raise ValueError("ANY derived PDB correspondence changed during the batch")
    finally:
        native.check_current()
