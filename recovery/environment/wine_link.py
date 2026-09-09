"""VC6/Wine link-input file observations for the portable build protocol.

No binary interpretation or acceptance policy lives here. Snapshot a conservative
library search population before linking; report the actual verbose searches
afterwards. The package decides which observations can be used as evidence.
"""

import hashlib
import re
from pathlib import Path


def host_path(text: str, cwd: Path) -> Path:
    if len(text) > 1 and text[1] == ":":
        if text[:2].lower() != "z:":
            raise ValueError("only the configured Wine Z: filesystem mapping is supported")
        text = text[2:]
    value = Path(text.replace("\\", "/"))
    value = value if value.is_absolute() else cwd / value
    # Wine resolves case-insensitively. Refuse collisions instead of selecting
    # the first directory entry on a case-sensitive host filesystem.
    current = Path(value.anchor)
    for part in value.parts[1:]:
        if part == "..":
            current = current.parent
            continue
        matches = [entry for entry in current.iterdir() if entry.name.casefold() == part.casefold()]
        if len(matches) != 1:
            raise ValueError("missing or ambiguous Wine path: " + str(value))
        current = matches[0]
        if current.is_symlink():
            raise ValueError("symlinked link input is unsupported: " + str(current))
    return current


def stamp(path: Path) -> dict:
    if not path.is_file() or any(item.is_symlink() for item in (path, *path.parents)):
        raise ValueError("expected a regular link input: " + str(path))
    before = path.stat()
    data = path.read_bytes()
    after = path.stat()
    if (before.st_dev, before.st_ino, before.st_size, before.st_mtime_ns, before.st_ctime_ns) != (
            after.st_dev, after.st_ino, after.st_size, after.st_mtime_ns, after.st_ctime_ns):
        raise ValueError("link input changed while hashing: " + str(path))
    return {"sha256": hashlib.sha256(data).hexdigest(), "size": len(data)}


def capture_before(arguments: list[str], executable: Path, library_dirs: list[str], cwd: Path) -> dict:
    cwd = cwd.resolve()
    roots, files, explicit, gaps = [cwd], {executable.absolute()}, [], []
    for value in [*library_dirs, *(word.split(":", 1)[1] for word in arguments
                                 if word.upper().startswith(("/LIBPATH:", "-LIBPATH:")))]:
        try:
            roots.append(host_path(value, cwd))
        except (OSError, ValueError) as error:
            gaps.append(str(error))
    for root in roots:
        try:
            files.update(path for path in root.iterdir() if path.suffix.casefold() == ".lib")
        except OSError as error:
            gaps.append(str(error))
    for word in arguments:
        if word.startswith("@"):
            gaps.append("unexpanded linker response file: " + word)
            continue
        # Options, including output paths, are not input filenames. Absolute
        # POSIX paths are supplied by CMake before the wrapper translates them.
        if word.startswith(("/", "-")) and not Path(word).is_file():
            continue
        if Path(word.replace("\\", "/")).suffix.casefold() == ".lib":
            # Bare library names resolve through the search list; explicit
            # library paths outside that list still need a pre-link snapshot.
            if "/" not in word and "\\" not in word:
                continue
        try:
            path = host_path(word, cwd)
            files.add(path)
            explicit.append(str(path))
        except (OSError, ValueError) as error:
            gaps.append(str(error))
    before = {}
    for path in sorted(files):
        try:
            before[str(path)] = stamp(path)
        except (OSError, ValueError) as error:
            gaps.append(str(error))
    return {"cwd": str(cwd), "arguments": arguments, "tool": str(executable.absolute()),
            "explicit_inputs": explicit, "before": before, "gaps": gaps}


def capture_after(before: dict, stdout: str) -> dict:
    searches, paths, gaps = [], set(before["before"]), list(before["gaps"])
    for line in stdout.splitlines():
        found = re.fullmatch(r"\s+Searching (.+):\s*", line)
        if found is None:
            continue
        path = None
        try:
            path = str(host_path(found[1], Path(before["cwd"])))
            paths.add(path)
        except (OSError, ValueError) as error:
            gaps.append(str(error))
        searches.append({"spelling": found[1], "path": path})
    files = []
    for path in sorted(paths):
        after = None
        try:
            after = stamp(Path(path))
        except (OSError, ValueError) as error:
            gaps.append(str(error))
        files.append({"path": path, "before": before["before"].get(path), "after": after})
    return {"schema_version": 1, "link_log_sha256": hashlib.sha256(stdout.encode("utf-8")).hexdigest(),
            "cwd": before["cwd"], "arguments": before["arguments"], "tool": before["tool"],
            "explicit_inputs": before["explicit_inputs"], "files": files,
            "library_searches": searches, "capture_gaps": gaps}
