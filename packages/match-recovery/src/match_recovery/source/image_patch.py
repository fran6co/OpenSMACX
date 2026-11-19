"""Declared post-link patches of the pinned original image.

A shipped image can carry bytes that no compiler produced: instructions
overwritten in the linked file after the build. The faithful source for such a
function is the code as it was compiled; the patch is a property of the image.
`// RECOVERY_IMAGE_PATCH: v1 ...` on the function's marker host declares the
patched image span and the exact form of its bytes, so a comparison can check
the image against the declaration and the compiled body against everything
else. Parsing establishes no correspondence: the image must hold exactly the
declared bytes and the compiled body must still match outside the span.

Two forms are supported, both over one contiguous `[start, end)` span:

    // RECOVERY_IMAGE_PATCH: v1 <start> <exclusive-end> nop
    // RECOVERY_IMAGE_PATCH: v1 <start> <exclusive-end> nop-jmp <jmp-address> <jmp-target>

`nop` is a span of 0x90 bytes. `nop-jmp` is a span of 0x90 bytes holding one
five-byte `E9 rel32` at `jmp-address` whose destination is `jmp-target`.
"""

from dataclasses import dataclass
import re


PREFIX = "// RECOVERY_IMAGE_PATCH:"
_ADDRESS = r"(0x[0-9A-Fa-f]{8})"
_NOP = re.compile(rf"// RECOVERY_IMAGE_PATCH: v1 {_ADDRESS} {_ADDRESS} nop")
_NOP_JMP = re.compile(rf"// RECOVERY_IMAGE_PATCH: v1 {_ADDRESS} {_ADDRESS} nop-jmp {_ADDRESS} {_ADDRESS}")
FORM_NOP, FORM_NOP_JMP = "nop", "nop-jmp"


@dataclass(frozen=True)
class ImagePatch:
    """One declared patched span; `end` is exclusive."""
    start: int
    end: int
    form: str
    line: int
    jmp_address: int | None = None
    jmp_target: int | None = None

    def __post_init__(self):
        values = (self.start, self.end, self.line)
        if any(type(value) is not int for value in values) or self.line < 1:
            raise ValueError("invalid image patch declaration")
        if not 0 <= self.start < self.end <= 0xFFFFFFFF:
            raise ValueError(f"image patch at line {self.line} has an empty or reversed span")
        if self.form == FORM_NOP:
            if self.jmp_address is not None or self.jmp_target is not None:
                raise ValueError(f"nop image patch at line {self.line} cannot carry a jump")
        elif self.form == FORM_NOP_JMP:
            if (type(self.jmp_address) is not int or type(self.jmp_target) is not int
                    or not self.start <= self.jmp_address
                    or self.jmp_address + 5 > self.end
                    or not 0 <= self.jmp_target <= 0xFFFFFFFF):
                raise ValueError(
                    f"image patch at line {self.line}: the five-byte jump must lie inside the span")
        else:
            raise ValueError(f"unsupported image patch form at line {self.line}")

    @property
    def expected(self) -> bytes:
        """The exact image bytes this declaration states."""
        data = bytearray(b"\x90" * (self.end - self.start))
        if self.form == FORM_NOP_JMP:
            offset = self.jmp_address - self.start
            displacement = (self.jmp_target - (self.jmp_address + 5)) & 0xFFFFFFFF
            data[offset:offset + 5] = b"\xE9" + displacement.to_bytes(4, "little")
        return bytes(data)

    def describe(self) -> str:
        text = f"0x{self.start:08X}-0x{self.end:08X} {self.form}"
        if self.form == FORM_NOP_JMP:
            text += f" 0x{self.jmp_address:08X} 0x{self.jmp_target:08X}"
        return text


def parse_image_patches(text: str) -> tuple[ImagePatch, ...]:
    """Every declaration in one source text, in address order.

    A line that begins with the prefix (after indentation) and is not exactly
    one supported form refuses, as do overlapping declarations: a malformed
    declaration never degrades to "no patch".
    """
    patches = []
    for line, source_line in enumerate(text.splitlines(), 1):
        stripped = source_line.strip()
        if not stripped.startswith(PREFIX):
            continue
        match = _NOP.fullmatch(stripped)
        if match is not None:
            start, end = (int(value, 16) for value in match.groups())
            patches.append(ImagePatch(start, end, FORM_NOP, line))
            continue
        match = _NOP_JMP.fullmatch(stripped)
        if match is None:
            raise ValueError(f"invalid image patch annotation at line {line}")
        start, end, jmp_address, jmp_target = (int(value, 16) for value in match.groups())
        patches.append(ImagePatch(start, end, FORM_NOP_JMP, line, jmp_address, jmp_target))
    patches.sort(key=lambda patch: (patch.start, patch.end))
    for before, after in zip(patches, patches[1:]):
        if after.start < before.end:
            raise ValueError(
                f"image patch annotations at lines {before.line} and {after.line} overlap")
    return tuple(patches)


def patches_for_spans(patches, spans) -> tuple[ImagePatch, ...]:
    """The declarations that lie wholly inside one of `spans`.

    A declaration that only partly overlaps a span refuses: a patch belongs
    to one function's mapped bytes or to none of them.
    """
    selected = []
    for patch in patches:
        inside = [span for span in spans if span[0] <= patch.start and patch.end <= span[1]]
        partial = [span for span in spans
                   if patch.start < span[1] and span[0] < patch.end and span not in inside]
        if partial:
            raise ValueError(
                f"image patch {patch.describe()} crosses the boundary of mapped span "
                f"0x{partial[0][0]:08X}-0x{partial[0][1]:08X}")
        if inside:
            selected.append(patch)
    return tuple(selected)
