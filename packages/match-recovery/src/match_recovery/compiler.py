"""Configured compiler interaction, without a target, object format or flag policy.

Commands are trusted project inputs, not a sandbox. Object bytes are transient;
this boundary never treats a successful compiler invocation as verification.
"""

import base64
from dataclasses import asdict
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
                                {"command": asdict(self.command), "profile": self.profile, "environment": environment})
        if any(value is None for value in result["inputs"].values()):
            raise ValueError("declared compiler input is missing")
        return result

    def __call__(self, source: Path, arguments: tuple[str, ...], flags: tuple[str, ...]) -> bytes:
        return self._compile(source, arguments, flags)

    def variant(self, source: Path, contents: bytes, arguments: tuple[str, ...], flags: tuple[str, ...]) -> bytes:
        """Compile proposed whole-TU bytes, never edit the original source.

        Protocol v3 requires explicit support in the configured command. The
        command owns include/path semantics; callers own replica controls and
        dependency freshness. An object is not a measurement or a verdict.
        """
        if not isinstance(contents, bytes):
            raise ValueError("compiler variant contents must be bytes")
        return self._compile(source, arguments, flags, contents=contents)

    def _compile(self, source, arguments, flags, *, contents=None):
        source = source.resolve()
        if not source.is_relative_to(self.root) or not source.is_file():
            raise ValueError("compiler source must be an existing file inside the project")
        original = source.read_bytes()
        for value in (*arguments, *flags):
            require_text(value, "compiler argument")
        if self.snapshot() != self.initial:
            raise ValueError("compiler inputs changed before compilation")
        request = {"schema_version": 2, "profile": self.profile, "source": source.relative_to(self.root).as_posix(),
                   "arguments": list(arguments), "flags": list(flags)}
        if contents is not None:
            request.update(schema_version=3, source_sha256=hashlib.sha256(original).hexdigest(),
                           source_base64=base64.b64encode(contents).decode("ascii"))
        try:
            code, output = invoke(self.command, self.root, "", request)
        except subprocess.SubprocessError as error:
            raise ValueError("compiler command did not complete") from error
        finally:
            if self.snapshot() != self.initial:
                raise ValueError("compiler inputs changed during compilation")
            if not source.is_file() or source.read_bytes() != original:
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
