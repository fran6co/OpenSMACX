"""Installed target-independent CLI with a configured project command boundary."""

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
from .records import read_json
from .tasks import Task, TaskStore


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(prog="recover", description="Structure-led matching recovery")
    result.add_argument("--version", action="version", version=__version__)
    commands = result.add_subparsers(dest="command", required=True)
    debug = commands.add_parser("debug-sidecar", help="Generate ELF32/DWARF debugger hints from an i386 PE, MSVC map and COFF objects")
    for option in ("exe", "map", "out", "build-root", "comp-dir"):
        debug.add_argument("--" + option, type=Path, required=True)
    debug.add_argument("--frame-base", choices=("none", "ebp"), default="none",
                       help="Emit locals only when the caller guarantees EBP-relative frames")
    debug.add_argument("--path-map", action="append", default=[], metavar="OLD=NEW",
                       help="Explicit source prefix translation, applied in argument order")
    debug.add_argument("--summary", action="store_true", help="Report diagnostic counts and bounded examples")
    agent = commands.add_parser("agent", help="Bridge a configured prompt-driven command to a recovery role protocol")
    agent.add_argument("role", choices=("worker", "reviewer", "coordinator", "planner"))
    agent.add_argument("--root", type=Path, default=Path.cwd())
    agent.add_argument("--config", type=Path, default=Path("recovery/agents.json"))
    init = commands.add_parser("init", help="Create non-overwriting project-local command configuration")
    init.add_argument("--root", type=Path, default=Path.cwd())
    init.add_argument("--target", required=True, help="Project-chosen identity; no registered targets required")
    init.add_argument("--python", default=sys.executable, help="Interpreter with match_recovery installed for native commands")
    init.add_argument("--image", type=Path, help="Original image to pin in source; relative to --root, never copied")
    init.add_argument("--source-root", default="src", help="Project-relative source directory; native starter measurements support PE/COFF i386")
    run = commands.add_parser("run", help="Drive bounded recovery through project-local JSON commands")
    run.add_argument("task")
    run.add_argument("--root", type=Path, default=Path.cwd())
    run.add_argument("--config", type=Path, default=Path("recovery/driver.json"))
    run.add_argument("--worker", type=Path, default=Path("recovery/worker.json"))
    run.add_argument("--reviewer", type=Path, help="Independent read-only reviewer command configuration")
    run.add_argument("--migration", type=Path, help="Coordinator permission for scoped temporary regressions")
    run.add_argument("--acceptance-scope", choices=("change", "full_project"), help="Override the configured acceptance boundary")
    run.add_argument("--coordinator", type=Path, help="Read-only discovery coordinator with explicit delegated scope")
    run.add_argument("--iterations", type=int, default=3)
    start = commands.add_parser("start", help="Select/checkpoint one ownership task through a delegated planner, then run recovery")
    start.add_argument("--root", type=Path, default=Path.cwd())
    start.add_argument("--planner", required=True, type=Path, help="Explicit selection and task-only commit authorization")
    start.add_argument("--config", type=Path, default=Path("recovery/driver.json"))
    start.add_argument("--worker", type=Path, default=Path("recovery/worker.json"))
    start.add_argument("--reviewer", type=Path)
    start.add_argument("--coordinator", type=Path)
    start.add_argument("--migration", type=Path)
    start.add_argument("--acceptance-scope", choices=("change", "full_project"))
    start.add_argument("--iterations", type=int, default=3)
    from .project_commands import register
    register(commands)
    history = commands.add_parser("history", help="Read durable decision events")
    history.add_argument("--root", type=Path, default=Path.cwd())
    history.add_argument("--stream", required=True)
    return result


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        if args.command == "debug-sidecar":
            from .debug_sidecar import build
            mappings = []
            for value in args.path_map:
                if "=" not in value:
                    raise ValueError("debug path mapping requires OLD=NEW")
                mappings.append(tuple(value.split("=", 1)))
            output = build(args.exe, args.map, args.out, args.comp_dir, args.build_root,
                           frame_base=args.frame_base, path_maps=mappings)
            if args.summary:
                notes = output.pop("diagnostics")
                output.update(diagnostic_count=len(notes), diagnostic_examples=notes[:8])
        elif args.command == "agent":
            from .agent_command import run_agent
            from .records import parse_json
            try:
                code, output = run_agent(args.root, args.role, args.config, parse_json(sys.stdin.read()))
            except (OSError, ValueError, subprocess.SubprocessError) as error:
                code, output = 3, {"error": {"kind": "refused", "message": str(error)}}
            print(json.dumps(output, indent=2, sort_keys=True))
            return code
        elif args.command == "init":
            from .project import initialize
            output = initialize(args.root, args.target, args.python, image=args.image, source_root=args.source_root)
        elif args.command in ("run", "start"):
            from .driver import drive
            # All relative configuration paths belong to --root, not whichever
            # directory the installed executable happened to be invoked from.
            config = lambda path: read_json(path if path.is_absolute() else args.root / path)
            options = dict(iterations=args.iterations,
                           reviewer_configuration=config(args.reviewer) if args.reviewer else None,
                           migration_configuration=config(args.migration) if args.migration else None,
                           coordinator_configuration=config(args.coordinator) if args.coordinator else None)
            configuration = config(args.config)
            if args.acceptance_scope is not None:
                configuration = {**configuration, "acceptance_scope": args.acceptance_scope}
            if args.command == "start":
                from .planning import start
                output = start(args.root, configuration, config(args.worker), config(args.planner), **options)
            else:
                output = drive(args.root, args.task, configuration, config(args.worker), **options)
            print(json.dumps(output, indent=2, sort_keys=True))
            return (0 if output["ready_for_collection"] or output["stop_reason"] == "change_accepted" else
                    2 if output["stop_reason"] == "migration_admitted" else 3)
        elif args.command == "tasks" and (args.action == "status" or (args.action == "create" and args.file)):
            store = TaskStore(args.root)
            if args.action == "create":
                task = Task.from_dict(read_json(args.file))
                output = {"created": store.create(task), "task": task.to_dict(), "verification": "not_measured"}
            else:
                scope, files = store.scope(args.task)
                output = {**store.status(args.task, scope), "task": store.read(args.task).to_dict(), "decisions": store.decisions(args.task),
                          "authorized_entities": sorted(scope), "editable_files": sorted(files),
                          "source_verification": "not_measured"}
        elif args.command == "tasks" and args.action in ("discover", "resolve"):
            store = TaskStore(args.root)
            common = {key: getattr(args, key) for key in ("request_id", "author", "expected_revision")}
            if args.action == "discover":
                output = store.discover(args.task, hypothesis=args.hypothesis, affects=args.affects,
                                        evidence=args.evidence, **common)
            else:
                output = store.resolve(args.task, discovery_id=args.discovery_id, disposition=args.disposition,
                                       source_evidence=args.source_evidence, **common)
        elif args.command == "history":
            output = [asdict(event) for event in Journal(args.root / "recovery/history").read(args.stream)]
        else:
            from .project_commands import invoke_project
            if args.command == "tasks" and args.action == "create" and not all((args.task, args.group, args.author, args.intent, args.evidence)):
                raise ValueError("task creation requires --file or a task, --group, --author, --intent and --evidence")
            code, output = invoke_project(args.root, vars(args))
            print(json.dumps(output, indent=2, sort_keys=True))
            return code
        print(json.dumps(output, indent=2, sort_keys=True))
        return 0
    except (OSError, ValueError, ImportError, subprocess.SubprocessError, tarfile.TarError) as error:
        print(f"recover: {error}", file=sys.stderr)
        return 3


if __name__ == "__main__":
    sys.exit(main())
