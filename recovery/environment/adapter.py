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
        if (set(request) != {"schema_version", "source", "arguments", "flags"}
                or type(request["schema_version"]) is not int or request["schema_version"] != 1):
            raise ValueError("invalid compiler request")
        for name in ("arguments", "flags"):
            if not isinstance(request[name], list) or any(not isinstance(value, str) for value in request[name]):
                raise ValueError("compiler arguments and flags must be string arrays")
        root = Path.cwd().resolve()
        command = tuple(request["arguments"]) or (sys.executable, str(root / "recovery/environment/wine.py"),
                                                 "--tool=cl", "/nologo")
        data = compile_object(local_path(root, request["source"]), command, tuple(request["flags"]))
        output, code = {"object_base64": base64.b64encode(data).decode("ascii")}, 0
    except (OSError, ValueError) as error:
        output, code = {"error": str(error)}, 3
    print(json.dumps(output))
    return code


if __name__ == "__main__":
    raise SystemExit(main())
