"""VC6 compilation shared by application and dependency verification."""

from pathlib import Path
import os
import subprocess
import tempfile

from match_recovery.measurement import Measurement
from match_recovery.processes import run_bounded
from match_recovery.source_replica import replica_roots, source_replica, source_tree



def compile_object(source: Path, command: tuple[str, ...], flags: tuple[str, ...], *, temporary_parent: Path | None = None) -> bytes:
    """Objects are disposable; a nonzero compiler exit never supplies evidence."""
    with tempfile.TemporaryDirectory(prefix="recover-vc6-", dir=temporary_parent) as temporary:
        work = Path(temporary)
        try:
            result = run_bounded([*command, *flags, "/Founit.obj", str(source.resolve())],
                                 cwd=work, timeout=300)
        except subprocess.TimeoutExpired as error:
            raise ValueError("VC6 compilation exceeded 300 seconds") from error
        if result.returncode != 0 or not (work / "unit.obj").is_file():
            raise ValueError("VC6 compilation failed: " +
                             (result.stdout + result.stderr).decode(errors="replace").strip())
        return (work / "unit.obj").read_bytes()


def _variant_arguments(arguments, root, replica, roots):
    """VC6 option semantics stay here, not in the installed compiler client."""
    def remap(value):
        if value.startswith(('Z:', 'z:')):
            value = value[2:].replace('\\', '/')
        elif len(value) > 1 and value[1] == ':':
            raise ValueError("source overrides support POSIX or Wine Z: include paths only")
        path = Path(value)
        if not value or not path.is_absolute():
            raise ValueError("source overrides require absolute include/forced-include paths")
        if path.resolve() != path:
            raise ValueError("source overrides require canonical include/forced-include paths")
        if not path.is_relative_to(root):
            return str(path)
        relative = path.relative_to(root)
        if not any(relative.is_relative_to(name) for name in roots):
            raise ValueError("project include path is outside the declared replica roots")
        return str(replica / relative)

    output, iterator = [], iter(arguments)
    for word in iterator:
        if word.lower().startswith(("/yu", "/yc", "/fp", "-yu", "-yc", "-fp", "@")):
            raise ValueError("source overrides require compilation without PCH or response files")
        prefix = next((prefix for prefix in ("/FI", "-FI", "/I", "-I") if word.startswith(prefix)), None)
        if prefix is not None:
            value = word[len(prefix):] or next(iterator, "")
            output.append("/" + prefix[1:] + remap(value))
        else:
            output.append(word)
    return tuple(output)


def compile_overlay(root: Path, source: Path, replacements: dict[str, bytes], command: tuple[str, ...], flags: tuple[str, ...],
                    *, tree_roots: tuple[str, ...] | None = None) -> bytes:
    """Compile a TU with all proposed headers/bodies in one private tree.

    An identical-content replica control is still required by the measurement
    caller: unusual includes and path/time-sensitive preprocessing can prevent
    equivalence. Success here establishes only that VC6 produced an object.
    """
    root, source = root.resolve(), source.resolve()
    settings = Measurement.read(root)
    roots = replica_roots((*settings.source.roots, *settings.input_roots))
    if tree_roots is not None and tree_roots != roots:
        raise ValueError("complete source tree roots must cover the configured compiler inputs")
    if any(os.environ.get(name) for name in ("CL", "_CL_", "VC6_INCLUDE")):
        raise ValueError("source overrides require explicit include/options, not CL/_CL_/VC6_INCLUDE overrides")

    # Validate unsupported flags before copying any source files.
    _variant_arguments((*command, *flags), root, root, roots)
    # Do not inject absolute #line filenames: VC6 uses those during quote-include
    # lookup, mixing original and copied headers and defeating #pragma once.
    # __FILE__ data differences must fail the caller's replica control instead.
    materialize = source_replica if tree_roots is None else source_tree
    with materialize(root, roots, replacements, artifacts="build/recovery/variants") as replica:
        return compile_object(replica / source.relative_to(root),
                              _variant_arguments(command, root, replica, roots),
                              _variant_arguments(flags, root, replica, roots), temporary_parent=replica)
