"""Resolve owner-qualified linker observations to exact physical COFF bytes.

The enclosing live build owns final source/artifact/input freshness. Instances
cache only bytes and parsed archives from that epoch, never selection verdicts
across epochs. A map basename alone is not physical object provenance.
"""

from collections import defaultdict
from copy import deepcopy
from hashlib import sha256
import os
from pathlib import Path, PurePosixPath, PureWindowsPath

from .coff_archive import CoffArchive
from .compile_file_evidence import _identity, _physical
from .link_file_evidence import _path, _stamp
from .model import Result, digest, require_digest, require_text
from .msvc_link_log import input_label


def _owner(row):
    member = row["member"]
    require_text(member, "final-pass object member")
    name = PureWindowsPath(member).name
    library = row["library"]
    if library is not None:
        require_text(library, "final-pass object library")
        name = PureWindowsPath(library).stem + ":" + name
    if not name or any(char.isspace() or ord(char) < 32 for char in name):
        raise ValueError("unsupported final-pass object owner")
    return name


def _read(path, expected):
    """Read once, binding descriptor and final physical path to the same inode."""
    path = Path(path)
    before = _identity(_physical(path))
    descriptor = os.open(path, os.O_RDONLY | getattr(os, "O_NONBLOCK", 0) | getattr(os, "O_NOFOLLOW", 0))
    with os.fdopen(descriptor, "rb") as stream:
        if _identity(os.fstat(stream.fileno())) != before:
            raise ValueError("selected object input changed before reading")
        raw = stream.read()
        if _identity(os.fstat(stream.fileno())) != before:
            raise ValueError("selected object input changed during reading")
    if _identity(_physical(path)) != before:
        raise ValueError("selected object input changed after reading")
    if {"sha256": sha256(raw).hexdigest(), "size": len(raw)} != expected:
        raise ValueError("selected object input differs from its captured file stamp")
    return raw


def _constant_names(names, optional):
    """Shared strict request grammar; batching confers no binding authority."""
    if (type(optional) is not bool or not isinstance(names, (tuple, list))
            or not 1 <= len(names) <= 4096):
        raise ValueError("absolute symbol lookup requires a bounded nonempty name sequence")
    names = tuple(names)
    for name in names:
        require_text(name, "absolute linker symbol")
        if any(ord(char) < 33 or ord(char) >= 127 for char in name):
            raise ValueError("unsupported absolute linker symbol spelling")
    if len(set(names)) != len(names):
        raise ValueError("absolute symbol lookup requires unique names")
    return names


class SelectedObjects:
    """A bounded index of one complete native link-input inventory.

    ``resolve`` returns identity/sha256/raw/owner only, not a code, storage,
    initializer or acceptance proof. Cached bytes require the caller's live
    context to remain unchanged through final consumption.
    """

    def __init__(self, evidence, coverage, records):
        try:
            if (coverage.get("result") != Result.PASS or coverage.get("gaps")
                    or coverage.get("unsupported_records") or evidence.get("capture_gaps")
                    or not isinstance(records, list) or not records
                    or type(coverage.get("record_count")) is not int or coverage["record_count"] != len(records)):
                raise ValueError("selected objects require complete successful native input coverage")
            self._stamps = {}
            for row in evidence["files"]:
                path = _path(row["path"])
                _stamp(row["before"])
                _stamp(row["after"])
                if path in self._stamps or row["after"] is None or row["before"] != row["after"]:
                    raise ValueError("selected object inputs require unique stable file stamps")
                self._stamps[path] = dict(row["after"])
            self._generated = set()
            for row in evidence["generated_exports"]:
                path = _path(row["path"])
                _stamp(row["after"])
                # Unconsumed generated outputs may legitimately be absent.
                if row["after"] is None:
                    continue
                if path in self._stamps or row["existed_before"] is not False:
                    raise ValueError("generated selected input is not uniquely fresh")
                self._stamps[path] = dict(row["after"])
                self._generated.add(path)
            self._explicit = {_path(path) for path in evidence["explicit_inputs"]}
            self._libraries = {_path(row["path"]) for row in evidence["library_searches"] if row["path"] is not None}
            self._library_names = defaultdict(set)
            for row in evidence["library_searches"]:
                require_text(row["spelling"], "observed library spelling")
                if row["path"] is not None:
                    self._library_names[PureWindowsPath(row["spelling"]).name.casefold()].add(row["path"])
            self._records = []
            for row in records:
                if row.get("result") != Result.PASS:
                    raise ValueError("unsupported selected-object inventory record")
                identity = {key: row[key] for key in ("path", "member", "occurrence", "generated") if key in row}
                path = _path(identity["path"])
                require_digest(row["sha256"])
                if path not in self._stamps:
                    raise ValueError("object inventory path has no physical input stamp")
                if "member" in identity:
                    require_text(identity["member"], "archive member identity")
                    if (type(identity.get("occurrence")) is not int or identity["occurrence"] < 0
                            or "generated" in identity or path not in self._libraries | self._explicit):
                        raise ValueError("invalid archive member occurrence or input identity")
                elif ("occurrence" in identity or path not in self._explicit | self._generated
                      or path in self._generated and identity.get("generated") != "msvc_export"
                      or "generated" in identity and (identity["generated"] != "msvc_export" or path not in self._generated)):
                    raise ValueError("invalid standalone object provenance")
                self._records.append({"identity": identity, "sha256": row["sha256"], "format": row["format"]})
            self._owners = defaultdict(list)
            if (not isinstance(coverage["inputs"], list) or type(coverage.get("input_count")) is not int
                    or coverage["input_count"] != len(coverage["inputs"])):
                raise ValueError("incomplete final-pass input observations")
            for row in coverage["inputs"]:
                label = input_label(row["label"])
                if label.library != row["library"] or label.member != row["member"]:
                    raise ValueError("final-pass label disagrees with its object identity")
                owner = _owner(row)
                if not isinstance(row["paths"], list):
                    raise ValueError("final-pass input paths must be explicit")
                for path in row["paths"]:
                    _path(path)
                self._owners[owner.casefold()].append({"owner": owner, "library": row["library"],
                    "member": row["member"], "paths": tuple(row["paths"]), "selection": row["selection"],
                    "candidate_coverage": row["candidate_coverage"], "candidate_count": row["candidate_count"]})
            self._files, self._archives, self._resolved = {}, {}, {}
            # Constant resolution needs the complete candidate population, not
            # merely the subset whose owners occur in a map. Keep the native
            # records defensive and verify them against physical inputs before
            # drawing any absence/uniqueness conclusion.
            self._constant_evidence = deepcopy(evidence)
            self._constant_records = deepcopy(records)
            self._constant_physical = None
            self._constant_inventory = None
        except (KeyError, TypeError, AttributeError) as error:
            raise ValueError("incomplete selected-object input evidence") from error

    def resolve(self, owner):
        require_text(owner, "link map owner")
        key = owner.casefold()
        if key in self._resolved:
            return deepcopy(self._resolved[key])
        observations = self._owners.get(key, ())
        if len(observations) != 1:
            raise ValueError("missing or ambiguous final-pass object owner: " + owner)
        observed, = observations
        if (observed["selection"] != Result.PASS or observed["candidate_coverage"] != Result.PASS
                or type(observed["candidate_count"]) is not int or observed["candidate_count"] != 1
                or len(observed["paths"]) != 1):
            raise ValueError("owner has no unique selected physical object: " + owner)
        path, = observed["paths"]
        basename = PureWindowsPath(observed["member"]).name.casefold()
        if observed["library"] is None:
            candidates = [row for row in self._records if "member" not in row["identity"]
                          and Path(row["identity"]["path"]).name.casefold() == basename]
        else:
            candidates = [row for row in self._records if row["identity"]["path"] == path
                          and "member" in row["identity"]
                          and PurePosixPath(row["identity"]["member"]).name.casefold() == basename]
            if path not in self._library_names[PureWindowsPath(observed["library"]).name.casefold()]:
                raise ValueError("selected archive lacks an observed library-search identity")
        if len(candidates) != 1 or candidates[0]["identity"]["path"] != path:
            raise ValueError("ambiguous physical candidate or archive member occurrence")
        row, = candidates
        if row["format"] != "coff":
            raise ValueError("selected owner is not an ordinary COFF object")
        if path not in self._files:
            self._files[path] = _read(path, self._stamps[path])
        raw = self._files[path]
        if "member" in row["identity"]:
            if path not in self._archives:
                self._archives[path] = CoffArchive(raw)
            actual = [(name, occurrence, body) for name, bodies in self._archives[path].members.items()
                      if PurePosixPath(name).name.casefold() == basename
                      for occurrence, body in enumerate(bodies)]
            if len(actual) != 1 or actual[0][:2] != (row["identity"]["member"], row["identity"]["occurrence"]):
                raise ValueError("physical archive member identity is missing or ambiguous")
            raw = actual[0][2]
        if sha256(raw).hexdigest() != row["sha256"]:
            raise ValueError("selected object bytes differ from their inventory")
        result = {"identity": dict(row["identity"]), "sha256": row["sha256"], "raw": raw, "owner": observed["owner"]}
        self._resolved[key] = result
        return deepcopy(result)

    def _constant_population(self):
        from .coff_inventory import inventory_object

        def fresh():
            for path, expected in self._constant_physical.items():
                if _identity(_physical(Path(path))) != expected:
                    raise ValueError("constant input population changed during its live epoch")

        if self._constant_inventory is not None:
            fresh()
            return self._constant_inventory, fresh
        paths = self._explicit | self._libraries | {
            row["path"] for row in self._constant_records if "generated" in row}
        physical, records = {}, []
        for path in sorted(paths):
            before = _identity(_physical(Path(path)))
            raw = _read(path, self._stamps[path])
            if _identity(_physical(Path(path))) != before:
                raise ValueError("constant input changed while reading the candidate population")
            physical[path] = before
            if path in self._libraries or raw.startswith(b"!<arch>\n"):
                archive = CoffArchive(raw)
                if not archive.members:
                    raise ValueError("constant input population contains an empty archive")
                for name, bodies in archive.members.items():
                    for occurrence, body in enumerate(bodies):
                        records.append({"path": path, "member": name, "occurrence": occurrence,
                                        **inventory_object(body)})
            else:
                identity = {"generated": "msvc_export"} if path in self._generated else {}
                records.append({"path": path, **identity, **inventory_object(raw)})
        if (any(row["result"] != Result.PASS for row in records)
                or digest(sorted(records, key=digest)) != digest(sorted(self._constant_records, key=digest))):
            raise ValueError("constant candidate inventory differs from the complete physical population")
        symbols, imports, weak_fallbacks = defaultdict(list), set(), defaultdict(set)
        interesting = set()
        for record in records:
            for symbol in record["symbols"]:
                symbols[symbol["name"]].append((record, symbol))
                if symbol["section"] == -1 and symbol["storage"] == 2:
                    interesting.add(symbol["name"])
                if symbol["declaration"] == "weak":
                    weak_fallbacks[symbol["fallback"]].add(symbol["name"])
            for imported in record["imports"]:
                imports.update((imported["symbol"], "__imp_" + imported["symbol"]))
        # A weak alias leading to a constant cannot evade the strict path by
        # being presented as an ordinary mapped image address instead.
        pending = list(interesting)
        while pending:
            for name in weak_fallbacks.get(pending.pop(), ()):
                if name not in interesting:
                    interesting.add(name)
                    pending.append(name)
        self._constant_symbols = symbols
        self._constant_imports = imports
        self._constant_weak_fallbacks = weak_fallbacks
        self._constant_interest = interesting
        self._constant_policy_gaps = None
        self._constant_physical = physical
        fresh()
        self._constant_inventory = records
        return records, fresh

    def resolve_constant(self, name, *, optional=False):
        """Single-name form of the guarded complete-population bulk lookup."""
        return self.resolve_constants((name,), optional=optional)[name]

    def resolve_local_symbols(self, owner, names, image):
        """Place uniquely named selected private data from this PE's COFF debug.

        Linked COFF tables need not retain file scopes. A static spelling is
        usable only when the *complete physical candidate population* contains
        exactly one occurrence, from the requested selected owner. No symbol
        order, target operand, byte search or other build supplies placement.
        These bindings prove neither target contents nor recovery acceptance.
        The enclosing live observer still owns source/output freshness guards.
        """
        from .common_allocation import _policy
        from .pe_coff import Coff
        from .pe_debug import linked_coff_symbols

        if not isinstance(names, (tuple, list)) or not 1 <= len(names) <= 4096:
            raise ValueError("private symbol lookup requires a bounded nonempty name sequence")
        names = tuple(names)
        for name in names:
            require_text(name, "private linker symbol")
            if any(ord(char) < 33 or ord(char) >= 127 for char in name):
                raise ValueError("unsupported private linker symbol spelling")
        if len(set(names)) != len(names):
            raise ValueError("private symbol lookup requires unique names")
        # This same physically revalidated population also backs constants;
        # cache bytes/parsing within one live epoch, never a placement verdict.
        records, fresh = self._constant_population()
        try:
            if self._constant_policy_gaps is None:
                self._constant_policy_gaps = _policy(self._constant_evidence, records)
            if self._constant_policy_gaps:
                raise ValueError("private resolution has unsupported linker policy or directives")
            selected = self.resolve(owner)
            obj = Coff(selected["raw"])
            linked = defaultdict(list)
            for symbol in linked_coff_symbols(image):
                linked[symbol.name].append(symbol)
            result = {}
            for name in names:
                if name in self._constant_imports or self._constant_weak_fallbacks.get(name):
                    raise ValueError("private symbol conflicts with an import or weak alias")
                candidates = self._constant_symbols.get(name, ())
                if len(candidates) != 1:
                    raise ValueError("private symbol requires exactly one complete-population occurrence")
                record, declared = candidates[0]
                identity = {key: record[key] for key in ("path", "member", "occurrence", "generated") if key in record}
                if identity != selected["identity"] or record["sha256"] != selected["sha256"]:
                    raise ValueError("private definition differs from the selected physical owner")
                symbol = obj.symbols.get(declared["index"])
                if (symbol is None or symbol.name != name or symbol.storage != 3 or symbol.type != 0
                        or symbol.section <= 0 or obj.auxiliary.get(symbol.index)):
                    raise ValueError("private placement requires a plain static data symbol")
                section = obj.sections[symbol.section - 1]
                if (section.flags & (0x20 | 0x20000000) or not section.flags & (0x40 | 0x80)
                        or name == section.name or not 0 <= symbol.value < obj.section_sizes[symbol.section - 1]):
                    raise ValueError("private placement requires bounded non-code object data")
                placements = linked.get(name, ())
                if len(placements) != 1:
                    raise ValueError("private symbol has no unique embedded COFF placement")
                placed, = placements
                if (placed.storage != 3 or placed.type != 0 or placed.section <= 0
                        or image.section_flags[placed.section - 1] & (0x20 | 0x20000000)
                        or not image.section_flags[placed.section - 1] & (0x40 | 0x80)):
                    raise ValueError("private embedded COFF placement is not plain image data")
                result[name] = {"symbol": name, "address": image.base + placed.value,
                    "owner": selected["owner"], "input": dict(identity), "object_sha256": selected["sha256"],
                    "object_index": symbol.index, "object_section": symbol.section, "object_offset": symbol.value,
                    "placement": "embedded_coff"}
            return result
        finally:
            fresh()

    def resolve_constants(self, names, *, optional=False):
        """Bind a bounded batch of absolute definitions, not image VAs.

        Unselected candidates still participate in this deliberately conservative
        uniqueness check. Neither a saved inventory, an unresolved operand, a
        zero value nor a familiar runtime spelling supplies a binding. The
        enclosing live observer must additionally verify actual linked operands
        without HIGHLOW fixups and retain its artifact/context guards.

        ``optional=True`` returns None only when the complete physical input
        population contains no absolute definition or weak path to one. A map
        row cannot override a known constant candidate. Ordinary no-candidate
        lookups do not need the stronger constant-selection policy subset.
        Complete input-population freshness is checked before and after the
        whole batch, not once per relocation. No partial batch is returned on
        refusal, and the enclosing live artifact guards remain mandatory.
        """
        from .common_allocation import _policy
        from .pe_coff import Coff

        names = _constant_names(names, optional)
        records, fresh = self._constant_population()

        def one(name):
            if optional and name not in self._constant_interest:
                return None
            if self._constant_policy_gaps is None:
                self._constant_policy_gaps = _policy(self._constant_evidence, records)
            if self._constant_policy_gaps:
                raise ValueError("constant resolution has unsupported linker policy or directives")
            if name in self._constant_imports:
                raise ValueError("absolute symbol conflicts with an import candidate")
            if self._constant_weak_fallbacks.get(name):
                raise ValueError("absolute symbol conflicts with a weak alias")
            candidates = []
            for record, symbol in self._constant_symbols.get(name, ()):
                kind = symbol["declaration"]
                if kind == "weak":
                    raise ValueError("absolute symbol conflicts with a weak alias")
                if kind == "local_or_metadata" or kind == "undefined" and not symbol["auxiliary_size"]:
                    continue
                if (kind != "absolute" or symbol["section"] != -1 or symbol["storage"] != 2
                        or symbol["type"] != 0 or symbol["auxiliary_size"]
                        or record["machine"] != 0x14C or record["optional_header_size"] != 0):
                    raise ValueError("absolute symbol has a common, overriding or unsupported definition")
                candidates.append((record, symbol))
            if len(candidates) != 1:
                raise ValueError("absolute symbol requires exactly one complete-population definition")
            record, declared = candidates[0]
            identity = {key: record[key] for key in ("path", "member", "occurrence", "generated") if key in record}
            owners = []
            basename = PurePosixPath(identity.get("member", identity["path"])).name.casefold()
            for observations in self._owners.values():
                for observed in observations:
                    if (observed["paths"] == (identity["path"],)
                            and bool(observed["library"]) == ("member" in identity)
                            and PureWindowsPath(observed["member"]).name.casefold() == basename):
                        owners.append(observed["owner"])
            if len(owners) != 1:
                raise ValueError("absolute definition lacks one selected physical owner")
            selected = self.resolve(owners[0])
            if selected["identity"] != identity or selected["sha256"] != record["sha256"]:
                raise ValueError("absolute definition differs from the selected physical object")
            obj = Coff(selected["raw"])
            matches = [symbol for symbol in obj.symbols.values() if symbol.name == name and symbol.storage == 2]
            if (len(matches) != 1 or matches[0].index != declared["index"]
                    or matches[0].section != -1 or matches[0].type != 0
                    or obj.auxiliary.get(matches[0].index) or matches[0].value != declared["value"]):
                raise ValueError("selected absolute COFF symbol differs from its native inventory")
            return {"symbol": name, "value": matches[0].value, "input": dict(identity),
                    "object_sha256": selected["sha256"], "owner": selected["owner"]}

        try:
            return {name: one(name) for name in names}
        finally:
            fresh()
