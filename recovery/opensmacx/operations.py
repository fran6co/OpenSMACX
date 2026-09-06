"""Project operation bridge during source-format and verifier migration.

The public CLI belongs to match_recovery. This module has no argument parser.
Reusable logic below is being moved behind configured source/environment APIs.
"""

from pathlib import Path
from types import SimpleNamespace

from match_recovery.model import Result


def execute(root: Path, arguments: dict):
    args = SimpleNamespace(**arguments)
    args.root = root.resolve()
    for name in ("image",):
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
    if args.command == 'tasks' and args.action == 'check-binary':
        from .opensmacx_binary import verify_task
        output = verify_task(args.root, args.task, args.image)
        if args.summary:
            output.pop('functions')
        return ({Result.PASS: 0, Result.FAIL: 1, Result.UNKNOWN: 3}[output['result']], output)
    raise ValueError("unsupported project verification operation")
