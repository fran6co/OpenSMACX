"""Project operation bridge during source-format and verifier migration.

The public CLI belongs to match_recovery. This module has no argument parser.
Reusable logic below is being moved behind configured source/environment APIs.
"""

from dataclasses import asdict
from pathlib import Path
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
    if args.command == 'tasks' and args.action == 'context':
        from match_recovery.task_source import task_context
        from .opensmacx_obligations import task_obligations
        return 0, task_context(args.root, args.task, restoration_for=task_obligations)
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
