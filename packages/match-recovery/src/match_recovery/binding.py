"""Guarded publication of a new unverified original-function source binding.

Caller-supplied extents/rationale are decisions, not measurements. Executable
backing does not prove an instruction or function boundary. No function body,
matching claim, compiler recipe or task authorization is generated here.
"""

from dataclasses import replace
import hashlib
from pathlib import Path
import re

from .binding_source import render
from .model import Origin, Span, require_digest
from .ownership_source import project_view
from .pe_coff import PeImage
from .source import read_text
from .source.model import DecompilationState, Mode, State
from .source.publication import publish
from .source.writer import write
from .source_config import SourceConfiguration
from .source_inventory import assert_snapshot_current
from .source_replica import artifact_parent


def bind(root: Path, *, image: Path, address: int, name: str, spans, source: str, line: int,
         source_sha256: str, reason: str, evidence, mode: Mode | str = Mode.BODY) -> dict:
    try:
        mode = Mode(mode)
    except (TypeError, ValueError) as error:
        raise ValueError("binding mode must be body or file") from error
    root = root.resolve()
    require_digest(source_sha256)
    if type(address) is not int or not 0 <= address <= 0xFFFFFFFF:
        raise ValueError("binding requires a PE32 original entry address")
    if (not isinstance(name, str) or not re.fullmatch(r"[A-Za-z0-9_?@$:.\-]+", name)
            or name in {"BYTE_EXACT", "SEMANTIC", "FILE", "EXCLUDED"}):
        raise ValueError("binding name must be one source symbol token")
    if not isinstance(spans, (tuple, list)) or not spans or not all(isinstance(span, Span) for span in spans):
        raise ValueError("binding requires explicit original spans")
    if spans[0].start != address or any(span.end > 0x100000000 for span in spans):
        raise ValueError("binding primary span must begin at its PE32 entry")
    ordered = sorted(spans)
    if any(left.end > right.start for left, right in zip(ordered, ordered[1:])):
        raise ValueError("binding spans overlap each other")
    rationale = render(reason, evidence)
    image = image if image.is_absolute() else root / image
    view, snapshot = project_view(root, image, allow_empty=True)
    config = SourceConfiguration.from_dict(snapshot["source_configuration"])
    if config.address_space != "pe:va":
        raise ValueError("native binding requires pe:va source configuration")
    path = config.path(root, source)
    if source not in snapshot["files"] or config.is_seed(source) or path.suffix in config.interface_extensions:
        raise ValueError("new binding host must be an existing non-seed source translation unit")
    raw = path.read_bytes()
    if hashlib.sha256(raw).hexdigest() != source_sha256:
        raise ValueError("binding source digest is stale")
    text = raw.decode("utf-8")
    lines = text.splitlines(keepends=True)
    if type(line) is not int or not 1 <= line <= len(lines):
        raise ValueError("binding insertion must name an existing source line")
    anchor = lines[line - 1].lstrip()
    include_anchor = mode is Mode.FILE and re.match(r"#[ \t]*include\b", anchor)
    if not anchor.strip() or anchor.startswith(("//", "/*", "*")) or (anchor.startswith("#") and not include_anchor):
        raise ValueError("binding insertion must precede source code, not another annotation or directive")
    origin = Origin(snapshot["image_sha256"], config.address_space, address)
    for group in view["groups"]:
        for entity in group["entities"]:
            for candidate in entity["candidates"]:
                if candidate["path"] == source and candidate["symbol"] == name and not candidate["seed"]:
                    raise ValueError("source symbol already has a binding in this translation unit")
                if entity["entity_id"] == origin.entity_id:
                    if not candidate["seed"] or tuple(Span(**span) for span in candidate["spans"]) != tuple(spans):
                        raise ValueError("original already has a product/component binding or contradictory seed spans; review/rehome it instead")
                elif any(a.start < b["end"] and b["start"] < a.end for a in spans for b in candidate["spans"]):
                    raise ValueError("new original spans overlap another binding; shared ownership needs separate review")
    original = PeImage(image.read_bytes(), snapshot["image_sha256"])
    for span in spans:
        original.read(span)
        if not any(flags & 0x20000000 and region.start <= span.start and span.end <= region.end
                   for (region, _, _), flags in zip(original.sections, original.section_flags)):
            raise ValueError("original function spans require executable file-backed storage")
    record = DecompilationState(address, mode, State.IMPLEMENTED, path, 1, name,
                                tuple((span.start, span.end) for span in spans))
    marker = write("", [record])
    newline = "\r\n" if lines[line - 1].endswith("\r\n") else "\n"
    block = (marker + newline + rationale + newline).encode("utf-8")
    offset = len("".join(lines[:line - 1]).encode("utf-8"))
    changed = raw[:offset] + block + raw[offset:]
    before_records, after_records = read_text(text, path), read_text(changed.decode("utf-8"), path)
    added = [row for row in after_records if row.line == line]
    if (len(after_records) != len(before_records) + 1 or len(added) != 1
            or added[0].address != address or added[0].name != name or added[0].mode is not mode
            or added[0].image_spans != record.image_spans
            or added[0].byte_exact or added[0].semantic or added[0].state == State.EXCLUDED):
        raise ValueError("new source binding did not round-trip as one unclaimed identity")
    for old in before_records:
        candidates = [row for row in after_records if row.line == old.line + (2 if old.line >= line else 0)]
        if len(candidates) != 1 or replace(candidates[0], line=old.line, region=old.region, origin=old.origin) != old:
            raise ValueError("binding insertion would alter an existing annotation's facts")
    def fresh():
        assert_snapshot_current(root, snapshot)
        with image.open("rb") as stream:
            if hashlib.file_digest(stream, "sha256").hexdigest() != snapshot["image_sha256"]:
                raise ValueError("original image changed before binding publication")
    fresh()
    staging = artifact_parent(root, "build/recovery/publications", excluded_roots=config.roots)
    publish({path: raw}, {path: changed}, validate=fresh, staging_directory=staging)
    return {"entity_id": origin.entity_id, "path": source, "line": line, "binding_added": True,
            "verification": "not_measured", "boundary_verification": "not_measured", "ready_for_collection": False,
            "next_action": "review binding evidence, checkpoint source, and create/expand an ownership task; recheck affected TU",
            "limitations": ["original spans and name are caller-supplied interpretations; only image identity/backing was checked",
                            "source code is unchanged; insertion does not prove definition ownership, compilation, exactness or runtime behavior",
                            "coordinator-owned publication; scope expansion, rehoming and overlapping/shared spans require separate review"]}
