"""Project-local VC6 implementation of the package's compiler JSON protocol."""

import base64
import json
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from recovery.environment.compile import compile_object, compile_overlay
from match_recovery.compiler import CompilationRequest
from match_recovery.records import parse_json


def main():
    try:
        root = Path.cwd().resolve()
        request = CompilationRequest.read(root, parse_json(sys.stdin.read()))
        source = request.source
        if request.profile not in ("default", "application"):
            raise ValueError("unsupported project compiler profile")
        command = request.arguments
        if not command:
            if request.profile == "application":
                from match_recovery.source.asm import build_command
                command = tuple(build_command(root / "build/compile_commands.json", source))
            else:
                command = (sys.executable, str(root / "recovery/environment/wine.py"), "--tool=cl", "/nologo")
        if request.tree_files is not None:
            data = compile_overlay(root, source, request.tree_files, command, request.flags, tree_roots=request.tree_roots)
        else:
            data = (compile_object(source, command, request.flags) if request.replacements is None else
                    compile_overlay(root, source, request.replacements, command, request.flags))
        output, code = {"object_base64": base64.b64encode(data).decode("ascii")}, 0
    except (OSError, ValueError) as error:
        output, code = {"error": str(error)}, 3
    print(json.dumps(output))
    return code


if __name__ == "__main__":
    raise SystemExit(main())
