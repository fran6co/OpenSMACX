"""JSON protocol adapter; all public command-line parsing belongs to match_recovery."""

import json
from pathlib import Path
import subprocess
import sys
import tarfile

# Executing this local script exposes recovery/, not the project root. The
# application is not an installed Python package; keep that path choice local.
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from recovery.opensmacx.operations import execute


def main():
    try:
        request = json.load(sys.stdin)
        if set(request) != {"schema_version", "arguments"} or request["schema_version"] != 1:
            raise ValueError("unsupported project adapter request")
        code, output = execute(Path.cwd(), request["arguments"])
    except (OSError, ValueError, ImportError, subprocess.SubprocessError, tarfile.TarError) as error:
        code, output = 3, {"error": str(error)}
    print(json.dumps({"output": output}))
    return code


if __name__ == "__main__":
    raise SystemExit(main())
