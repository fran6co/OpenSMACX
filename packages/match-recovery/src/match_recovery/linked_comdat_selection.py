"""Live, per-key COFF ANY discard certificates, not scope or acceptance.

An independently verified native winner and a complete physical same-key
candidate inventory establish only that the other ANY definitions (and their
associative descendants) were removed. Different loser bytes are permitted.
Missing MAP entries alone never establish REF removal or winner identity.

PDB module/address correspondence is distinct from characteristics equivalence:
https://github.com/microsoft/microsoft-pdb/blob/master/PDB/include/dbicommon.h
https://github.com/microsoft/microsoft-pdb/blob/master/PDB/dbi/mod.cpp
"""

from contextlib import contextmanager
from copy import deepcopy
from dataclasses import asdict
from weakref import WeakKeyDictionary

from .coff_comdat import comdat_key, comdat_selection
from .common_allocation import _policy, _tokens
from .linked_code_ownership import _contents
from .linked_code_region import verify_code_region
from .linked_codeview import LinkedProcedures
from .linked_constant_batch import constant_lookups
from .linked_context import LinkedBuild, _same
from .linked_interval_index import IntervalIndex
from .linked_input_epoch import input_epoch, _borrow_epoch
from .linked_map_index import index_for
from .model import Result, digest
from .pdb_contributions import bound_contributions
from .pe_coff import Coff, CoffRecords


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
    # Every child keeps its own original section identity and parent relation.
    rows = []
    for number in sorted(sections):
        if number != root and comdat_selection(obj, number) != 5:
            raise ValueError("ANY descendant lacks exact associative selection metadata")
        section = obj.sections[number - 1]
        rows.append({"object_section": number, "name": section.name,
                     "parent": obj.associations.get(number), "flags": section.flags,
                     "size": obj.section_sizes[number - 1],
                     "symbols": [asdict(item) for item in obj.symbols.values() if item.section == number]})
    return rows


class AnySelection:
    """Expiring native query handle. Serialized reports cannot recreate it."""

    __slots__ = ("__weakref__",)

    def __new__(cls, *args, **kwargs):
        raise TypeError("ANY selection requires its live context manager")

    def __reduce__(self):
        raise TypeError("ANY selection handles are not serializable")

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
        return deepcopy(list(_entry(self)["certificates"].values()))


_live = WeakKeyDictionary()


def _entry(handle):
    if type(handle) is not AnySelection or handle not in _live:
        raise ValueError("ANY selection handle is unrecognized or expired")
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


def _no_other_fold(state, obj, root, candidates):
    from .linked_comdat_nofold import sections_distinct

    # The complete same-key ANY inventory has already qualified these precise
    # independent roots. Associated code children are NOT excluded with them.
    excluded = {(digest(row["input"]), row["object_sha256"], row["root"]["section"])
                for row in candidates}
    code = obj.sections[root.section - 1]
    size = obj.section_sizes[root.section - 1]
    checked = 0
    for identity, stamp, number, other, other_size in _code_candidates(state):
        if (identity, stamp, number) in excluded:
            continue
        if not sections_distinct(code, size, other, other_size):
            raise ValueError("ANY winner has a possible distinct-section ICF competitor")
        checked += 1
    return {"result": Result.PASS, "method": "immutable_code_prefix_mismatch",
            "other_physical_code_sections": checked,
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
                    or symbol["value"] or symbol["type"] != 0x20 or symbol["auxiliary_size"]):
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
                or comdat_selection(obj, root.section) != 2
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
    selected = inputs.objects.resolve(label.owner)
    winners = [row for row in candidates if row["input"] == selected["identity"]
               and row["object_sha256"] == selected["sha256"]]
    if len(winners) != 1:
        raise ValueError("ANY MAP winner differs from the complete physical candidate family")
    winner = winners[0]
    obj = state["objects"][(digest(winner["input"]), winner["object_sha256"])]
    root = obj.symbols[winner["root"]["index"]]
    if name in state["folding_names"]:
        raise ValueError("ANY winner has an ICF group requiring complete folded-member composition")
    # Absence from reported ICF groups is not a no-fold proof. Every differently
    # identified possible code section must have an immutable byte mismatch.
    no_fold = _no_other_fold(state, obj, root, candidates)
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


def _loser(state, owner, number):
    selected = state["native"].inputs.objects.resolve(owner)
    if type(number) is not int:
        raise ValueError("ANY query requires an exact original section number")
    records = state["records"].get((digest(selected["identity"]), selected["sha256"]), ())
    if len(records) != 1:
        raise ValueError("ANY query differs from its unique physical input inventory")
    obj = _object(state, records[0])
    if not 1 <= number <= len(obj.sections):
        raise ValueError("ANY query section is outside its selected physical input")
    root_number = number
    while root_number in obj.associations:
        root_number = obj.associations[root_number]
    if not obj.sections[root_number - 1].flags & 0x1000 or comdat_selection(obj, root_number) != 2:
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
    if any(row.owner.casefold() == selected["owner"].casefold() and row.symbol in names for row in labels):
        raise ValueError("ANY loser has contradictory independently placed original labels")
    return {"certificate_id": certificate["certificate_id"], "key": root.name,
            "queried_section": number, "sections": [row["object_section"] for row in candidates[0]["sections"]],
            "disposition": "discarded_any_duplicate"}


@contextmanager
def any_selection(native, *, epoch=None):
    """Issue per-key queries only inside the original native input lifetime."""
    if type(native) is not LinkedBuild:
        raise ValueError("ANY selection requires its original live native linked build")
    if epoch is None:
        with input_epoch(native.context, native.inputs.objects) as owned:
            with any_selection(native, epoch=owned) as handle:
                yield handle
        return
    native.check_current()
    try:
        with _borrow_epoch(epoch, native.context, native.inputs.objects) as observed:
            inputs = native.inputs
            records, interest = {}, {}
            for record in observed.records:
                if record["result"] != Result.PASS:
                    raise ValueError("ANY selection requires complete physical candidate declarations")
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
                handle = object.__new__(AnySelection)
                state = {"native": native, "epoch": epoch, "inputs": observed.inputs,
                    "interest": interest, "records": records, "objects": {}, "certificates": {},
                    "object_witnesses": {}, "certificate_witnesses": {},
                    "refusals": {}, "refusal_witnesses": {},
                    "metadata": None, "metadata_witness": None,
                    "code_candidates": None, "code_candidates_witness": None,
                    "policy_gaps": _policy(inputs.evidence, observed.records),
                    "merges": tuple(tuple(word.split(":", 1)[1].split("=")) for word in words
                                    if word.startswith(("/merge:", "-merge:"))),
                    "nonincremental": any(word.lower() in ("/incremental:no", "-incremental:no")
                                          for word in inputs.evidence["arguments"]),
                    "folding_names": frozenset(name for _, name in inputs.folding.candidate_keys())
                        if inputs.folding is not None else frozenset(),
                    "map_index": index_for(inputs.mapping), "constants": constants, "codeview": LinkedProcedures(native)}
                _live[handle] = state
                fixed = ("native", "epoch", "inputs", "map_index", "constants", "codeview")
                pointers = {key: state[key] for key in fixed}
                indexed = ("interest", "records", "policy_gaps", "merges", "nonincremental", "folding_names")
                before = deepcopy({key: state[key] for key in indexed})
                map_before = deepcopy(state["map_index"].__dict__)
                try:
                    yield handle
                finally:
                    del _live[handle]
                    if (any(state[key] is not pointer for key, pointer in pointers.items())
                            or not _same({key: state[key] for key in indexed}, before)
                            or not _same(state["map_index"].__dict__, map_before)
                            or not _same(state["certificates"], state["certificate_witnesses"])
                            or not _same(state["refusals"], state["refusal_witnesses"])
                            or not _same(state["code_candidates"], state["code_candidates_witness"])
                            or state["objects"].keys() != state["object_witnesses"].keys()):
                        raise ValueError("ANY native derived selection indices changed during the batch")
                    for key, (obj, before_obj) in state["object_witnesses"].items():
                        if state["objects"][key] is not obj or not _same(obj.__dict__, before_obj):
                            raise ValueError("ANY parsed physical COFF changed during the batch")
                    if state["metadata"] is not None:
                        inventory, index, spans = state["metadata"]
                        original, old_inventory, old_index, old_spans = state["metadata_witness"]
                        if (inventory is not original or not _same(inventory, old_inventory) or not _same(index, old_index)
                                or not _same(spans.__dict__, old_spans)):
                            raise ValueError("ANY derived PDB correspondence changed during the batch")
    finally:
        native.check_current()
