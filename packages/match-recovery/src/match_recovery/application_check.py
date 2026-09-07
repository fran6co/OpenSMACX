"""Joint scoped evidence for structure-led recovery, never full acceptance."""

import hashlib
from pathlib import Path

from match_recovery.model import Result, digest
from match_recovery.tasks import TaskStore
from .application_binary import verify_task
from .application_layout import verify_layout
from .compiler import Compiler
from .measurement import Measurement


def check_task(root: Path, task_name: str, image: Path, *,
               binary_checker=None, layout_checker=None,
               compiler=None, command_for=None) -> dict:
    root = root.resolve()
    store = TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    settings = Measurement.read(root)

    def snapshot():
        with image.open("rb") as stream:
            image_sha = hashlib.file_digest(stream, "sha256").hexdigest()
        return settings.snapshot(root,
                              {"task": store.read(task_name), "decisions": store.decisions(task_name),
                               "image_sha256": image_sha})

    before = snapshot()
    objects = {}
    compilation = {"attempts": 0, "overlay_attempts": 0, "reused_objects": 0,
                   "debug": list(settings.layout_flags), "persistent_cache": False}

    class SharedCompiler:
        def provider(self):
            nonlocal compiler
            if snapshot() != before:
                raise ValueError("inputs changed before shared compilation")
            if compiler is None:
                compiler = Compiler(root, profile=settings.compiler_profile)
            return compiler

        def unchanged(self):
            if snapshot() != before:
                raise ValueError("inputs changed during shared compilation")

        def __call__(self, source, command, flags):
            # Only this invocation owns these objects. Check all live inputs
            # even before a hit; exceptions are never memoized.
            provider = self.provider()
            key = (source.resolve(), tuple(command), tuple(flags))
            if key in objects:
                compilation["reused_objects"] += 1
                return objects[key]
            compilation["attempts"] += 1
            try:
                value = provider(source, command, flags)
            finally:
                self.unchanged()
            objects[key] = value
            return value

        def overlay(self, source, replacements, command, flags):
            provider = self.provider()
            if not callable(getattr(provider, "overlay", None)):
                raise ValueError("compiler does not support private source overlays")
            compilation["attempts"] += 1
            compilation["overlay_attempts"] += 1
            # Never satisfy an unchanged-overlay control from the ordinary
            # object cache. Nor may a probe object replace product code in it.
            try:
                return provider.overlay(source, replacements, command, flags)
            finally:
                self.unchanged()

    shared_compile = SharedCompiler()

    if binary_checker is None:
        binary_checker = lambda *args: verify_task(*args, compiler=shared_compile,
                                                  command_for=command_for, debug_types=True)
    if layout_checker is None:
        layout_checker = lambda *args: verify_layout(*args, compiler=shared_compile,
                                                    command_for=command_for, witness_missing=True)
    checks = {}
    # A failed byte match must not prevent discovery of a structural regression,
    # and a passing body must not hide unknown or regressed layout evidence.
    for name, checker, args in (("binary", binary_checker, (root, task_name, image)),
                                ("layout", layout_checker, (root, task_name))):
        try:
            report = checker(*args)
            if not isinstance(report, dict) or report.get("task_id") != task_name:
                raise ValueError("checker returned a report for another task")
            Result(report.get("result"))
            checks[name] = report
        except (OSError, ValueError) as error:
            checks[name] = {"task_id": task_name, "result": Result.UNKNOWN, "error": str(error)}
    if before != snapshot() or task != store.read(task_name) or decisions != store.decisions(task_name):
        raise ValueError("source, image, build inputs or task changed between scoped checks; retry at a checkpoint")
    results = [report["result"] for report in checks.values()]
    result = (Result.UNKNOWN if Result.UNKNOWN in results else
              Result.FAIL if Result.FAIL in results else Result.PASS)
    return {"task_id": task_name, "check": "scoped_binary_and_layout", "result": result,
            "checks": checks, "compilation": compilation, "input_fingerprint": digest(before),
            "scope_complete": False, "ready_for_collection": False, "persisted": False,
            "build_verification": Result.UNKNOWN, "structure_verification": Result.UNKNOWN,
            "runtime_verification": Result.UNKNOWN,
            "limitations": ["joint scoped diagnostics only; neither byte matching nor declared offsets prove full structure",
                            "no full-project build, outside-scope obligations, runtime or independent review acceptance",
                            "fresh sequential measurements; diagnostic fingerprint is not a reusable cache key"]}
