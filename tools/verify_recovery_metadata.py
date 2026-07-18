#!/usr/bin/env python3
"""Verify recovery metadata determinism with a content-hash stamp cache.

Regenerating the canonical inventory parses the multi-hundred-megabyte IDB and
re-disassembles the image, which costs minutes per invocation. Every input of
that regeneration is hash-pinned, so when the complete input closure and every
committed output are byte-identical to a previous fully verified run, skipping
regeneration cannot weaken the determinism guarantee. The stamp key covers the
committed outputs too, so hand-editing a generated catalog always invalidates
the cache and fails the byte comparison on the regeneration that follows.
"""

import argparse
import hashlib
from pathlib import Path
import subprocess
import sys
import tempfile

from local_artifact import require_local_artifact_path


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_CACHE_DIR = REPO_ROOT / "build" / "recovery-metadata-cache"
DEFAULT_IDB = (
    REPO_ROOT / ".opensmacx" / "analysis" /
    "terranx_ORIG_200_v3_7.5.SP3.idb")
CACHE_KEEP_COUNT = 5

EXPORT_TOOL = REPO_ROOT / "tools" / "export_recovery_inventory.py"
CORRELATE_TOOL = REPO_ROOT / "tools" / "correlate_recovery_analyses.py"
WRAPPER_TOOL = Path(__file__).resolve()

COMPARED_OUTPUTS = (
    "functions.csv",
    "callgraph.json",
    "summary.json",
    "analysis-correlation.csv",
    "priorities.csv",
    "analysis-summary.json",
)

STATIC_INPUTS = (
    EXPORT_TOOL,
    CORRELATE_TOOL,
    WRAPPER_TOOL,
    REPO_ROOT / "src" / "OpenSMACX.def",
    REPO_ROOT / "docs" / "recovery-overrides.csv",
    REPO_ROOT / "docs" / "recovery-binding-classifications.csv",
    REPO_ROOT / "docs" / "recovery" / "ida9-functions.csv",
    REPO_ROOT / "docs" / "recovery" / "ghidra-functions.csv",
    REPO_ROOT / "docs" / "recovery" / "ghidra-interior-references.csv",
)


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def manifest_lines(idb_path):
    paths = [idb_path, *STATIC_INPUTS]
    paths.extend(sorted((REPO_ROOT / "src").glob("*.cpp")))
    paths.extend(
        REPO_ROOT / "docs" / "recovery" / name for name in COMPARED_OUTPUTS)
    lines = [f"python:{sys.version_info[0]}.{sys.version_info[1]}"]
    for path in paths:
        if not path.is_file():
            raise RuntimeError(f"required metadata input missing: {path}")
        try:
            label = path.resolve().relative_to(REPO_ROOT).as_posix()
        except ValueError:
            label = str(path.resolve())
        lines.append(f"{label}:{sha256(path)}")
    return lines


def run_regeneration(idb_path, verify_dir):
    subprocess.run(
        [sys.executable, str(EXPORT_TOOL),
         "--idb", str(idb_path),
         "--output-dir", str(verify_dir)],
        check=True)
    subprocess.run(
        [sys.executable, str(CORRELATE_TOOL),
         "--canonical", str(verify_dir / "functions.csv"),
         "--canonical-summary", str(verify_dir / "summary.json"),
         "--output-dir", str(verify_dir)],
        check=True)


def compare_outputs(verify_dir):
    for name in COMPARED_OUTPUTS:
        generated = verify_dir / name
        committed = REPO_ROOT / "docs" / "recovery" / name
        if not generated.is_file():
            raise RuntimeError(f"regeneration did not produce {generated}")
        if generated.read_bytes() != committed.read_bytes():
            raise RuntimeError(
                f"regenerated {name} differs from committed docs/recovery/{name}")


def write_stamp(cache_dir, key, manifest_text):
    cache_dir.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=cache_dir, delete=False) as handle:
        handle.write(manifest_text)
        temporary = Path(handle.name)
    temporary.replace(cache_dir / f"{key}.stamp")
    stamps = sorted(
        cache_dir.glob("*.stamp"),
        key=lambda path: path.stat().st_mtime,
        reverse=True)
    for stale in stamps[CACHE_KEEP_COUNT:]:
        stale.unlink()


def main():
    parser = argparse.ArgumentParser(
        description="Verify deterministic recovery metadata with stamp caching")
    parser.add_argument("--idb", type=Path, default=DEFAULT_IDB)
    parser.add_argument("--verify-dir", type=Path, required=True)
    parser.add_argument("--cache-dir", type=Path, default=DEFAULT_CACHE_DIR)
    parser.add_argument("--force", action="store_true",
                        help="Regenerate even when a matching stamp exists")
    args = parser.parse_args()

    verify_dir = require_local_artifact_path(
        args.verify_dir, "metadata verify directory")
    cache_dir = require_local_artifact_path(
        args.cache_dir, "metadata stamp cache directory")
    if not args.idb.is_file():
        raise SystemExit(f"IDB not found: {args.idb}")

    manifest_text = "\n".join(manifest_lines(args.idb)) + "\n"
    key = hashlib.sha256(manifest_text.encode("utf-8")).hexdigest()
    stamp = cache_dir / f"{key}.stamp"
    if not args.force and stamp.is_file():
        if stamp.read_text(encoding="utf-8") == manifest_text:
            print(f"verify-recovery-metadata: cached ({key[:16]})")
            return 0
        stamp.unlink()

    if verify_dir.exists():
        for entry in sorted(verify_dir.rglob("*"), reverse=True):
            if entry.is_file() or entry.is_symlink():
                entry.unlink()
            else:
                entry.rmdir()
    verify_dir.mkdir(parents=True, exist_ok=True)

    run_regeneration(args.idb, verify_dir)
    compare_outputs(verify_dir)
    write_stamp(cache_dir, key, manifest_text)
    print(f"verify-recovery-metadata: verified and stamped ({key[:16]})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
