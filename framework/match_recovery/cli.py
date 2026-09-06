"""The framework CLI. Machine-readable stdout; diagnostics only on stderr."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys
from typing import Sequence

from . import __version__
from .store import ObjectStore


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(prog="recover", description="Structure-led matching recovery")
    result.add_argument("--version", action="version", version=__version__)
    commands = result.add_subparsers(dest="command", required=True)
    inventory = commands.add_parser("inventory", help="Inspect legacy source without altering it")
    inventory.add_argument("--root", type=Path, default=Path.cwd())
    inventory.add_argument("--image", type=Path, help="Verify the original against its pin")
    importing = commands.add_parser("import", help="Preserve an immutable evidence snapshot")
    providers = importing.add_subparsers(dest="provider", required=True)
    legacy = providers.add_parser("legacy", help="Import old OpenSMACX annotations as evidence")
    legacy.add_argument("--root", type=Path, default=Path.cwd())
    legacy.add_argument("--image", type=Path)
    return result


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        from .adapters.opensmacx import inventory, legacy_snapshot
        snapshot = legacy_snapshot(args.root, args.image)
        result = inventory(snapshot)
        if args.command == "import":
            # Do not offer an arbitrary output path for raw source derivatives.
            # The durable knowledge importer will separately emit public metadata.
            store = ObjectStore(args.root / ".opensmacx" / "recovery" / "objects")
            result["snapshot_id"] = store.put(snapshot)
        print(json.dumps(result, indent=2, sort_keys=True))
        return 0
    except (OSError, ValueError, ImportError) as error:
        print(f"recover: {error}", file=sys.stderr)
        return 3


if __name__ == "__main__":
    sys.exit(main())
