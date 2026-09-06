"""Project-local VC6 implementation of the package's compiler JSON protocol."""

import base64
import hashlib
import json
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from recovery.environment.compile import compile_object, compile_variant
from match_recovery.records import parse_json
from match_recovery.components import local_path


def main():
    try:
        request = parse_json(sys.stdin.read())
        version = request.get("schema_version")
        fields = {"schema_version", "profile", "source", "arguments", "flags"}
        if version == 3:
            fields |= {"source_base64", "source_sha256"}
        if set(request) != fields or type(version) is not int or version not in (2, 3):
            raise ValueError("invalid compiler request")
        for name in ("arguments", "flags"):
            if not isinstance(request[name], list) or any(not isinstance(value, str) for value in request[name]):
                raise ValueError("compiler arguments and flags must be string arrays")
        root = Path.cwd().resolve()
        source = local_path(root, request["source"])
        contents = None
        if version == 3:
            if (not isinstance(request["source_base64"], str)
                    or request["source_sha256"] != hashlib.sha256(source.read_bytes()).hexdigest()):
                raise ValueError("source override requires the unchanged original source digest")
            contents = base64.b64decode(request["source_base64"], validate=True)
        if request["profile"] not in ("default", "application"):
            raise ValueError("unsupported project compiler profile")
        command = tuple(request["arguments"])
        if not command:
            if request["profile"] == "application":
                from match_recovery.source.asm import build_command
                command = tuple(build_command(root / "build/compile_commands.json", source))
            else:
                command = (sys.executable, str(root / "recovery/environment/wine.py"), "--tool=cl", "/nologo")
        data = (compile_object(source, command, tuple(request["flags"])) if contents is None else
                compile_variant(root, source, contents, command, tuple(request["flags"])))
        output, code = {"object_base64": base64.b64encode(data).decode("ascii")}, 0
    except (OSError, ValueError) as error:
        output, code = {"error": str(error)}, 3
    print(json.dumps(output))
    return code


if __name__ == "__main__":
    raise SystemExit(main())
