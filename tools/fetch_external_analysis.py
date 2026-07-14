#!/usr/bin/env python3

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath, PureWindowsPath
import ssl
import urllib.request


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_CATALOG = REPO_ROOT / "docs" / "recovery" / "external-analysis-sources.json"
DEFAULT_OUTPUT = REPO_ROOT / ".opensmacx" / "external-analysis" / "sources"


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def validated_relative_path(value):
    path = PurePosixPath(value)
    windows_path = PureWindowsPath(value)
    if (path.is_absolute() or windows_path.is_absolute() or windows_path.drive
            or ".." in path.parts or "\\" in value):
        raise RuntimeError(f"unsafe artifact path: {value}")
    return Path(*path.parts)


def tls_context():
    defaults = ssl.get_default_verify_paths()
    candidates = [
        os.environ.get("SSL_CERT_FILE"),
        defaults.cafile,
        "/etc/ssl/cert.pem",
        "/opt/homebrew/etc/ca-certificates/cert.pem",
    ]
    for candidate in candidates:
        if candidate and Path(candidate).is_file():
            return ssl.create_default_context(cafile=candidate)
    return ssl.create_default_context()


def fetch_artifact(artifact, output_dir):
    request = urllib.request.Request(
        artifact["download_url"], headers={"User-Agent": "OpenSMACX-recovery"})
    with urllib.request.urlopen(request, timeout=120, context=tls_context()) as response:
        data = response.read(artifact["size"] + 1)
    if len(data) != artifact["size"]:
        raise RuntimeError(
            f"size mismatch for {artifact['path']}: expected {artifact['size']}, got {len(data)}")
    digest = sha256(data)
    if digest != artifact["sha256"]:
        raise RuntimeError(
            f"SHA-256 mismatch for {artifact['path']}: expected {artifact['sha256']}, got {digest}")
    destination = output_dir / validated_relative_path(artifact["path"])
    destination.parent.mkdir(parents=True, exist_ok=True)
    if not destination.parent.resolve().is_relative_to(output_dir.resolve()):
        raise RuntimeError(f"artifact destination escapes output directory: {artifact['path']}")
    if destination.is_symlink():
        raise RuntimeError(f"artifact destination must not be a symlink: {destination}")
    destination.write_bytes(data)
    return destination


def main():
    parser = argparse.ArgumentParser(
        description="Fetch hash-pinned external analysis into ignored local storage")
    parser.add_argument("--catalog", type=Path, default=DEFAULT_CATALOG)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--source-path", action="append", default=[])
    args = parser.parse_args()

    if not args.catalog.is_file():
        parser.error(f"catalog not found: {args.catalog}")
    catalog = json.loads(args.catalog.read_text(encoding="utf-8"))
    selected = set(args.source_path)
    artifacts = [
        artifact for artifact in catalog["artifacts"]
        if not selected or artifact["path"] in selected
    ]
    missing = selected - {artifact["path"] for artifact in artifacts}
    if missing:
        parser.error(f"source paths are absent from catalog: {', '.join(sorted(missing))}")
    for artifact in artifacts:
        destination = fetch_artifact(artifact, args.output)
        print(f"Fetched {artifact['path']} to {destination}")


if __name__ == "__main__":
    main()
