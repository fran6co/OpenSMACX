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
    run = commands.add_parser("run", help="Drive bounded recovery through configured command adapters")
    run.add_argument("task")
    run.add_argument("--root", type=Path, default=Path.cwd())
    run.add_argument("--config", type=Path, required=True)
    run.add_argument("--worker", type=Path, required=True, help="Trusted worker command configuration")
    run.add_argument("--reviewer", type=Path, help="Independent read-only reviewer command configuration")
    run.add_argument("--migration", type=Path, help="Coordinator permission for scoped temporary matching regressions")
    run.add_argument("--iterations", type=int, default=3)
    inventory = commands.add_parser("inventory", help="Inspect legacy source without altering it")
    inventory.add_argument("--root", type=Path, default=Path.cwd())
    inventory.add_argument("--image", type=Path, help="Verify the original against its pin")
    grouping = commands.add_parser("partition", help="Derive provisional ownership units, not historical TU claims")
    grouping.add_argument("--root", type=Path, default=Path.cwd())
    grouping.add_argument("--group", help="For example class:InfoWin or component:zlib")
    grouping.add_argument("--details", action="store_true", help="Include individual source candidates")
    grouping.add_argument("--impact", action="store_true", help="Include conservative structural recheck scope")
    tasks = commands.add_parser("tasks", help="Scope recovery work and preserve structural discoveries")
    task_actions = tasks.add_subparsers(dest="action", required=True)
    create = task_actions.add_parser("create")
    create.add_argument("task")
    create.add_argument("--root", type=Path, default=Path.cwd())
    create.add_argument("--group", required=True)
    create.add_argument("--intent", required=True)
    create.add_argument("--author", required=True)
    create.add_argument("--evidence", action="append", required=True)
    create.add_argument("--edit-file", action="append", help="Explicit additional source editing scope")
    status = task_actions.add_parser("status")
    status.add_argument("task")
    status.add_argument("--root", type=Path, default=Path.cwd())
    context = task_actions.add_parser("context", help="Rebuild a source-first task brief; no stored context ledger")
    context.add_argument("task")
    context.add_argument("--root", type=Path, default=Path.cwd())
    obligations = task_actions.add_parser("obligations", help="Derive restoration requirements from source history; not measurements")
    obligations.add_argument("task")
    obligations.add_argument("--root", type=Path, default=Path.cwd())
    layout = task_actions.add_parser("check-layout", help="Compile and compare declared member offsets; not full structural verification")
    layout.add_argument("task")
    layout.add_argument("--root", type=Path, default=Path.cwd())
    task_binary = task_actions.add_parser("check-binary", help="Compile task TUs and check bounded code/references; not an admission gate")
    task_binary.add_argument("task")
    task_binary.add_argument("--root", type=Path, default=Path.cwd())
    task_binary.add_argument("--image", type=Path, required=True)
    task_binary.add_argument("--summary", action="store_true")
    discovery = task_actions.add_parser("discover")
    discovery.add_argument("task")
    discovery.add_argument("--root", type=Path, default=Path.cwd())
    discovery.add_argument("--request-id", required=True)
    discovery.add_argument("--author", required=True)
    discovery.add_argument("--hypothesis", required=True)
    discovery.add_argument("--affects", action="append", required=True, help="Original entity id, not a mutable name")
    discovery.add_argument("--evidence", action="append", required=True)
    discovery.add_argument("--expected-revision", type=int, required=True)
    expand = task_actions.add_parser("expand")
    expand.add_argument("task")
    expand.add_argument("--root", type=Path, default=Path.cwd())
    expand.add_argument("--request-id", required=True)
    expand.add_argument("--author", required=True)
    expand.add_argument("--subject", action="append", default=[], help="Additional original entity id")
    expand.add_argument("--edit-file", action="append", default=[])
    expand.add_argument("--evidence", action="append", required=True)
    expand.add_argument("--expected-revision", type=int, required=True)
    resolve = task_actions.add_parser("resolve")
    resolve.add_argument("task")
    resolve.add_argument("--root", type=Path, default=Path.cwd())
    resolve.add_argument("--discovery-id", required=True)
    resolve.add_argument("--request-id", required=True)
    resolve.add_argument("--author", required=True)
    resolve.add_argument("--disposition", choices=("incorporated", "rejected"), required=True)
    resolve.add_argument("--source-evidence", action="append", required=True)
    resolve.add_argument("--expected-revision", type=int, required=True)
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
    binary = actions.add_parser("check-binary", help="Verify original boundaries and resolved reference bytes")
    binary.add_argument("component")
    binary.add_argument("--root", type=Path, default=Path.cwd())
    binary.add_argument("--image", type=Path, required=True)
    binary.add_argument("--summary", action="store_true", help="Omit per-function relocation detail")
    history = commands.add_parser("history", help="Read durable execution events")
    history.add_argument("--root", type=Path, default=Path.cwd())
    history.add_argument("--stream", required=True)
    return result


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        if args.command == "run":
            from .driver import drive
            from .records import read_json
            output = drive(args.root, args.task, read_json(args.config), read_json(args.worker), iterations=args.iterations,
                           reviewer_configuration=read_json(args.reviewer) if args.reviewer else None,
                           migration_configuration=read_json(args.migration) if args.migration else None)
            print(json.dumps(output, indent=2, sort_keys=True))
            return (0 if output["ready_for_collection"] else
                    2 if output["stop_reason"] == "migration_admitted" else 3)
        if args.command == "tasks" and args.action == "check-layout":
            from .adapters.opensmacx_layout import verify_layout
            output = verify_layout(args.root, args.task)
            print(json.dumps(output, indent=2, sort_keys=True))
            return {Result.PASS: 0, Result.FAIL: 1, Result.UNKNOWN: 3}[output["result"]]
        if args.command == "tasks" and args.action == "obligations":
            from .adapters.opensmacx_obligations import task_obligations
            print(json.dumps(task_obligations(args.root, args.task), indent=2, sort_keys=True))
            return 0
        if args.command == "tasks" and args.action == "check-binary":
            from .adapters.opensmacx_binary import verify_task
            output = verify_task(args.root, args.task, args.image)
            if args.summary:
                output.pop("functions")
            print(json.dumps(output, indent=2, sort_keys=True))
            return {Result.PASS: 0, Result.FAIL: 1, Result.UNKNOWN: 3}[output["result"]]
        if args.command in ("partition", "tasks"):
            from .adapters.opensmacx_project import project_view, structural_impact
            from .model import Origin
            from .partition import select_group
            from .tasks import Task, TaskStore
            store = TaskStore(args.root, private=".opensmacx/recovery")
            if args.command == "tasks" and args.action == "discover":
                output = store.discover(args.task, request_id=args.request_id, author=args.author,
                                        hypothesis=args.hypothesis, affects=args.affects, evidence=args.evidence,
                                        expected_revision=args.expected_revision)
            elif args.command == "tasks" and args.action == "resolve":
                output = store.resolve(args.task, discovery_id=args.discovery_id, request_id=args.request_id,
                                       author=args.author, disposition=args.disposition,
                                       source_evidence=args.source_evidence, expected_revision=args.expected_revision)
            else:
                view, snapshot = project_view(args.root)
                if args.command == "tasks" and args.action == "expand":
                    entities = {entity["entity_id"]: entity for group in view["groups"] for entity in group["entities"]}
                    if not set(args.subject) <= entities.keys():
                        raise ValueError("scope expansion names an entity missing from current source")
                    output = store.expand(args.task, request_id=args.request_id, author=args.author,
                                          subjects=(Origin(**entities[subject]["origin"]) for subject in args.subject),
                                          editable_files=args.edit_file, evidence=args.evidence,
                                          expected_revision=args.expected_revision)
                elif args.command == "tasks" and args.action == "status":
                    output = store.status(args.task, (entity["entity_id"] for group in view["groups"]
                                                     for entity in group["entities"]))
                elif args.command == "tasks" and args.action == "context":
                    from .adapters.opensmacx_obligations import task_obligations
                    scope, files = store.scope(args.task)
                    selected = [entity for group in view["groups"] for entity in group["entities"]
                                if entity["entity_id"] in scope]
                    group = {"entities": selected, "source_homes": sorted(files)}
                    output = {"task": store.read(args.task).to_dict(), "subjects": selected,
                              "decision_state": store.status(args.task, (entity["entity_id"] for entity in selected)),
                              "impact": structural_impact(snapshot, group),
                              "restoration": task_obligations(args.root, args.task),
                              "binding_hints": [{"entity_id": row["entity_id"], "path": row["path"],
                                                 **row["binding_hint"]} for row in snapshot["records"]
                                                if row["entity_id"] in scope and row["binding_hint"]],
                              "instructions": ["Read source declarations, definitions and evidence before editing.",
                                               "Treat archived bodies and class/name labels as seeds, not mandatory source form.",
                                               "Report structural discoveries even when they affect other functions or units.",
                                               "Do not edit beyond authorized scope or discard matching obligations.",
                                               "A scope decision does not approve a regression; migration review and fresh checks are separate."],
                              "persisted": False}
                else:
                    groups = [select_group(view, args.group)] if args.group else view["groups"]
                    if args.command == "tasks":
                        group = groups[0]
                        if group["next_action"] != "review_ownership_and_structure":
                            raise ValueError("structural recovery tasks require an unambiguous source/class group")
                        changes = subprocess.run(["git", "status", "--porcelain", "--untracked-files=all", "--", "src"],
                                                 cwd=args.root, capture_output=True, text=True, check=True)
                        if changes.stdout.strip():
                            raise ValueError("checkpoint source changes before fixing a task's committed baseline")
                        roots = sorted(set(structural_impact(snapshot, group)["root_files"])
                                       | set(args.edit_file or []))
                        baseline = snapshot["revision"]
                        upstream = subprocess.run(["git", "merge-base", "HEAD", "master"], cwd=args.root,
                                                  capture_output=True, text=True, check=False)
                        if upstream.returncode == 0:
                            upstream_revision = upstream.stdout.strip()
                            scope_paths = sorted(set(roots) | {item["path"] for entity in group["entities"]
                                                             for item in entity["candidates"]})
                            equivalent = subprocess.run(["git", "diff", "--quiet", upstream_revision, "HEAD", "--", *scope_paths],
                                                        cwd=args.root, capture_output=True, check=False)
                            if equivalent.returncode == 0:
                                # Prefer an upstream ancestor when it contains the
                                # exact same scoped source: rebasing tooling commits
                                # must not strand the task's baseline reference.
                                baseline = upstream_revision
                            elif equivalent.returncode != 1:
                                raise ValueError("could not verify the task baseline source")
                        task = Task(args.task, "opensmacx", args.author, args.intent, baseline,
                                    tuple(Origin(**entity["origin"]) for entity in group["entities"]),
                                    tuple(roots), tuple(args.evidence))
                        output = {"created": store.create(task), "task": task.to_dict(),
                                  "verification": "not_measured"}
                    else:
                        if args.impact and not args.group:
                            raise ValueError("--impact needs one --group")
                        if args.impact:
                            groups[0]["impact"] = structural_impact(snapshot, groups[0])
                        if not args.details:
                            groups = [{key: value for key, value in group.items() if key != "entities"} for group in groups]
                        output = {**view, "groups": groups}
            print(json.dumps(output, indent=2, sort_keys=True))
            return 0
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
            if args.action == "check-binary":
                from .component_binary import verify_component
                from .adapters.opensmacx_components import compile_vendor
                result = verify_component(args.root, selected[0], args.image.read_bytes(),
                                          lambda path, flags: compile_vendor(args.root, path, flags))
                if read_components(args.root / "src") != manifests:
                    raise ValueError("component annotations changed during measurement")
                if args.summary:
                    result.pop("functions")
            else:
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
    except (OSError, ValueError, ImportError, subprocess.SubprocessError, tarfile.TarError) as error:
        print(f"recover: {error}", file=sys.stderr)
        return 3


if __name__ == "__main__":
    sys.exit(main())
