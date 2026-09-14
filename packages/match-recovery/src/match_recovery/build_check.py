"""Fresh configured builds, without compiler/project-specific dispatch code.

Build commands are trusted tools, not sandboxed programs. Success establishes
only that the declared artifact was produced; never source coverage, byte
agreement, structural correctness or working application behavior.
"""

from dataclasses import dataclass
import hashlib
import os
from pathlib import Path
import re
import subprocess
import tempfile
from time import monotonic

from .components import relative_path
from .driver import Command, changes, working_state
from .fingerprint import input_snapshot
from .model import Result, digest
from .processes import run_bounded
from .records import read_json
from .source_replica import artifact_parent
from .toolchain import snapshot as toolchain_snapshot


def _input_parents(path, root):
    """Select the same in-root ancestors without testing every outer parent."""
    if not path.is_relative_to(root):
        return ()
    return path.parents[:len(path.parts) - len(root.parts)]


@dataclass(frozen=True)
class Build:
    path: str
    artifacts: str
    commands: tuple[Command, ...]
    outputs: tuple[tuple[str, str], ...]
    inputs: tuple[str, ...]
    input_roots: tuple[str, ...]
    environment: tuple[str, ...]
    versioned_inputs: tuple[str, ...] = ()
    compile_link: dict | None = None

    @classmethod
    def read(cls, root, *, key="build"):
        project = read_json(root / "recovery/project.json")
        path = relative_path(project.get(key))
        value = read_json(root / path)
        fields = {"schema_version", "artifacts", "commands", "outputs", "inputs", "input_roots", "environment"}
        if (not fields <= set(value) or set(value) - fields - {"versioned_inputs", "compile_link"}
                or type(value["schema_version"]) is not int or value["schema_version"] != 1):
            raise ValueError("invalid build configuration")
        for name in ("commands", "outputs", "inputs", "input_roots", "environment"):
            if not isinstance(value[name], list):
                raise ValueError(f"build {name} must be an array")
        if not value["commands"] or not value["outputs"] or not value["input_roots"]:
            raise ValueError("build needs commands, output artifacts and explicit input roots")
        commands = tuple(Command.read(item) for item in value["commands"])
        if not any("{build}" in word for command in commands for word in command.argv):
            raise ValueError("build commands must use their fresh {build} directory")
        outputs = []
        for item in value["outputs"]:
            if not isinstance(item, dict) or set(item) != {"path", "format"} or item["format"] not in ("file", "pe32-i386", "compilation-database", "msvc-map", "msvc-link-log", "link-input-files", "msvc-pdb"):
                raise ValueError("build outputs require a path and supported format")
            outputs.append((relative_path(item["path"]), item["format"]))
        if len({path for path, _ in outputs}) != len(outputs):
            raise ValueError("duplicate build output")
        from .build_compile_evidence import validate_configuration
        compile_link = validate_configuration(value.get("compile_link"), outputs)
        roots = tuple(relative_path(name) for name in value["input_roots"])
        inputs = tuple(relative_path(name) for name in value["inputs"])
        versioned = value.get("versioned_inputs", [])
        if not isinstance(versioned, list):
            raise ValueError("build versioned_inputs must be an array")
        versioned = tuple(relative_path(name) for name in versioned)
        if len(set(versioned)) != len(versioned):
            raise ValueError("duplicate versioned build input")
        for name in versioned:
            if (name in ("recovery/project.json", path)
                    or any(Path(name).is_relative_to(other) or Path(other).is_relative_to(name)
                           for other in (*roots, value["artifacts"], *(item for item in versioned if item != name)))):
                raise ValueError("versioned build inputs must be separate files outside policy, source and artifact roots")
        inputs = (*inputs, *(name for name in versioned if name not in inputs))
        if any(not isinstance(name, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", name)
               for name in value["environment"]):
            raise ValueError("invalid build environment name")
        environment = {"PATH", *value["environment"]}
        for command in commands:
            for word in command.argv:
                environment.update(re.findall(r"\{env:([A-Za-z_][A-Za-z_0-9]*)\}", word))
        if any(not isinstance(name, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", name) for name in environment):
            raise ValueError("invalid build environment name")
        return cls(path, relative_path(value["artifacts"]), commands, tuple(outputs), inputs, roots,
                   tuple(sorted(environment)), versioned, compile_link)

    def snapshot(self, root):
        files = {"recovery/project.json", self.path, *self.inputs}
        directories = set()
        links = {}
        for name in self.input_roots:
            directory = root / name
            if not directory.is_dir():
                raise ValueError(f"missing build input root: {name}")
            for path in (directory, *directory.rglob("*")):
                relative = path.relative_to(root).as_posix()
                if path.is_symlink():
                    raise ValueError(f"build inputs must not be symlinked: {relative}")
                if path.is_file():
                    files.add(relative)
                elif path.is_dir():
                    directories.add(relative)
                else:
                    raise ValueError(f"nonregular build input: {relative}")
        for name in files:
            path = root / name
            if name in self.versioned_inputs and (path.resolve() != path or not path.is_file()
                                                  or path.stat().st_nlink != 1):
                raise ValueError(f"versioned build input must be a canonical unaliased regular file: {name}")
            if (not path.is_file() or not path.resolve().is_relative_to(root)
                    or any(parent.is_symlink() for parent in _input_parents(path, root))):
                raise ValueError(f"missing or symlinked build input: {name}")
            # Explicit file aliases (e.g. compiler vs linker entry points) are
            # valid inputs. Pin their spelling, resolved target and file bytes;
            # never follow directory aliases or links outside the project.
            if path.is_symlink():
                links[name] = {"link": os.readlink(path), "resolved": path.resolve().relative_to(root).as_posix()}
        package = Path(__file__).parent
        implementation = {path.relative_to(package).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
                          for path in sorted(package.rglob("*.py"))}
        environment = {name: hashlib.sha256(os.environ[name].encode()).hexdigest() if name in os.environ else None
                       for name in self.environment}
        return input_snapshot(root, files, {"build": self, "directories": sorted(directories), "links": links,
                                           "implementation": implementation, "environment": environment,
                                           "toolchain": toolchain_snapshot(root)})


def check_build(root: Path, *, retain=False) -> dict:
    return generate_build(root, Build.read(root), retain=retain)


def generate_build(root: Path, settings: Build, *, retain=False) -> dict:
    """Run one fresh generator/build; retain compiler context if requested.

    Preparation databases may reference generated headers in this directory.
    They cannot outlive a temporary directory that has already been removed.
    Retained output (including failed commands) is disposable, ignored compiler
    context, never a verdict cache.
    """
    started = monotonic()
    root = root.resolve()
    before = settings.snapshot(root)
    state = working_state(root)
    parent = artifact_parent(root, settings.artifacts, excluded_roots=settings.input_roots)
    reports, outputs = [], []
    compile_link = {"result": Result.UNKNOWN, "verification": "not_measured",
                    "reason": "compile/link evidence requires successful commands and valid fresh artifacts"}
    result = Result.UNKNOWN
    reason = "build did not finish"
    try:
        with tempfile.TemporaryDirectory(prefix="build-", dir=parent, delete=not retain) as temporary:
            directory = Path(temporary)
            def expand(match):
                name = match[1]
                if name == "root":
                    return str(root)
                if name == "build":
                    return str(directory)
                if name.startswith("env:") and name[4:] in os.environ:
                    return os.environ[name[4:]]
                raise ValueError(f"missing build command input: {name}")
            for command in settings.commands:
                if settings.snapshot(root) != before:
                    raise ValueError("inputs changed before build command")
                argv = [re.sub(r"\{(root|build|env:[A-Za-z_][A-Za-z_0-9]*)\}", expand, word)
                        for word in command.argv]
                command_started = monotonic()
                try:
                    completed = run_bounded(argv, cwd=root, timeout=command.timeout)
                except (OSError, subprocess.TimeoutExpired) as error:
                    result = Result.UNKNOWN
                    message = (f"timed out after {command.timeout} seconds" if isinstance(error, subprocess.TimeoutExpired)
                               else f"{type(error).__name__}: {error.strerror}")
                    reports.append({"command": list(command.argv), "result": Result.UNKNOWN, "error": message,
                                    "elapsed_seconds": monotonic() - command_started})
                    reason = "build command unavailable or timed out"
                    break
                result = Result.PASS if completed.returncode == 0 else Result.FAIL if completed.returncode > 0 else Result.UNKNOWN
                reports.append({"command": list(command.argv), "result": result, "exit_code": completed.returncode,
                                "elapsed_seconds": monotonic() - command_started,
                                "stdout": completed.stdout.decode(errors="replace")[-8000:],
                                "stderr": completed.stderr.decode(errors="replace")[-8000:]})
                if result != Result.PASS:
                    reason = "build command failed or was interrupted"
                    break
            else:
                for name, format_name in settings.outputs:
                    path = directory / name
                    try:
                        if (not path.is_file() or any(part.is_symlink() for part in (path, *path.parents)
                                                     if part.is_relative_to(directory))):
                            raise ValueError("expected a fresh regular build artifact")
                        data = path.read_bytes()
                        if not data:
                            raise ValueError("build artifact is empty")
                        fingerprint = hashlib.sha256(data).hexdigest()
                        if format_name == "pe32-i386":
                            from .pe_coff import PeImage
                            PeImage(data, fingerprint)
                        elif format_name == "compilation-database":
                            from .compilation_database import entries
                            entries(data, directory)
                        elif format_name == "msvc-map":
                            from .msvc_link_map import parse_link_map
                            parse_link_map(data)
                        elif format_name == "msvc-link-log":
                            from .msvc_link_log import parse_link_log
                            parse_link_log(data)
                        elif format_name == "msvc-pdb":
                            from .msf import SmallMsf
                            # Container validity alone grants no image pairing,
                            # fixup completeness or recovery acceptance.
                            SmallMsf(data)
                        elif format_name == "link-input-files":
                            from .link_file_evidence import parse_link_files
                            parse_link_files(data)
                        outputs.append({"path": name, "format": format_name, "result": Result.PASS,
                                        "sha256": fingerprint, "size": len(data)})
                    except (OSError, ValueError) as error:
                        outputs.append({"path": name, "format": format_name, "result": Result.UNKNOWN, "reason": str(error)})
                result = Result.PASS if all(item["result"] == Result.PASS for item in outputs) else Result.UNKNOWN
                reason = "fresh configured artifacts produced" if result == Result.PASS else "build artifacts missing or invalid"
                if settings.compile_link is not None and result == Result.PASS:
                    from .build_compile_evidence import verify_build_compilations
                    try:
                        observed = verify_build_compilations(root, directory, settings.compile_link, outputs=outputs)
                        if not isinstance(observed, dict) or observed.get("result") not in tuple(Result):
                            raise ValueError("compile/link verifier returned no valid result")
                        compile_link = observed
                    except (OSError, ValueError) as error:
                        compile_link = {"result": Result.UNKNOWN, "verification": "not_measured", "reason": str(error)}
                    result = Result(compile_link["result"])
                    reason = ("fresh configured artifacts and compile/link evidence verified" if result == Result.PASS
                              else "compile/link evidence failed or remains unverified")
    finally:
        if changes(state, working_state(root)) or settings.snapshot(root) != before:
            raise ValueError("build changed source, policy or Git state; preserve changes and inspect before retrying")
    return {"check": "fresh_configured_build", "result": result, "reason": reason,
            "elapsed_seconds": monotonic() - started,
            "commands": reports, "outputs": outputs, "input_fingerprint": digest(before),
            **({"compile_link": compile_link} if settings.compile_link is not None else {}),
            **({"directory": directory.relative_to(root).as_posix()} if retain else {}),
            "fresh_directory": True, "persisted": False, "ready_for_collection": False,
            "limitations": ["trusted configured build commands; no sandbox or proof of source coverage",
                            "artifact format checks do not establish byte/reference, structure or runtime correctness",
                            "external toolchain/dependency installation identity remains project responsibility"]}
