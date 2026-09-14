"""Live application-build evidence for an explicit counterfactual source tree.

This does not choose a historical revision, authorize a changed graph or prove
preservation. The caller must consume evidence before the private context closes.
"""

from contextlib import contextmanager
from copy import deepcopy
from dataclasses import dataclass
import json
from pathlib import Path
from typing import Callable

from .build_check import Build, generate_build
from .build_compile_evidence import verify_build_compilations
from .compile_file_evidence import _observe
from .compiler import Compiler
from .driver import working_state
from .fingerprint import input_snapshot
from .project_replica import project_replica
from .records import read_json
from .source_replica import replica_snapshot, validate_tree


@dataclass(frozen=True)
class ApplicationBuild:
    root: Path
    directory: Path
    _report: dict
    check_current: Callable[[], None]

    @property
    def report(self):
        return deepcopy(self._report)


@contextmanager
def application_build(root, roots, files, *, versioned_files, retain=False):
    """Build exactly these sources/graph files under current configured policy.

    Explicit ignored inputs are copied; unrelated ignored caches are not. No
    preparation command runs, nor is its versioned-input policy inherited. A
    successful context still has only the configured build's evidence coverage.
    Retention keeps the complete private input/receipt/artifact tree for later
    inspection, not the live verification capability, which always expires.
    """
    root = Path(root).resolve()
    roots = validate_tree(roots, files)
    settings = Build.read(root)
    original = settings.snapshot(root)
    original_state = working_state(root)
    original_source = replica_snapshot(root, roots)
    project = read_json(root / "recovery/project.json")
    compiler = Compiler(root) if "compiler" in project else None
    # Build.snapshot enumerates declared input roots as well as explicit files.
    # project_replica's ordinary Git copy alone would miss ignored input files.
    inputs = set(original["inputs"])
    if compiler is not None:
        inputs.update(compiler.initial["inputs"])
    toolchain = original["configuration"]["toolchain"]
    if toolchain is not None:
        inputs.update(toolchain["policy"]["inputs"])
    policy_files = dict(compiler.initial["inputs"]) if compiler is not None else {}
    if toolchain is not None:
        policy_files.update(toolchain["policy"]["inputs"])
    if any(Path(name).is_relative_to(base) for name in ("recovery/project.json", settings.path)
           for base in roots):
        raise ValueError("application source roots cannot substitute recovery/build policy")
    if any(Path(settings.artifacts).is_relative_to(name) or Path(name).is_relative_to(settings.artifacts)
           for name in (*roots, *inputs, *settings.input_roots)):
        raise ValueError("application artifacts must be separate from explicit source and build inputs")

    def original_fresh():
        if (settings.snapshot(root) != original or working_state(root) != original_state
                or replica_snapshot(root, roots) != original_source
                or compiler is not None and compiler.snapshot() != compiler.initial):
            raise ValueError("original application inputs or Git state changed")

    try:
        with project_replica(root, roots, files, inputs=tuple(sorted(inputs)),
                             versioned_inputs=settings.versioned_inputs,
                             versioned_files=versioned_files, retain=retain) as replica:
            original_fresh()
            # Snapshotting a private toolchain executes its configured discovery
            # command. Refuse substituted scripts before that first execution.
            if input_snapshot(replica, policy_files, {})["inputs"] != policy_files:
                raise ValueError("application source/graph substitution changed compiler or toolchain policy inputs")
            private = settings.snapshot(replica)
            private_state = working_state(replica)
            private_source = replica_snapshot(replica, roots)
            private_compiler = Compiler(replica) if compiler is not None else None
            private_toolchain = private["configuration"]["toolchain"]
            if (compiler is not None and private_compiler.initial["inputs"] != compiler.initial["inputs"]
                    or toolchain is not None and private_toolchain["policy"]["inputs"] != toolchain["policy"]["inputs"]):
                raise ValueError("application source/graph substitution changed compiler or toolchain policy inputs")

            def retained(name):
                return name not in settings.versioned_inputs and not any(Path(name).is_relative_to(base) for base in roots)

            if ({name: value for name, value in private["inputs"].items() if retained(name)}
                    != {name: value for name, value in original["inputs"].items() if retained(name)}
                    or [name for name in private["configuration"]["directories"] if retained(name)]
                    != [name for name in original["configuration"]["directories"] if retained(name)]):
                raise ValueError("application replica did not retain exact non-source build inputs")

            def inputs_fresh():
                original_fresh()
                if (settings.snapshot(replica) != private or working_state(replica) != private_state
                        or replica_snapshot(replica, roots) != private_source
                        or private_compiler is not None and private_compiler.snapshot() != private_compiler.initial):
                    raise ValueError("private application inputs or Git state changed")

            report = generate_build(replica, settings, retain=True)
            inputs_fresh()
            if report["result"] != "pass":
                raise ValueError("counterfactual application build failed or remains unverified: "
                                 + json.dumps(report, sort_keys=True))
            directory = replica / report["directory"]
            expected = deepcopy(report)
            live = True

            def fresh():
                if not live:
                    raise ValueError("application build context has expired")
                inputs_fresh()
                try:
                    for output in expected["outputs"]:
                        if _observe(directory / output["path"]) != {
                                "sha256": output["sha256"], "size": output["size"]}:
                            raise ValueError("application build artifact changed")
                    if settings.compile_link is not None:
                        observed = verify_build_compilations(replica, directory, settings.compile_link,
                                                             outputs=expected["outputs"])
                        # Includes the original receipt/file epoch fingerprint.
                        # A newly self-consistent replacement cannot refresh it.
                        if observed != expected["compile_link"]:
                            raise ValueError("application compile/link evidence changed")
                finally:
                    inputs_fresh()

            try:
                fresh()
                yield ApplicationBuild(replica, directory, deepcopy(expected), fresh)
            finally:
                try:
                    fresh()
                finally:
                    live = False
    finally:
        original_fresh()
