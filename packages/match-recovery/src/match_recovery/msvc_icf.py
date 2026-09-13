"""Bounded verbose MSVC ICF observations, not physical selection authority.

The selected/replaced diagnostics precede Pass2 and end with the ICF savings
summary. Names and original owner labels are retained independently of folded
map owners. Repeated rows are evidence, never a reason to deduplicate members.
"""

from dataclasses import asdict
import re

from .model import Result
from .msvc_link_log import input_label, parse_link_log


MAX_LOG_BYTES = 32 * 1024 * 1024
MAX_GROUPS = 4096
MAX_ROWS = 65536


def _row(line, number):
    if len(line) > 16384:
        raise ValueError("oversized MSVC ICF member observation")
    decorated = re.fullmatch(r'"([^"\r\n]+)" \(([^\s"()]+)\) from (.+)', line)
    quoted = re.fullmatch(r'"([^\s"()]+)" from (.+)', line)
    plain = re.fullmatch(r'([^\s"()]+) from (.+)', line)
    if decorated:
        display, symbol, label = decorated.groups()
        form = "decorated"
    elif quoted:
        symbol, label = quoted.groups()
        display, form = None, "quoted"
    elif plain:
        symbol, label = plain.groups()
        display, form = None, "plain"
    else:
        raise ValueError("malformed or ambiguous MSVC ICF member observation")
    if len(symbol) > 4096 or len(label) > 4096 or any(ord(char) < 32 or ord(char) == 127 for char in label):
        raise ValueError("unsupported MSVC ICF symbol or owner label")
    return {"symbol": symbol, "display": display, "form": form,
            "owner": asdict(input_label(label)), "line": number, "text": line}


def parse_icf_log(data: bytes) -> dict:
    """Parse complete framed groups, retaining row order and multiplicity.

    No groups means only that none were reported; it is not a no-folding proof.
    A successful native build and physical selection/alias binding remain
    independent prerequisites. Malformed or unsupported groups raise, never
    return a prefix that could silently omit an original member.
    """
    if not isinstance(data, bytes) or len(data) > MAX_LOG_BYTES:
        raise ValueError("MSVC ICF observations require bounded immutable log bytes")
    framing = parse_link_log(data)
    lines = data.decode("ascii").splitlines()
    final_start = next(index for index, raw in enumerate(lines) if raw.strip() == "Start Pass2")
    first_end = max(index for index, raw in enumerate(lines[:final_start]) if raw.strip() == "End Pass1")
    groups, current, state, row_count, savings = [], None, "outside", 0, None
    for index, raw in enumerate(lines):
        line = raw.strip()
        if not line:
            continue
        folded = line.casefold()
        marker = folded.startswith(("selected symbol", "replaced symbol", "icf total"))
        if marker and not first_end < index < final_start:
            raise ValueError("MSVC ICF observation is outside executable optimization phase")
        if line == "Selected symbol:":
            if savings is not None or (current is not None and (state != "replacements" or not current["replaced"])):
                raise ValueError("incomplete or misplaced MSVC ICF selected group")
            if current is not None:
                groups.append(current)
            if len(groups) >= MAX_GROUPS:
                raise ValueError("MSVC ICF group count exceeds bound")
            current = {"line": index + 1, "selected": None, "replaced": []}
            state = "selected"
        elif line == "Replaced symbol(s):":
            if current is None or state != "separator":
                raise ValueError("MSVC ICF replacements lack one selected member")
            state = "replacements"
        elif folded.startswith("icf total"):
            summary = re.fullmatch(r"ICF total savings: (0|[1-9][0-9]{0,11}) bytes", line)
            if (summary is None or savings is not None
                    or (current is not None and (state != "replacements" or not current["replaced"]))):
                raise ValueError("invalid or incomplete MSVC ICF summary")
            if current is not None:
                groups.append(current)
                current = None
            savings = int(summary[1])
            if not groups and savings:
                raise ValueError("MSVC ICF savings lack reported groups")
            state = "finished"
        elif marker:
            raise ValueError("unsupported MSVC ICF group marker")
        elif state in ("selected", "separator", "replacements"):
            if state == "separator":
                raise ValueError("MSVC ICF group has multiple selected members or lacks separator")
            if not first_end < index < final_start:
                raise ValueError("MSVC ICF group lacks a terminating summary")
            row = _row(line, index + 1)
            row_count += 1
            if row_count > MAX_ROWS:
                raise ValueError("MSVC ICF member count exceeds bound")
            if state == "selected":
                current["selected"] = row
                state = "separator"
            else:
                current["replaced"].append(row)
        elif state == "finished" and first_end < index < final_start:
            raise ValueError("unexpected diagnostics after MSVC ICF summary")
        elif first_end < index < final_start and (
                line.startswith('"') or re.fullmatch(r"[^\s]+ from .+", line)):
            raise ValueError("orphan MSVC ICF member observation")
    if current is not None or groups and savings is None:
        raise ValueError("incomplete MSVC ICF diagnostics")
    return {"check": "msvc_icf_observations", "result": Result.PASS,
            "sha256": framing["sha256"], "groups": groups, "group_count": len(groups),
            "member_count": row_count, "savings_bytes": savings,
            "input_identity": Result.UNKNOWN, "selection_verification": Result.UNKNOWN,
            "limitations": ["linker labels and reported groups only, not physical section ownership",
                            "folded map owners need not be the original owners retained here",
                            "duplicate labels and section-only names do not identify unique COFF roots",
                            "no successful build, no-folding, alias or runtime acceptance proof"],
            "persisted": False, "ready_for_collection": False}
