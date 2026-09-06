"""Public recovery operations and the project-local JSON command boundary."""

from pathlib import Path

from .driver import Command, invoke
from .records import read_json


def register(commands):
    inventory = commands.add_parser("inventory", help="Inspect configured source and seed annotations without altering them")
    inventory.add_argument("--root", type=Path, default=Path.cwd())
    inventory.add_argument("--image", type=Path, help="Verify the original against its pin")
    grouping = commands.add_parser("partition", help="Derive provisional ownership units, not historical TU claims")
    grouping.add_argument("--root", type=Path, default=Path.cwd())
    grouping.add_argument("--group", help="A source-derived class or component group identity")
    grouping.add_argument("--details", action="store_true", help="Include individual source candidates")
    grouping.add_argument("--impact", action="store_true", help="Include conservative structural recheck scope")
    tasks = commands.add_parser("tasks", help="Scope recovery work and preserve structural discoveries")
    task_actions = tasks.add_subparsers(dest="action", required=True)
    create = task_actions.add_parser("create")
    create.add_argument("task", nargs="?")
    create.add_argument("--root", type=Path, default=Path.cwd())
    create.add_argument("--group")
    create.add_argument("--target", help="Task target; defaults to recovery/driver.json")
    create.add_argument("--base", help="Full ancestor revision with equivalent source/configuration; defaults to HEAD")
    create.add_argument("--file", type=Path, help="Explicit generic task decision instead of a source-derived group")
    create.add_argument("--intent")
    create.add_argument("--author")
    create.add_argument("--evidence", action="append")
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
    layout = task_actions.add_parser("check-layout", help="Check declared member offsets and nonvirtual bases; not full structural verification")
    layout.add_argument("task")
    layout.add_argument("--root", type=Path, default=Path.cwd())
    layout.add_argument("--summary", action="store_true", help="Keep coverage and verdicts, omit repeated per-member observations")
    task_binary = task_actions.add_parser("check-binary", help="Compile task TUs and check bounded code/references; not an admission gate")
    task_binary.add_argument("task")
    task_binary.add_argument("--root", type=Path, default=Path.cwd())
    task_binary.add_argument("--image", type=Path, required=True)
    task_binary.add_argument("--summary", action="store_true")
    task_check = task_actions.add_parser("check", help="Joint scoped binary and layout evidence; not full acceptance")
    task_check.add_argument("task")
    task_check.add_argument("--root", type=Path, default=Path.cwd())
    task_check.add_argument("--image", type=Path, required=True)
    task_check.add_argument("--summary", action="store_true")
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
    importing = commands.add_parser("import", help="Import mappings or external evidence")
    importing.add_argument("provider", help="Configured source/evidence provider")
    importing.add_argument("--root", type=Path, default=Path.cwd())
    importing.add_argument("--image", type=Path)
    importing.add_argument("--archive", type=Path)
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


def invoke_project(root: Path, arguments: dict) -> tuple[int, object]:
    """Native configured source operations, otherwise the local JSON boundary."""
    root = root.resolve()
    configuration = read_json(root / "recovery/project.json")
    if (configuration.keys() - {"schema_version", "adapter", "source"}
            or type(configuration.get("schema_version")) is not int or configuration["schema_version"] != 1):
        raise ValueError("invalid local project command configuration")
    if "source" in configuration and arguments.get("command") in ("inventory", "partition"):
        if arguments["command"] == "inventory":
            from .source_inventory import inventory, source_snapshot
            image = arguments.get("image")
            if image is not None:
                image = Path(image)
                image = image if image.is_absolute() else root / image
            return 0, inventory(source_snapshot(root, image))
        from .ownership_source import partition_report
        return 0, partition_report(root, group=arguments.get("group"), details=arguments.get("details", False),
                                   impact=arguments.get("impact", False))
    if "source" in configuration and arguments.get("command") == "tasks":
        if arguments.get("action") == "create":
            from .task_source import create_task
            return 0, create_task(root, name=arguments["task"], group=arguments["group"],
                                  author=arguments["author"], intent=arguments["intent"], evidence=arguments["evidence"],
                                  editable_files=arguments.get("edit_file") or (),
                                  target=arguments.get("target"), base=arguments.get("base"))
        if arguments.get("action") == "expand":
            from .task_source import expand_task
            return 0, expand_task(root, name=arguments["task"], subjects=arguments["subject"],
                                  editable_files=arguments["edit_file"],
                                  **{key: arguments[key] for key in ("request_id", "author", "evidence", "expected_revision")})
    if "adapter" not in configuration:
        raise ValueError("this operation needs a configured project adapter")
    return _invoke_adapter(root, configuration, arguments)


def _invoke_adapter(root: Path, configuration: dict, arguments: dict) -> tuple[int, object]:
    command = Command.read(configuration["adapter"])
    arguments = {key: str(value) if isinstance(value, Path) else value for key, value in arguments.items()}
    code, response = invoke(command, root, arguments.get("task") or "",
                            {"schema_version": 1, "arguments": arguments})
    if set(response) != {"output"}:
        raise ValueError("project adapter must return an output envelope")
    if code == 3 and isinstance(response["output"], dict) and set(response["output"]) == {"error"}:
        raise ValueError(response["output"]["error"])
    return code, response["output"]
