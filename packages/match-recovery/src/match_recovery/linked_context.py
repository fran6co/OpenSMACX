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
from .msvc_link_map import parse_link_map
from .pe_coff import PeImage
from .graph_limits import DEFAULT_NODES


def _same(left, right, seen=None):
    """Exact parsed-state equality, including scalar types (False is not 0)."""
    if type(left) is not type(right):
        return False
    if left is right:
        return True
    if seen is None:
        seen = set()
    pair = id(left), id(right)
    if pair in seen:
        return True
    seen.add(pair)
    if isinstance(left, dict):
        return len(left) == len(right) and all(_same(a, b, seen) and _same(x, y, seen)
            for (a, x), (b, y) in zip(left.items(), right.items()))
    if isinstance(left, (list, tuple)):
        return len(left) == len(right) and all(_same(a, b, seen) for a, b in zip(left, right))
    if isinstance(left, (set, frozenset)):
        return {(type(value), value) for value in left} == {(type(value), value) for value in right}
    if hasattr(left, "__dict__"):
        return _same(left.__dict__, right.__dict__, seen)
    return left == right


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

    def ordinary_allocations(self):
        from .ordinary_allocation_coverage import observe_ordinary_allocations
        try:
            self.check_current()
            return deepcopy(observe_ordinary_allocations(self))
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
        try:
            self.check_current()
            value = self.inputs
            return observe_code_graph(value.objects, value.mapping, value.image, roots,
                                      fixups=value.fixups, folding=value.folding, max_functions=max_functions,
                                      codeview=LinkedProcedures(self))
        finally:
            self.check_current()


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
            if not _same({key: value for key, value in actual.__dict__.items() if key not in mutable_caches}, selected_state):
                raise ValueError("parsed selected-object state changed within its live epoch")
            population_guard()
            # Include stamped tools, which are not COFF candidate records.
            for path, expected in stamps.items():
                raw = _read(path, expected)
                if path in actual._files and not _same(actual._files[path], raw):
                    raise ValueError("selected object file cache changed")
            if set(actual._files) - stamps.keys() or set(actual._archives) - actual._files.keys():
                raise ValueError("selected object cache has an unobserved physical input")
            for path, archive in actual._archives.items():
                if path not in archive_witnesses:
                    archive_witnesses[path] = deepcopy(CoffArchive(actual._files[path]).__dict__)
                if not isinstance(archive, CoffArchive) or not _same(archive.__dict__, archive_witnesses[path]):
                    raise ValueError("selected archive cache changed")
            for owner, value in actual._resolved.items():
                if not _same(value, witness.resolve(owner)):
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
                if not _same(context.report, context_report):
                    raise ValueError("application report changed within its linked epoch")
                if (any(getattr(inputs, name) is not value for name, value in bound_objects.items())
                        or not _same(metadata_state(), metadata_before)
                        or inputs.folding is not None and (inputs.folding._objects is not inputs.objects
                            or inputs.folding._mapping is not inputs.mapping or inputs.folding._image is not inputs.image
                            or inputs.folding._fixups is not inputs.fixups)):
                    raise ValueError("linked PDB/folding binding changed within its live epoch")
                if (not _same(inputs.image.__dict__, image_state) or not _same(
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
