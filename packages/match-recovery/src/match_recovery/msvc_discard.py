"""Framed MSVC REF discard observations, never physical selection authority.

Microsoft documents /OPT:REF as transitive COMDAT elimination and /VERBOSE
as reporting removed functions separately from ICF folding:
https://learn.microsoft.com/en-us/cpp/build/reference/opt-optimizations
https://learn.microsoft.com/en-us/cpp/build/reference/verbose-print-progress-messages

These references describe the operations, not a portable transcript ABI. This
parser supports the captured legacy grammar only: explicit Discarded records
after the executable's completed first pass and before its ICF phase or second
pass. A native caller must separately bind a successful fresh link, options,
physical inputs and the exact original root. No missing event proves retention.
"""

import re

from .model import Result
from .msvc_icf import _row, parse_icf_log


MAX_LOG_BYTES = 32 * 1024 * 1024
MAX_ROWS = 65536
MAX_LINE_BYTES = 16384


def parse_discard_log(data: bytes) -> dict:
    """Preserve every supported event, including unresolved ownerless rows.

    Owner/section-looking labels remain exactly linker-reported labels. The
    shared diagnostic row grammar calls input_label; neither parser resolves a
    filesystem path, archive occurrence, COFF symbol or association forest.
    """
    if type(data) is not bytes or len(data) > MAX_LOG_BYTES:
        raise ValueError("REF observations require bounded immutable log bytes")
    # Reuse executable/nested-export framing and validate the separate ICF
    # transcript in full; a malformed group cannot turn a later row into REF.
    framing = parse_icf_log(data)
    lines = data.decode("ascii").splitlines()
    final_start = next(index for index, raw in enumerate(lines) if raw.strip() == "Start Pass2")
    first_end = max(index for index, raw in enumerate(lines[:final_start]) if raw.strip() == "End Pass1")
    end = min((index for index, raw in enumerate(lines[first_end + 1:final_start], first_end + 1)
               if raw.strip() == "Selected symbol:" or raw.strip().startswith("ICF total savings:")),
              default=final_start)
    rows = []
    for index, raw in enumerate(lines):
        line = raw.strip()
        if not line.casefold().startswith("discarded"):
            continue
        if not first_end < index < end:
            raise ValueError("REF discard observation is outside the executable pre-ICF optimization phase")
        if not line.startswith("Discarded ") or len(line) > MAX_LINE_BYTES:
            raise ValueError("malformed or oversized REF discard observation")
        body = line[len("Discarded "):]
        if re.fullmatch(r'[^\s"()]+', body):
            if len(body) > 4096 or any(not 32 < ord(char) < 127 for char in body):
                raise ValueError("unsupported ownerless REF discard identity")
            row = {"symbol": body, "display": None, "form": "ownerless", "owner": None,
                   "line": index + 1}
        else:
            try:
                row = _row(body, index + 1)
            except ValueError as error:
                raise ValueError("malformed or ambiguous REF discard target/owner") from error
        row.update(text=line, phase="executable_ref", input_identity=Result.UNKNOWN,
                   selection_verification=Result.UNKNOWN)
        rows.append(row)
        if len(rows) > MAX_ROWS:
            raise ValueError("REF discard observation count exceeds its bound")
    return {"check": "msvc_ref_discard_observations", "result": Result.PASS,
            "sha256": framing["sha256"], "rows": rows, "row_count": len(rows),
            # Preserve contradictory folding observations even without a PDB
            # or a successful native physical ICF binding. These are labels,
            # never a positive no-fold/selection certificate.
            "icf_groups": framing["groups"],
            "ownerless_count": sum(row["owner"] is None for row in rows),
            "phase": {"executable_pass1_end_line": first_end + 1,
                      "discard_window_end_line": end + 1, "executable_pass2_start_line": final_start + 1},
            "input_identity": Result.UNKNOWN, "selection_verification": Result.UNKNOWN,
            "limitations": ["framed reported events, not a successful link or enabled REF policy",
                            "owner labels and section-like names do not identify physical original roots",
                            "ownerless events remain unresolved and duplicate events retain multiplicity",
                            "absence of an event proves neither retention nor removal",
                            "no scope, association, allocation or runtime acceptance proof"],
            "persisted": False, "ready_for_collection": False}
