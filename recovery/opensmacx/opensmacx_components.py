"""Migrate OpenSMACX's upstream identification evidence, not copied implementations."""

from __future__ import annotations

from pathlib import Path
import re
import subprocess
import sys

from match_recovery.components import SourceFile, VendorBinding, VendorManifest, source_integrity
from match_recovery.model import Component, Delivery, Origin, Ownership, Result, Span
from .opensmacx import image_pin
from recovery.environment.compile import compile_object


ZLIB_REMOVAL = "f5c72d351c696fd91665a009a6d48a76fed05284"
ZLIB_SHA256 = "517f7df278536bf9b91bb14104fcb4b13ce7b3c791d174195fec1618621b24e6"
ZLIB_URL = "https://zlib.net/fossils/zlib-1.0.2.tar.gz"
ZLIB_ROOT = "src/vendor/zlib-1.0.2"


def historical_binding(text: str, pin: str, evidence: str) -> VendorBinding:
    """Read the retired wrapper grammar explicitly, not with today's reader.

    The old name/spans fields are independent evidence. Never infer an extent
    from the compiled candidate or silently accept an ambiguous historical file.
    The legacy match token establishes an obligation, not a fresh result.
    """
    markers = re.findall(r'^// ORIGINAL: (0x[0-9A-Fa-f]+) BYTE_EXACT FILE\s*$', text, re.MULTILINE)
    names = re.findall(r'^// name\s+(\S+)\s*$', text, re.MULTILINE)
    spans = re.findall(r'^// spans\s+([^\r\n]+)$', text, re.MULTILINE)
    includes = re.findall(r'^#include\s+"([^"/]+\.c)"\s*$', text, re.MULTILINE)
    if any(len(items) != 1 for items in (markers, names, spans, includes)):
        raise ValueError(f"ambiguous historical vendor mapping: {evidence}")
    bounds = []
    for span in spans[0].split():
        match = re.fullmatch(r'(0x[0-9A-Fa-f]+)-(0x[0-9A-Fa-f]+)', span)
        if match is None:
            raise ValueError(f"invalid historical vendor span: {evidence}")
        bounds.append(Span(*(int(part, 16) for part in match.groups())))
    # The retired grammar mixed catalogued C identifiers and already decorated
    # symbols. This one-time C/i386 import normalizes the explicit C spelling;
    # the live verifier only accepts the exact resulting COFF symbol.
    symbol = names[0] if names[0].startswith("_") else "_" + names[0]
    return VendorBinding(Origin(pin, "pe:va", int(markers[0], 16)), tuple(bounds),
                         symbol, includes[0], evidence)


def zlib_manifest(root: Path, archive: Path) -> VendorManifest:
    pin = image_pin(root)
    source = root / ZLIB_ROOT
    files = []
    for path in sorted(source.iterdir()):
        if path.is_file() and path.suffix in (".c", ".h"):
            files.append(SourceFile(path.name, "zlib-1.0.2/" + path.name))
    parent = subprocess.run(["git", "rev-parse", ZLIB_REMOVAL + "^"], cwd=root,
                            capture_output=True, text=True, check=True).stdout.strip()
    paths = subprocess.run(["git", "ls-tree", "-r", "--name-only", parent, "--", ZLIB_ROOT],
                           cwd=root, capture_output=True, text=True, check=True).stdout.splitlines()
    bindings = []
    for name in paths:
        if not Path(name).name.startswith("recovered_"):
            continue
        text = subprocess.run(["git", "show", parent + ":" + name], cwd=root,
                              capture_output=True, text=True, check=True).stdout
        bindings.append(historical_binding(text, pin, "git:" + parent + ":" + name))
    if not bindings:
        raise ValueError("no historical zlib bindings found; refusing an empty import")
    component = Component("zlib", Ownership.THIRD_PARTY, Delivery.UPSTREAM_SOURCE,
                          (ZLIB_ROOT + "/PROVENANCE.md", "git:" + ZLIB_REMOVAL), "1.0.2")
    manifest = VendorManifest(component, ZLIB_ROOT, ZLIB_URL, ZLIB_SHA256, "zlib.h",
                              tuple(files), "c", tuple(sorted({binding.unit for binding in bindings})),
                              ("/c", "/O2", "/Gy", "/GR-", "/GX"), tuple(bindings))
    if source_integrity(root, manifest, archive)["result"] != Result.PASS:
        raise ValueError("vendored sources are not the pinned upstream release")
    return manifest


def compile_vendor(root: Path, source: Path, flags: tuple[str, ...]) -> bytes:
    """Invoke the project-owned compiler script from any compilation directory."""
    return compile_object(source, (sys.executable, str(root / "recovery/environment/wine.py"),
                                   "--tool=cl", "/nologo"), flags)
