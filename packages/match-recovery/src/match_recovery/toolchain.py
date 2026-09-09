"""Fresh identities of project-declared external compiler/SDK inputs.

The project describes locations; the package reads their actual contents. This
does not install tools, execute target programs, or prove a complete/hermetic
toolchain. Content hashes may be cached in process memory with coherent POSIX
metadata invalidation; no saved report or verdict is an input to verification.
"""

import hashlib
from collections import OrderedDict
from copy import deepcopy
import os
from pathlib import Path
import re
import stat
import threading

from .components import relative_path
from .driver import Command, invoke
from .fingerprint import _content_digest, input_snapshot
from .model import digest
from .records import read_json


_observations = OrderedDict()
_CACHE_LIMIT = 16
_cache_lock = threading.Lock()


def _after_fork():
    # A child must not inherit a lock held by a vanished parent thread, or an
    # observation from a different process lifetime.
    global _cache_lock, _observations
    _cache_lock = threading.Lock()
    _observations = OrderedDict()


if hasattr(os, "register_at_fork"):
    os.register_at_fork(after_in_child=_after_fork)


def _stamp(value):
    # Reading may change atime. Other metadata detects changes to a previously
    # hashed inode, including writes with restored mtime. Identity remains SHA256.
    return (value.st_dev, value.st_ino, value.st_mode, value.st_size, value.st_mtime_ns, value.st_ctime_ns)


def _tree_paths(root):
    # pathlib globbing suppresses scan errors. Missing an unreadable directory
    # would silently omit headers that a compiler can open by known filename.
    pending, paths = [root], []
    while pending:
        with os.scandir(pending.pop()) as entries:
            for entry in entries:
                path = Path(entry.path)
                paths.append(path)
                if entry.is_dir(follow_symlinks=False):
                    pending.append(path)
    return sorted(paths)


def _inventory(description):
    groups, entries = [], {}
    for kind in ("files", "directories"):
        for name, spelling in sorted(description[kind].items()):
            relative_path(name)
            if not isinstance(spelling, str) or not spelling or "\0" in spelling:
                raise ValueError("toolchain input requires an absolute path")
            path = Path(spelling)
            if not path.is_absolute() or path.resolve() != path:
                raise ValueError("toolchain input requires a canonical absolute path")
            if not (path.is_file() if kind == "files" else path.is_dir()):
                raise ValueError("declared toolchain input is missing or has the wrong kind")
            paths = [path] if kind == "files" else _tree_paths(path)
            groups.append((kind, name, spelling, path, paths))
            for item in (path, *paths):
                entries[item] = _stamp(item.lstat())
    return groups, entries


def snapshot(root):
    """Return declared external content identity, or None for unconfigured tools."""
    root = Path(root).resolve()
    project = read_json(root / "recovery/project.json")
    if "toolchain" not in project:
        return None
    value = project["toolchain"]
    if (not isinstance(value, dict) or set(value) != {"command", "inputs", "environment"}
            or not isinstance(value["inputs"], list) or not isinstance(value["environment"], list)):
        raise ValueError("toolchain requires command, inputs and environment")
    command = Command.read(value["command"])
    for name in value["inputs"]:
        relative_path(name)
    names = {"PATH", *value["environment"]} if all(isinstance(name, str) for name in value["environment"]) else set()
    if not names or any(not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", name) for name in names):
        raise ValueError("invalid toolchain environment variable")
    for argument in command.argv:
        names.update(re.findall(r"\{env:([A-Za-z_][A-Za-z_0-9]*)\}", argument))

    def policy():
        environment = {name: hashlib.sha256(os.environ[name].encode()).hexdigest()
                       if name in os.environ else None for name in sorted(names)}
        result = input_snapshot(root, ["recovery/project.json", *value["inputs"]],
                                {"toolchain": value, "environment": environment})
        if any(content is None for content in result["inputs"].values()):
            raise ValueError("declared toolchain policy input is missing")
        return result

    before = policy()
    code, description = invoke(command, root, "", {"schema_version": 1, "operation": "toolchain_inputs"})
    if code != 0:
        raise ValueError("toolchain input discovery failed")
    if (set(description) != {"schema_version", "files", "directories"}
            or type(description["schema_version"]) is not int or description["schema_version"] != 1
            or any(not isinstance(description[key], dict) for key in ("files", "directories"))
            or not (description["files"] or description["directories"])
            or set(description["files"]) & set(description["directories"])):
        raise ValueError("invalid toolchain input description")

    # Windows ctime is not a reliable content-change timestamp. Default to fresh
    # reads there; unusual/noncoherent filesystems can disable caching explicitly.
    mode = os.environ.get("MATCH_RECOVERY_TOOLCHAIN_CACHE", "on")
    if mode not in ("on", "off"):
        raise ValueError("MATCH_RECOVERY_TOOLCHAIN_CACHE must be on or off")
    caching = os.name == "posix" and mode == "on"
    key = digest({"policy": before, "description": description})
    groups, entries = _inventory(description)

    def unchanged():
        if _inventory(description)[1] != entries:
            raise ValueError("toolchain inputs or directory membership changed during observation")
        if policy() != before:
            raise ValueError("toolchain discovery policy changed during observation")

    with _cache_lock:
        cached = _observations.get(key) if caching else None
        if cached is not None:
            _observations.move_to_end(key)
    if cached is not None and cached[0] == entries:
        unchanged()
        return deepcopy(cached[1])

    buffer = bytearray(2**18)
    observed = {}
    identities = {}
    for kind, name, spelling, path, paths in groups:
        root_parts = path.parts
        root_length = len(root_parts)
        contents = {}
        for item in paths:
            entry = item.lstat()
            if _stamp(entry) != entries[item]:
                raise ValueError("toolchain input changed before hashing")
            mode = entry.st_mode
            alias = stat.S_ISLNK(mode)
            # The root is canonical and the scan never follows directory
            # aliases. Normal entries need no repeated ancestor resolution.
            resolved = item.resolve() if alias else item
            if alias:
                if (kind != "directories" or resolved == path or resolved.parts[:root_length] != root_parts
                        or not (resolved.is_file() or resolved.is_dir())):
                    raise ValueError("toolchain aliases must remain within their declared directory")
                mode = resolved.stat().st_mode
            elif not (stat.S_ISREG(mode) or stat.S_ISDIR(mode)) or resolved != item:
                raise ValueError("toolchain trees require regular files and directories, without directory aliases")
            label = "." if item == path else "/".join(item.parts[root_length:])
            if stat.S_ISDIR(mode):
                contents[label] = {"kind": "directory"}
                if alias:
                    # The scan does not follow directory symlinks. Pin the
                    # alias; its canonical target is inventoried separately.
                    contents[label]["alias"] = "/".join(resolved.parts[root_length:])
            else:
                stamp = _stamp(resolved.stat())
                if resolved not in observed:
                    with resolved.open("rb") as stream:
                        hashed = _content_digest(stream, buffer)
                        if _stamp(os.fstat(stream.fileno())) != stamp or _stamp(resolved.stat()) != stamp:
                            raise ValueError("toolchain input changed while hashing")
                    observed[resolved] = (stamp, hashed)
                if observed[resolved][0] != stamp:
                    raise ValueError("toolchain input changed between aliases")
                contents[label] = {"sha256": observed[resolved][1], "executable": bool(mode & 0o111)}
                if alias:
                    contents[label]["alias"] = "/".join(resolved.parts[root_length:])
        identities[name] = {"kind": kind, "location_sha256": hashlib.sha256(spelling.encode()).hexdigest(),
                            "contents_sha256": digest(contents), "file_count": sum("sha256" in row for row in contents.values())}
    if any(_stamp(path.stat()) != stamp for path, (stamp, _) in observed.items()):
        raise ValueError("toolchain input changed during observation")
    unchanged()
    result = {"schema_version": 1, "policy": before, "inputs": identities}
    if caching:
        with _cache_lock:
            _observations[key] = (entries, deepcopy(result))
            _observations.move_to_end(key)
            while len(_observations) > _CACHE_LIMIT:
                _observations.popitem(last=False)
    return result
