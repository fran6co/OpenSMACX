"""Native dependency inventory and checks using source/configuration inputs."""

from dataclasses import asdict
import hashlib
from pathlib import Path

from .component_source import parse_components
from .components import ToolchainManifest, source_integrity
from .fingerprint import input_snapshot
from .model import Result
from .source_config import SourceConfiguration
from .source_inventory import image_pin


def component_snapshot(root: Path):
    root = root.resolve()
    configuration = SourceConfiguration.read(root)

    def paths():
        return [path for path in configuration.files(root)
                if path.name.endswith(".recovery.h") and not configuration.is_seed(path.relative_to(root).as_posix())]

    selected = paths()
    names = [path.relative_to(root).as_posix() for path in selected]
    inputs = ["recovery/project.json", configuration.image_pin, *names]
    before = input_snapshot(root, inputs, asdict(configuration))
    pin = image_pin(root, configuration)
    manifests = parse_components({name: configuration.path(root, name).read_text(encoding="utf-8") for name in names})
    if any(binding.origin.image_sha256 != pin or binding.origin.address_space != configuration.address_space
           for manifest in manifests for binding in manifest.bindings):
        raise ValueError("component bindings refer to a different original image/address space")
    if (configuration != SourceConfiguration.read(root) or selected != paths()
            or before != input_snapshot(root, inputs, asdict(configuration))):
        raise ValueError("component source changed during inventory")
    return manifests, before


def execute(root: Path, arguments: dict):
    root = root.resolve()
    manifests, before = component_snapshot(root)
    action = arguments["action"]
    if action == "list":
        return 0, [{"component": asdict(manifest.component), "workflow": manifest.component.workflow,
                    "bindings": len(manifest.bindings)} for manifest in manifests]
    selected = [manifest for manifest in manifests if manifest.component.component_id == arguments["component"]]
    if len(selected) != 1:
        raise ValueError(f"expected one component named {arguments['component']}")
    manifest, = selected
    toolchain = isinstance(manifest, ToolchainManifest)
    if toolchain and action != "check-binary":
        raise ValueError("precompiled toolchain dependencies have no upstream source; use check-binary --archive")
    inputs = [] if toolchain else [manifest.source_root + "/" + item.path for item in manifest.files]
    sources = input_snapshot(root, inputs, manifest.to_dict())
    external = arguments.get("image" if action == "check-binary" else "archive")
    if external is not None:
        external = Path(external)
        external = external if external.is_absolute() else root / external
    elif action == "check-binary":
        raise ValueError("component binary verification requires an original image")

    def external_digest():
        if external is None:
            return None
        with external.open("rb") as stream:
            return hashlib.file_digest(stream, "sha256").hexdigest()

    original_digest = external_digest()
    archive_path, archive_data = None, None
    if toolchain:
        if arguments.get("archive") is None:
            raise ValueError("toolchain binary verification requires --archive")
        archive_path = Path(arguments["archive"])
        archive_path = archive_path if archive_path.is_absolute() else root / archive_path
        archive_data = archive_path.read_bytes()
    elif action == "check-binary" and arguments.get("archive") is not None:
        raise ValueError("--archive in check-binary is for precompiled toolchain dependencies")
    if action == "check-source":
        result = source_integrity(root, manifest, external)
    elif action == "check-binary":
        if toolchain:
            from .component_binary import verify_toolchain
            result = verify_toolchain(manifest, external.read_bytes(), archive_data)
        else:
            from .compiler import Compiler
            from .component_binary import verify_component
            compiler = Compiler(root)
            result = verify_component(root, manifest, external.read_bytes(),
                                      lambda path, flags: compiler(path, (), flags))
            if compiler.snapshot() != compiler.initial:
                raise ValueError("compiler inputs changed during component verification")
        if arguments.get("summary"):
            result.pop("functions")
    else:
        raise ValueError("unsupported component operation")
    if (component_snapshot(root) != (manifests, before)
            or input_snapshot(root, inputs, manifest.to_dict()) != sources
            or external_digest() != original_digest
            or archive_path is not None and archive_path.read_bytes() != archive_data):
        raise ValueError("component inputs changed during verification")
    return {Result.PASS: 0, Result.FAIL: 1, Result.UNKNOWN: 3}[result["result"]], result
