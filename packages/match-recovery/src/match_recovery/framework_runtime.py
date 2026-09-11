"""Explicit installed framework selection, separate from project compiler hooks.

No environment variable activates this binding. The caller supplies a pinned
configuration; only its exact declared recovery launchers are redirected. This
identifies a local installation, not a hermetic Python/OS or behavioral proof.
"""

from contextlib import contextmanager
from contextvars import ContextVar
import hashlib
from pathlib import Path
import subprocess

from .model import digest, require_digest
from .records import parse_json, read_json


_ACTIVE = ContextVar("match_recovery_framework_runtime", default=None)
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
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def _tree(path, *, installed=False):
    if path.resolve() != path or not path.is_dir():
        raise ValueError("framework runtime requires canonical installed directories")
    values = {}
    for item in sorted(path.rglob("*")):
        if item.is_symlink():
            raise ValueError("framework installation cannot contain file or directory aliases")
        if item.is_file():
            if item.suffix in (".pyc", ".pyo"):
                if installed:
                    raise ValueError("framework runtime must be installed without bytecode caches; execute with -B")
                continue
            values[item.relative_to(path).as_posix()] = _file(item)
        elif not item.is_dir():
            raise ValueError("framework installation contains a nonregular input")
    return digest(values)


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
    return {"python": str(python), "executable": str(executable), "executable_sha256": _file(executable),
            "package": str(package), "package_sha256": _tree(package, installed=True),
            "purelib": str(purelib), "environment_sha256": _tree(purelib, installed=True),
            "prefix": str(prefix), "configuration_sha256": _file(configuration),
            "version": info["version"], "python_version": info["python_version"]}


def _unchanged(runtime):
    python, executable = _interpreter(runtime["python"])
    configuration = Path(runtime["prefix"]) / "pyvenv.cfg"
    if not configuration.is_file() or configuration.resolve() != configuration:
        raise ValueError("framework runtime configuration became unavailable or aliased")
    if (str(executable) != runtime["executable"] or _file(executable) != runtime["executable_sha256"]
            or _tree(Path(runtime["package"]), installed=True) != runtime["package_sha256"]
            or _tree(Path(runtime["purelib"]), installed=True) != runtime["environment_sha256"]
            or _file(configuration) != runtime["configuration_sha256"]
            or _tree(Path(__file__).parent) != runtime["package_sha256"]):
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
    if fingerprint() is None:
        return list(argv)
    _, value = _ACTIVE.get()
    for prefix in sorted(value["launchers"], key=len, reverse=True):
        if list(argv[:len(prefix)]) == prefix:
            return native_argv(argv[len(prefix):])
    return list(argv)


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
            "Compiler/provider hooks remain project-local and unchanged.\n")
