"""Fresh toolchain-component verification publishing scoped relocation targets.

A component annotation alone is never evidence that the linked build uses the
declared library code. When the pinned archive freshly verifies against the
live image — hash pin, member reads and relocation-aware checks — its bound
symbols may resolve task-scoped relocations against their verified original
origins. Everything unverified stays unresolved; this publishes targets, not
product claims or acceptance.
"""

from pathlib import Path

from .binary import AbsoluteConstant
from .component_binary import verify_toolchain
from .components import ToolchainManifest
from .model import Result
from .records import read_json


def component_targets(root: Path, snapshot: dict, image_data: bytes) -> tuple[dict, list]:
    """Return ({symbol: address or AbsoluteConstant}, reports) from fresh components.

    Configured archives whose verification does not pass contribute no
    targets and keep their symbols unresolved. A missing or invalid
    configuration refuses rather than inferring any dependency identity.
    """
    from .ownership_source import component_entries
    configuration = read_json(root / "recovery/project.json")
    declared = configuration.get("components") if isinstance(configuration, dict) else None
    archives = declared.get("toolchain_archives") if isinstance(declared, dict) else None
    if archives is None:
        return {}, []
    if (not isinstance(archives, dict) or not archives
            or not all(isinstance(key, str) and isinstance(value, str) and value
                       for key, value in archives.items())):
        raise ValueError("component toolchain archives must map component ids to archive paths")
    targets, reports = {}, []
    for _, manifest in component_entries(snapshot):
        if not isinstance(manifest, ToolchainManifest):
            continue
        path = archives.get(manifest.component.component_id)
        if path is None:
            continue
        report = verify_toolchain(manifest, image_data, Path(path).read_bytes())
        reports.append(report)
        if report["archive"]["result"] != Result.PASS:
            continue
        for row in report["functions"]:
            # A failed row contradicts the recorded extent or bytes and must
            # not publish. A passing or unresolved-relocation row verified its
            # extent and body; deeper CRT-internal obligations stay with the
            # component and never transfer to the scoped caller.
            if row["result"] == Result.FAIL:
                continue
            targets[row["symbol"]] = (row["address"] if type(row["address"]) is int
                                      else int(str(row["address"]), 16))
        for row in report.get("absolute_symbols", ()):
            # A member-verified absolute constant resolves every link's fields
            # to the same value; an unverified one stays unresolved.
            if row["result"] == Result.PASS:
                targets[row["symbol"]] = AbsoluteConstant(int(str(row["value"]), 16))
    return targets, reports
