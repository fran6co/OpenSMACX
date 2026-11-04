"""Reusable native link observations within one live application-build epoch.

Parsed inputs are disposable context, not selection permissions or saved proof.
Re-rooting code observation never substitutes another build's artifacts.
"""

from contextlib import contextmanager
from copy import deepcopy
from dataclasses import dataclass
from hashlib import sha256

from .link_file_evidence import parse_link_files
from .link_input_coverage import _input_inventory
from .linked_code_graph import observe_code_graph
from .linked_objects import SelectedObjects
from .model import Result
from .parsed_state import same
from .msvc_link_map import parse_link_map
from .pe_coff import PeImage
from .graph_limits import DEFAULT_NODES

SOURCE_REGION_MAX_BYTES = min(1 << 30, DEFAULT_NODES * 65536)
SOURCE_REGION_MAX_REFERENCES = min(4000000, DEFAULT_NODES * 1024)
SOURCE_REGION_MAX_ROOTS = min(1000000, DEFAULT_NODES * 16)


def _selected_root_fold_group(folding, owner, symbol, candidates):
    """Resolve only advisory ICF candidates through complete native group APIs."""
    if folding is None or (owner.casefold(), symbol) not in candidates:
        return None
    group = folding.for_map(owner, symbol, optional=True)
    if group is None:
        group = folding.group(owner, symbol, optional=True)
    return group


@dataclass(frozen=True)
class LinkedInputs:
    artifacts: dict
    image: object
    mapping: object
    evidence: dict
    coverage: dict
    records: list
    objects: SelectedObjects
    fixups: object
    folding: object


def parse_inputs(artifacts):
    """Parse independently captured inputs; the native caller owns freshness."""
    artifacts = dict(artifacts)
    raw = artifacts["pe32-i386"]
    image = PeImage(raw, sha256(raw).hexdigest())
    fixups = None
    if "msvc-pdb" in artifacts:
        from .pdb_fixups import bound_fixups
        fixups = bound_fixups(image, artifacts["msvc-pdb"])
    mapping = parse_link_map(artifacts["msvc-map"])
    mapping.validate(image)
    coverage, records = _input_inventory(artifacts["link-input-files"], artifacts["msvc-link-log"])
    if coverage["result"] != Result.PASS:
        raise ValueError("linked observation requires complete input candidate coverage")
    evidence = parse_link_files(artifacts["link-input-files"])
    objects = SelectedObjects(evidence, coverage, records)
    folding = None
    if fixups is not None:
        from .linked_folding import bind_folding
        folding = bind_folding(objects, mapping, image, fixups, artifacts["msvc-link-log"])
    return LinkedInputs(artifacts, image, mapping, evidence, coverage, records, objects, fixups, folding)


@dataclass(frozen=True)
class LinkedBuild:
    context: object
    inputs: LinkedInputs
    check_current: object

    def module_bindings(self):
        from .linked_modules import observe_modules
        try:
            self.check_current()
            return deepcopy(observe_modules(self.context, self.inputs))
        finally:
            self.check_current()

    def contributions(self):
        from .pdb_contributions import bound_contributions
        try:
            self.check_current()
            pdb = self.inputs.artifacts.get("msvc-pdb")
            if pdb is None:
                raise ValueError("linked contributions require this build's PDB sidecar")
            return bound_contributions(self.inputs.image, pdb).report()
        finally:
            self.check_current()

    def common_contributions(self, symbols):
        from .linked_contributions import common_contributions
        try:
            self.check_current()
            return deepcopy(common_contributions(self.inputs, symbols))
        finally:
            self.check_current()

    def ordinary_contributions(self):
        from .linked_contributions import ordinary_contributions
        try:
            self.check_current()
            return deepcopy(ordinary_contributions(self.context, self.inputs))
        finally:
            self.check_current()

    def ordinary_allocations(self, *, folded_groups=()):
        from .ordinary_allocation_coverage import observe_ordinary_allocations
        try:
            self.check_current()
            return deepcopy(observe_ordinary_allocations(self, folded_groups=folded_groups))
        finally:
            self.check_current()

    def data_section(self, owner, number):
        from .linked_storage import observe_data_section
        try:
            self.check_current()
            return deepcopy(observe_data_section(self.inputs, owner, number))
        finally:
            self.check_current()

    def data_contents(self, owner, number):
        """Verify complete ordinary initial bytes and references in this epoch."""
        from .linked_data_contents import verify_data_contents
        try:
            self.check_current()
            return deepcopy(verify_data_contents(self.inputs, owner, number))
        finally:
            self.check_current()

    def storage_census(self, *, max_sections=100000):
        from .linked_storage_census import observe_storage
        try:
            self.check_current()
            return deepcopy(observe_storage(self.inputs, max_sections=max_sections))
        finally:
            self.check_current()

    def common_intersections(self, symbols, sections=None):
        from .linked_storage import common_intersections
        try:
            self.check_current()
            return deepcopy(common_intersections(self.inputs, symbols, sections, context=self.context))
        finally:
            self.check_current()

    def common(self, symbols):
        """Observe requested common storage, without granting trial coverage."""
        from .common_allocation import _verify_common_inputs
        try:
            self.check_current()
            value = self.inputs
            report = _verify_common_inputs(value.evidence, value.coverage, value.records,
                                           value.image, value.mapping, symbols)
            report["map_sha256"] = sha256(value.artifacts["msvc-map"]).hexdigest()
            # The parsed helper retains the coverage observation in its report;
            # callers may edit diagnostics without mutating this live epoch.
            return deepcopy(report)
        finally:
            self.check_current()

    def initializers(self, *, max_functions=DEFAULT_NODES):
        from .linked_initializers import _observe_inputs, _stamp
        try:
            self.check_current()
            report = _observe_inputs(self.inputs, max_functions=max_functions)
            report["artifacts"] = {kind: _stamp(raw) for kind, raw in self.inputs.artifacts.items()}
            return report
        finally:
            self.check_current()

    def code(self, roots, *, max_functions=DEFAULT_NODES):
        from .linked_codeview import LinkedProcedures
        from .linked_code_region import _live_weak_reference_scope
        weak_scope = None
        try:
            self.check_current()
            value = self.inputs
            weak_scope = _live_weak_reference_scope(value.objects, value.mapping, value.image,
                                                    self.context, self.check_current)
            return self._code_in_scope(roots, weak_scope, max_functions=max_functions)
        finally:
            if weak_scope is not None:
                weak_scope.expire()
            self.check_current()

    def _code_in_scope(self, roots, weak_scope, *, max_functions=DEFAULT_NODES):
        """Observe code using the caller's still-live private weak scope."""
        self.check_current()
        from .linked_code_region import _LiveWeakReferenceScope
        if type(weak_scope) is not _LiveWeakReferenceScope:
            raise ValueError("linked code requires its live private weak-reference scope")
        if (weak_scope._objects is not self.inputs.objects
                or weak_scope._mapping is not self.inputs.mapping
                or weak_scope._image is not self.inputs.image):
            raise ValueError("weak-reference scope belongs to another linked build")
        from .linked_codeview import LinkedProcedures
        return observe_code_graph(self.inputs.objects, self.inputs.mapping, self.inputs.image, roots,
                                  fixups=self.inputs.fixups, folding=self.inputs.folding,
                                  max_functions=max_functions, codeview=LinkedProcedures(self),
                                  _weak_scope=weak_scope)

    def _weak_scope(self):
        """Create one native-bound weak scope for an enclosing source comparison."""
        from .linked_code_region import _live_weak_reference_scope
        self.check_current()
        return _live_weak_reference_scope(self.inputs.objects, self.inputs.mapping,
                                          self.inputs.image, self.context, self.check_current)

    def _source_regions_in_scope(self, roots, weak_scope):
        """Verify selected weak caller regions without traversing their callees."""
        from .linked_code_region import _LiveWeakReferenceScope, select_code_region, verify_code_region
        from .linked_codeview import LinkedProcedures
        from .linked_constant_batch import constant_lookups
        from .linked_map_index import MapIndexes
        from .model import Result
        from .pe_coff import Coff
        from .graph_limits import node_limit

        self.check_current()
        if type(weak_scope) is not _LiveWeakReferenceScope:
            raise ValueError("source-region verification requires its live weak-reference scope")
        if (weak_scope._objects is not self.inputs.objects
                or weak_scope._mapping is not self.inputs.mapping
                or weak_scope._image is not self.inputs.image):
            raise ValueError("source-region scope belongs to another linked build")
        rows = []
        folding = self.inputs.folding
        node_limit(DEFAULT_NODES)
        if len(roots) > SOURCE_REGION_MAX_ROOTS:
            raise ValueError("selected weak source-root request resource bound exceeded")
        fold_candidate_error = None
        try:
            fold_candidates = folding.candidate_keys() if folding is not None else frozenset()
        except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
            fold_candidates, fold_candidate_error = frozenset(), str(error)
        codeview = LinkedProcedures(self)
        map_indexes = MapIndexes()
        observed_bytes, observed_edges = 0, 0
        fallback_keys = set()
        budget_exhausted_reason = None

        def charge(proof):
            nonlocal observed_bytes, observed_edges, budget_exhausted_reason
            regions = proof.get("regions")
            if regions is None:
                observed_bytes += proof.get("size", 0)
                observed_edges += len(proof.get("references", ()))
            else:
                observed_bytes += sum(region.get("size", 0) for region in regions)
                observed_edges += sum(len(region.get("references", ())) for region in regions)
            for key, fallback in weak_scope._verified_fallbacks.items():
                if key not in fallback_keys:
                    fallback_keys.add(key)
                    regions = fallback.get("regions")
                    if regions is None:
                        observed_bytes += fallback.get("size", 0)
                        observed_edges += len(fallback.get("references", ()))
                    else:
                        observed_bytes += sum(region.get("size", 0) for region in regions)
                        observed_edges += sum(len(region.get("references", ())) for region in regions)
            if observed_bytes > SOURCE_REGION_MAX_BYTES:
                budget_exhausted_reason = "selected weak source-region observed-byte resource bound exceeded"
                raise ValueError(budget_exhausted_reason)
            if observed_edges > SOURCE_REGION_MAX_REFERENCES:
                budget_exhausted_reason = "selected weak source-region reference resource bound exceeded"
                raise ValueError(budget_exhausted_reason)

        try:
            with constant_lookups(self.inputs.objects) as constants:
                for request in roots:
                    owner, name = request["owner"], request["symbol"]
                    row = {"owner": owner, "symbol": name,
                           "source_unit": request.get("unit"), "entity_id": request.get("entity_id"),
                           "result": Result.UNKNOWN}
                    obj = subject = weak_relocation = None
                    proof_attempted = False
                    try:
                        if budget_exhausted_reason is not None:
                            weak_scope.refuse_source_root(owner, name, budget_exhausted_reason)
                            row["blocks_original_qualification"] = True
                            raise ValueError(budget_exhausted_reason)
                        selected = self.inputs.objects.resolve(owner)
                        if selected["owner"].casefold() != owner.casefold() or selected["sha256"] != sha256(selected["raw"]).hexdigest():
                            raise ValueError("selected root differs from its physical owner identity")
                        obj = Coff(selected["raw"])
                        unit, receipt = weak_scope.source_receipt(owner, obj)
                        if unit is None or receipt is None or request.get("unit") not in (None, unit):
                            raise ValueError("selected root lacks its matching source compile receipt")
                        subject = select_code_region(obj, name)
                        weak_relocations = [relocation for number in obj.code_sections(subject)
                                            for relocation in obj.section_subject(number)[1]
                                            if obj.symbols[int(relocation.target)].storage == 105]
                        if len(weak_relocations) != 1:
                            raise ValueError("selected source region requires exactly one weak relocation")
                        weak_relocation, = weak_relocations
                        if fold_candidate_error is not None:
                            proof_attempted = True
                            raise ValueError("folding candidate coverage unavailable: " + fold_candidate_error)
                        if (owner.casefold(), name) in fold_candidates:
                            proof_attempted = True
                            group = _selected_root_fold_group(folding, owner, name, fold_candidates)
                            if group is not None:
                                raise ValueError("selected weak caller belongs to an ICF group; direct region proof refuses folding")
                            proof_attempted = False
                        proof_attempted = True
                        proof = verify_code_region(
                            obj, subject, owner, self.inputs.mapping, self.inputs.image,
                            objects=self.inputs.objects, fixups=self.inputs.fixups,
                            _index=map_indexes.for_map(self.inputs.mapping), _constants=constants,
                            codeview=codeview, _weak_scope=weak_scope)
                        charge(proof)
                        row.update(object_identity=selected["identity"], object_sha256=selected["sha256"],
                                   receipt_sha256=receipt, proof=proof, result=proof["result"])
                    except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
                        row["reason"] = str(error)
                        if proof_attempted:
                            weak_scope.refuse_current_edge(obj, subject, weak_relocation, owner, str(error))
                            row["blocks_original_qualification"] = True
                    rows.append(row)
        finally:
            self.check_current()
        return rows


@contextmanager
def linked_build(context):
    """Retain parsed evidence only while the original native context is live."""
    from .linked_initializers import _artifacts
    context.check_current()
    try:
        context_report = deepcopy(context.report)
        inputs = parse_inputs(_artifacts(context))
        bound_objects = {name: getattr(inputs, name) for name in ("image", "mapping", "objects", "fixups", "folding")}

        def metadata_state():
            fixups = inputs.fixups
            folding = inputs.folding
            return ((fixups.image_sha256, fixups.pdb_sha256, dict(fixups), fixups.fpo_records)
                    if fixups is not None else None,
                    (folding._groups, folding.image_sha256, folding.log_sha256)
                    if folding is not None else None)

        metadata_before = deepcopy(metadata_state())
        # Populate immutable indices before pinning them. Lazy proof results
        # are not allowed to alter the candidate population or linker policy.
        from .common_allocation import _policy
        from .coff_archive import CoffArchive
        from .linked_objects import _read
        records, population_guard = inputs.objects._constant_population()
        inputs.objects._constant_policy_gaps = _policy(inputs.evidence, records)
        mutable_caches = {"_files", "_archives", "_resolved"}
        selected_state = deepcopy({key: value for key, value in inputs.objects.__dict__.items()
                                   if key not in mutable_caches})
        # Independently reconstruct growing owner caches. They are acceleration,
        # never a second source of physical selection authority.
        witness = SelectedObjects(inputs.evidence, inputs.coverage, inputs.records)
        archive_witnesses = {}
        stamps = deepcopy(inputs.objects._stamps)

        def selected_fresh():
            actual = inputs.objects
            if not same({key: value for key, value in actual.__dict__.items() if key not in mutable_caches}, selected_state):
                raise ValueError("parsed selected-object state changed within its live epoch")
            population_guard()
            # Include stamped tools, which are not COFF candidate records.
            for path, expected in stamps.items():
                raw = _read(path, expected)
                if path in actual._files and not same(actual._files[path], raw):
                    raise ValueError("selected object file cache changed")
            if set(actual._files) - stamps.keys() or set(actual._archives) - actual._files.keys():
                raise ValueError("selected object cache has an unobserved physical input")
            for path, archive in actual._archives.items():
                if path not in archive_witnesses:
                    archive_witnesses[path] = deepcopy(CoffArchive(actual._files[path]).__dict__)
                if not isinstance(archive, CoffArchive) or not same(archive.__dict__, archive_witnesses[path]):
                    raise ValueError("selected archive cache changed")
            for owner, value in actual._resolved.items():
                if not same(value, witness.resolve(owner)):
                    raise ValueError("selected owner cache differs from physical evidence")
        image_state = deepcopy(inputs.image.__dict__)
        observation_state = deepcopy((inputs.artifacts, inputs.mapping, inputs.evidence,
                                      inputs.coverage, inputs.records))
        live = True

        def fresh():
            if not live:
                raise ValueError("linked observation context has expired")
            try:
                context.check_current()
                if not same(context.report, context_report):
                    raise ValueError("application report changed within its linked epoch")
                if (any(getattr(inputs, name) is not value for name, value in bound_objects.items())
                        or not same(metadata_state(), metadata_before)
                        or inputs.folding is not None and (inputs.folding._objects is not inputs.objects
                            or inputs.folding._mapping is not inputs.mapping or inputs.folding._image is not inputs.image
                            or inputs.folding._fixups is not inputs.fixups)):
                    raise ValueError("linked PDB/folding binding changed within its live epoch")
                if (not same(inputs.image.__dict__, image_state) or not same(
                        (inputs.artifacts, inputs.mapping, inputs.evidence, inputs.coverage, inputs.records), observation_state)):
                    raise ValueError("parsed linked evidence changed within its live epoch")
                selected_fresh()
            finally:
                context.check_current()

        try:
            fresh()
            yield LinkedBuild(context, inputs, fresh)
        finally:
            try:
                fresh()
            finally:
                live = False
    finally:
        context.check_current()
