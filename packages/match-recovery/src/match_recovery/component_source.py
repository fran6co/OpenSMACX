"""Load-bearing vendor mappings in companion C/C++ source annotations.

Upstream files stay byte-for-byte unmodified. A *.recovery.h beside the vendor
directory owns its image bindings and build inputs; no external JSON ledger is
needed. This representation stores no current measurement or derived file hash.
"""

from __future__ import annotations

import json
from pathlib import Path
import re

from .components import VendorManifest
from .model import canonical
from .records import parse_json, publish_text


BEGIN = "// RECOVERY_COMPONENT_BEGIN v1"
END = "// RECOVERY_COMPONENT_END"


def _hex(value: object) -> int:
    if not isinstance(value, str) or not re.fullmatch(r"0x[0-9a-fA-F]+", value):
        raise ValueError("source addresses must be hexadecimal strings")
    return int(value, 16)


def render_component(manifest: VendorManifest) -> str:
    data = json.loads(canonical(manifest.to_dict()))
    bindings = data.pop("bindings")
    origins = {(binding["origin"]["image_sha256"], binding["origin"]["address_space"])
               for binding in bindings}
    if len(origins) != 1:
        raise ValueError("a component source mapping must identify one original image/address space")
    image, space = origins.pop()
    data["image"] = {"sha256": image, "address_space": space}
    files = data.pop("files")
    # One line per selected file and function keeps original boundaries/symbols
    # navigable without repeating a 64-character image pin at every function.
    lines = json.dumps(data, indent=2, ensure_ascii=False).splitlines()[:-1]
    lines[-1] += ","
    lines.append('  "files": [')
    for index, item in enumerate(files):
        lines.append("    " + json.dumps(item, ensure_ascii=False) + ("," if index + 1 < len(files) else ""))
    lines.extend(['  ],', '  "bindings": ['])
    for index, binding in enumerate(bindings):
        binding["origin"] = f'0x{binding["origin"]["address"]:08X}'
        binding["spans"] = [f'0x{span["start"]:08X}-0x{span["end"]:08X}' for span in binding["spans"]]
        lines.append("    " + json.dumps(binding, ensure_ascii=False)
                     + ("," if index + 1 < len(bindings) else ""))
    lines.extend(["  ]", "}"])
    return ("// Original-image bindings for this upstream dependency.\n"
            "// These are verification obligations, not claims of current matching.\n"
            "// Upstream source is unmodified; this companion owns its recovery metadata.\n"
            + BEGIN + "\n" + "\n".join("// " + line for line in lines) + "\n" + END + "\n")


def parse_component(text: str) -> VendorManifest:
    lines = text.splitlines()
    starts = [index for index, line in enumerate(lines) if line.startswith("// RECOVERY_COMPONENT_BEGIN")]
    ends = [index for index, line in enumerate(lines) if line.startswith("// RECOVERY_COMPONENT_END")]
    if len(starts) != 1 or len(ends) != 1 or starts[0] >= ends[0]:
        raise ValueError("expected exactly one complete component annotation")
    if lines[starts[0]] != BEGIN or lines[ends[0]] != END:
        raise ValueError("unsupported component annotation syntax")
    body = lines[starts[0] + 1:ends[0]]
    if any(not line.startswith("// ") for line in body):
        raise ValueError("component metadata must remain C/C++ comments")
    data = parse_json("\n".join(line[3:] for line in body))
    image = data.pop("image", None)
    if not isinstance(image, dict) or set(image) != {"sha256", "address_space"}:
        raise ValueError("component annotation needs one pinned image/address space")
    if not isinstance(data.get("bindings"), list) or not data["bindings"]:
        raise ValueError("component annotation needs original bindings")
    for binding in data["bindings"]:
        if not isinstance(binding, dict) or "origin" not in binding or not isinstance(binding.get("spans"), list):
            raise ValueError("invalid source binding")
        binding["origin"] = {"image_sha256": image["sha256"], "address_space": image["address_space"],
                             "address": _hex(binding["origin"])}
        spans = []
        for span in binding["spans"]:
            if not isinstance(span, str) or len(span.split("-")) != 2:
                raise ValueError("invalid source span")
            start, end = span.split("-")
            spans.append({"start": _hex(start), "end": _hex(end)})
        binding["spans"] = spans
    return VendorManifest.from_dict(data)


def write_component(path: Path, manifest: VendorManifest) -> bool:
    text = render_component(manifest)
    if parse_component(text) != manifest:
        raise ValueError("component annotation roundtrip changed a binding")
    return publish_text(path, text, allow_identical=True)


def read_components(source_root: Path) -> list[VendorManifest]:
    sources = {}
    for path in sorted(source_root.rglob("*.recovery.h")):
        if not path.resolve().is_relative_to(source_root.resolve()):
            raise ValueError("component annotation resolves outside source root")
        sources[path.relative_to(source_root).as_posix()] = path.read_text(encoding="utf-8")
    return parse_components(sources)


def parse_components(sources: dict[str, str]) -> list[VendorManifest]:
    """Validate one coherent source snapshot, including ownership across roots."""
    manifests = [parse_component(text) for name, text in sorted(sources.items()) if name.endswith(".recovery.h")]
    ids = [manifest.component.component_id for manifest in manifests]
    if len(ids) != len(set(ids)):
        raise ValueError("duplicate component ids")
    entities = [binding.origin.entity_id for manifest in manifests for binding in manifest.bindings]
    if len(entities) != len(set(entities)):
        raise ValueError("original binding owned by multiple components")
    return manifests
