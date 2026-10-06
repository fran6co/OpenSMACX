"""Pinned upstream components, separate from application recovery accounting.

Source integrity is one check, not a substitute for binary/ABI verification.
Manifests are source annotations and contain no compiler output or image bytes.
Archive inspection never extracts paths onto the filesystem. Per-file hashes
and verification reports are derived outputs, not part of the manifest.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass
import hashlib
from pathlib import Path, PurePosixPath
import tarfile
from typing import Any

from .model import Component, Delivery, Origin, Ownership, Result, Span, require_digest, require_text


def relative_path(value: str) -> str:
    require_text(value, "relative path")
    path = PurePosixPath(value)
    if (path.is_absolute() or ".." in path.parts or "\\" in value
            or ":" in value or str(path) != value or value == "."):
        raise ValueError(f"expected a normalized relative path: {value}")
    return value


def archive_member_identity(value: str) -> str:
    """Validate an opaque archive key, never a path to extract or open."""
    if not isinstance(value, str) or not value:
        raise ValueError("archive member identity must be nonempty text")
    try:
        value.encode("ascii")
    except UnicodeEncodeError as error:
        raise ValueError("archive member identity must be ASCII") from error
    if any(ord(char) < 32 or ord(char) == 127 for char in value):
        raise ValueError("archive member identity contains a control character")
    if "\\" in value:
        raise ValueError("archive member identity must use canonical forward slashes")
    return value


def local_path(root: Path, value: str) -> Path:
    path = root / relative_path(value)
    if not path.resolve().is_relative_to(root.resolve()):
        raise ValueError(f"component input resolves outside its root: {value}")
    return path


def _fields(value: Any, required: set[str], label: str) -> dict:
    if not isinstance(value, dict) or set(value) != required:
        raise ValueError(f"invalid {label} fields; expected {sorted(required)}")
    return value


def _list(value: Any, label: str) -> list:
    if not isinstance(value, list):
        raise ValueError(f"{label} must be an array")
    return value


@dataclass(frozen=True)
class SourceFile:
    path: str
    archive_member: str

    def __post_init__(self):
        relative_path(self.path)
        relative_path(self.archive_member)


@dataclass(frozen=True)
class VendorBinding:
    origin: Origin
    spans: tuple[Span, ...]
    symbol: str
    unit: str
    evidence: str

    def __post_init__(self):
        require_text(self.symbol, "symbol")
        relative_path(self.unit)
        require_text(self.evidence, "binding evidence")
        if not self.spans or self.spans[0].start != self.origin.address:
            raise ValueError("binding must have an independently recorded entry span")


@dataclass(frozen=True)
class ToolchainBinding:
    """Original function mapping to an exact, opaque COFF archive member key."""
    origin: Origin
    spans: tuple[Span, ...]
    symbol: str
    unit: str
    evidence: str

    def __post_init__(self):
        require_text(self.symbol, "symbol")
        archive_member_identity(self.unit)
        require_text(self.evidence, "binding evidence")
        if not self.spans or self.spans[0].start != self.origin.address:
            raise ValueError("binding must have an independently recorded entry span")


@dataclass(frozen=True)
class VendorManifest:
    component: Component
    source_root: str
    archive_url: str
    archive_sha256: str
    license_file: str
    files: tuple[SourceFile, ...]
    language: str
    units: tuple[str, ...]
    flags: tuple[str, ...]
    bindings: tuple[VendorBinding, ...]

    def __post_init__(self):
        if self.component.delivery != Delivery.UPSTREAM_SOURCE:
            raise ValueError("vendor manifests describe upstream-source delivery")
        if self.component.ownership not in (Ownership.THIRD_PARTY, Ownership.RUNTIME):
            raise ValueError("upstream reuse is not an exemption for application code")
        relative_path(self.source_root)
        require_text(self.archive_url, "upstream URL")
        require_digest(self.archive_sha256)
        relative_path(self.license_file)
        names = {item.path for item in self.files}
        if not names or len(names) != len(self.files):
            raise ValueError("source pins must be nonempty and unique")
        if len({item.archive_member for item in self.files}) != len(self.files):
            raise ValueError("upstream archive members must be unique")
        if self.license_file not in names:
            raise ValueError("the license notice must be among pinned source files")
        if self.language not in ("c", "c++"):
            raise ValueError("unsupported source language")
        if not self.units or len(set(self.units)) != len(self.units) or not set(self.units) <= names:
            raise ValueError("translation units must be unique pinned sources")
        for flag in self.flags:
            require_text(flag, "compiler flag")
        identities = {binding.origin.entity_id for binding in self.bindings}
        if len(identities) != len(self.bindings):
            raise ValueError("duplicate original component binding")
        if any(binding.unit not in self.units for binding in self.bindings):
            raise ValueError("original binding names an unbuilt translation unit")

    def to_dict(self) -> dict:
        return {"schema_version": 1, "kind": "vendor_component", **asdict(self)}

    @classmethod
    def from_dict(cls, value: dict) -> VendorManifest:
        _fields(value, {"schema_version", "kind", "component", "source_root", "archive_url",
                        "archive_sha256", "license_file", "files", "language", "units",
                        "flags", "bindings"}, "vendor manifest")
        if type(value["schema_version"]) is not int or value["schema_version"] != 1:
            raise ValueError("unsupported vendor manifest schema")
        if value["kind"] != "vendor_component":
            raise ValueError("not a vendor component manifest")
        component = _fields(value["component"], {"component_id", "ownership", "delivery",
                                                "evidence", "version"}, "component")
        if component["version"] is not None:
            require_text(component["version"], "component version")
        owner = Component(component["component_id"], Ownership(component["ownership"]),
                          Delivery(component["delivery"]),
                          tuple(_list(component["evidence"], "component evidence")), component["version"])
        files = tuple(SourceFile(**_fields(item, {"path", "archive_member"}, "source file"))
                      for item in _list(value["files"], "files"))
        bindings = []
        for item in _list(value["bindings"], "bindings"):
            _fields(item, {"origin", "spans", "symbol", "unit", "evidence"}, "binding")
            origin = Origin(**_fields(item["origin"], {"image_sha256", "address_space", "address"}, "origin"))
            spans = tuple(Span(**_fields(span, {"start", "end"}, "span"))
                          for span in _list(item["spans"], "spans"))
            bindings.append(VendorBinding(origin, spans, item["symbol"], item["unit"], item["evidence"]))
        return cls(owner, value["source_root"], value["archive_url"], value["archive_sha256"],
                   value["license_file"], files, value["language"],
                   tuple(_list(value["units"], "units")), tuple(_list(value["flags"], "flags")), tuple(bindings))


@dataclass(frozen=True)
class ToolchainManifest:
    """A pinned precompiled library, not reconstructed application source.

    Binding.unit is an exact archive member identity, not a filesystem path.
    Archive location is supplied by the environment/CLI, not committed here.
    """
    component: Component
    archive_sha256: str
    license_evidence: str
    bindings: tuple[VendorBinding | ToolchainBinding, ...]
    absolute_symbols: tuple[AbsoluteSymbolBinding, ...] = ()
    data_sections: tuple[ToolchainDataSectionBinding, ...] = ()

    def __post_init__(self):
        if (self.component.delivery != Delivery.TOOLCHAIN
                or self.component.ownership not in (Ownership.RUNTIME, Ownership.THIRD_PARTY)):
            raise ValueError("toolchain manifests require a runtime or third-party toolchain dependency")
        require_digest(self.archive_sha256)
        require_text(self.license_evidence, "toolchain license/provenance evidence")
        identities = {item.origin.entity_id for item in self.bindings}
        if not self.bindings or len(identities) != len(self.bindings):
            raise ValueError("toolchain original bindings must be nonempty and unique")
        if len({item.symbol for item in self.bindings}) != len(self.bindings):
            raise ValueError("toolchain symbol bindings must be unique")
        if len({item.symbol for item in self.absolute_symbols}) != len(self.absolute_symbols):
            raise ValueError("toolchain absolute symbol bindings must be unique")
        if {item.symbol for item in self.absolute_symbols} & {item.symbol for item in self.bindings}:
            raise ValueError("toolchain symbols cannot be both function and absolute bindings")
        for item in self.bindings:
            archive_member_identity(item.unit)
            ordered = sorted(item.spans)
            if any(left.end > right.start for left, right in zip(ordered, ordered[1:])):
                raise ValueError("toolchain function spans overlap")
        for item in self.data_sections:
            archive_member_identity(item.unit)
        if len({(item.unit, item.section) for item in self.data_sections}) != len(self.data_sections):
            raise ValueError("duplicate toolchain data-section binding")
        if len({item.origin.entity_id for item in self.data_sections}) != len(self.data_sections):
            raise ValueError("duplicate toolchain data-section identity")
        if identities & {item.origin.entity_id for item in self.data_sections}:
            raise ValueError("one original identity cannot be both a function and a data section")
        origins = [item.origin for item in self.bindings] + [item.origin for item in self.data_sections]
        if any(origin.address_space != "pe:va" for origin in origins):
            raise ValueError("toolchain bindings require PE virtual addresses")
        if len({origin.image_sha256 for origin in origins}) != 1:
            raise ValueError("toolchain bindings must refer to one pinned image")
        spans = [span for item in self.bindings for span in item.spans]
        spans.extend(item.span for item in self.data_sections)
        ordered_spans = sorted(spans)
        if any(left.end > right.start for left, right in zip(ordered_spans, ordered_spans[1:])):
            raise ValueError("toolchain function/data-section spans overlap")

    def to_dict(self) -> dict:
        return {"schema_version": 1, "kind": "toolchain_component", **asdict(self)}

    @classmethod
    def from_dict(cls, value: dict) -> ToolchainManifest:
        allowed = {"schema_version", "kind", "component", "archive_sha256",
                   "license_evidence", "bindings", "absolute_symbols", "data_sections"}
        required = allowed - {"data_sections"}
        if not isinstance(value, dict) or not required <= set(value) or set(value) - allowed:
            raise ValueError(f"invalid toolchain manifest fields; expected required {sorted(required)}")
        if type(value["schema_version"]) is not int or value["schema_version"] != 1:
            raise ValueError("unsupported toolchain manifest schema")
        if value["kind"] != "toolchain_component":
            raise ValueError("not a toolchain component manifest")
        component = _fields(value["component"], {"component_id", "ownership", "delivery",
                                                "evidence", "version"}, "component")
        if component["version"] is not None:
            require_text(component["version"], "component version")
        owner = Component(component["component_id"], Ownership(component["ownership"]),
                          Delivery(component["delivery"]),
                          tuple(_list(component["evidence"], "component evidence")), component["version"])
        bindings = []
        for item in _list(value["bindings"], "bindings"):
            _fields(item, {"origin", "spans", "symbol", "unit", "evidence"}, "binding")
            origin = Origin(**_fields(item["origin"], {"image_sha256", "address_space", "address"}, "origin"))
            spans = tuple(Span(**_fields(span, {"start", "end"}, "span"))
                          for span in _list(item["spans"], "spans"))
            try:
                binding = VendorBinding(origin, spans, item["symbol"], item["unit"], item["evidence"])
            except ValueError:
                binding = ToolchainBinding(origin, spans, item["symbol"], item["unit"], item["evidence"])
            bindings.append(binding)
        absolutes = []
        for item in _list(value.get("absolute_symbols") or [], "absolute_symbols"):
            _fields(item, {"symbol", "value", "unit", "evidence"}, "absolute symbol binding")
            declared = item["value"]
            declared = int(str(declared), 16) if isinstance(declared, str) else declared
            absolutes.append(AbsoluteSymbolBinding(item["symbol"], declared, item["unit"], item["evidence"]))
        data_sections = []
        for item in _list(value.get("data_sections", []), "data_sections"):
            _fields(item, {"unit", "section", "section_name", "origin", "span", "symbols", "evidence"},
                    "data-section binding")
            origin = Origin(**_fields(item["origin"], {"image_sha256", "address_space", "address"}, "origin"))
            span = Span(**_fields(item["span"], {"start", "end"}, "data-section span"))
            symbols = tuple(DataSymbolAnchor(**_fields(anchor, {"name", "offset", "storage"}, "data symbol anchor"))
                            for anchor in _list(item["symbols"], "data-section symbols"))
            data_sections.append(ToolchainDataSectionBinding(item["unit"], item["section"],
                               item["section_name"], origin, span, symbols, item["evidence"]))
        return cls(owner, value["archive_sha256"], value["license_evidence"], tuple(bindings),
                   tuple(absolutes), tuple(data_sections))


ComponentManifest = VendorManifest | ToolchainManifest


@dataclass(frozen=True)
class AbsoluteSymbolBinding:
    """An absolute linker constant a pinned archive member defines.

    The member must define the symbol with a negative section (COFF absolute)
    and exactly this value; the value is what every link resolves it to, so a
    verified binding may resolve reference fields byte-exactly. It is never a
    function extent or an image address.
    """
    symbol: str
    value: int
    unit: str
    evidence: str

    def __post_init__(self):
        require_text(self.symbol, "absolute symbol name")
        if type(self.value) is not int or not 0 <= self.value < 2**32:
            raise ValueError("absolute symbol value must be an unsigned 32-bit integer")
        archive_member_identity(self.unit)
        require_text(self.evidence, "absolute symbol evidence")


@dataclass(frozen=True)
class DataSymbolAnchor:
    """Section-relative name/offset only; deliberately has no size or type."""
    name: str
    offset: int
    storage: int

    def __post_init__(self):
        require_text(self.name, "data symbol anchor")
        if type(self.offset) is not int or self.offset < 0:
            raise ValueError("data symbol offset must be a nonnegative integer")
        if type(self.storage) is not int or self.storage != 3:
            raise ValueError("toolchain data anchor must be a static COFF definition")


@dataclass(frozen=True)
class ToolchainDataSectionBinding:
    """One complete section placement, not an asserted per-symbol object extent."""
    unit: str
    section: int
    section_name: str
    origin: Origin
    span: Span
    symbols: tuple[DataSymbolAnchor, ...]
    evidence: str

    def __post_init__(self):
        archive_member_identity(self.unit)
        if type(self.section) is not int or self.section < 1:
            raise ValueError("data section ordinal must be positive")
        require_text(self.section_name, "COFF data section name")
        require_text(self.evidence, "data-section evidence")
        if self.origin.address_space != "pe:va" or self.span.start != self.origin.address or self.span.end <= self.span.start:
            raise ValueError("data-section binding requires a nonempty original PE span")
        if not self.symbols or len({item.name for item in self.symbols}) != len(self.symbols):
            raise ValueError("data-section binding needs unique symbol anchors")
        if any(item.offset >= self.span.end - self.span.start for item in self.symbols):
            raise ValueError("data-section symbol anchor is outside the section span")


def source_integrity(root: Path, manifest: VendorManifest, archive: Path | None = None) -> dict:
    """Recompute source identity from the pinned archive; no stored hash ledger."""
    source = local_path(root, manifest.source_root)
    rows = []
    for item in manifest.files:
        path = local_path(source, item.path)
        try:
            with path.open("rb") as stream:
                actual = hashlib.file_digest(stream, "sha256").hexdigest()
        except FileNotFoundError:
            actual = None
        rows.append({"path": item.path, "expected": None, "actual": actual, "result": Result.UNKNOWN})
    archive_result = {"result": Result.UNKNOWN, "reason": "pinned archive required"}
    if archive is not None:
        with archive.open("rb") as stream:
            archive_digest = hashlib.file_digest(stream, "sha256").hexdigest()
        if archive_digest != manifest.archive_sha256:
            archive_result = {"result": Result.FAIL, "reason": "archive digest mismatch"}
        else:
            failures = []
            with tarfile.open(archive, "r:*") as bundle:
                members = bundle.getmembers()
                names = [member.name for member in members]
                if len(names) != len(set(names)):
                    raise ValueError("ambiguous duplicate archive members")
                for item, row in zip(manifest.files, rows, strict=True):
                    try:
                        member = bundle.getmember(item.archive_member)
                    except KeyError:
                        failures.append(item.path)
                        continue
                    if not member.isfile():
                        failures.append(item.path)
                        continue
                    with bundle.extractfile(member) as stream:
                        expected = hashlib.file_digest(stream, "sha256").hexdigest()
                    row["expected"] = expected
                    if row["actual"] is not None:
                        row["result"] = Result.PASS if row["actual"] == expected else Result.FAIL
            archive_result = {"result": Result.FAIL if failures else Result.PASS,
                              "sha256": archive_digest, "mismatched_members": failures}
    results = [row["result"] for row in rows]
    if archive_result:
        results.append(archive_result["result"])
    overall = Result.FAIL if Result.FAIL in results else Result.UNKNOWN if Result.UNKNOWN in results else Result.PASS
    return {"component_id": manifest.component.component_id, "check": "source_integrity",
            "result": overall, "files": rows, "archive": archive_result,
            "binary_verification": Result.UNKNOWN, "binding_count": len(manifest.bindings),
            "application_recoveries": 0}
