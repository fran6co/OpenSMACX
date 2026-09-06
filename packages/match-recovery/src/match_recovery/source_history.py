"""Immutable source history with per-commit configuration, not a second ledger."""

from dataclasses import dataclass
from pathlib import Path
import re
import subprocess

from .model import require_text
from .records import parse_json, read_json
from .source_config import SourceConfiguration


def git(root: Path, *args: str, input: bytes | None = None) -> bytes:
    result = subprocess.run(["git", "--literal-pathspecs", *args], cwd=root, input=input, capture_output=True)
    if result.returncode:
        raise ValueError("cannot read source history: " + result.stderr.decode(errors="replace").strip())
    return result.stdout


@dataclass(frozen=True)
class HistoryConfiguration:
    before_configuration: SourceConfiguration | None = None
    evidence: tuple[str, ...] = ()

    @classmethod
    def read(cls, root: Path):
        path = root / "recovery/project.json"
        if not path.resolve().is_relative_to(root.resolve()):
            raise ValueError("history configuration resolves outside the worktree")
        project = read_json(path)
        if "source_history" not in project:
            return cls()
        value = project["source_history"]
        if (not isinstance(value, dict) or set(value) != {"before_configuration", "evidence"}
                or not isinstance(value["evidence"], list) or not value["evidence"]):
            raise ValueError("historical source configuration requires explicit source inputs and evidence")
        for item in value["evidence"]:
            require_text(item, "historical configuration evidence")
        return cls(SourceConfiguration.from_dict(value["before_configuration"]), tuple(value["evidence"]))


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
    entries, seen, established, small_blobs = [], set(), {}, {}

    def small_blob(entry, path):
        oid = _regular(entry, path)
        if oid not in small_blobs:
            small_blobs[oid] = git(root, "cat-file", "blob", oid)
        return small_blobs[oid]

    for revision, parents in revisions:
        config_path = "recovery/project.json"
        config_tree = _tree(root, revision, (config_path,))
        value = parse_json(small_blob(config_tree[config_path], config_path)) if config_path in config_tree else {}
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
        tree = _tree(root, revision, (*selected.roots, selected.image_pin))
        if any(name in tree for name in selected.roots):
            raise ValueError(f"historical source root is not a directory at {revision}")
        if selected.image_pin not in tree:
            raise ValueError(f"source history lacks required image pin at {revision}")
        pin_text = small_blob(tree[selected.image_pin], selected.image_pin).decode("utf-8")
        if re.findall(r"^sha256\s+([0-9a-f]{64})\s*$", pin_text, re.MULTILINE) != [image_sha256]:
            raise ValueError("source image pin changed within task history")
        for path, entry in tree.items():
            if path == selected.image_pin or Path(path).suffix not in selected.extensions:
                continue
            oid = _regular(entry, path)
            key = (path, oid, selected)
            if key not in seen:
                seen.add(key)
                entries.append((revision, path, oid, selected))
    oids = sorted({oid for _, _, oid, _ in entries})
    if not oids:
        return ()
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
    return tuple(SourceVersion(revision, path, oid, blobs[oid], selected)
                 for revision, path, oid, selected in entries)
