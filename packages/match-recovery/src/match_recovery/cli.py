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
    agent = commands.add_parser("agent", help="Bridge a configured prompt-driven command to the worker/reviewer protocol")
    agent.add_argument("role", choices=("worker", "reviewer"))
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
    run.add_argument("--iterations", type=int, default=3)
    from .project_commands import register
    register(commands)
    history = commands.add_parser("history", help="Read durable decision events")
    history.add_argument("--root", type=Path, default=Path.cwd())
    history.add_argument("--stream", required=True)
    return result


def main(argv: Sequence[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        if args.command == "agent":
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
        elif args.command == "run":
            from .driver import drive
            # All relative configuration paths belong to --root, not whichever
            # directory the installed executable happened to be invoked from.
            config = lambda path: read_json(path if path.is_absolute() else args.root / path)
            output = drive(args.root, args.task, config(args.config), config(args.worker), iterations=args.iterations,
                           reviewer_configuration=config(args.reviewer) if args.reviewer else None,
                           migration_configuration=config(args.migration) if args.migration else None)
            print(json.dumps(output, indent=2, sort_keys=True))
            return (0 if output["ready_for_collection"] else
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
