"""Bounded observations of unlinked COFF sections, never comparison evidence."""

from bisect import bisect_left
import hashlib

from .model import canonical


MAX_REPORT_BYTES = 4 * 1024 * 1024
MAX_TRIAL_BYTES = 256 * 1024
MAX_SECTION_BYTES = 16 * 1024
MAX_INSTRUCTIONS = 4096


def trial_budget(trials):
    # Reserve the report envelope independently of completion order. Failed
    # trials keep their share; no parallel worker can consume another's share.
    return min(MAX_TRIAL_BYTES, max(0, MAX_REPORT_BYTES - 2048) // max(1, trials))


def compiled_diagnostic(obj, binding, *, name, flags, budget):
    """Inspect an already compiled target, using only explicit COFF ownership.

    None means even the identity/metadata cannot fit. All offsets belong to the
    named candidate section, not the original image or a linked address space.
    """
    row = {"name": name, "flags": list(flags), "entity_id": binding.origin.entity_id,
           "unit": binding.unit, "symbol": binding.symbol, "status": "unavailable",
           "sections": [], "omitted_sections": 0, "truncated": False}
    if len(canonical(row)) > budget:
        return None
    if obj is None:
        return row
    try:
        subject = obj.function(binding.symbol)
        sections = obj.code_sections(subject)
    except ValueError:
        # The ordinary measurement retains the full selection error. Do not
        # guess among same-named symbols or adjacent/ordinary text sections.
        return row
    row.update(status="observed", omitted_sections=len(sections), truncated=False)
    remaining = budget - len(canonical(row))
    if remaining < 0:
        return None

    def append(items, value):
        nonlocal remaining
        cost = len(canonical(value)) + 1  # Conservatively includes the comma.
        if cost > remaining:
            return False
        remaining -= cost
        items.append(value)
        return True

    try:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CsError
    except ImportError:
        decoder = None
    else:
        try:
            decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        except CsError:
            decoder = None
    for number in sections:
        section = obj.sections[number - 1]
        data = section.data
        references = [(offset, index, kind) for offset, index, kind in section.relocations if kind != 0]
        size = len(data)
        part = {"section": number, "section_name": section.name,
                "ownership": "entry" if number == subject.section else "associative_comdat",
                "association_parent": obj.associations.get(number),
                "raw_size": size, "sha256": hashlib.sha256(data).hexdigest(),
                "raw_bytes_hex": "", "reported_bytes": size, "omitted_bytes": size,
                "decode_status": "instruction_limit", "decoded_bytes": size,
                "undecoded_reported_bytes": size, "decoded_instruction_count": size,
                "listed_instruction_count": size, "omitted_decoded_instructions": size,
                "instructions": [], "relocations": [], "omitted_relocations": len(references)}
        if not append(row["sections"], part):
            break
        row["omitted_sections"] -= 1
        # Bytes are unmodified (including padding/tables). Reserve space for
        # relocation identities and instruction rows, rather than spending the
        # whole budget on a hexadecimal blob.
        shown = min(size, MAX_SECTION_BYTES, max(0, remaining // 8))
        part["raw_bytes_hex"] = data[:shown].hex()
        remaining -= 2 * shown
        part.update(reported_bytes=shown, omitted_bytes=size - shown)
        for offset, index, kind in references:
            target = obj.symbols[index]
            valid = kind in (6, 20) and 0 <= offset <= size - 4
            evidence = {"offset": offset, "coff_type": kind, "symbol_index": index,
                        "symbol": target.name, "storage_class": target.storage,
                        "target_section": target.section, "target_value": target.value,
                        "raw_addend": int.from_bytes(data[offset:offset + 4], "little", signed=True) if valid else None}
            if append(part["relocations"], evidence):
                part["omitted_relocations"] -= 1
            else:
                break
        # Relocations may overlap an instruction even when their start is in
        # its predecessor. Unknown relocation widths are not guessed.
        starts = sorted({offset for offset, _, _ in references})
        wide = sorted({offset for offset, _, kind in references if kind in (6, 20)})
        decoded = count = listed = 0
        part["decode_status"] = "unavailable" if decoder is None else "linear"
        if decoder is not None:
            try:
                for offset, length, mnemonic, operands in decoder.disasm_lite(data[:shown], 0, count=MAX_INSTRUCTIONS):
                    decoded += length
                    count += 1
                    overlaps = sorted(set(starts[bisect_left(starts, offset):bisect_left(starts, offset + length)])
                                      | set(wide[bisect_left(wide, offset - 3):bisect_left(wide, offset)]))
                    instruction = {"offset": offset, "bytes": data[offset:offset + length].hex(),
                                   "mnemonic": mnemonic, "operands_unlinked": operands,
                                   "relocation_offsets": overlaps}
                    if append(part["instructions"], instruction):
                        listed += 1
            except CsError:
                part["decode_status"] = "decode_error"
            else:
                part["decode_status"] = ("instruction_limit" if count == MAX_INSTRUCTIONS and decoded < shown
                                         else "stopped" if decoded < shown else "linear_complete")
        part.update(decoded_bytes=decoded, undecoded_reported_bytes=shown - decoded,
                    decoded_instruction_count=count, listed_instruction_count=listed,
                    omitted_decoded_instructions=count - listed)
    row["truncated"] = bool(row["omitted_sections"] or any(
        part["omitted_bytes"] or part["omitted_relocations"] or part["omitted_decoded_instructions"]
        or part["decode_status"] == "instruction_limit"
        for part in row["sections"]))
    # Counters only shrink from their conservative initial widths. This final
    # guard also keeps future additions from silently breaking the byte cap.
    return row if len(canonical(row)) <= budget else None


def diagnostic_report(entries, requested):
    """Collect in caller-specified trial order, with a final report-wide cap."""
    report = {"schema_version": 1, "byte_budget": MAX_REPORT_BYTES,
              "requested_trials": requested, "omitted_trials": requested,
              "omitted_trial_indices": [], "unlisted_omitted_trials": requested,
              "omission_reason": "per-trial or report-wide serialized byte limit",
              "entries": [], "limitations": [
                  "trial indices follow scored baseline rows, then candidate rows, in report order",
                  "target only; raw candidate COFF sections, not inferred function extents or original addresses",
                  "linear decoding may include padding or jump-table data; undecoded bytes remain visible",
                  "unlinked operands and raw addends do not establish call destinations, original bindings or linker selection",
                  "diagnostic omission or decoding never changes comparison, ranking or acceptance"]}
    remaining = MAX_REPORT_BYTES - len(canonical(report))
    for index, entry in enumerate(entries):
        entry = {**entry, "trial_index": index} if entry is not None else None
        cost = len(canonical(entry)) + 1 if entry is not None else remaining + 1
        if entry is not None and cost <= remaining:
            remaining -= cost
            report["entries"].append(entry)
            report["omitted_trials"] -= 1
            report["unlisted_omitted_trials"] -= 1
        elif len(canonical(index)) + 1 <= remaining:
            remaining -= len(canonical(index)) + 1
            report["omitted_trial_indices"].append(index)
            report["unlisted_omitted_trials"] -= 1
    return report
