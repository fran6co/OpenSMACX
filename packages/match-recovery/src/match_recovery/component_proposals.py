"""Advisory proposals for named entries inside verified toolchain contributions."""

from collections import Counter
from dataclasses import replace
import json

from .coff_archive import CoffArchive
from .components import (CodeEntryAnchor, ToolchainCodeSectionBinding,
                         ToolchainManifest)
from .component_binary import verify_toolchain
from .model import canonical
from .pe_coff import Coff


def _entry_inventory(obj: Coff, binding: ToolchainCodeSectionBinding) -> tuple[tuple[str, int], ...]:
    """Return the complete unique external function inventory for one section."""
    if not 1 <= binding.section <= len(obj.sections):
        raise ValueError("code contribution section ordinal is outside its COFF member")
    section = obj.sections[binding.section - 1]
    if section.name != binding.section_name or not section.flags & 0x20:
        raise ValueError("code contribution does not name an executable COFF section")
    symbols = [symbol for symbol in obj.symbols.values()
               if symbol.section == binding.section and symbol.storage == 2]
    if not symbols:
        raise ValueError("code contribution has no defined external function symbols")
    if any((symbol.type & 0x30) != 0x20 for symbol in symbols):
        raise ValueError("code contribution defines an external non-function symbol")
    names = [symbol.name for symbol in symbols]
    if len(set(names)) != len(names):
        raise ValueError("code contribution has duplicate external function names")
    member_name_counts = Counter(symbol.name for symbol in obj.symbols.values())
    for symbol in symbols:
        if member_name_counts[symbol.name] != 1:
            raise ValueError(f"external function name {symbol.name!r} is ambiguous in its member")
        if not 0 <= symbol.value < len(section.data):
            raise ValueError(f"external function {symbol.name!r} lies outside its code contribution")
    return tuple((symbol.name, symbol.value) for symbol in sorted(symbols, key=lambda item: (item.value, item.name)))


def _section_summary(row: dict) -> dict:
    return {key: row.get(key) for key in (
        "unit", "section", "section_name", "span", "boundary", "bytes",
        "nonrelocation_bytes", "references", "result", "entry_inventory")}


def _candidate_section_summary(row: dict) -> dict:
    result = _section_summary(row)
    result["entries"] = [{key: item.get(key) for key in (
        "symbol", "offset", "address", "result",
        "scoped_reference_target", "link_target", "function_extent")}
        for item in row.get("entries", [])]
    return result


def propose_toolchain_entries(manifest, image_data: bytes, archive_data: bytes, *, import_bindings=()) -> dict:
    """Build a reviewable manifest candidate; never write source or grant acceptance.

    Existing code sections are first checked by the native toolchain verifier.
    New anchors are considered only for unannotated sections whose complete
    original placement and nonrelocation bytes pass. Every external definition
    in such a section must be a unique, in-bounds COFF function symbol. Existing
    entries and all other manifest bindings are preserved exactly.
    """
    if not isinstance(manifest, ToolchainManifest):
        raise ValueError("entry proposals require a precompiled toolchain manifest")
    image_data, archive_data, import_bindings = (bytes(image_data), bytes(archive_data),
                                                tuple(import_bindings))
    baseline = verify_toolchain(manifest, image_data, archive_data,
                                import_bindings=import_bindings)
    rows = {(row.get("unit"), row.get("section")): row
            for row in baseline.get("code_sections", [])}
    archive_ok = baseline.get("archive", {}).get("result") == "pass"
    archive = CoffArchive(archive_data) if archive_ok else None
    objects = {}
    candidate_sections, proposals = list(manifest.code_sections), []
    additions = 0
    for index, binding in enumerate(manifest.code_sections):
        row = rows.get((binding.unit, binding.section), {})
        if binding.entries:
            proposals.append({**_section_summary(row), "decision": "preserved_existing_entries",
                              "entries": [{"name": item.name, "offset": item.offset,
                                           "evidence": item.evidence} for item in binding.entries]})
            continue
        reason = None
        if not archive_ok:
            reason = "pinned archive did not verify"
        elif row.get("boundary") != "pass":
            reason = "whole contribution boundary is not proven"
        elif row.get("nonrelocation_bytes") != "pass":
            reason = "nonrelocation code bytes are not exact"
        elif row.get("references") == "fail" or row.get("result") == "fail":
            reason = "known whole-contribution reference or verification failure"
        if reason is None:
            try:
                obj = objects.get(binding.unit)
                if obj is None:
                    obj = Coff(archive.member(binding.unit))
                    objects[binding.unit] = obj
                inventory = _entry_inventory(obj, binding)
                if not inventory:
                    raise ValueError("code contribution has no external function inventory")
                anchors = tuple(CodeEntryAnchor(name, offset,
                    "complete unique external function inventory in pinned COFF section; entry offset only")
                    for name, offset in inventory)
                proposed = replace(binding, entries=anchors)
                trial_sections = list(candidate_sections)
                trial_sections[index] = proposed
                replace(manifest, code_sections=tuple(trial_sections))
                candidate_sections[index] = proposed
                additions += len(anchors)
                proposals.append({**_section_summary(row), "decision": "proposed",
                                  "entries": [{"name": item.name, "offset": item.offset,
                                               "evidence": item.evidence} for item in anchors]})
                continue
            except (KeyError, ValueError) as error:
                reason = str(error)
        proposals.append({**_section_summary(row), "decision": "skipped",
                          "reason": reason, "entries": []})

    candidate_manifest = replace(manifest, code_sections=tuple(candidate_sections))
    candidate = verify_toolchain(candidate_manifest, image_data, archive_data,
                                 import_bindings=import_bindings)
    before_counts = baseline.get("code_entry_counts", {})
    after_counts = candidate.get("code_entry_counts", {})
    changed = []
    before_rows = {(item.get("unit"), item.get("section")): item
                   for item in baseline.get("code_sections", [])}
    after_rows = {(item.get("unit"), item.get("section")): item
                  for item in candidate.get("code_sections", [])}
    for key in sorted(set(before_rows) | set(after_rows)):
        before, after = before_rows.get(key, {}), after_rows.get(key, {})
        if before.get("entries") != after.get("entries") or before.get("result") != after.get("result"):
            changed.append({"unit": key[0], "section": key[1],
                            "before": _section_summary(before), "after": _section_summary(after)})
    return {
        "check": "toolchain_entry_proposal",
        "result": "advisory",
        "component_id": manifest.component.component_id,
        "baseline_verification": {"result": baseline["result"], "archive": baseline.get("archive"),
                                   "code_entry_counts": before_counts},
        "proposals": proposals,
        "changes": {"new_entries": additions, "code_sections": changed,
                    "code_entry_counts_before": before_counts,
                    "code_entry_counts_after": after_counts},
        "candidate_manifest": json.loads(canonical(candidate_manifest.to_dict())),
        "candidate_verification": {"result": candidate["result"],
                                    "archive": candidate.get("archive"),
                                    "code_sections": [_candidate_section_summary(row)
                                                      for row in candidate.get("code_sections", [])],
                                    "code_entry_counts": after_counts,
                                    "unresolved_symbols": candidate.get("unresolved_symbols", [])},
        "limitations": ["advisory manifest data only; no source is written",
                        "anchors carry entry offsets only, never function extents or callability",
                        "unresolved references remain unknown in native candidate verification",
                        "a verified contribution does not prove original archive selection, ABI, runtime behavior, or application acceptance"]}
