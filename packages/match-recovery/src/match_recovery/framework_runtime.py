"""Explicit installed framework selection, separate from project compiler hooks.

No environment variable activates this binding. The caller supplies a pinned
configuration; only its exact declared recovery launchers are redirected. An
explicit {framework_python} executable selects this framework's interpreter for
project-owned Python protocol clients without rewriting arbitrary commands. This
identifies a local installation, not a hermetic Python/OS or behavioral proof.
"""

from contextlib import contextmanager
from contextvars import ContextVar
from collections import OrderedDict
import hashlib
import os
from pathlib import Path
import stat
import subprocess
import threading

from .model import digest, require_digest
from .records import parse_json, read_json


_ACTIVE = ContextVar("match_recovery_framework_runtime", default=None)
_FILES = OrderedDict()
_FILE_LIMIT = 4096
_FILE_LOCK = threading.Lock()


def _after_fork():
    global _FILES, _FILE_LOCK
    _FILES, _FILE_LOCK = OrderedDict(), threading.Lock()


if hasattr(os, "register_at_fork"):
    os.register_at_fork(after_in_child=_after_fork)


def _stamp(value):
    return (value.st_dev, value.st_ino, value.st_mode, value.st_nlink,
            value.st_size, value.st_mtime_ns, value.st_ctime_ns)


def _caching():
    mode = os.environ.get("MATCH_RECOVERY_RUNTIME_CACHE", "on")
    if mode not in ("on", "off"):
        raise ValueError("MATCH_RECOVERY_RUNTIME_CACHE must be on or off")
    # Restored mtime does not conceal a POSIX ctime change. Filesystems without
    # coherent metadata must opt out; Windows defaults to fresh content reads.
    return os.name == "posix" and mode == "on"


_PROBE = """
import importlib.metadata as metadata
import importlib.util
import json
import pathlib
import sys
import sysconfig
spec = importlib.util.find_spec('match_recovery')
if spec is None or not spec.submodule_search_locations or len(spec.submodule_search_locations) != 1:
    raise ValueError('one installed match_recovery package is required')
distribution = metadata.distribution('match-recovery')
direct = json.loads(distribution.read_text('direct_url.json') or '{}')
print(json.dumps(dict(package=str(pathlib.Path(next(iter(spec.submodule_search_locations))).resolve()),
                     purelib=str(pathlib.Path(sysconfig.get_path('purelib')).resolve()),
                     prefix=str(pathlib.Path(sys.prefix).resolve()), version=distribution.version,
                     python_version=sys.version, editable=direct.get('dir_info',{}).get('editable',False))))
"""


def _file(path):
    caching = _caching()
    before = path.lstat()
    if not stat.S_ISREG(before.st_mode):
        raise ValueError("framework runtime input must remain a regular file")
    stamp = _stamp(before)
    with _FILE_LOCK:
        cached = _FILES.get(path) if caching else None
        if cached is not None:
            _FILES.move_to_end(path)
    if cached is not None and cached[0] == stamp:
        if _stamp(path.lstat()) != stamp:
            raise ValueError("framework runtime input changed during observation")
        return cached[1]
    # Do not follow a raced-in alias or block on a raced-in FIFO after lstat.
    descriptor = os.open(path, os.O_RDONLY | getattr(os, "O_NOFOLLOW", 0) | getattr(os, "O_NONBLOCK", 0))
    with os.fdopen(descriptor, "rb") as stream:
        if _stamp(os.fstat(stream.fileno())) != stamp:
            raise ValueError("framework runtime input changed before hashing")
        value = hashlib.file_digest(stream, "sha256").hexdigest()
        if _stamp(os.fstat(stream.fileno())) != stamp or _stamp(path.lstat()) != stamp:
            raise ValueError("framework runtime input changed during hashing")
    if caching:
        with _FILE_LOCK:
            _FILES[path] = (stamp, value)
            _FILES.move_to_end(path)
            while len(_FILES) > _FILE_LIMIT:
                _FILES.popitem(last=False)
    return value


def _inventory(path, installed):
    if path.resolve() != path or not path.is_dir():
        raise ValueError("framework runtime requires canonical installed directories")
    entries, pending = {path: _stamp(path.lstat())}, [path]
    while pending:
        # Unlike pathlib.rglob, propagate directory enumeration failures; an
        # unreadable directory must not silently disappear from runtime identity.
        with os.scandir(pending.pop()) as children:
            for child in children:
                item, info = Path(child.path), child.stat(follow_symlinks=False)
                if stat.S_ISLNK(info.st_mode):
                    raise ValueError("framework installation cannot contain file or directory aliases")
                if stat.S_ISDIR(info.st_mode):
                    pending.append(item)
                elif not stat.S_ISREG(info.st_mode):
                    raise ValueError("framework installation contains a nonregular input")
                elif installed and item.suffix in (".pyc", ".pyo"):
                    raise ValueError("framework runtime must be installed without bytecode caches; execute with -B")
                entries[item] = _stamp(info)
    return entries


def _tree_values(path, *, installed=False):
    before = _inventory(path, installed)
    values = {item.relative_to(path).as_posix(): _file(item) for item, stamp in before.items()
              if stat.S_ISREG(stamp[2]) and item.suffix not in (".pyc", ".pyo")}
    if _inventory(path, installed) != before:
        raise ValueError("framework installation changed during observation")
    return values


def _tree(path, *, installed=False):
    values = _tree_values(path, installed=installed)
    return digest(values)


def _installation(package, purelib):
    if package.parent != purelib or not package.is_dir():
        raise ValueError("framework runtime requires its package inside the pinned installation")
    values = _tree_values(purelib, installed=True)
    prefix = package.name + "/"
    return digest({name[len(prefix):]: value for name, value in values.items() if name.startswith(prefix)}), digest(values)


def _interpreter(path):
    path = Path(path)
    if (not path.is_absolute() or path.parent.resolve() != path.parent or not path.is_file()
            or str(path) != str(path.absolute())):
        raise ValueError("framework interpreter needs an absolute canonical parent directory")
    # A venv's python symlink is normal; pin both its spelling and executable.
    return path, path.resolve()


def inspect(python):
    """Describe installed content read-only; installing packages is separate."""
    python, executable = _interpreter(python)
    try:
        result = subprocess.run([str(python), "-I", "-B", "-c", _PROBE],
                                stdin=subprocess.DEVNULL, capture_output=True, timeout=30)
        if result.returncode:
            raise ValueError("framework interpreter probe failed: " + result.stderr.decode(errors="replace")[-2000:])
        info = parse_json(result.stdout)
    except (OSError, subprocess.TimeoutExpired) as error:
        raise ValueError("framework interpreter is unavailable") from error
    package, purelib, prefix = (Path(info[key]) for key in ("package", "purelib", "prefix"))
    if (info["editable"] is not False or package.parent != purelib
            or package.name != "match_recovery" or not (package / "__init__.py").is_file()
            or not purelib.is_relative_to(prefix)):
        raise ValueError("framework runtime requires a noneditable package in an isolated installation")
    configuration = prefix / "pyvenv.cfg"
    if not configuration.is_file() or configuration.resolve() != configuration:
        raise ValueError("framework runtime requires an explicit virtual environment")
    options = dict(line.lower().split("=", 1) for line in configuration.read_text().splitlines() if "=" in line)
    if {key.strip(): value.strip() for key, value in options.items()}.get("include-system-site-packages") != "false":
        raise ValueError("framework runtime must not inherit system site packages")
    package_sha, environment_sha = _installation(package, purelib)
    return {"python": str(python), "executable": str(executable), "executable_sha256": _file(executable),
            "package": str(package), "package_sha256": package_sha,
            "purelib": str(purelib), "environment_sha256": environment_sha,
            "prefix": str(prefix), "configuration_sha256": _file(configuration),
            "version": info["version"], "python_version": info["python_version"]}


def _unchanged(runtime):
    python, executable = _interpreter(runtime["python"])
    configuration = Path(runtime["prefix"]) / "pyvenv.cfg"
    if not configuration.is_file() or configuration.resolve() != configuration:
        raise ValueError("framework runtime configuration became unavailable or aliased")
    if str(executable) != runtime["executable"] or _file(executable) != runtime["executable_sha256"]:
        raise ValueError("framework runtime content changed; reauthorize before continuing")
    package = Path(runtime["package"])
    package_sha, environment_sha = _installation(package, Path(runtime["purelib"]))
    loaded_sha = package_sha if Path(__file__).parent == package else _tree(Path(__file__).parent)
    if (package_sha != runtime["package_sha256"] or environment_sha != runtime["environment_sha256"]
            or _file(configuration) != runtime["configuration_sha256"] or loaded_sha != runtime["package_sha256"]):
        raise ValueError("framework runtime content changed; reauthorize before continuing")


def _require_current(runtime):
    import sys
    import sysconfig
    if (Path(__file__).parent.resolve() != Path(runtime["package"])
            or Path(sys.prefix).resolve() != Path(runtime["prefix"])
            or Path(sysconfig.get_path("purelib")).resolve() != Path(runtime["purelib"])
            or not sys.dont_write_bytecode
            or sys.version != runtime["python_version"]
            or _file(Path(sys.executable).resolve()) != runtime["executable_sha256"]):
        raise ValueError("current environment differs from selected runtime; invoke the selected installed CLI with -B")


def _configuration(path):
    path = Path(path)
    if not path.is_absolute():
        path = path.absolute()
    if path.resolve() != path or not path.is_file() or path.stat().st_nlink != 1:
        raise ValueError("framework runtime binding must be a canonical unaliased file")
    value = read_json(path)
    if set(value) != {"schema_version", "runtime", "launchers"} or type(value["schema_version"]) is not int or value["schema_version"] != 1:
        raise ValueError("invalid framework runtime binding")
    launchers = value["launchers"]
    if not isinstance(launchers, list) or not launchers:
        raise ValueError("framework runtime requires explicit native launchers")
    for launcher in launchers:
        if (not isinstance(launcher, list) or not launcher
                or any(not isinstance(word, str) or not word or "{" in word or "}" in word for word in launcher)
                or not (Path(launcher[-1]).name in ("recover", "recover.exe")
                        or len(launcher) >= 3 and launcher[-2:] == ["-m", "match_recovery"])):
            raise ValueError("runtime launchers must explicitly name the native recovery CLI")
    if len({tuple(item) for item in launchers}) != len(launchers):
        raise ValueError("duplicate framework runtime launcher")
    runtime = value["runtime"]
    fields = {"python", "executable", "executable_sha256", "package", "package_sha256", "purelib",
              "environment_sha256", "prefix", "configuration_sha256", "version", "python_version"}
    if (not isinstance(runtime, dict) or set(runtime) != fields
            or any(not isinstance(item, str) or not item for item in runtime.values())):
        raise ValueError("invalid installed framework identity")
    for key in ("package_sha256", "environment_sha256", "configuration_sha256", "executable_sha256"):
        require_digest(runtime[key])
    # Check pins before executing even the read-only interpreter probe. A
    # replaced interpreter must not run merely to tell us it was replaced.
    try:
        _unchanged(runtime)
    except OSError as error:
        raise ValueError("framework runtime input disappeared or became unavailable") from error
    actual = inspect(Path(runtime["python"]))
    if actual != runtime:
        raise ValueError("framework runtime identity does not match the installed content")
    if _tree(Path(__file__).parent) != runtime["package_sha256"]:
        raise ValueError("current and selected framework implementations differ; use the selected installed CLI")
    _require_current(runtime)
    return path, value


@contextmanager
def binding(path):
    """Activate only an explicit caller-supplied configuration for this context."""
    path, value = _configuration(path)
    current = _ACTIVE.get()
    if current is not None and (path, value) != current:
        raise ValueError("cannot replace an active framework runtime binding")
    token = _ACTIVE.set((path, value))
    try:
        yield
    except BaseException:
        raise
    else:
        fingerprint()
    finally:
        _ACTIVE.reset(token)


def fingerprint():
    current = _ACTIVE.get()
    if current is None:
        return None
    path, value = current
    if path.resolve() != path or not path.is_file() or path.stat().st_nlink != 1 or read_json(path) != value:
        raise ValueError("framework runtime binding changed during execution")
    try:
        _unchanged(value["runtime"])
    except OSError as error:
        raise ValueError("framework runtime input disappeared or became unavailable") from error
    return digest(value)


def export_binding():
    """Serialize explicit authority for a newly created executor worker."""
    return str(_ACTIVE.get()[0]) if fingerprint() is not None else None


def initialize_worker(path):
    """Validate the selected installation in an executor's own process.

    The process owns this binding until exit. Each command rechecks its content;
    no environment variable or implicit inheritance supplies authority.
    """
    if path is None:
        if _ACTIVE.get() is not None:
            raise ValueError("executor omitted its inherited framework runtime authority")
        return
    selected = _configuration(path)
    current = _ACTIVE.get()
    if current is not None and current != selected:
        raise ValueError("executor cannot replace inherited runtime authority")
    _ACTIVE.set(selected)


def native_argv(suffix):
    import sys
    if fingerprint() is None:
        return [sys.executable, "-I", "-m", "match_recovery", *suffix]
    path, value = _ACTIVE.get()
    return [value["runtime"]["python"], "-I", "-B", "-m", "match_recovery", "--runtime", str(path), *suffix]


def resolve(argv):
    active = fingerprint()
    if argv and argv[0] == "{framework_python}":
        import sys
        python = sys.executable if active is None else _ACTIVE.get()[1]["runtime"]["python"]
        return [python, *argv[1:]]
    if active is None:
        return list(argv)
    _, value = _ACTIVE.get()
    for prefix in sorted(value["launchers"], key=len, reverse=True):
        if list(argv[:len(prefix)]) == prefix:
            return native_argv(argv[len(prefix):])
    return list(argv)


def child_environment():
    """Keep shebang and nested Python commands from changing pinned inputs.

    The parent's -B flag does not propagate through a build tool to env python3
    wrappers. Supply the standard interpreter cache control to children only;
    never mutate the coordinator's global environment or rewrite compiler argv.
    """
    if fingerprint() is None:
        return None
    import os
    return {**os.environ, "PYTHONDONTWRITEBYTECODE": "1"}


@contextmanager
def command(argv):
    """All configured command boundaries check runtime freshness, not just run."""
    selected = resolve(argv)
    yield selected
    # Exceptional commands cannot grant acceptance; preserve their original
    # interruption type so compiler leases retain their quarantine semantics.
    fingerprint()


def instructions():
    if fingerprint() is None:
        return ""
    from .model import canonical
    return ("For native recovery diagnostics use this exact installed framework argv prefix: "
            + canonical(native_argv([])).decode() + ". Append the recovery subcommand/options. "
            "Do not use a checkout-local recovery launcher or change the runtime binding. "
            "Compiler/provider hooks remain project-local; only explicit "
            "{framework_python} executables select this installation's interpreter.\n")
