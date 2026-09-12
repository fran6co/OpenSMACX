"""VC6 compile-file observations for the portable recovery protocol.

Capture only explicitly requested reproducible object compilations. The package
verifies these disposable observations; they are not source or acceptance records.
This module stays standalone so CMake does not need to import the framework.
"""

import hashlib
from pathlib import Path


def stamp(path):
    path = Path(path)
    if path.resolve() != path or not path.is_file():
        raise ValueError("compile observation requires a canonical regular file: " + str(path))
    before = path.stat()
    raw = path.read_bytes()
    after = path.stat()
    fields = lambda value: (value.st_dev, value.st_ino, value.st_size, value.st_mtime_ns, value.st_ctime_ns)
    if fields(before) != fields(after):
        raise ValueError("compile input changed while hashing: " + str(path))
    return {"sha256": hashlib.sha256(raw).hexdigest(), "size": len(raw)}


def capture_before(reproducible, arguments, executable):
    if reproducible is None or not reproducible.inputs:
        raise ValueError("compile observations require verified reproducible inputs")
    outputs = [word[3:] for word in arguments if word.startswith("/Fo")]
    if len(outputs) != 1 or not outputs[0]:
        raise ValueError("compile observations require one explicit object output")
    output = Path(outputs[0])
    if (not output.is_absolute() or output.parent.resolve() != output.parent
            or not output.parent.is_dir() or output.is_symlink()
            or output.exists() and (not output.is_file() or output.stat().st_nlink != 1)):
        raise ValueError("compile observation output must be a canonical file path")
    source, executable = reproducible.source, Path(executable).absolute()
    paths = {source, executable, *(Path(resolved) for resolved, _ in reproducible.inputs.values())}
    if output in paths:
        raise ValueError("compile output overlaps an observed input")
    observed = {str(path): stamp(path) for path in sorted(paths)}
    # Reuse the actual preprocessor's observed closure, not an include regex.
    for name, (resolved, digest) in reproducible.inputs.items():
        logical = Path(name)
        # SDK spelling aliases are legitimate (stdio.h -> STDIO.H). Observe
        # physical bytes here; the reproducibility policy checks the complete
        # logical-to-physical bindings again before publishing the observation.
        if str(logical.resolve()) != resolved or observed[resolved]["sha256"] != digest:
            raise ValueError("compile observations differ from verified preprocessing inputs")
    return {"schema_version": 1, "source": str(source), "tool": str(executable),
            "cwd": str(reproducible.root), "arguments": list(arguments),
            "inputs": [{"path": name, "before": value} for name, value in observed.items()],
            "output": {"path": str(output), "existed_before": output.exists()}}


def capture_after(before, exit_code):
    def observed(name):
        try:
            return stamp(Path(name))
        except (OSError, ValueError):
            return None
    return {**before, "exit_code": exit_code,
            "inputs": [{**item, "after": observed(item["path"])} for item in before["inputs"]],
            "output": {**before["output"], "after": observed(before["output"]["path"])}}
