"""Project operation bridge during source-format and verifier migration.

The public CLI belongs to match_recovery. This module has no argument parser.
Reusable logic below is being moved behind configured source/environment APIs.
"""

from dataclasses import asdict
from pathlib import Path
import subprocess
from types import SimpleNamespace

from match_recovery.model import Result


def execute(root: Path, arguments: dict):
    args = SimpleNamespace(**arguments)
    args.root = root.resolve()
    for name in ("image", "archive"):
        value = getattr(args, name, None)
        if value is not None:
            path = Path(value)
            setattr(args, name, path if path.is_absolute() else args.root / path)
    if args.command == 'tasks' and args.action == 'check':
        from .opensmacx_check import check_task
        output = check_task(args.root, args.task, args.image)
        if args.summary:
            output['checks']['binary'].pop('functions', None)
            output['checks']['layout'].pop('members', None)
            output['checks']['layout'].pop('bases', None)
        return ({Result.PASS: 0, Result.FAIL: 1, Result.UNKNOWN: 3}[output['result']], output)
    if args.command == 'tasks' and args.action == 'check-layout':
        from .opensmacx_layout import verify_layout
        output = verify_layout(args.root, args.task)
        if args.summary:
            output.pop('members')
            output.pop('bases', None)
        return ({Result.PASS: 0, Result.FAIL: 1, Result.UNKNOWN: 3}[output['result']], output)
    if args.command == 'tasks' and args.action == 'obligations':
        from .opensmacx_obligations import task_obligations
        output = task_obligations(args.root, args.task)
        return (0, output)
    if args.command == 'tasks' and args.action == 'check-binary':
        from .opensmacx_binary import verify_task
        output = verify_task(args.root, args.task, args.image)
        if args.summary:
            output.pop('functions')
        return ({Result.PASS: 0, Result.FAIL: 1, Result.UNKNOWN: 3}[output['result']], output)
    if args.command == 'tasks':
        from match_recovery.ownership_source import project_view, structural_impact
        from match_recovery.model import Origin
        from match_recovery.partition import select_group
        from match_recovery.tasks import Task, TaskStore
        store = TaskStore(args.root)
        if args.command == 'tasks' and args.action == 'discover':
            output = store.discover(args.task, request_id=args.request_id, author=args.author, hypothesis=args.hypothesis, affects=args.affects, evidence=args.evidence, expected_revision=args.expected_revision)
        elif args.command == 'tasks' and args.action == 'resolve':
            output = store.resolve(args.task, discovery_id=args.discovery_id, request_id=args.request_id, author=args.author, disposition=args.disposition, source_evidence=args.source_evidence, expected_revision=args.expected_revision)
        else:
            view, snapshot = project_view(args.root)
            if args.command == 'tasks' and args.action == 'expand':
                entities = {entity['entity_id']: entity for group in view['groups'] for entity in group['entities']}
                if not set(args.subject) <= entities.keys():
                    raise ValueError('scope expansion names an entity missing from current source')
                output = store.expand(args.task, request_id=args.request_id, author=args.author, subjects=(Origin(**entities[subject]['origin']) for subject in args.subject), editable_files=args.edit_file, evidence=args.evidence, expected_revision=args.expected_revision)
            elif args.command == 'tasks' and args.action == 'status':
                output = store.status(args.task, (entity['entity_id'] for group in view['groups'] for entity in group['entities']))
            elif args.command == 'tasks' and args.action == 'context':
                from .opensmacx_obligations import task_obligations
                scope, files = store.scope(args.task)
                selected = [entity for group in view['groups'] for entity in group['entities'] if entity['entity_id'] in scope]
                group = {'entities': selected, 'source_homes': sorted(files)}
                output = {'task': store.read(args.task).to_dict(), 'subjects': selected, 'decision_state': store.status(args.task, (entity['entity_id'] for entity in selected)), 'impact': structural_impact(snapshot, group), 'restoration': task_obligations(args.root, args.task), 'binding_hints': [{'entity_id': row['entity_id'], 'path': row['path'], **row['binding_hint']} for row in snapshot['records'] if row['entity_id'] in scope and row['binding_hint']], 'instructions': ['Read source declarations, definitions and evidence before editing.', 'Treat archived bodies and class/name labels as seeds, not mandatory source form.', 'Report structural discoveries even when they affect other functions or units.', 'Do not edit beyond authorized scope or discard matching obligations.', 'A scope decision does not approve a regression; migration review and fresh checks are separate.'], 'persisted': False}
            else:
                groups = [select_group(view, args.group)] if args.group else view['groups']
                if args.command == 'tasks':
                    group = groups[0]
                    if group['next_action'] != 'review_ownership_and_structure':
                        raise ValueError('structural recovery tasks require an unambiguous source/class group')
                    changes = subprocess.run(['git', 'status', '--porcelain', '--untracked-files=all', '--', 'src'], cwd=args.root, capture_output=True, text=True, check=True)
                    if changes.stdout.strip():
                        raise ValueError("checkpoint source changes before fixing a task's committed baseline")
                    roots = sorted(set(structural_impact(snapshot, group)['root_files']) | set(args.edit_file or []))
                    baseline = snapshot['revision']
                    upstream = subprocess.run(['git', 'merge-base', 'HEAD', 'master'], cwd=args.root, capture_output=True, text=True, check=False)
                    if upstream.returncode == 0:
                        upstream_revision = upstream.stdout.strip()
                        scope_paths = sorted(set(roots) | {item['path'] for entity in group['entities'] for item in entity['candidates']})
                        equivalent = subprocess.run(['git', 'diff', '--quiet', upstream_revision, 'HEAD', '--', *scope_paths], cwd=args.root, capture_output=True, check=False)
                        if equivalent.returncode == 0:
                            baseline = upstream_revision
                        elif equivalent.returncode != 1:
                            raise ValueError('could not verify the task baseline source')
                    task = Task(args.task, 'opensmacx', args.author, args.intent, baseline, tuple((Origin(**entity['origin']) for entity in group['entities'])), tuple(roots), tuple(args.evidence))
                    output = {'created': store.create(task), 'task': task.to_dict(), 'verification': 'not_measured'}
        return (0, output)
    if args.command == 'components':
        from match_recovery.components import source_integrity
        from match_recovery.component_source import read_components
        manifests = read_components(args.root / 'src')
        if args.action == 'list':
            output = [{'component': asdict(manifest.component), 'workflow': manifest.component.workflow, 'bindings': len(manifest.bindings)} for manifest in manifests]
            return (0, output)
        selected = [manifest for manifest in manifests if manifest.component.component_id == args.component]
        if len(selected) != 1:
            raise ValueError(f'expected one component named {args.component}')
        if args.action == 'check-binary':
            from match_recovery.component_binary import verify_component
            from .opensmacx_components import compile_vendor
            result = verify_component(args.root, selected[0], args.image.read_bytes(), lambda path, flags: compile_vendor(args.root, path, flags))
            if read_components(args.root / 'src') != manifests:
                raise ValueError('component annotations changed during measurement')
            if args.summary:
                result.pop('functions')
        else:
            result = source_integrity(args.root, selected[0], args.archive)
        output = result
        return ({Result.PASS: 0, Result.FAIL: 1, Result.UNKNOWN: 3}[result['result']], output)
    if args.command == 'import' and args.provider == 'opensmacx-zlib':
        from .opensmacx_components import zlib_manifest
        from match_recovery.component_source import write_component
        manifest = zlib_manifest(args.root, args.archive)
        path = args.root / 'src' / 'vendor' / 'zlib.recovery.h'
        created = write_component(path, manifest)
        output = {'component': 'zlib', 'manifest': str(path.relative_to(args.root)), 'created': created, 'source_integrity': 'pass', 'historical_bindings': len(manifest.bindings), 'binary_verification': 'unknown'}
        return (0, output)
    raise ValueError("unsupported project operation; source inventory and partition belong to the package")
