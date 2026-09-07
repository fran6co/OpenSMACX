"""Public recovery operations and the project-local JSON command boundary."""

from pathlib import Path

from .driver import Command, invoke
from .records import read_json


def register(commands):
    gate = commands.add_parser("check", help="Run the whole configured project gate; incomplete evidence is not green")
    gate.add_argument("--root", type=Path, default=Path.cwd())
    gate.add_argument("--image", type=Path, required=True)
    gate.add_argument("--jobs", type=int, default=1, help="Concurrent match TUs; must fit isolated compiler capacity")
    gate.add_argument("--summary", action="store_true")
    matches = commands.add_parser("check-matches", help="Remeasure all current source claims and product candidates; not full acceptance")
    matches.add_argument("--root", type=Path, default=Path.cwd())
    matches.add_argument("--image", type=Path, required=True)
    matches.add_argument("--jobs", type=int, default=1, help="Concurrent TUs; must fit isolated compiler capacity")
    matches.add_argument("--summary", action="store_true")
    layout = commands.add_parser("check-layout", help="Freshly check source-owned layout expectations across product TUs; not full structure proof")
    layout.add_argument("--root", type=Path, default=Path.cwd())
    layout.add_argument("--unit", dest="units", action="append", help="Exact TU diagnostic; omitted product units keep coverage incomplete")
    layout.add_argument("--summary", action="store_true")
    symbols = commands.add_parser("check-symbols", help="Freshly verify source-mapped function names through configured compilation")
    symbols.add_argument("--root", type=Path, default=Path.cwd())
    symbols.add_argument("--in", dest="files", action="append", help="Exact product source path; omit for all product mappings")
    symbols.add_argument("--summary", action="store_true")
    data = commands.add_parser("check-data", help="Compare source-bounded object initialization and reference targets; not lifetime proof")
    data.add_argument("--root", type=Path, default=Path.cwd())
    data.add_argument("--image", type=Path, required=True)
    data.add_argument("--symbol", dest="symbols", action="append", help="Select exact emitted object symbols; default checks all source object bindings")
    data.add_argument("--summary", action="store_true")
    recording = commands.add_parser("record", help="Measure and publish historical byte obligations; not structural acceptance")
    recording.add_argument("targets", nargs="+")
    recording.add_argument("--root", type=Path, default=Path.cwd())
    recording.add_argument("--image", type=Path, required=True)
    recording.add_argument("--in", dest="in_file", help="Exact project-relative source mapping host")
    recording.add_argument("--arguments", dest="arguments_file", type=Path,
                           help="Project-relative JSON compiler argv array; default uses configured compiler recipes")
    recording.add_argument("--jobs", type=int, default=1, help="Concurrent TUs; caller must provide isolated compiler capacity")
    claims = commands.add_parser("check-claims", help="Check source-owned obligation identities against history; not current matching")
    claims.add_argument("--root", type=Path, default=Path.cwd())
    claims.add_argument("--summary", action="store_true")
    mappings = commands.add_parser("check-mappings", help="Check source identity and intended build ownership; not binary proof")
    mappings.add_argument("--root", type=Path, default=Path.cwd())
    mappings.add_argument("--summary", action="store_true")
    source = commands.add_parser("check-source", help="Verify annotation text integrity without compiling or stamping")
    source.add_argument("--root", type=Path, default=Path.cwd())
    source.add_argument("--jobs", type=int, default=1)
    source.add_argument("--summary", action="store_true")
    build = commands.add_parser("check-build", help="Run configured commands in a fresh ignored build directory; not recovery acceptance")
    build.add_argument("--root", type=Path, default=Path.cwd())
    build.add_argument("--summary", action="store_true")
    audits = commands.add_parser("audits", help="Recompute configured source-policy ratchets; not structural proof")
    audit_actions = audits.add_subparsers(dest="action", required=True)
    source_audit = audit_actions.add_parser("check")
    source_audit.add_argument("audit")
    source_audit.add_argument("--root", type=Path, default=Path.cwd())
    source_audit.add_argument("--summary", action="store_true")
    source_audit.add_argument("--in", dest="files", action="append", help="Select source detail only; counts and reference analysis remain whole-product")
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
    restoration = task_actions.add_parser("check-restoration", help="Remeasure historical obligations alongside scoped checks; never waive regressions")
    restoration.add_argument("task")
    restoration.add_argument("--root", type=Path, default=Path.cwd())
    restoration.add_argument("--image", type=Path, required=True)
    restoration.add_argument("--summary", action="store_true")
    preservation = task_actions.add_parser("check-preservation", help="Compare baseline/current output outside task-owned functions; not migration admission")
    preservation.add_argument("task")
    preservation.add_argument("--root", type=Path, default=Path.cwd())
    preservation.add_argument("--unit", action="append", help="Diagnostic subset; omitted units keep the overall result unknown")
    preservation.add_argument("--diagnostic", action="store_true", help="Inspect fresh compiler differences without per-file damage controls; always unverified")
    preservation.add_argument("--summary", action="store_true")
    variants = task_actions.add_parser("variants", help="Score authorized source candidates without editing or banking them")
    variants.add_argument("task")
    variants.add_argument("--root", type=Path, default=Path.cwd())
    variants.add_argument("--image", type=Path, required=True)
    variants.add_argument("--deck", type=Path, required=True, help="Exact-text replacements with current source digest and a damage control")
    variants.add_argument("--jobs", type=int, default=1, help="Compiler concurrency; must fit the project's isolated compiler capacity")
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
    if arguments.get("command") == "record":
        from .recording import record
        return record(root, arguments["targets"], image=arguments["image"], in_file=arguments.get("in_file"),
                      arguments_file=arguments.get("arguments_file"), jobs=arguments.get("jobs", 1))
    """Native configured source operations, otherwise the local JSON boundary."""
    root = root.resolve()
    configuration = read_json(root / "recovery/project.json")
    if (configuration.keys() - {"schema_version", "adapter", "source", "source_history", "compiler", "measurement", "audits", "build", "mappings", "gate"}
            or type(configuration.get("schema_version")) is not int or configuration["schema_version"] != 1):
        raise ValueError("invalid local project command configuration")
    if arguments.get("command") == "check":
        import sys
        from .gate import check_project, summary
        def progress(event):
            print(f"{event['check']}: {event.get('result', 'checking')}", file=sys.stderr, flush=True)
        output = check_project(root, arguments["image"], jobs=arguments.get("jobs", 1), progress=progress)
        if arguments.get("summary"):
            output = summary(output)
        return output["exit_code"], output
    if arguments.get("command") == "check-matches":
        from .match_check import check_matches
        output = check_matches(root, arguments["image"], jobs=arguments.get("jobs", 1))
        if arguments.get("summary"):
            output["measured"] = len(output.pop("measurements"))
        return {"pass": 0, "fail": 1, "unknown": 3}[output["result"]], output
    if arguments.get("command") == "check-layout":
        from .layout_check import check_layout
        output = check_layout(root, units=arguments.get("units"))
        if arguments.get("summary"):
            output["expectations"] = [row for row in output["expectations"] if row["result"] != "pass"]
            output["observations"] = [row for row in output["observations"] if row["result"] != "pass"]
            output.pop("coverage")
        return {"pass": 0, "fail": 1, "unknown": 3}[output["result"]], output
    if arguments.get("command") == "check-symbols":
        from .symbol_check import check_symbols
        output = check_symbols(root, files=arguments.get("files"))
        if arguments.get("summary"):
            output["functions"] = [row for row in output["functions"] if row["result"] != "pass"]
            output.pop("trials")
        return {"pass": 0, "fail": 1, "unknown": 3}[output["result"]], output
    if arguments.get("command") == "check-mappings":
        from .mapping_check import check_mappings
        output = check_mappings(root)
        if arguments.get("summary"):
            output["seed_alternative_groups"] = len(output.pop("seed_alternatives"))
            output["forwarder_sites"] = len(output.pop("forwarders"))
            output.pop("findings")
        return {"pass": 0, "fail": 1, "unknown": 3}[output["result"]], output
    if arguments.get("command") == "check-data":
        from .data_check import check_data
        output = check_data(root, arguments["image"], symbols=arguments.get("symbols"))
        if arguments.get("summary"):
            output.pop("objects")
        return {"pass": 0, "fail": 1, "unknown": 3}[output["result"]], output
    if arguments.get("command") == "check-claims":
        from .claim_integrity import check_claims
        output = check_claims(root)
        output["missing_requirements"] = len(output["missing"])
        if arguments.get("summary"):
            output.pop("missing")
        return {"pass": 0, "fail": 1, "unknown": 3}[output["result"]], output
    if arguments.get("command") == "check-source":
        from .source_integrity import check_source
        output = check_source(root, jobs=arguments.get("jobs", 1))
        if arguments.get("summary"):
            output.pop("findings")
        return {"pass": 0, "fail": 1, "unknown": 3}[output["result"]], output
    if arguments.get("command") == "check-build":
        from .build_check import check_build
        output = check_build(root)
        if arguments.get("summary"):
            for command in output["commands"]:
                command.pop("stdout", None)
                command.pop("stderr", None)
        return {"pass": 0, "fail": 1, "unknown": 3}[output["result"]], output
    if arguments.get("command") == "audits":
        from .source_audits import audit_source
        output = audit_source(root, arguments["audit"], files=arguments.get("files"))
        if arguments.get("summary"):
            output.pop("per_file")
            output.pop("explanations")
            output.pop("findings")
        return (0 if output["result"] == "pass" else 1), output
    if "source" in configuration and arguments.get("command") == "components":
        from .component_commands import execute
        return execute(root, arguments)
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
        if arguments.get("action") == "check-preservation":
            from .preservation import check_preservation
            units = tuple(arguments["unit"]) if arguments.get("unit") is not None else None
            output = check_preservation(root, arguments["task"], units=units, diagnostic=arguments.get("diagnostic", False))
            if arguments.get("summary"):
                output["units"] = [{key: value for key, value in row.items()
                                    if key not in ("baseline_scoped_functions", "current_scoped_functions", "changed_sections", "symbol_changes")}
                                   for row in output["units"]]
            return {"pass": 0, "fail": 1, "unknown": 3}[output["result"]], output
        if arguments.get("action") == "check-restoration":
            from .restoration import measure_restoration
            image = Path(arguments["image"])
            output = measure_restoration(root, arguments["task"], image if image.is_absolute() else root / image)
            if arguments.get("summary"):
                output.pop("candidate")
                output.pop("measurements")
                output.pop("current")
                output["structural_requirements"] = [{key: value for key, value in row.items() if key != "trials"}
                                                      for row in output["structural_requirements"]]
            return {"pass": 0, "fail": 1, "unknown": 3}[output["result"]], output
        if arguments.get("action") == "variants":
            from .variants import measure_variants
            paths = {name: Path(arguments[name]) for name in ("image", "deck")}
            paths = {name: path if path.is_absolute() else root / path for name, path in paths.items()}
            output = measure_variants(root, arguments["task"], paths["image"], read_json(paths["deck"]), jobs=arguments.get("jobs", 1))
            return (0 if output["status"] == "scored" else 3), output
        if arguments.get("action") in ("check", "check-layout", "check-binary"):
            from .model import Result
            image = arguments.get("image")
            if image is not None:
                image = Path(image)
                image = image if image.is_absolute() else root / image
            action = arguments["action"]
            if action == "check":
                from .application_check import check_task
                output = check_task(root, arguments["task"], image)
                summaries = ((output["checks"]["binary"], ("functions",)),
                             (output["checks"]["layout"], ("members", "bases")))
            elif action == "check-layout":
                from .application_layout import verify_layout
                output = verify_layout(root, arguments["task"])
                summaries = ((output, ("members", "bases")),)
            else:
                from .application_binary import verify_task
                output = verify_task(root, arguments["task"], image)
                summaries = ((output, ("functions",)),)
            if arguments.get("summary"):
                for report, fields in summaries:
                    for field in fields:
                        report.pop(field, None)
            return {Result.PASS: 0, Result.FAIL: 1, Result.UNKNOWN: 3}[output["result"]], output
        if arguments.get("action") in ("obligations", "context"):
            from .source_obligations import task_obligations
            if arguments["action"] == "obligations":
                return 0, task_obligations(root, arguments["task"])
            from .task_source import task_context
            return 0, task_context(root, arguments["task"], restoration_for=task_obligations)
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
