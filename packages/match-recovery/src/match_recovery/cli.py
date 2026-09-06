"""Installed target-independent CLI. Project commands live in the project."""

import argparse
from dataclasses import asdict
import json
from pathlib import Path
import subprocess
import sys
from typing import Sequence

from . import __version__
from .journal import Journal
from .records import read_json
from .tasks import Task, TaskStore


def parser() -> argparse.ArgumentParser:
    result = argparse.ArgumentParser(prog="recover", description="Structure-led matching recovery")
    result.add_argument("--version", action="version", version=__version__)
    commands = result.add_subparsers(dest="command", required=True)
    init = commands.add_parser("init", help="Create non-overwriting project-local command configuration")
    init.add_argument("--root", type=Path, default=Path.cwd())
    init.add_argument("--target", required=True, help="Project-chosen identity; no registered targets required")
    init.add_argument("--python", default=sys.executable, help="Interpreter for the local starter adapter")
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
        if args.command == "init":
            from .project import initialize
            output = initialize(args.root, args.target, args.python)
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
    except (OSError, ValueError, ImportError, subprocess.SubprocessError) as error:
        print(f"recover: {error}", file=sys.stderr)
        return 3


if __name__ == "__main__":
    sys.exit(main())
