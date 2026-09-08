"""Non-overwriting local project setup; no target registry or bundled project code."""

import hashlib
from pathlib import Path, PurePosixPath

from .components import relative_path
from .model import require_text
from .records import publish_json, publish_text


def initialize(root: Path, target: str, python: str, *, image: Path | None = None,
               source_root: str = "src") -> dict:
    """Configure native operations; compiler/build semantics remain external.

    The supplied image establishes identity only. No compiler flags, original
    ownership, historical baseline or completed verification are inferred.
    Current native measurement supports PE/COFF i386, not arbitrary binaries.
    """
    require_text(target, "target")
    require_text(python, "Python executable")
    root = root.resolve()
    relative_path(source_root)
    source = PurePosixPath(source_root)
    if any(source.is_relative_to(name) or PurePosixPath(name).is_relative_to(source)
           for name in ("recovery", "build", ".git")):
        raise ValueError("source root must not overlap recovery, build or Git metadata")
    pin_path = source_root + "/IMAGE_PIN"
    files = ["recovery/" + name for name in
             ("driver.json", "project.json", "build.json", "gate.json", "audits.json",
              "worker.json", "reviewer.json", "agents.json")]
    if image is not None:
        files.append(pin_path)
    for name in [*files, source_root]:
        path = root / name
        if any(parent.is_symlink() for parent in (path, *path.parents) if parent.is_relative_to(root)):
            raise ValueError("project setup paths must stay inside the project without symlinks")
        if any(parent.exists() and not parent.is_dir() for parent in path.parents if parent.is_relative_to(root)):
            raise ValueError("project setup parent is not a directory")
    for name in files:
        path = root / name
        if path.exists() or path.is_symlink():
            raise ValueError(f"project setup refuses to overwrite {path}")
    if (root / source_root).exists() and not (root / source_root).is_dir():
        raise ValueError("project source root is not a directory")
    pin = None
    if image is not None:
        image = image if image.is_absolute() else root / image
        with image.open("rb") as stream:
            pin = hashlib.file_digest(stream, "sha256").hexdigest()
    directory = root / "recovery"
    command = lambda *args: {"argv": [python, "-m", "match_recovery.cli", *args], "timeout": 3600}
    publish_json(directory / "driver.json", {"schema_version": 1, "target": target,
                 "context": command("tasks", "context", "{task}"),
                 "obligations": command("tasks", "obligations", "{task}"),
                 "check": command("tasks", "check-restoration", "{task}", "--image", "{env:RECOVERY_IMAGE}"),
                 "build": command("check-build"),
                 "migration_check": command("tasks", "check-migration", "{task}", "--image", "{env:RECOVERY_IMAGE}"),
                 "acceptance": command("tasks", "check-acceptance", "{task}", "--image", "{env:RECOVERY_IMAGE}")})
    publish_json(directory / "project.json", {"schema_version": 1,
                 "source": {"image_pin": pin_path, "address_space": "pe:va", "roots": [source_root],
                            "include_paths": [source_root]},
                 "compiler": {"command": {"argv": ["{env:RECOVERY_COMPILER}"], "timeout": 330},
                              "inputs": [], "environment": []},
                 "measurement": {"backend": "pe-coff-i386", "compiler_profile": "application",
                                 "flag_sets": [[]], "layout_flags": [],
                                 "unit_extensions": [".cpp", ".c", ".cc", ".cxx"],
                                 "inputs": [], "input_roots": []},
                 "mappings": {"compile_commands": "build/compile_commands.json"},
                 "build": "recovery/build.json", "gate": "recovery/gate.json", "audits": "recovery/audits.json"})
    publish_json(directory / "build.json", {"schema_version": 1, "artifacts": "build/recovery/builds",
                 "commands": [{"argv": ["{env:RECOVERY_BUILD}", "{root}", "{build}"], "timeout": 3600}],
                 "outputs": [{"path": "application.exe", "format": "pe32-i386"}],
                 "inputs": [], "input_roots": [source_root], "environment": []})
    publish_json(directory / "gate.json", {"schema_version": 1, "checks": {}, "inputs": [],
                                          "input_roots": [], "environment": []})
    publish_json(directory / "audits.json", {"schema_version": 1, "audits": {}})
    agents = {"schema_version": 1, "artifacts": "build/recovery/agents"}
    for role in ("worker", "reviewer"):
        publish_json(directory / (role + ".json"), {"author": role,
                     "command": {"argv": [python, "-m", "match_recovery.cli", "agent", role], "timeout": 3600}})
        agents[role] = {"command": {"argv": ["{env:RECOVERY_" + role.upper() + "}"], "timeout": 3300},
                        "instructions": [], "response": "stdout"}
    publish_json(directory / "agents.json", agents)
    (root / source_root).mkdir(parents=True, exist_ok=True)
    if pin is not None:
        publish_text(root / pin_path, "# Original image identity; not a recovery verdict.\nsha256 " + pin + "\n")
    return {"created": files, "target": target, "image_sha256": pin,
            "ready_for_recovery": False,
            "next_steps": ["Pin the original with --image during init, or supply the configured source IMAGE_PIN; set RECOVERY_IMAGE when running checks.",
                           "Configure RECOVERY_COMPILER (compiler JSON protocol) and RECOVERY_BUILD (root and fresh build directory arguments), or replace their argv arrays.",
                           "Configure actual flags, debug-type flags, build outputs, compile database and all compiler/build inputs; no compiler or ABI was inferred.",
                           "The native measurement starter supports PE/COFF i386 only; other backends need verification support, not a relabeled configuration.",
                           "Use recover survey --image <original> for bounded starting hints, then establish source-owned mappings and evidence; no function/TU bounds are inferred.",
                           "Configure independent prompt-driven worker/reviewer commands in recovery/agents.json.",
                           "Configure history scope and source audits from evidence, then create an ownership task with recover tasks create.",
                           "Ignore build/ caches and checkpoint configuration/task decisions in Git before recover run.",
                           "Configure full structural/runtime acceptance before claiming completed recovery; successful scaffolding is not acceptance."]}
