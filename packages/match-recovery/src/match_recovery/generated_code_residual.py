"""Native generated-root code preservation, not a linked-effect waiver.

Omitting a selected initializer body never prunes its former dependencies.
Both builds must independently place and verify their union to a fixed point.
Source-scoped helper bodies, storage and runtime effects gain no exemption.
"""

from contextlib import ExitStack
from copy import deepcopy
import json

from .initializer_order import _compare_order, _project, _LocalRoots
from .linked_context import linked_build
from .linked_code_ownership import identities, resolve
from .linked_code_residual import normalize_residual, compare_residual
from .model import Result


def _key(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False)


def _complete(graph):
    if graph.get("result") != Result.PASS:
        return Result.FAIL if graph.get("result") == Result.FAIL else Result.UNKNOWN
    return Result.PASS


def _compare_contexts(baseline, current, changes, *, max_nodes=4096, max_rounds=8):
    """Internal native code dimension; caller separately validates task authority."""
    if (type(max_nodes) is not int or not 1 <= max_nodes <= 4096
            or type(max_rounds) is not int or not 1 <= max_rounds <= 64):
        raise ValueError("generated residual requires bounded positive node/round limits")
    contexts = {"baseline": baseline, "current": current}
    result = {"check": "application_generated_code_residual", "result": Result.UNKNOWN,
              "remaining_linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
              "limitations": ["exact authorized local initializer bodies only; other task functions gain no body waiver",
                              "unreachable dependencies still need independent placement in both builds; absence is not discard proof",
                              "code correspondence is not data contents, allocation, dependency selection or runtime acceptance"]}

    def fresh():
        try:
            baseline.check_current()
        finally:
            current.check_current()

    try:
        fresh()
        with ExitStack() as stack:
            live = {version: stack.enter_context(linked_build(context)) for version, context in contexts.items()}
            observed = {version: value.initializers() for version, value in live.items()}
            checks = [_complete(value) for value in observed.values()]
            checks += [_complete(value.get("code_dependencies", {})) for value in observed.values()]
            if any(value != Result.PASS for value in checks):
                result.update(result=Result.FAIL if Result.FAIL in checks else Result.UNKNOWN,
                              reason="initializer or reachable code observation is incomplete", initializers=observed)
                return result
            order = _compare_order(baseline, current, observed["baseline"], observed["current"], changes)
            result["projected_order"] = order
            if order["result"] != Result.PASS:
                result.update(result=order["result"], reason="unowned initializer order is not preserved")
                return result

            # Only an independently proven slot-local rename connects different
            # spellings. Source entity anchors do not rename arbitrary helpers.
            renames = {version: {} for version in contexts}
            reverse = {version: {} for version in contexts}
            for rename in order.get("root_renamings", ()):
                stable = {"kind": "initializer_occurrence", "unit": rename["unit"], "index": rename["index"]}
                for version in contexts:
                    original = {"kind": "compiled", "unit": rename["unit"], "symbol": rename[version + "_symbol"]}
                    if _key(original) in renames[version] or _key(stable) in reverse[version]:
                        raise ValueError("initializer renaming has ambiguous source correspondence")
                    renames[version][_key(original)] = stable
                    reverse[version][_key(stable)] = original

            omitted, graphs, producers = {}, {}, {}
            for version, context in contexts.items():
                selected = [row for row in changes if row["version"] == version]
                _, projection = _project(context, observed[version], selected)
                locality = _LocalRoots(context, observed[version], projection) if selected else None
                omitted[version] = set()
                for change in selected:
                    # Order selection alone cannot remove aliases, protected
                    # source bindings, arbitrary children or shared roots.
                    locality.check(change["unit"], change["symbol"])
                    identity = {"kind": "compiled", "unit": change["unit"], "symbol": change["symbol"]}
                    omitted[version].add(_key(identity))
                graphs[version] = observed[version]["code_dependencies"]

            def index(version):
                original = identities(contexts[version], graphs[version]["nodes"])
                if not omitted[version] <= {_key(value) for value in original.values()}:
                    raise ValueError("authorized root disappeared from its own native graph")
                producers[version] = {node: renames[version].get(_key(value), value)
                                      for node, value in original.items()}
                return {node for node, value in original.items() if _key(value) in omitted[version]}

            removed = {version: index(version) for version in contexts}
            from .generated_storage_residual import StorageClosure
            from .linked_data_ownership import section_producers
            data_producers = {version: stack.enter_context(section_producers(context, live[version].inputs.objects))
                              for version, context in contexts.items()}
            storage = StorageClosure(live, data_producers, limit=max_nodes)
            initial_producers = {version: {_key(value) for value in producers[version].values()}
                                 for version in contexts}

            def union():
                values = {_key(value): value for version in contexts for node, value in producers[version].items()
                          if node not in removed[version]}
                values.update(storage.code_required)
                if len(values) > max_nodes:
                    raise ValueError("generated residual producer union exceeds its node bound")
                return values

            storage_report = storage.observe(graphs, producers)
            result["storage_residual"] = storage_report
            required = union()
            for round_number in range(1, max_rounds + 1):
                for version, context in contexts.items():
                    native = live[version]
                    native.check_current()
                    # The original complete graph already independently places
                    # its own producers in this exact build. Only cross-version
                    # additions need extra roots. Requeuing an already observed
                    # ICF member under its original owner would duplicate its
                    # map-owner alias, which the graph correctly refuses.
                    missing = sorted(required.keys() - initial_producers[version])
                    targets = [reverse[version].get(key, required[key]) for key in missing]
                    if not targets:
                        native.check_current()
                        continue
                    roots = resolve(context, targets, native.inputs.objects, native.inputs.mapping, native.inputs.folding)
                    native.check_current()
                    # Initial roots remain observed too. Their body omission is
                    # version-specific and occurs only after all proofs pass.
                    roots = [row["function"] for row in observed[version]["sequence"]] + list(roots)
                    graphs[version] = native.code(roots, max_functions=max_nodes)
                    if _complete(graphs[version]) != Result.PASS:
                        result.update(result=_complete(graphs[version]), reason="expanded dependency code is unverified",
                                      version=version, graph=graphs[version], rounds=round_number)
                        return result
                    removed[version] = index(version)
                storage_report = storage.observe(graphs, producers)
                result["storage_residual"] = storage_report
                expanded = union()
                if not required.keys() <= expanded.keys():
                    raise ValueError("a required original producer vanished during dependency expansion")
                if required.keys() == expanded.keys():
                    normalized = {version: normalize_residual(graphs[version], producers[version], removed[version])
                                  for version in contexts}
                    comparison = compare_residual(normalized["baseline"], normalized["current"])
                    result.update(result=comparison["result"], code_residual=comparison,
                                  rounds=round_number, preserved_producers=len(expanded),
                                  omitted_bodies={version: len(value) for version, value in removed.items()},
                                  residuals=normalized,
                                  other_dependencies={version: {key: deepcopy(graphs[version][key]) for key in
                                      ("data_references", "import_references")} for version in contexts})
                    return result
                required = expanded
            raise ValueError("generated residual dependency closure did not converge within its round bound")
    except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
        result.update(result=Result.UNKNOWN, reason=str(error))
        return result
    finally:
        fresh()


def compare_generated_code_residual(graph):
    """Re-derive source authority; never consume an earlier diagnostic as scope."""
    from .build_graph import ApplicationGraph, require_bound_graph
    from .build_graph_commands import compare_build_commands
    from .generated_permissions import generated_sources
    from .integration import preservation_base
    from .measurement import Measurement
    from .source_inventory import image_pin
    from .source_history import revision_files
    from .source_replica import replica_snapshot, replica_roots
    from .tasks import TaskStore

    if not isinstance(graph, ApplicationGraph):
        raise ValueError("generated residual requires a live native application graph")
    require_bound_graph(graph)
    try:
        store, settings = TaskStore(graph.root), Measurement.read(graph.root)
        task, decisions = store.read(graph.task_name), store.decisions(graph.task_name)
        scope, editable = store._scope(task, decisions)
        authority = graph.authorization
        permission = authority.get("permission")
        if (not isinstance(permission, dict) or not permission.get("generated_changes")
                or authority != {"task": task.to_dict(), "decisions": decisions,
                                 "integration": authority.get("integration"), "permission": permission}
                or graph.baseline_revision != preservation_base(graph.root, task, authority.get("integration"))
                or graph.roots != replica_roots(settings.source.roots)):
            raise ValueError("generated residual task authority differs from its live application graph")
        sources = {"baseline": revision_files(graph.root, graph.baseline_revision, graph.roots),
                   "current": {name: raw for name, raw in replica_snapshot(graph.root, graph.roots).items()
                               if raw is not None}}
        if sources != graph.source_versions:
            raise ValueError("generated residual source versions differ from original task history")
        changes = generated_sources(permission, task, scope, editable, sources, settings,
                                    image_pin(graph.root, settings.source), graph.root)
        commands = compare_build_commands(graph.baseline, graph.current, editable=editable)
        if commands["result"] != Result.PASS:
            return {"check": "application_generated_code_residual", "result": commands["result"],
                    "commands": commands, "remaining_linked_effects": Result.UNKNOWN, "ready_for_collection": False}
        return _compare_contexts(graph.baseline, graph.current, changes)
    finally:
        require_bound_graph(graph)
