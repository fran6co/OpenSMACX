"""Task-scoped native application residual, retaining all physical obligations.

This composes source body scope with full-input code/data observation. It cannot
yet qualify outside-scope preservation: anonymous sections, retained selection,
allocation isolation and linker-generated contributions still need completion.
"""

from .model import Result


def _owned_bodies(context, graph, bindings):
    """Bind task names to exact native single-function regions, not whole TUs.

    These private inputs come from the original live task/source authority. No
    header-copy or ordinary multi-entry section is an inferred function body.
    Incoming references and alias relationships retain each stable task identity.
    """
    from .linked_code_ownership import identities, _Inputs, _contents
    from .pe_coff import Coff

    producers = identities(context, graph["nodes"])
    inputs = _Inputs()
    result = []
    try:
        for node in graph["nodes"]:
            producer = producers[node["id"]]
            if producer["kind"] != "compiled":
                continue
            allowed = bindings.get(producer["unit"], {})
            entity = allowed.get(producer["symbol"])
            if entity is None:
                continue
            obj = Coff(_contents(node["input"], node["object_sha256"], inputs))
            subject = obj.function(producer["symbol"])
            numbers = obj.code_sections(subject)
            if (len(numbers) != 1 or numbers[0] != subject.section
                    or not obj.sections[subject.section - 1].flags & 0x1000
                    or subject.section in obj.associations.values()):
                raise ValueError("task function body requires one independent code section; shared regions are not whole-body permissions")
            # A named function cannot absorb co-located externally usable code.
            # The compiled residual also independently protects all unowned
            # sections and metadata; this dimension supplies no header waiver.
            for other in obj.symbols.values():
                if (other.section == subject.section and other.index != subject.index
                        and (other.storage == 2 or other.type & 0x20)):
                    raise ValueError("task body shares its physical code section with another definition")
            result.append((producer, {"kind": "task_function", "entity_id": entity}))
        return result
    finally:
        try:
            inputs.check_current()
        finally:
            context.check_current()


def compare_application_residual(graph):
    """Re-derive current task/history; serialized reports cannot supply scope."""
    from .build_graph import require_bound_graph
    from .build_graph_commands import compare_build_commands
    from .generated_code_residual import _compare_contexts
    from .generated_permissions import generated_sources
    from .integration import preservation_base
    from .measurement import Measurement
    from .preservation import _bindings, _unit_scope
    from .source_history import revision_files
    from .source_inventory import image_pin
    from .source_replica import replica_roots, replica_snapshot
    from .tasks import TaskStore

    require_bound_graph(graph)
    try:
        store, settings = TaskStore(graph.root), Measurement.read(graph.root)
        task, decisions = store.read(graph.task_name), store.decisions(graph.task_name)
        selected, editable = store._scope(task, decisions)
        authority = graph.authorization
        expected = {"task": task.to_dict(), "decisions": decisions, "integration": authority.get("integration")}
        if "permission" in authority:
            expected["permission"] = authority["permission"]
        if (authority != expected
                or graph.baseline_revision != preservation_base(graph.root, task, authority.get("integration"))
                or graph.roots != replica_roots(settings.source.roots)):
            raise ValueError("application residual task authority differs from its live paired graph")
        sources = {"baseline": revision_files(graph.root, graph.baseline_revision, graph.roots),
                   "current": {name: raw for name, raw in replica_snapshot(graph.root, graph.roots).items()
                               if raw is not None}}
        if sources != graph.source_versions:
            raise ValueError("application residual sources differ from original task history")
        pin = image_pin(graph.root, settings.source)
        scoped = {}
        for version, files in sources.items():
            bindings = _bindings(graph.root, files, settings.source, pin)
            scoped[version] = {unit: _unit_scope(bindings, selected, unit)[0] for unit in files}
        changes = []
        if "permission" in authority:
            changes = generated_sources(authority["permission"], task, selected, editable,
                                        sources, settings, pin, graph.root)
        commands = compare_build_commands(graph.baseline, graph.current, editable=editable)
        report = {"check": "application_task_residual", "result": Result.UNKNOWN,
                  "outside_scope_verification": Result.UNKNOWN, "scope_complete": False,
                  "commands": commands, "ready_for_collection": False, "persisted": False}
        if commands["result"] != Result.PASS:
            report["reason"] = "paired application commands lack complete correspondence"
            return report
        report["scoped_code"] = _compare_contexts(graph.baseline, graph.current, changes,
                                                  application_scope=scoped)
        report["reason"] = ("physical code/data observations do not yet establish complete retained section "
                            "selection, anonymous contribution assignment or allocation isolation")
        return report
    finally:
        require_bound_graph(graph)
