"""Configured compiler interaction, without a target, object format or flag policy.

Commands are trusted project inputs, not a sandbox. Object bytes are transient;
this boundary never treats a successful compiler invocation as verification.
"""

import base64
from dataclasses import asdict, dataclass
import hashlib
import os
from pathlib import Path
import re
import subprocess

from .components import local_path, relative_path
from .driver import Command, invoke
from .fingerprint import input_snapshot
from .model import require_text
from .records import read_json
from .source_replica import replica_roots, replica_snapshot, snapshot_digest, validate_tree
from .toolchain import snapshot as toolchain_snapshot


@dataclass(frozen=True)
class CompilationRequest:
    """Validated compiler-command input; compiler option semantics stay local.

    Use this at the project command boundary, before invoking any compiler.
    Existing-file digests identify the overlay inputs, not proof that the
    command used them. Replica and damage controls still belong to measurement.
    """
    source: Path
    profile: str
    arguments: tuple[str, ...]
    flags: tuple[str, ...]
    replacements: dict[str, bytes] | None
    tree_roots: tuple[str, ...] | None = None
    tree_files: dict[str, bytes] | None = None

    @classmethod
    def read(cls, root: Path, value: dict):
        root = root.resolve()
        if not isinstance(value, dict):
            raise ValueError("compiler request must be an object")
        version = value.get("schema_version")
        fields = {"schema_version", "profile", "source", "arguments", "flags"}
        if version == 4:
            fields.add("overrides")
        if version == 5:
            fields.add("tree")
        if type(version) is not int or version not in (2, 4, 5) or set(value) != fields:
            raise ValueError("invalid compiler request")
        require_text(value["profile"], "compiler profile")
        for name in ("arguments", "flags"):
            if not isinstance(value[name], list):
                raise ValueError("compiler arguments and flags must be string arrays")
            for word in value[name]:
                require_text(word, "compiler argument")
        def existing(name):
            path = local_path(root, name)
            if path.resolve() != path or not path.is_file():
                raise ValueError("compiler request requires existing canonical regular files")
            return path
        tree_roots, tree_files = None, None
        if version == 5:
            tree = value["tree"]
            if (not isinstance(tree, dict) or set(tree) != {"roots", "current_sha256", "files"}
                    or not isinstance(tree["roots"], list) or not isinstance(tree["files"], dict)):
                raise ValueError("invalid complete source tree request")
            tree_roots = replica_roots(tree["roots"])
            if list(tree_roots) != tree["roots"]:
                raise ValueError("source tree roots must be canonical and nonoverlapping")
            tree_files = {}
            for name, encoded in tree["files"].items():
                if not isinstance(encoded, str):
                    raise ValueError("source tree files require base64 strings")
                tree_files[name] = base64.b64decode(encoded, validate=True)
            validate_tree(tree_roots, tree_files)
            if snapshot_digest(replica_snapshot(root, tree_roots)) != tree["current_sha256"]:
                raise ValueError("source tree requires the unchanged current input digest")
            source = local_path(root, value["source"])
            if source.resolve() != source or value["source"] not in tree_files:
                raise ValueError("compiler source must be a canonical file in the supplied tree")
        else:
            source = existing(value["source"])
        replacements = None
        if version == 4:
            if not isinstance(value["overrides"], dict) or not value["overrides"]:
                raise ValueError("source overlay requires nonempty file replacements")
            replacements = {}
            for name, item in value["overrides"].items():
                path = existing(name)
                if (not isinstance(item, dict) or set(item) != {"sha256", "base64"}
                        or not isinstance(item["base64"], str)
                        or item["sha256"] != hashlib.sha256(path.read_bytes()).hexdigest()):
                    raise ValueError("source overlay requires the unchanged original file digest")
                replacements[name] = base64.b64decode(item["base64"], validate=True)
        return cls(source, value["profile"], tuple(value["arguments"]), tuple(value["flags"]), replacements, tree_roots, tree_files)


class Compiler:
    def __init__(self, root: Path, *, profile: str = "default"):
        self.root = root.resolve()
        require_text(profile, "compiler profile")
        self.profile = profile
        project = read_json(local_path(self.root, "recovery/project.json"))
        if type(project.get("schema_version")) is not int or project["schema_version"] != 1:
            raise ValueError("unsupported project configuration version")
        self.configuration = project.get("compiler")
        value = self.configuration
        if (not isinstance(value, dict) or set(value) != {"command", "inputs", "environment"}
                or not isinstance(value["inputs"], list) or not isinstance(value["environment"], list)):
            raise ValueError("compiler configuration needs command, inputs and environment")
        self.command = Command.read(value["command"])
        for name in value["inputs"]:
            relative_path(name)
        for name in value["environment"]:
            if not isinstance(name, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z_0-9]*", name):
                raise ValueError("invalid compiler environment variable name")
        self.environment = {"PATH", *value["environment"]}
        for argument in self.command.argv:
            self.environment.update(re.findall(r"\{env:([A-Za-z_][A-Za-z_0-9]*)\}", argument))
        self.initial = self.snapshot()

    def snapshot(self):
        if read_json(local_path(self.root, "recovery/project.json")).get("compiler") != self.configuration:
            raise ValueError("compiler configuration changed")
        # Hash environment values rather than including potentially sensitive
        # strings in diagnostics. An unset variable differs from an empty one.
        environment = {name: hashlib.sha256(os.environ[name].encode()).hexdigest()
                       if name in os.environ else None for name in sorted(self.environment)}
        result = input_snapshot(self.root, ["recovery/project.json", *self.configuration["inputs"]],
                                {"command": asdict(self.command), "profile": self.profile, "environment": environment,
                                 "toolchain": toolchain_snapshot(self.root)})
        if any(value is None for value in result["inputs"].values()):
            raise ValueError("declared compiler input is missing")
        return result

    def __call__(self, source: Path, arguments: tuple[str, ...], flags: tuple[str, ...]) -> bytes:
        return self._compile(source, arguments, flags)

    def overlay(self, source: Path, replacements: dict[str, bytes], arguments: tuple[str, ...], flags: tuple[str, ...]) -> bytes:
        """Compile an atomic set of proposed TU/header bytes using protocol v4.

        Paths name existing project files. The local command owns include/path
        semantics; the caller must control replica equivalence and coverage.
        No successful object response is itself a preservation/recovery proof.
        """
        if not isinstance(replacements, dict) or not replacements:
            raise ValueError("compiler overlay requires nonempty file replacements")
        originals, overrides = {}, {}
        for name, data in sorted(replacements.items()):
            relative_path(name)
            path = local_path(self.root, name)
            if path.resolve() != path or not path.is_file():
                raise ValueError("compiler overlay requires existing canonical regular files")
            if not isinstance(data, bytes):
                raise ValueError("compiler overlay contents must be bytes")
            originals[name] = path.read_bytes()
            overrides[name] = {"sha256": hashlib.sha256(originals[name]).hexdigest(),
                               "base64": base64.b64encode(data).decode("ascii")}
        try:
            return self._compile(source, arguments, flags, overrides=overrides)
        finally:
            for name, original in originals.items():
                path = self.root / name
                if path.resolve() != path or not path.is_file() or path.read_bytes() != original:
                    raise ValueError("compiler overlay input changed during compilation")

    def materialize(self, source: Path, roots: tuple[str, ...], files: dict[str, bytes],
                    arguments: tuple[str, ...], flags: tuple[str, ...]) -> bytes:
        """Compile a complete private source tree via protocol v5.

        No compiler recipe is inferred for a missing/renamed translation unit.
        Commands without this protocol must refuse it, not fall back to v4.
        """
        roots = validate_tree(roots, files)
        before = replica_snapshot(self.root, roots)
        tree = {"roots": list(roots), "current_sha256": snapshot_digest(before),
                "files": {name: base64.b64encode(data).decode("ascii") for name, data in sorted(files.items())}}
        try:
            return self._compile(source, arguments, flags, tree=tree)
        finally:
            if replica_snapshot(self.root, roots) != before:
                raise ValueError("compiler source tree inputs changed during compilation")

    def _compile(self, source, arguments, flags, *, overrides=None, tree=None):
        if tree is not None and source.resolve() != source:
            raise ValueError("compiler source tree path must be canonical")
        source = source.resolve()
        if not source.is_relative_to(self.root) or (tree is None and not source.is_file()):
            raise ValueError("compiler source must be an existing file inside the project")
        if tree is not None and source.relative_to(self.root).as_posix() not in tree["files"]:
            raise ValueError("compiler source is absent from supplied tree")
        original = source.read_bytes() if source.is_file() else None
        for value in (*arguments, *flags):
            require_text(value, "compiler argument")
        if self.snapshot() != self.initial:
            raise ValueError("compiler inputs changed before compilation")
        request = {"schema_version": 2, "profile": self.profile, "source": source.relative_to(self.root).as_posix(),
                   "arguments": list(arguments), "flags": list(flags)}
        if overrides is not None:
            request.update(schema_version=4, overrides=overrides)
        if tree is not None:
            request.update(schema_version=5, tree=tree)
        try:
            code, output = invoke(self.command, self.root, "", request)
        except subprocess.SubprocessError as error:
            raise ValueError("compiler command did not complete") from error
        finally:
            if self.snapshot() != self.initial:
                raise ValueError("compiler inputs changed during compilation")
            if (source.read_bytes() if source.is_file() else None) != original:
                raise ValueError("compiler source changed during compilation")
        if code != 0:
            raise ValueError("compiler command failed: " + str(output.get("error", "no diagnostic"))[:2000])
        if set(output) != {"object_base64"} or not isinstance(output["object_base64"], str):
            raise ValueError("compiler success must contain only object_base64")
        try:
            data = base64.b64decode(output["object_base64"], validate=True)
        except ValueError as error:
            raise ValueError("invalid compiler object encoding") from error
        if not data:
            raise ValueError("compiler returned an empty object")
        return data
