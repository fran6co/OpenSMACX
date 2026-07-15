#!/usr/bin/env python3

from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
LOCAL_ARTIFACT_ROOTS = (
    (REPO_ROOT / ".opensmacx").resolve(),
    (REPO_ROOT / "build").resolve(),
)


def require_local_artifact_path(value, description="output"):
    path = Path(value).expanduser().absolute()
    for component in (path, *path.parents):
        if component.is_symlink():
            raise RuntimeError(
                f"{description} must not contain symlinks: {component}")
    resolved = path.resolve()
    if not any(root in resolved.parents for root in LOCAL_ARTIFACT_ROOTS):
        raise RuntimeError(
            f"{description} must be inside the ignored .opensmacx/ or build/ directory")
    return resolved
