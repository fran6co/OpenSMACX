"""Detect missing nonzero initial storage from conventional source hints.

This migration lens probes a configured prefix at trailing-address declarations;
it is not object extent, relocation, initializer expression or runtime proof.
Use source-owned object bindings and check-data for those stronger checks.
"""

import base64
from collections import Counter
from hashlib import sha256
from pathlib import Path, PurePosixPath
import re

from .includes import COMMENTS_OR_STRINGS
from .model import Span, digest
from .pe_coff import PeImage
from .records import parse_json
from .source_config import SourceConfiguration
from .source_inventory import source_snapshot, assert_snapshot_current


DEFINITION = re.compile(
    r"^((?:const\s+)?(?:unsigned\s+|signed\s+)?[A-Za-z_][\w:]*)\s+(\*?)(\w+)"
    r"(\[\d*\])?\s*(?:=\s*([^;]+?))?\s*;[ \t]*//[ \t]*(0x[0-9A-Fa-f]{6,8})\b", re.M)
NEUTRAL = frozenset(("0", "nullptr", "NULL", "{}", "{0}", "0u", "false"))


def check_initializers(root: Path, image_path: Path) -> dict:
    root = root.resolve()
    project_path = root / "recovery/project.json"
    if project_path.resolve() != project_path:
        raise ValueError("initializer configuration must be an unaliased local file")
    project_bytes = project_path.read_bytes()
    project = parse_json(project_bytes)
    config = project.get("initializers")
    required = {"ruleset", "probe_bytes", "scalar_types", "unit_extensions", "exclude_roots"}
    if (not isinstance(config, dict) or set(config) != required
            or config["ruleset"] != "cpp-trailing-address-v1"
            or type(config["probe_bytes"]) is not int or not 1 <= config["probe_bytes"] <= 16):
        raise ValueError("invalid or missing initializer probe configuration")
    for key in ("scalar_types", "unit_extensions", "exclude_roots"):
        values = config[key]
        if (not isinstance(values, list) or any(not isinstance(v, str) or not v.strip() for v in values)
                or len(set(values)) != len(values)):
            raise ValueError("initializer " + key + " must be unique nonempty strings")
    if not config["scalar_types"] or not config["unit_extensions"]:
        raise ValueError("initializer probes require scalar types and source extensions")
    snapshot = source_snapshot(root, allow_empty=True)
    source = SourceConfiguration.from_dict(snapshot["source_configuration"])
    if source.address_space != "pe:va" or not set(config["unit_extensions"]) <= set(source.extensions):
        raise ValueError("initializer probes require PE virtual addresses and configured source extensions")
    for name in config["exclude_roots"]:
        source.path(root, name)
        if not any(PurePosixPath(name).is_relative_to(parent) for parent in source.roots):
            raise ValueError("initializer exclusions must belong to configured source roots")
    image_path = image_path if image_path.is_absolute() else root / image_path
    image_bytes = image_path.read_bytes()
    image = PeImage(image_bytes, snapshot["image_sha256"])
    implementation = Path(__file__).read_bytes(), Path(__file__).with_name("pe_coff.py").read_bytes()
    observations, findings, errors = [], [], []
    counts = Counter(files_scanned=0, candidates=0, checked=0, unsupported_types=0)
    for name, item in sorted(snapshot["files"].items()):
        if (Path(name).suffix not in config["unit_extensions"] or source.is_seed(name)
                or any(PurePosixPath(name).is_relative_to(parent) for parent in config["exclude_roots"])):
            continue
        counts["files_scanned"] += 1
        text = base64.b64decode(item["content_base64"]).decode("utf-8")
        code = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), text)
        for match in DEFINITION.finditer(text):
            if not code[match.start():match.start() + 1].strip():
                continue
            ty, star, symbol, array, initializer, address = match.groups()
            counts["candidates"] += 1
            if not (star or array or ty.strip() in config["scalar_types"]):
                counts["unsupported_types"] += 1
                continue
            row = {"path": name, "line": text.count("\n", 0, match.start()) + 1,
                   "symbol": symbol, "address": address, "probe_bytes": config["probe_bytes"]}
            try:
                start = int(address, 16)
                raw = image.read_initial(Span(start, start + config["probe_bytes"]))
            except ValueError as error:
                errors.append({**row, "reason": str(error)})
                continue
            counts["checked"] += 1
            zero = initializer is None or initializer.strip() in NEUTRAL
            nonzero = any(raw)
            status = "missing_nonzero_initializer" if zero and nonzero else "no_missing_initializer_observed"
            observations.append({**row, "result": status, "source_zero": zero, "image_nonzero": nonzero})
            if zero and nonzero:
                findings.append({**row, "reason": "source has no nonzero initializer but original storage does"})
    if not counts["checked"]:
        errors.append({"reason": "no supported original initializer probes were measured"})
    assert_snapshot_current(root, snapshot)
    if (project_path.read_bytes() != project_bytes or image_path.read_bytes() != image_bytes
            or implementation != (Path(__file__).read_bytes(), Path(__file__).with_name("pe_coff.py").read_bytes())):
        raise ValueError("initializer source policy, original image or checker changed during verification")
    return {"schema_version": 1, "check": "source_initializers",
            "result": "fail" if findings else "unknown" if errors else "pass",
            "counts": dict(counts), "findings": findings, "errors": errors, "observations": observations,
            "input_fingerprint": digest({"source": {name: row["sha256"] for name, row in snapshot["files"].items()},
                                         "project": sha256(project_bytes).hexdigest(), "image": snapshot["image_sha256"],
                                         "implementation": [sha256(value).hexdigest() for value in implementation]}),
            "persisted": False, "ready_for_collection": False,
            "limitations": ["column-zero trailing-address declaration lens, not C++ parsing or complete global coverage",
                            "configured prefix size is probe policy, not an original object extent or candidate ABI proof",
                            "nonzero source expressions are not evaluated or proved correct",
                            "initial image storage only, not relocation targets, CRT constructors or runtime state"]}
