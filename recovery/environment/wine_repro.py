"""Fail-closed VC6 reproducibility policy, shared by build and measurements.

This is project compiler interaction, not a recovery verdict. /Brepro removes
the time-seeded namespace nonce but also changes predefined macros. Three fresh
preprocessing passes must preserve source-visible text before it may be used.
"""

from dataclasses import dataclass, field
import hashlib
import os
from pathlib import Path
import re


ROOT_OPTION = "--repro-root="
_PATH_OPTIONS = ("/Fo", "/Fd", "/Fa", "/Fe", "/FI", "/I")
_OUTPUT_OPTIONS = ("/Fo", "/Fd", "/Fa", "/Fe")
_LINE = re.compile(rb'^[ \t]*#line[ \t]+[0-9]+[ \t]+"((?:[^"\\]|\\[\\"])*)"[ \t]*\r?\n?$')


def _path(value, cwd):
    if len(value) > 1 and value[1] == ":":
        if value[0].lower() != "z":
            raise ValueError("reproducible compilation supports POSIX or Wine Z: paths only")
        value = value[2:].replace("\\", "/")
    path = Path(value)
    return path if path.is_absolute() else cwd / path


def _preprocess_arguments(arguments):
    return [word for word in arguments if word not in ("/c", "-c")
            and not word.startswith(_OUTPUT_OPTIONS)] + ["/E"]


def _preprocessed(data, cwd):
    """Strip only parsed line metadata; never rewrite code/literal filenames."""
    text, files = [], set()
    for line in data.splitlines(keepends=True):
        match = _LINE.fullmatch(line)
        if match:
            name = re.sub(rb'\\([\\"])', rb'\1', match[1])
            path = _path(os.fsdecode(name), cwd)
            if not path.is_file():
                raise ValueError("unobserved preprocessor file: " + str(path))
            files.add(path)
        elif line.lstrip().startswith(b"#line"):
            raise ValueError("unsupported preprocessor line metadata: " + repr(line[:240]))
        else:
            text.append(line)
    if not files:
        raise ValueError("preprocessor supplied no observed input files")
    return b"".join(text), files


def _snapshot(files):
    return {str(path): (str(path.resolve()), hashlib.sha256(path.read_bytes()).hexdigest())
            for path in files}


def _configuration(tool, arguments):
    roots = [word[len(ROOT_OPTION):] for word in arguments if word.startswith(ROOT_OPTION)]
    if not roots:
        return None
    if tool != "cl" or len(roots) != 1 or not roots[0]:
        raise ValueError("one explicit reproducible source root is supported only for cl")
    root = Path(roots[0])
    if not root.is_absolute() or root.resolve() != root or not root.is_dir():
        raise ValueError("reproducible source root must be a canonical absolute directory")
    if any(value for key, value in os.environ.items() if key.upper() in ("CL", "_CL_")):
        raise ValueError("reproducible compilation refuses implicit CL options")
    return root, [word for word in arguments if not word.startswith(ROOT_OPTION)]


def preprocessing_probe(tool, arguments):
    """IDE /E probes observe the ordinary compiler, not reproducible objects.

    Keep their cwd, external temporary source and macros intact. /Be and /Bd
    are diagnostic switches used by IDEs; opaque compiler forwarding and file
    outputs are not a preprocessing-only escape from the object guard.
    """
    configured = _configuration(tool, arguments)
    if configured is None:
        return None
    _, ordinary = configured
    # A standalone /I or /D consumes its next word: '/I /E' must not be
    # mistaken for preprocessing and let an ordinary compile escape its guard.
    # Classify the actual forwarded argv, after removing wrapper metadata.
    words = []
    iterator = iter(ordinary)
    valued = (*_PATH_OPTIONS, "/D", "/U", "/Tc", "/Tp", "/Fp", "/Fi",
              "/Fr", "/FR", "/F", "/wd", "/we", "/wo", "/Zm")
    for raw in iterator:
        word = "/" + raw[1:] if raw.startswith("-") else raw
        words.append(word)
        if word in valued:
            next(iterator, None)
    if not any(word in ("/E", "/EP") for word in words) or "/c" in words:
        return None
    for raw in ordinary:
        word = "/" + raw[1:] if raw.startswith("-") else raw
        if (word.startswith(("/Y", "/Fp", "/Fo", "/Fd", "/Fa", "/Fe", "/Fi", "/Fr", "/FR", "/d", "@"))
                or (word.startswith("/B") and word not in ("/Be", "/Bd"))
                or word in ("/P", "/link", "/Zs")):
            raise ValueError("unsupported preprocessing probe option: " + raw)
    return ordinary


@dataclass
class ReproducibleCompile:
    root: Path
    cwd: Path
    source: Path
    original: list[str]
    stable: list[str]
    inputs: dict = field(default_factory=dict)

    @classmethod
    def read(cls, tool, arguments, cwd):
        configured = _configuration(tool, arguments)
        if configured is None:
            return None
        root, arguments = configured
        if not any(word in ("/c", "-c") for word in arguments):
            raise ValueError("reproducible compilation requires compile-only /c")
        stable, original, sources = [], [], []
        iterator = iter(arguments)
        for raw in iterator:
            word = "/" + raw[1:] if raw.startswith("-") else raw
            if word.startswith(("/Y", "/Fp", "/B", "/d", "/Fi", "/Tc", "/Tp", "/Fr", "/FR", "@")) or word in ("/E", "/EP", "/P", "/Zi", "/ZI", "/link", "/F", "/Zs"):
                raise ValueError("unsupported reproducible compiler option: " + raw)
            prefix = next((part for part in _PATH_OPTIONS if word.startswith(part)), None)
            if prefix:
                value = word[len(prefix):] or next(iterator, "")
                if not value:
                    raise ValueError("missing compiler path argument: " + word)
                resolved = str(_path(value, cwd).absolute())
                original.append(prefix + value)
                stable.append(prefix + resolved)
            elif Path(raw).suffix.lower() in (".cpp", ".c", ".cxx") and _path(raw, cwd).is_file():
                source = _path(raw, cwd)
                if source.resolve() != source or not source.is_relative_to(root):
                    raise ValueError("reproducible source must be canonical and inside its explicit root")
                sources.append(source)
                original.append(raw)
                stable.append(source.relative_to(root).as_posix())
            elif not raw.startswith(("/", "-")):
                raise ValueError("unsupported reproducible compiler input: " + raw)
            else:
                original.append(raw)
                stable.append(raw)
        if len(sources) != 1 or not any(word.startswith("/Fo") for word in stable):
            raise ValueError("reproducible compilation requires one source and an explicit object output")
        return cls(root, cwd, sources[0], original, stable)

    def verify(self, preprocess):
        """The callback receives argv/cwd and returns unmodified stdout bytes."""
        ordinary, files = _preprocessed(preprocess(_preprocess_arguments(self.original), self.cwd), self.cwd)
        files.add(self.source)
        self.inputs = _snapshot(files)
        for extra in ([], ["/Brepro"]):
            self.check_inputs()
            text, observed = _preprocessed(preprocess(_preprocess_arguments(self.stable) + extra, self.root), self.root)
            self.check_inputs()
            if text != ordinary or {path.resolve() for path in observed} != {path.resolve() for path in files}:
                raise ValueError("reproducible compilation changes preprocessing or observed inputs")
        return [*self.stable, "/Brepro"]

    def check_inputs(self):
        if not self.inputs or _snapshot(Path(name) for name in self.inputs) != self.inputs:
            raise ValueError("compiler inputs changed during reproducibility verification")

    def write_depfile(self, path, target):
        def escape(value):
            return str(value).replace("$", "$$").replace("\\", "\\\\").replace(" ", "\\ ").replace("#", "\\#")
        destination = _path(path, self.cwd)
        destination.parent.mkdir(parents=True, exist_ok=True)
        continuation = " " + chr(92) + "\n  "
        destination.write_text(escape(target) + ":" + "".join(continuation + escape(name) for name in sorted(self.inputs)) + "\n")
