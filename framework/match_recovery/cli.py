"""The framework CLI. Machine-readable stdout; diagnostics only on stderr."""

from __future__ import annotations

import argparse
from dataclasses import asdict
import json
from pathlib import Path
import subprocess
import sys
import tarfile
from typing import Sequence

from . import __version__
from .journal import Journal
from .model import Result
from .store import ObjectStore


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(prog="recover", description="Structure-led matching recovery")
    result.add_argument("--version", action="version", version=__version__)
    commands = result.add_subparsers(dest="command", required=True)
    inventory = commands.add_parser("inventory", help="Inspect legacy source without altering it")
    inventory.add_argument("--root", type=Path, default=Path.cwd())
    inventory.add_argument("--image", type=Path, help="Verify the original against its pin")
    importing = commands.add_parser("import", help="Import mappings or cache a legacy parse")
    providers = importing.add_subparsers(dest="provider", required=True)
    legacy = providers.add_parser("legacy", help="Import old OpenSMACX annotations as evidence")
    legacy.add_argument("--root", type=Path, default=Path.cwd())
    legacy.add_argument("--image", type=Path)
    vendor = providers.add_parser("opensmacx-zlib", help="Restore pinned zlib provenance and original mappings")
    vendor.add_argument("--root", type=Path, default=Path.cwd())
    vendor.add_argument("--archive", type=Path, required=True)
    components = commands.add_parser("components", help="Inspect and verify reused dependencies")
    actions = components.add_subparsers(dest="action", required=True)
    listing = actions.add_parser("list")
    listing.add_argument("--root", type=Path, default=Path.cwd())
    checking = actions.add_parser("check-source", help="Verify source integrity, not binary equivalence")
    checking.add_argument("component")
    checking.add_argument("--root", type=Path, default=Path.cwd())
    checking.add_argument("--archive", type=Path)
    history = commands.add_parser("history", help="Read durable execution events")
    history.add_argument("--root", type=Path, default=Path.cwd())
    history.add_argument("--stream", required=True)
    return result


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        if args.command == "components":
            from .components import source_integrity
            from .component_source import read_components
            manifests = read_components(args.root / "src")
            if args.action == "list":
                print(json.dumps([{"component": asdict(manifest.component),
                                   "workflow": manifest.component.workflow,
                                   "bindings": len(manifest.bindings)} for manifest in manifests], indent=2))
                return 0
            selected = [manifest for manifest in manifests if manifest.component.component_id == args.component]
            if len(selected) != 1:
                raise ValueError(f"expected one component named {args.component}")
            result = source_integrity(args.root, selected[0], args.archive)
            print(json.dumps(result, indent=2, sort_keys=True))
            return {Result.PASS: 0, Result.FAIL: 1, Result.UNKNOWN: 3}[result["result"]]
        if args.command == "import" and args.provider == "opensmacx-zlib":
            from .adapters.opensmacx_components import zlib_manifest
            from .component_source import write_component
            manifest = zlib_manifest(args.root, args.archive)
            path = args.root / "src" / "vendor" / "zlib.recovery.h"
            created = write_component(path, manifest)
            print(json.dumps({"component": "zlib", "manifest": str(path.relative_to(args.root)),
                              "created": created, "source_integrity": "pass",
                              "historical_bindings": len(manifest.bindings),
                              "binary_verification": "unknown"}, indent=2))
            return 0
        if args.command == "history":
            events = Journal(args.root / "recovery" / "history").read(args.stream)
            print(json.dumps([asdict(event) for event in events], indent=2, sort_keys=True))
            return 0
        from .adapters.opensmacx import inventory, legacy_snapshot
        snapshot = legacy_snapshot(args.root, args.image)
        result = inventory(snapshot)
        if args.command == "import":
            # A legacy parse is reproducible from source: cache it, never journal
            # its inventory or treat it as a second authority over source claims.
            store = ObjectStore(args.root / ".opensmacx" / "recovery" / "objects")
            result["snapshot_id"] = store.put(snapshot)
        print(json.dumps(result, indent=2, sort_keys=True))
        return 0
    except (OSError, ValueError, ImportError, subprocess.CalledProcessError, tarfile.TarError) as error:
        print(f"recover: {error}", file=sys.stderr)
        return 3


if __name__ == "__main__":
    sys.exit(main())
