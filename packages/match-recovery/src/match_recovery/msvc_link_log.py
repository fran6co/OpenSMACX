"""Bounded MSVC /VERBOSE final-pass input observations.

These are linker-reported labels, not file identities, retained section extents
or common-allocation proof. Export-library generation has its own nested first
pass and must not be confused with the executable's final input list.
"""

from dataclasses import asdict, dataclass
import hashlib
import re

from .model import Result


@dataclass(frozen=True)
class LinkInput:
    label: str
    library: str | None
    member: str


def input_label(text: str) -> LinkInput:
    archive = re.fullmatch(r"([^()]+\.lib)\(([^()]+)\)", text, re.IGNORECASE)
    if archive:
        library, member = archive.groups()
    elif re.fullmatch(r"[^()]+\.(?:obj|exp)", text, re.IGNORECASE):
        library, member = None, text
    else:
        raise ValueError("unsupported final-pass input label")
    if any(value != value.strip() or '"' in value for value in (library, member) if value is not None):
        raise ValueError("ambiguous final-pass input label")
    return LinkInput(text, library, member)


def parse_link_log(data: bytes) -> dict:
    """Require one complete executable pass sequence; preserve repeated inputs.

    Only final-pass row syntax and phase framing are verified. Other verbose
    diagnostics (including section discards and ICF) are not interpreted as
    allocation/ownership evidence. A completed successful build is a separate
    prerequisite supplied by the caller, never inferred from this transcript.
    """
    text = data.decode("ascii")
    # DEL is used in traditional import-descriptor symbol names. It is valid
    # in diagnostics, but never in the final-pass filename labels below.
    if any(ord(char) < 32 and char not in "\r\n\t" for char in text):
        raise ValueError("invalid control character in MSVC link diagnostics")
    stack, inputs, searched = [], [], []
    first_started = first_done = final_started = final_done = False
    exports = 0
    for raw in text.splitlines():
        line = raw.strip()
        if line == "Start Pass1":
            if not stack and not first_started:
                first_started = True
            elif stack != ["pass1", "export"]:
                raise ValueError("duplicate or misplaced first link pass")
            stack.append("pass1")
        elif line == "End Pass1":
            if not stack or stack[-1] != "pass1":
                raise ValueError("unbalanced first link pass")
            stack.pop()
            if not stack:
                first_done = True
        elif line == "Start Generating .exp file":
            if stack != ["pass1"]:
                raise ValueError("misplaced export-library generation")
            exports += 1
            stack.append("export")
        elif line == "End Generating .exp file":
            if stack != ["pass1", "export"]:
                raise ValueError("unbalanced export-library generation")
            stack.pop()
        elif line == "Start Pass2":
            if stack or not first_done or final_started:
                raise ValueError("duplicate or misplaced final link pass")
            final_started = True
            stack.append("pass2")
        elif line == "End Pass2":
            if stack != ["pass2"] or not inputs:
                raise ValueError("empty or unbalanced final link pass")
            stack.pop()
            final_done = True
        elif stack == ["pass2"] and line:
            if any(ord(char) < 32 or ord(char) == 127 for char in line):
                raise ValueError("invalid final-pass input filename")
            inputs.append(asdict(input_label(line)))
        elif line.startswith("Searching ") and line.endswith(":"):
            if not stack or stack[-1] != "pass1":
                raise ValueError("library search outside a first link pass")
            # Keep the nested-library observations separate, including path
            # spelling/case. Resolving a Wine path is compiler interaction.
            searched.append({"path": line[len("Searching "):-1],
                             "phase": "executable" if len(stack) == 1 else "export_library"})
        elif line.startswith(("Start Pass", "End Pass", "Start Generating", "End Generating")):
            raise ValueError("unsupported link phase marker")
    if stack or not first_done or not final_done:
        raise ValueError("incomplete executable link diagnostics")
    return {"check": "msvc_final_pass_inputs", "result": Result.PASS,
            "sha256": hashlib.sha256(data).hexdigest(), "inputs": inputs,
            "input_count": len(inputs), "searched_libraries": searched,
            "export_generations": exports, "input_identity": Result.UNKNOWN,
            "allocation_verification": Result.UNKNOWN,
            "limitations": ["complete supported phase framing, not successful link or file identity proof",
                            "final-pass labels can repeat and omit archive member directories",
                            "input presence does not prove retained sections, common extents or ownership",
                            "other verbose diagnostics are uninterpreted"],
            "persisted": False, "ready_for_collection": False}
