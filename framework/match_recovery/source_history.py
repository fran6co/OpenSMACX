"""Read immutable source versions without a second ledger or checkout changes."""

from dataclasses import dataclass
from pathlib import Path
import re
import subprocess


def git(root: Path, *args: str, input: bytes | None = None) -> bytes:
    result = subprocess.run(["git", *args], cwd=root, input=input, capture_output=True)
    if result.returncode:
        raise ValueError("cannot read source history: " + result.stderr.decode(errors="replace").strip())
    return result.stdout


@dataclass(frozen=True)
class SourceVersion:
    revision: str
    path: str
    blob: str
    content: bytes


def source_versions(root: Path, baseline: str, head: str, *,
                    required_paths: tuple[str, ...] = ()) -> tuple[SourceVersion, ...]:
    """All distinct source blobs at baseline and subsequent reachable commits.

    Include merged side histories, not just the first-parent chain: a claim
    introduced and then removed on a side branch must not disappear on merge.
    This deliberately does not infer obligations predating the task baseline.
    """
    for revision in (baseline, head):
        if not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", revision):
            raise ValueError("source history requires full committed revisions")
    git(root, "merge-base", "--is-ancestor", baseline, head)
    revisions = [baseline, *git(root, "rev-list", "--reverse", "--full-history",
                               f"{baseline}..{head}", "--", "src").decode().splitlines()]
    entries, seen = [], set()
    for revision in revisions:
        present = set()
        for entry in git(root, "ls-tree", "-r", "-z", revision, "--", "src").split(b"\0"):
            if not entry:
                continue
            metadata, raw_path = entry.split(b"\t", 1)
            mode, kind, oid = metadata.decode().split()
            path = raw_path.decode("utf-8")
            present.add(path)
            if path not in required_paths and Path(path).suffix not in (".cpp", ".c", ".h"):
                continue
            if kind != "blob" or mode not in ("100644", "100755"):
                raise ValueError(f"unsupported historical source entry: {path}")
            key = (path, oid)
            if key not in seen:
                seen.add(key)
                entries.append((revision, path, oid))
        if not set(required_paths) <= present:
            raise ValueError(f"source history lacks required paths at {revision}")
    oids = sorted({oid for _, _, oid in entries})
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
    return tuple(SourceVersion(revision, path, oid, blobs[oid]) for revision, path, oid in entries)
