"""Project-local VC6 implementation of the package's compiler JSON protocol."""

import base64
import json
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from recovery.environment.compile import compile_object
from match_recovery.records import parse_json
from match_recovery.components import local_path


def main():
    try:
        request = parse_json(sys.stdin.read())
        if (set(request) != {"schema_version", "profile", "source", "arguments", "flags"}
                or type(request["schema_version"]) is not int or request["schema_version"] != 2):
            raise ValueError("invalid compiler request")
        for name in ("arguments", "flags"):
            if not isinstance(request[name], list) or any(not isinstance(value, str) for value in request[name]):
                raise ValueError("compiler arguments and flags must be string arrays")
        root = Path.cwd().resolve()
        source = local_path(root, request["source"])
        if request["profile"] not in ("default", "application"):
            raise ValueError("unsupported project compiler profile")
        command = tuple(request["arguments"])
        if not command:
            if request["profile"] == "application":
                from match_recovery.source.asm import build_command
                command = tuple(build_command(root / "build/compile_commands.json", source))
            else:
                command = (sys.executable, str(root / "recovery/environment/wine.py"), "--tool=cl", "/nologo")
        data = compile_object(source, command, tuple(request["flags"]))
        output, code = {"object_base64": base64.b64encode(data).decode("ascii")}, 0
    except (OSError, ValueError) as error:
        output, code = {"error": str(error)}, 3
    print(json.dumps(output))
    return code


if __name__ == "__main__":
    raise SystemExit(main())
