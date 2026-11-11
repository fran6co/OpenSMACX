"""Immutable source history with per-commit configuration, not a second ledger."""

from dataclasses import dataclass
from pathlib import Path
import re
import subprocess

from .model import require_text
from .records import parse_json, read_json
from .source_config import SourceConfiguration
from .components import relative_path


def git(root: Path, *args: str, input: bytes | None = None) -> bytes:
    result = subprocess.run(["git", "--literal-pathspecs", *args], cwd=root, input=input, capture_output=True)
    if result.returncode:
        raise ValueError("cannot read source history: " + result.stderr.decode(errors="replace").strip())
    return result.stdout


@dataclass(frozen=True)
class HistoryConfiguration:
    before_configuration: SourceConfiguration | None = None
    evidence: tuple[str, ...] = ()
    baseline_revision: str | None = None

    @classmethod
    def read(cls, root: Path):
        path = root / "recovery/project.json"
        if not path.resolve().is_relative_to(root.resolve()):
            raise ValueError("history configuration resolves outside the worktree")
        project = read_json(path)
        if "source_history" not in project:
            return cls()
        value = project["source_history"]
        if (not isinstance(value, dict) or set(value) - {"before_configuration", "evidence", "baseline_revision"}
                or "evidence" not in value
                or not isinstance(value["evidence"], list) or not value["evidence"]):
            raise ValueError("historical source configuration requires explicit source inputs and evidence")
        for item in value["evidence"]:
            require_text(item, "historical configuration evidence")
        baseline = value.get("baseline_revision")
        if baseline is not None and (not isinstance(baseline, str)
                                    or not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", baseline)):
            raise ValueError("source history baseline requires a full committed revision")
        if "before_configuration" not in value and baseline is None:
            raise ValueError("source history needs baseline_revision or before_configuration")
        return cls(SourceConfiguration.from_dict(value["before_configuration"])
                   if "before_configuration" in value else None, tuple(value["evidence"]), baseline)


@dataclass(frozen=True)
class SourceVersion:
    revision: str
    path: str
    blob: str
    content: bytes
    configuration: SourceConfiguration


def _tree(root, revision, paths):
    entries = {}
    for entry in git(root, "ls-tree", "-r", "-z", revision, "--", *paths).split(b"\0"):
        if entry:
            metadata, raw_path = entry.split(b"\t", 1)
            entries[raw_path.decode("utf-8")] = tuple(metadata.decode().split())
    return entries


def _regular(entry, path):
    mode, kind, oid = entry
    if kind != "blob" or mode not in ("100644", "100755"):
        raise ValueError(f"unsupported historical source entry: {path}")
    return oid


def _blobs(root, oids):
    oids = sorted(set(oids))
    if not oids:
        return {}
    batch = git(root, "cat-file", "--batch", input=("\n".join(oids) + "\n").encode())
    blobs, offset = {}, 0
    for oid in oids:
        end = batch.index(b"\n", offset)
        header = batch[offset:end].decode().split()
        if len(header) != 3 or header[:2] != [oid, "blob"]:
            raise ValueError("invalid source blob response")
        size = int(header[2])
        offset = end + 1
        blobs[oid] = batch[offset:offset + size]
        if len(blobs[oid]) != size or batch[offset + size:offset + size + 1] != b"\n":
            raise ValueError("truncated source blob response")
        offset += size + 1
    if offset != len(batch):
        raise ValueError("unexpected source blob response tail")
    return blobs


def revision_files(root: Path, revision: str, roots: tuple[str, ...]) -> dict[str, bytes]:
    """Read regular files under explicit committed roots, including non-C++ includes."""
    if not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", revision) or not roots:
        raise ValueError("historical files require a full revision and explicit roots")
    for name in roots:
        relative_path(name)
    tree = _tree(root, revision, roots)
    if any(name in tree for name in roots):
        raise ValueError("historical source root is not a directory")
    entries = {name: _regular(entry, name) for name, entry in tree.items()}
    blobs = _blobs(root, entries.values())
    return {name: blobs[oid] for name, oid in entries.items()}


def _batch_check(root, specs):
    """Resolve "<revision>:<path>" specs in one git call: spec -> (type, oid) or None."""
    if not specs:
        return {}
    output = git(root, "cat-file", "--batch-check=%(objecttype) %(objectname)",
                 input=("\n".join(specs) + "\n").encode()).decode("utf-8").split("\n")
    result = {}
    for spec, line in zip(specs, output):
        fields = line.split()
        result[spec] = (fields[0], fields[1]) if len(fields) == 2 and not line.endswith(" missing") else None
    if len(output) < len(specs):
        raise ValueError("truncated source object response")
    return result


def _objects(root, oids):
    """Read raw objects (any type) in one git call: oid -> (type, bytes)."""
    oids = sorted(set(oids))
    if not oids:
        return {}
    batch = git(root, "cat-file", "--batch", input=("\n".join(oids) + "\n").encode())
    objects, offset = {}, 0
    for oid in oids:
        end = batch.index(b"\n", offset)
        header = batch[offset:end].decode().split()
        if len(header) != 3 or header[0] != oid:
            raise ValueError("invalid source object response")
        size = int(header[2])
        offset = end + 1
        objects[oid] = (header[1], batch[offset:offset + size])
        if len(objects[oid][1]) != size or batch[offset + size:offset + size + 1] != b"\n":
            raise ValueError("truncated source object response")
        offset += size + 1
    if offset != len(batch):
        raise ValueError("unexpected source object response tail")
    return objects


def _parse_tree(data):
    """Raw git tree object -> [(name, mode, oid)] in stored (git) order."""
    entries, offset = [], 0
    while offset < len(data):
        space = data.index(b" ", offset)
        nul = data.index(b"\0", space)
        mode = data[offset:space].decode()
        name = data[space + 1:nul].decode("utf-8")
        entries.append((name, mode, data[nul + 1:nul + 21].hex()))
        offset = nul + 21
    return entries


def _suffix(path):
    """Path(path).suffix without constructing a Path (hot in the history walk)."""
    name = path.rsplit("/", 1)[-1]
    dot = name.rfind(".")
    return name[dot:] if 0 < dot < len(name) - 1 else ""


class _TreeCache:
    """Recursive listings of tree objects, each tree read once (batched)."""

    def __init__(self, root):
        self.root, self.raw = root, {}

    def load(self, oids):
        pending = {oid for oid in oids if oid not in self.raw}
        while pending:
            fetched = _objects(self.root, pending)
            pending = set()
            for oid, (kind, data) in fetched.items():
                if kind != "tree":
                    raise ValueError("invalid source tree object")
                self.raw[oid] = entries = _parse_tree(data)
                pending.update(child for _, mode, child in entries
                               if mode == "40000" and child not in self.raw)

    def walk(self, prefix, oid, visited):
        """Yield (path, (mode, kind, oid)) below oid in git tree order, skipping
        any (prefix, subtree) already in visited: its files were already seen."""
        if (prefix, oid) in visited:
            return
        visited.add((prefix, oid))
        for name, mode, child in self.raw[oid]:
            path = prefix + "/" + name
            if mode == "40000":
                yield from self.walk(path, child, visited)
            else:
                yield path, (mode, "commit" if mode == "160000" else "blob", child)


def source_versions(root: Path, baseline: str, head: str, *,
                    configuration: SourceConfiguration | None = None,
                    history: HistoryConfiguration | None = None,
                    image_sha256: str | None = None) -> tuple[SourceVersion, ...]:
    """Distinct source blobs under each commit's declared source interpretation.

    Traverse merged side histories too. Root/suffix/seed changes cannot drop old
    claims or retroactively classify old product evidence as seed hints. Missing
    pre-framework configuration needs an explicit, evidenced project input;
    removal after configuration was established is an error, not a fallback.
    """
    for revision in (baseline, head):
        if not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", revision):
            raise ValueError("source history requires full committed revisions")
    git(root, "merge-base", "--is-ancestor", baseline, head)
    configuration = configuration or SourceConfiguration.read(root)
    history = history or HistoryConfiguration.read(root)
    if image_sha256 is None:
        from .source_inventory import image_pin
        image_sha256 = image_pin(root, configuration)
    revisions = [(baseline, [])]
    for line in git(root, "rev-list", "--reverse", "--topo-order", "--full-history", "--parents",
                    f"{baseline}..{head}").decode().splitlines():
        revision, *parents = line.split()
        revisions.append((revision, parents))
    entries, seen, established = [], set(), {}
    # Every lookup is batched: per-revision git calls made this walk dominate
    # record/check-claims (thousands of subprocesses) and crawl under load.
    config_path = "recovery/project.json"
    config_ids = _batch_check(root, [f"{revision}:{config_path}" for revision, _ in revisions])
    config_blobs = _objects(root, {found[1] for found in config_ids.values() if found and found[0] == "blob"})
    parsed_configs = {}
    selections = []
    for revision, parents in revisions:
        found = config_ids[f"{revision}:{config_path}"]
        if found is None:
            value = {}
        else:
            if found[0] != "blob":
                raise ValueError(f"unsupported historical source entry: {config_path}")
            if found[1] not in parsed_configs:
                parsed_configs[found[1]] = parse_json(config_blobs[found[1]][1])
            value = parsed_configs[found[1]]
        if value and (type(value.get("schema_version")) is not int or value["schema_version"] != 1):
            raise ValueError("unsupported historical project configuration")
        inherited = any(established.get(parent, False) for parent in parents)
        established[revision] = inherited or "source" in value
        if "source" in value:
            selected = SourceConfiguration.from_dict(value["source"])
        elif inherited:
            raise ValueError(f"source configuration removed within task history at {revision}")
        elif history.before_configuration is not None:
            selected = history.before_configuration
        else:
            raise ValueError(f"source history lacks configuration at {revision}; declare evidenced source_history inputs")
        if selected.address_space != configuration.address_space:
            raise ValueError("source address space changed within task history")
        selections.append((revision, selected))
    path_ids = _batch_check(root, [f"{revision}:{name}" for revision, selected in selections
                                   for name in (*selected.roots, selected.image_pin)])
    pin_blobs = _objects(root, {path_ids[f"{revision}:{selected.image_pin}"][1]
                                for revision, selected in selections
                                if path_ids[f"{revision}:{selected.image_pin}"]
                                and path_ids[f"{revision}:{selected.image_pin}"][0] == "blob"})
    trees = _TreeCache(root)
    trees.load({found[1] for revision, selected in selections for name in selected.roots
                if (found := path_ids[f"{revision}:{name}"]) and found[0] == "tree"})
    pin_ok, visited = {}, {}
    for revision, selected in selections:
        roots = []
        for name in selected.roots:
            found = path_ids[f"{revision}:{name}"]
            if found is None:
                continue
            if found[0] != "tree":
                raise ValueError(f"historical source root is not a directory at {revision}")
            roots.append((name, found[1]))
        pin = path_ids[f"{revision}:{selected.image_pin}"]
        if pin is None:
            raise ValueError(f"source history lacks required image pin at {revision}")
        if pin[0] != "blob":
            raise ValueError(f"unsupported historical source entry: {selected.image_pin}")
        if pin[1] not in pin_ok:
            pin_text = pin_blobs[pin[1]][1].decode("utf-8")
            pin_ok[pin[1]] = re.findall(r"^sha256\s+([0-9a-f]{64})\s*$", pin_text, re.MULTILINE) == [image_sha256]
        if not pin_ok[pin[1]]:
            raise ValueError("source image pin changed within task history")
        # Roots in git tree order; each root's walk is already in tree order.
        # A (path, subtree) walked before under the same configuration holds
        # only (path, oid) pairs already recorded, so skipping it keeps order.
        seen_trees = visited.setdefault(selected, set())
        for name, tree_oid in sorted(roots, key=lambda item: (item[0] + "/").encode("utf-8")):
            for path, entry in trees.walk(name, tree_oid, seen_trees):
                if path == selected.image_pin or _suffix(path) not in selected.extensions:
                    continue
                oid = _regular(entry, path)
                key = (path, oid, selected)
                if key not in seen:
                    seen.add(key)
                    entries.append((revision, path, oid, selected))
    blobs = _blobs(root, (oid for _, _, oid, _ in entries))
    return tuple(SourceVersion(revision, path, oid, blobs[oid], selected)
                 for revision, path, oid, selected in entries)
