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
from .model import Result, require_digest, require_text
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
