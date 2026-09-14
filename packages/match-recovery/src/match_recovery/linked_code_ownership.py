"""Stable physical producers for live cross-build code comparison.

Compilation receipts supply source TU identity, not object basenames or byte
similarity. Dependency identity retains its exact input/member and content pin.
These are correspondence descriptions, never scope or acceptance permissions.
"""

from hashlib import sha256
from pathlib import Path

from .build_graph_commands import _context
from .coff_archive import CoffArchive
from .components import relative_path
from .linked_initializers import _read
from .model import Result, require_digest, require_text
from .pe_coff import Coff, PeImage


def _input(value):
    if not isinstance(value, dict) or set(value) - {"path", "member", "occurrence", "generated"} or "path" not in value:
        raise ValueError("code producer requires an exact physical input identity")
    path = Path(value["path"])
    if not path.is_absolute() or path.resolve() != path or not path.is_file() or path.stat().st_nlink != 1:
        raise ValueError("code producer input is not an unaliased physical file")
    if "member" in value:
        require_text(value["member"], "archive member")
        if type(value.get("occurrence")) is not int or value["occurrence"] < 0 or "generated" in value:
            raise ValueError("invalid code producer archive occurrence")
    elif "occurrence" in value or ("generated" in value and value["generated"] != "msvc_export"):
        raise ValueError("invalid code producer input attributes")
    return path


class _Inputs:
    """Call-local byte/parse memo with a final guard over every physical file."""

    def __init__(self):
        self.files = {}
        self.archives = {}

    def read(self, path):
        if path not in self.files:
            raw = path.read_bytes()
            self.files[path] = raw, {"sha256": sha256(raw).hexdigest(), "size": len(raw)}
        return self.files[path][0]

    def members(self, path):
        if path not in self.archives:
            self.archives[path] = CoffArchive(self.read(path)).members
        return self.archives[path]

    def check_current(self):
        for path, (_, expected) in self.files.items():
            _read(path, expected)


def _contents(identity, expected, inputs):
    path = _input(identity)
    raw = inputs.read(path)
    if "member" in identity:
        members = inputs.members(path)
        values = members.get(identity["member"], ())
        if identity["occurrence"] >= len(values):
            raise ValueError("code producer archive member is missing")
        raw = values[identity["occurrence"]]
    require_digest(expected)
    if sha256(raw).hexdigest() != expected:
        raise ValueError("code producer contents differ from their observed identity")
    return raw


def _compilations(context):
    root, directory, rows, _ = _context(context)
    receipts = {row["source"]: row["compilation"]["output"]["after"]
                for row in context.report["compile_link"]["compilations"]}
    compiled = {}
    for unit, row in rows.items():
        expected = receipts[str(root / unit)]
        _read(row["output"], expected)
        compiled[unit] = {"path": str(row["output"]), "sha256": expected["sha256"]}
    return root, directory, compiled


def _portable(path, root, directory):
    if path.is_relative_to(directory):
        return {"kind": "build", "path": path.relative_to(directory).as_posix()}
    if path.is_relative_to(root):
        return {"kind": "project", "path": path.relative_to(root).as_posix()}
    return {"kind": "external", "path": str(path)}


def _symbol(value):
    require_text(value, "code producer symbol")
    if any(ord(char) < 33 or ord(char) >= 127 for char in value):
        raise ValueError("invalid code producer symbol spelling")
    return value


def identities(context, nodes):
    """Index freshly observed nodes by explicit IDs and stable exact producers."""
    context.check_current()
    inputs = _Inputs()
    parsed = {}
    try:
        root, directory, compiled = _compilations(context)
        by_output = {row["path"]: unit for unit, row in compiled.items()}
        result = {}
        previous = -1
        for node in nodes:
            if (not isinstance(node, dict) or type(node.get("id")) is not int or node["id"] < 0
                    or node["id"] <= previous or node.get("result") != Result.PASS):
                raise ValueError("code producer nodes require ordered unique nonnegative IDs and complete observations")
            previous = node["id"]
            symbol = _symbol(node["symbol"])
            owner = node.get("source_owner", node.get("owner"))
            require_text(owner, "original code producer owner")
            proof = node.get("proof", {})
            if not isinstance(proof, dict):
                raise ValueError("code producer node requires its native proof")
            proof_owner = proof.get("owner", owner)
            require_text(proof_owner, "code proof original owner")
            if proof.get("symbol") != symbol or proof_owner.casefold() != owner.casefold():
                raise ValueError("code producer node contradicts its original proof identity")
            source = node["input"]
            path = _input(source)
            raw = _contents(source, node["object_sha256"], inputs)
            if proof.get("extent_kind") != "i386_short_import_thunk":
                from .linked_code_region import select_code_region
                key = (tuple(sorted(source.items())), node["object_sha256"])
                if key not in parsed:
                    parsed[key] = Coff(raw)
                select_code_region(parsed[key], symbol)
            if set(source) == {"path"} and str(path) in by_output:
                unit = by_output[str(path)]
                if node["object_sha256"] != compiled[unit]["sha256"]:
                    raise ValueError("compiled code producer differs from its exact current receipt")
                identity = {"kind": "compiled", "unit": unit, "symbol": symbol}
            else:
                identity = {"kind": "dependency", "input": _portable(path, root, directory),
                            "object_sha256": node["object_sha256"], "symbol": symbol,
                            **{key: source[key] for key in ("member", "occurrence", "generated") if key in source}}
            result[node["id"]] = identity
        return result
    finally:
        try:
            inputs.check_current()
        finally:
            context.check_current()


def _target(identity, root, directory, compiled):
    if not isinstance(identity, dict):
        raise ValueError("invalid stable code producer identity")
    symbol = _symbol(identity.get("symbol"))
    if identity.get("kind") == "compiled":
        if set(identity) != {"kind", "unit", "symbol"}:
            raise ValueError("invalid compiled code producer identity")
        unit = relative_path(identity["unit"])
        if unit not in compiled:
            raise ValueError("code producer source has no current compilation")
        return {"path": compiled[unit]["path"]}, compiled[unit]["sha256"], symbol
    if (identity.get("kind") != "dependency"
            or set(identity) - {"kind", "input", "object_sha256", "symbol", "member", "occurrence", "generated"}
            or not {"kind", "input", "object_sha256", "symbol"} <= set(identity)):
        raise ValueError("invalid dependency code producer identity")
    source = identity["input"]
    if not isinstance(source, dict) or set(source) != {"kind", "path"}:
        raise ValueError("invalid code producer portable input")
    if source["kind"] in ("project", "build"):
        path = (root if source["kind"] == "project" else directory) / relative_path(source["path"])
    elif source["kind"] == "external":
        path = Path(source["path"])
    else:
        raise ValueError("invalid code producer input namespace")
    if _portable(path, root, directory) != source:
        raise ValueError("code producer input namespace is not canonical")
    target = {"path": str(path), **{key: identity[key] for key in ("member", "occurrence", "generated") if key in identity}}
    _input(target)
    require_digest(identity["object_sha256"])
    return target, identity["object_sha256"], symbol


def _image(context):
    rows = [row for row in context.report.get("outputs", ()) if row.get("format") == "pe32-i386"]
    if len(rows) != 1:
        raise ValueError("code producer resolution requires one live native image")
    row, = rows
    raw = _read(Path(context.directory) / relative_path(row["path"]),
                {key: row[key] for key in ("sha256", "size")})
    return PeImage(raw, sha256(raw).hexdigest())


def resolve(context, selected_identities, objects, mapping, folding=None):
    """Resolve stable producers independently in this current native build."""
    context.check_current()
    inputs = _Inputs()
    try:
        root, directory, compiled = _compilations(context)
        image = _image(context)
        mapping.validate(image)
        if folding is not None:
            if (folding._objects is not objects or folding._mapping != mapping
                    or folding.image_sha256 != sha256(image.data).hexdigest()):
                raise ValueError("code producer folding evidence belongs to another current build")
            folding.check_current()
        fold_candidates = folding.candidate_keys() if folding is not None else frozenset()
        records, fresh = objects._constant_population()
        try:
            from .linked_import_thunks import import_thunk_candidates, verify_import_thunk
            thunks = import_thunk_candidates(objects)
            result = []
            for identity in selected_identities:
                target, expected, symbol = _target(identity, root, directory, compiled)
                _contents(target, expected, inputs)
                matching = [row for row in records if {key: row[key] for key in
                            ("path", "member", "occurrence", "generated") if key in row} == target]
                if len(matching) != 1 or matching[0]["sha256"] != expected:
                    raise ValueError("code producer lacks its exact current physical input record")
                candidates = []
                for placed in (*mapping.publics, *(mapping.statics or ())):
                    if placed.symbol != symbol or not placed.function or not placed.segment:
                        continue
                    if symbol in thunks:
                        proof = verify_import_thunk(objects, mapping, image, placed.owner, symbol)
                        if proof["result"] == Result.PASS and proof["input"] == target and proof["object_sha256"] == expected:
                            candidates.append({"owner": placed.owner, "symbol": symbol})
                        continue
                    group = (folding.for_map(placed.owner, symbol, optional=True)
                             if folding is not None and (placed.owner.casefold(), symbol) in fold_candidates else None)
                    if group is not None:
                        matches = [member for member in group.members if member.symbol == symbol
                                   and dict(member.input) == target and member.object_sha256 == expected]
                        if len(matches) == 1:
                            candidates.append({"owner": matches[0].owner, "symbol": symbol})
                        continue
                    actual = objects.resolve(placed.owner)
                    if actual["identity"] == target and actual["sha256"] == expected:
                        from .linked_code_region import select_code_region
                        entry = select_code_region(Coff(actual["raw"]), symbol)
                        pool = mapping.publics if entry.storage == 2 else mapping.statics or ()
                        if placed not in pool:
                            raise ValueError("code producer map storage differs from original symbol")
                        candidates.append({"owner": actual["owner"], "symbol": symbol})
                if len(candidates) != 1:
                    raise ValueError("code producer is missing, discarded or ambiguously placed in current build")
                result.append(candidates[0])
            return tuple(result)
        finally:
            fresh()
    finally:
        try:
            inputs.check_current()
        finally:
            context.check_current()
