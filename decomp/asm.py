"""The assembly behind a record: the original image's, VC6's, and the verdict.

One call answers the recovery's central question for one record:

    from pathlib import Path
    from decomp import read_file
    from decomp.asm import compare_record

    records = read_file(Path("src/buffer.cpp"))
    record = next(r for r in records if r.address == 0x005DAC70)
    compare_record(record,
                   Path(".opensmacx/game/terranx_original.exe"),
                   Path("build/compile_commands.json")).verdict

and the two sides are still available separately, as `original_asm` and
`compiled_asm`.

THE ORIGINAL SIDE reads the pinned executable's bytes at the record's span
and disassembles them. THE COMPILED SIDE takes the build's own compile
command for the record's file from compile_commands.json, keeps its
compiler and its include and define flags, swaps in the caller's
optimisation flags, and disassembles the function pulled out of the
resulting object. The compiler the entry names is the build's own wrapper -
it knows where VC6 lives and how to run it under wine, so this module never
asks. Both return a `Listing`: lines shaped like tools/disasm.py's output,
carrying the bytes and the relocation mask behind them.

THREE THINGS A CORRECT COMPARISON NEEDS, each of which this module got
wrong until it was measured against the tree. Four bodies carrying a
BYTE_EXACT claim in `src/` all reported MISMATCH:

  * THE FLAG SET IS PER FUNCTION - see `compare_record`, which takes as
    many as the caller cares to name. Asking one answers half the image.
  * A RELOCATED BYTE IS NOT A WRONG ANSWER. The object holds zeros where
    the image holds linked addresses; `compare_asm` discounts both sides.
  * THE CATALOGUE'S NAME IS NOT ALWAYS THE COMPILER'S. What the image
    calls a piece and what this tree emits for it are two facts, and the
    annotation carries both - `name` and `symbol`. The lookup reads them;
    it does not guess, and a missing fact is an error that names itself.

WHAT IS STILL ELSEWHERE. The tier ladder here is the three verdicts above;
SHAPE_EXACT, SHARED_TAIL, REFUSED, the EH funclet classification, batching,
caching and the ledger stay in tools/byte_match.py.

CAPSTONE is imported like anything else. It is a declared dependency, so a
lazy import bought nothing but a failure that arrives halfway through a
measurement instead of at import; the readers that must not need it live in
other modules, and `python -m decomp` checks that split. The PE and COFF
readers below are small struct walks instead of dependencies, because all
they need is the section table, the symbol table and two relocation
directories.
"""

from __future__ import annotations

from collections.abc import Iterable

import difflib
import functools
import json
import re
import shlex
import struct
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

from .model import DecompilationState, Tier

class Listing(list):
    """Disassembly lines, plus the bytes and the mask behind them.

    A `list[str]` first, so everything that treats a listing as lines of text
    keeps working unchanged. The attributes carry what a listing CANNOT: the
    code itself, the address it was disassembled at, and the offsets a
    relocation makes unknowable.

    THE MASK IS WHY THIS EXISTS. An object file's `push <global>` is
    `68 00 00 00 00` with a relocation on the immediate; the image has the
    linked address in those four bytes. Comparing the rendered text calls that
    a divergence, which is how a body that reproduces the shipped bytes
    exactly was reported as MISMATCH. A byte a relocation determines is not a
    wrong answer, and `compare_asm` discounts it - but only if it is told
    which bytes those are.
    """

    def __init__(self, lines: Iterable[str], *, code: bytes = b"",
                 base: int = 0, mask: frozenset[int] = frozenset(),
                 flags: str = "") -> None:
        super().__init__(lines)
        self.code = code
        self.base = base
        self.mask = mask        # offsets into `code`, not addresses
        self.flags = flags      # the flag set that produced it, "" for the image


# WHAT THE COMPARISON ACCEPTS. `original_asm` and `compiled_asm` return
# `Listing`s and a caller may hand in bare lines; the difference is not
# cosmetic, because only a `Listing` carries the mask that lets a relocated
# byte be discounted. Spelling the union out is the honest signature: a
# `list[str]` alone would say the mask branch cannot be reached, and a bare
# `Listing` would say plain lines are refused when they are not.
Disassembly = Listing | list[str]


# -------------------------------------------------------------- the original


def _pe_bytes(exe: Path, address: int, size: int) -> bytes:
    """The image's bytes at `address`, mapped through the section table."""
    data = exe.read_bytes()
    (e_lfanew,) = struct.unpack_from("<I", data, 0x3C)
    if data[e_lfanew:e_lfanew + 4] != b"PE\x00\x00":
        raise ValueError(f"{exe.name}: not a PE image")
    _machine, n_sections, _ts, _sp, _ns, opt_size, _ch = \
        struct.unpack_from("<HHIIIHH", data, e_lfanew + 4)
    (image_base,) = struct.unpack_from("<I", data, e_lfanew + 24 + 28)
    table = e_lfanew + 24 + opt_size
    for i in range(n_sections):
        off = table + i * 40
        va, raw_size, raw_ptr = struct.unpack_from("<III", data, off + 12)
        begin = image_base + va
        if begin <= address < begin + raw_size:
            delta = address - begin
            if delta + size > raw_size:
                raise ValueError(
                    f"{exe.name}: 0x{address:08X}+{size} runs past the "
                    f"section's raw data")
            return data[raw_ptr + delta:raw_ptr + delta + size]
    raise ValueError(f"{exe.name}: 0x{address:08X} is in no section")


def _base_relocations(data: bytes) -> frozenset[int]:
    """Every address the loader rewrites when the image is rebased.

    Only HIGHLOW (type 3) marks anything: it names four bytes holding an
    address the linker filled in, and those are exactly the bytes an object
    file leaves at zero for a relocation to fix. Type 0 is ABSOLUTE, a
    block-alignment pad that names no byte at all - this image carries 362 of
    them, and counting those would mask real code.
    """
    (e_lfanew,) = struct.unpack_from("<I", data, 0x3C)
    optional = e_lfanew + 24
    (image_base,) = struct.unpack_from("<I", data, optional + 28)
    # PE32 lays its data directories at optional-header offset 96, eight bytes
    # each; the base relocation table is the sixth.
    directory_rva, directory_size = struct.unpack_from(
        "<II", data, optional + 96 + 5 * 8)
    if not directory_rva or not directory_size:
        return frozenset()

    n_sections, = struct.unpack_from("<H", data, e_lfanew + 6)
    opt_size, = struct.unpack_from("<H", data, e_lfanew + 20)
    table = e_lfanew + 24 + opt_size

    def file_offset(rva: int) -> int | None:
        for index in range(n_sections):
            off = table + index * 40
            va, raw_size, raw_ptr = struct.unpack_from("<III", data, off + 12)
            if va <= rva < va + raw_size:
                return raw_ptr + (rva - va)
        return None

    start = file_offset(directory_rva)
    if start is None:
        return frozenset()
    addresses, cursor, end = [], start, start + directory_size
    while cursor + 8 <= end:
        page, block = struct.unpack_from("<II", data, cursor)
        if block < 8:
            break
        for entry_at in range(cursor + 8, min(cursor + block, end), 2):
            (entry,) = struct.unpack_from("<H", data, entry_at)
            if entry >> 12 == 3:                       # HIGHLOW
                addresses.append(image_base + page + (entry & 0xFFF))
        cursor += block
    return frozenset(addresses)


@functools.lru_cache(maxsize=4)
def _image_relocations(exe: Path) -> frozenset[int]:
    """`_base_relocations` for a file, read once per image."""
    return _base_relocations(exe.read_bytes())


def _image_mask(exe: Path, low: int, high: int) -> frozenset[int]:
    """Offsets in `[low, high)`, relative to `low`, a base relocation owns.

    A HIGHLOW entry names the FIRST of four bytes, so each one masks four.
    """
    relocated = _image_relocations(exe)
    masked = set()
    # Walked over the SPAN rather than over the relocation table: the table
    # holds 107,760 entries and a body is tens to thousands of bytes, so
    # asking the span is orders of magnitude less work per record.
    for address in range(low - 3, high):
        if address in relocated:
            for byte in range(4):
                if low <= address + byte < high:
                    masked.add(address + byte - low)
    return frozenset(masked)


def original_asm(record: DecompilationState, exe: Path | str) -> Listing:
    """The shipped image's assembly for `record` - the bytes at its
    primary span in `exe`, disassembled. A record can carry no spans - a
    fresh annotation the catalogue has not stamped yet - and then nothing
    locates it in the image, so the call says so."""
    if not record.image_spans:
        raise ValueError(
            f"{record.address_hex}: the record carries no spans - nothing "
            f"locates it in the image")
    exe = Path(exe)
    if not exe.is_file():
        raise ValueError(f"no executable at {exe}")
    low, high = record.image_spans[0]
    code = _pe_bytes(exe, low, high - low)
    # SYMMETRY WITH THE OBJECT SIDE. A COMDAT is padded to alignment with
    # `int3`/`nop` and the catalogued span can include that padding, so both
    # sides are stripped or neither is: `0x0050F630` is a thirteen-byte body
    # in a sixteen-byte span.
    end = len(code)
    while end > 0 and code[end - 1] in (0x90, 0xCC):
        end -= 1
    code = code[:end]
    return Listing(_disasm(code, low), code=code, base=low,
                   mask=_image_mask(exe, low, low + len(code)))


# --------------------------------------------------------------- the compile


def _entry_for(source: Path, compile_commands: Path) -> dict[str, str]:
    """The compile_commands entry describing how the build compiles
    `source`."""
    if not compile_commands.is_file():
        raise ValueError(
            f"no {compile_commands} - configure the build first")
    table = json.loads(compile_commands.read_text())
    resolved = source.resolve()
    for candidate in table:
        file = Path(candidate["file"])
        if not file.is_absolute():
            file = Path(candidate.get("directory", ".")) / file
        if file.resolve() == resolved:
            return candidate
    raise ValueError(
        f"{source.name} is not a build input in {compile_commands.name}")


def _compile_command(entry: dict[str, str], flags: str) -> list[str]:
    """The build's own compile command with its optimisation flags swapped
    for `flags`.

    The leading tokens up to the first flag are the compiler the build
    invokes - this tree's wrapper, which knows where VC6 lives and how to
    run it under wine, and translates POSIX paths on its own. The include
    and define flags stay exactly as the build parses them; its PCH flags
    are dropped, because they point at a precompiled header built for the
    build's own object directory. `/Fo` and `/Fd` are replaced by the
    caller.
    """
    words = shlex.split(entry.get("command")
                        or " ".join(entry.get("arguments", [])))
    split = 0
    for index, word in enumerate(words):
        if index and word.startswith(("-", "/", "@")):
            split = index
            break
    compiler, kept = words[:split], []
    for word in words[split:]:
        if "cmake_pch" in word or word.startswith(("/Fo", "/Fd", "/Fp")):
            continue
        if word.startswith(("-I", "-D", "-FI")):
            word = "/" + word[1:]
        if (word.startswith(("/I", "/D")) and len(word) > 2) or \
                (word.startswith("/FI") and len(word) > 3):
            kept.append(word)
    return [*compiler, "/nologo", *flags.split(), *kept]


def _compile(source: Path, flags: str, compile_commands: Path) -> bytes:
    """Compile `source` with the build's own command; return the object's
    bytes."""
    command = _compile_command(_entry_for(source, compile_commands), flags)
    with tempfile.TemporaryDirectory() as work:
        result = subprocess.run([*command, "/Founit.obj", str(source)],
                                cwd=work, capture_output=True, text=True,
                                timeout=120)
        obj = Path(work) / "unit.obj"
        if not obj.is_file():
            raise ValueError("the compile produced no object:\n"
                             f"{result.stdout}\n{result.stderr}")
        return obj.read_bytes()


def _coff_function_masked(obj: bytes, symbol: str, emitted: str = "",
                          ) -> tuple[bytes, frozenset[int]]:
    """The symbol's code out of a COFF object - the `.text` section named
    by the symbol, from the symbol's value to the section's end, trailing
    NOP/INT3 padding stripped. With `/Gy` each function is its own COMDAT
    section, so the section end is the function end."""
    machine, n_sections, _ts, sym_ptr, n_syms, _opt, _ch = \
        struct.unpack_from("<HHIIIHH", obj, 0)
    if machine != 0x14C:
        raise ValueError("not an i386 COFF object")
    str_ptr = sym_ptr + n_syms * 18

    def sym_name(field: bytes) -> str:
        if field[:4] == b"\x00\x00\x00\x00":
            (off,) = struct.unpack("<I", field[4:])
            end = obj.index(b"\x00", str_ptr + off)
            return obj[str_ptr + off:end].decode()
        return field.rstrip(b"\x00").decode()

    sections = []
    for i in range(n_sections):
        off = 20 + i * 40
        name = obj[off:off + 8].rstrip(b"\x00").decode()
        raw_size, raw_ptr = struct.unpack_from("<II", obj, off + 16)
        reloc_ptr, = struct.unpack_from("<I", obj, off + 24)
        n_relocs, = struct.unpack_from("<H", obj, off + 32)
        sections.append((name, raw_size, raw_ptr, reloc_ptr, n_relocs))

    externals = []
    i = 0
    while i < n_syms:
        off = sym_ptr + i * 18
        name = sym_name(obj[off:off + 8])
        value, section_no, _type, storage, n_aux = \
            struct.unpack_from("<IhHBB", obj, off + 8)
        if storage == 2 and 0 < section_no <= len(sections) \
                and sections[section_no - 1][0] == ".text":
            externals.append((name, value, section_no - 1))
        i += 1 + n_aux

    # ONE RULE, AND IT READS A FACT. `emitted` is what the annotation says
    # this tree's compiler produces for the piece; `name` is what the IMAGE
    # calls it, and the two differ whenever the source re-expresses a
    # compiler-generated construct - an adjustor thunk, an atexit destructor,
    # a scalar deleting destructor - under a name of its own.
    #
    # THERE IS DELIBERATELY NO FALLBACK. Earlier versions guessed: a symbol
    # sharing the mangled head, then an identifier scraped out of the
    # region's definition head. Both existed to paper over a catalogued name
    # that was wrong or absent, and a lookup that guesses returns the
    # assembly of a NEIGHBOURING function, which reads as a mismatch nobody
    # can explain. The guessing lives in the migration that writes the fact
    # (`reader.region_identifiers`); what measures reads it.
    wanted = emitted or symbol
    matches = [e for e in externals
               if e[0] == wanted or (not emitted and e[0] == "_" + symbol)]
    if not matches:
        # TWO DIFFERENT FAULTS, SAID DIFFERENTLY. A recorded symbol that is
        # absent is a STALE fact - the annotation made a claim and the
        # compiler disagrees, so the fix is in the annotation. No recorded
        # symbol is a MISSING fact, and the fix is to write one.
        raise ValueError(
            f"{symbol}: the annotation records the emitted symbol "
            f"{emitted!r}, which is not among the object's .text symbols "
            f"- the fact is stale"
            if emitted else
            f"{symbol} not found among the object's .text symbols, and the "
            f"annotation records no `symbol` fact saying what this tree "
            f"emits for it")
    if len({(value, sec) for _n, value, sec in matches}) > 1:
        raise ValueError(f"{wanted} is ambiguous in the object")
    _name, value, sec = matches[0]
    _sec_name, raw_size, raw_ptr, reloc_ptr, n_relocs = sections[sec]
    code = obj[raw_ptr + value:raw_ptr + raw_size]
    end = len(code)
    while end > 0 and code[end - 1] in (0x90, 0xCC):
        end -= 1
    code = code[:end]

    # THE OBJECT'S OWN RELOCATIONS, in the same shape the image side returns:
    # offsets into `code`. DIR32 is an absolute address the linker writes,
    # REL32 a displacement it computes; both leave four bytes the object
    # cannot know and the image does.
    masked = set()
    for index in range(n_relocs):
        at = reloc_ptr + index * 10
        va, _symbol_index, kind = struct.unpack_from("<IIH", obj, at)
        if kind not in (0x0006, 0x0014):               # DIR32, REL32
            continue
        for byte in range(4):
            offset = va + byte - value
            if 0 <= offset < len(code):
                masked.add(offset)
    return code, frozenset(masked)


def _coff_function(obj: bytes, symbol: str) -> bytes:
    """The symbol's code alone, without the relocation mask beside it."""
    return _coff_function_masked(obj, symbol)[0]


def compiled_asm(record: DecompilationState, compile_commands: Path | str,
                 flags: str) -> Listing:
    """The assembly VC6 produces for the record's code: the record's file
    compiled by the build's own compile command with its optimisation flags
    swapped for `flags`, the function pulled out of the object by the
    record's mangled name, disassembled at the record's address so the
    listing lines up with `original_asm`."""
    if not record.name:
        raise ValueError(
            f"{record.address_hex}: no name fact under its marker")
    obj = _compile(record.path, flags, Path(compile_commands))
    code, mask = _coff_function_masked(obj, record.name, record.symbol)
    return Listing(_disasm(code, record.address), code=code,
                   base=record.address, mask=mask, flags=flags)


# ------------------------------------------------------------------ disasm


def _disasm(data: bytes, base: int) -> list[str]:
    """Lines of `0xADDR  bytes  mnemonic operands`, capstone under the hood."""
    return [f"0x{ins.address:08X}  "
            f"{' '.join(f'{b:02X}' for b in ins.bytes):<24}  "
            f"{ins.mnemonic} {ins.op_str}".rstrip()
            for ins in Cs(CS_ARCH_X86, CS_MODE_32).disasm(data, base)]


# -------------------------------------------------------------- comparison


@dataclass
class AsmComparison:
    """How far one assembly listing reproduces another."""
    verdict: Tier                   # see model.Tier
    original_count: int
    compiled_count: int
    matching_lines: int             # positionally identical listing lines
    mnemonic_similarity: float      # 0..1 across the instruction sequences
    first_divergence: int | None    # first differing instruction, or None
    context: tuple[list[str], list[str]]
                                    # (original, compiled) lines around it
    compared_bytes: int = 0         # bytes the verdict actually rests on
    masked_bytes: int = 0           # bytes a relocation determined, discounted
    flags: str = ""                 # the flag set that produced `compiled`


def _mnemonic_sequence(listing: list[str]) -> list[str]:
    """The instruction sequence of a listing, for similarity.

    `ret` keeps its operand: `ret` and `ret 4` are different instructions,
    and the pop count is exactly what a calling-convention slip changes.
    """
    out = []
    for line in listing:
        parts = re.split(r"\s{2,}", line.strip(), maxsplit=2)
        if len(parts) < 3:
            out.append(line.strip())
            continue
        instruction = parts[2].strip()
        out.append(instruction
                   if instruction.split()[0].startswith("ret")
                   else instruction.split()[0])
    return out


def _zeroed(listing: Listing) -> list[str]:
    """The listing re-rendered with every masked byte set to zero."""
    code = bytearray(listing.code)
    for offset in listing.mask:
        if 0 <= offset < len(code):
            code[offset] = 0
    return _disasm(bytes(code), listing.base)


def _byte_verdict(original: Disassembly,
                  compiled: Disassembly) -> bool | None:
    """Do the two agree byte for byte once relocations are discounted?

    `None` when the pair carries no bytes - plain listings, compared as text
    by the caller. THIS IS WHAT DECIDES BYTE_EXACT, on the bytes themselves
    rather than on rendered text, because rendering is where a mask can lie:
    see `_discount_relocations`.
    """
    if not (isinstance(original, Listing) and isinstance(compiled, Listing)):
        return None
    if not (original.code and compiled.code):
        return None
    if len(original.code) != len(compiled.code):
        return False
    mask = frozenset(original.mask) | frozenset(compiled.mask)
    return all(a == b for index, (a, b)
               in enumerate(zip(original.code, compiled.code))
               if index not in mask)


def _discount_relocations(original: Disassembly,
                          compiled: Disassembly,
                          ) -> tuple[list[str], list[str]]:
    """Both listings with relocated bytes neutralised, or both unchanged.

    The mask is applied to BOTH sides, not just the object's: the image has
    linked addresses where the object has zeros, so zeroing only one side
    leaves the same disagreement pointing the other way.

    ONLY WHEN THE TWO ARE THE SAME LENGTH, and that guard is load-bearing.
    A mask is a set of OFFSETS, and offsets only name the same thing in two
    bodies that are laid out alike. Where they are not, zeroing the image at
    an offset the OBJECT relocates overwrites bytes that carry no relocation
    at all, and re-disassembling the result splits instructions that were
    never there - `0x004E0F80` reported nine image instructions for a body
    that has seven. Bodies of different lengths have already lost, so they
    are reported exactly as they are.
    """
    if not (isinstance(original, Listing) and isinstance(compiled, Listing)):
        return original, compiled
    if not (original.code and compiled.code):
        return original, compiled
    if not (original.mask or compiled.mask):
        return original, compiled
    if len(original.code) != len(compiled.code):
        return original, compiled
    union = frozenset(original.mask) | frozenset(compiled.mask)
    return (_zeroed(Listing(original, code=original.code,
                            base=original.base, mask=union)),
            _zeroed(Listing(compiled, code=compiled.code,
                            base=compiled.base, mask=union)))


def compare_asm(original: Disassembly,
                compiled: Disassembly) -> AsmComparison:
    """How far the compiled listing reproduces the original one.

    The verdicts mirror tools/byte_match.py at the listing level:
    BYTE_EXACT when every instruction agrees byte for byte, MNEMONIC_ONLY
    when the instruction sequence agrees but constants or encodings differ,
    MISMATCH otherwise. The report says how many lines match outright, how
    similar the instruction sequences are, and where the first divergence
    is, with a window of both listings around it.

    RELOCATED BYTES ARE DISCOUNTED where both sides are `Listing`s that carry
    a mask. An object file's `push <global>` is `68 00 00 00 00` and the
    image's is `68` plus the linked address; the four bytes disagree because
    one side has been linked and the other has not, which is not a fact about
    the source. The listings are re-rendered with every masked byte zeroed on
    BOTH sides before they are compared, so such a pair reads as identical -
    while `context` still shows the real lines, because a reader wants the
    address the image actually holds. Plain lists carry no mask and compare
    as text, exactly as before.
    """
    left, right = _discount_relocations(original, compiled)
    n, m = len(left), len(right)
    matching = 0
    first_divergence = None
    context: tuple = ([], [])
    for index, (a, b) in enumerate(zip(left, right)):
        if a == b:
            matching += 1
        elif first_divergence is None:
            first_divergence = index
            lo = max(0, index - 3)
            # The window shows what each side REALLY holds, masked bytes
            # included: a reader chasing a divergence wants the address the
            # image carries, not the zero the comparison stood in for it.
            context = (list(original[lo:index + 4]),
                       list(compiled[lo:index + 4]))
    if first_divergence is None and n != m:
        first_divergence = min(n, m)
        lo = max(0, min(n, m) - 3)
        context = (list(original[lo:]), list(compiled[lo:]))

    seq_original = _mnemonic_sequence(left)
    seq_compiled = _mnemonic_sequence(right)
    similarity = difflib.SequenceMatcher(
        None, seq_original, seq_compiled).ratio()

    identical = _byte_verdict(original, compiled)
    if identical is True or (identical is None and n == m and matching == n):
        verdict = Tier.BYTE_EXACT
    elif seq_original == seq_compiled:
        verdict = Tier.MNEMONIC_ONLY
    else:
        verdict = Tier.MISMATCH
    masked = 0
    if isinstance(original, Listing) and isinstance(compiled, Listing):
        masked = len(frozenset(original.mask) | frozenset(compiled.mask))
    size = len(getattr(original, "code", b"") or b"")
    return AsmComparison(verdict, n, m, matching, similarity,
                         first_divergence, context,
                         compared_bytes=max(0, size - masked),
                         masked_bytes=masked,
                         flags=getattr(compiled, "flags", ""))


def compare_record(record: DecompilationState, exe: Path | str,
                   compile_commands: Path | str,
                   flags: tuple | str) -> AsmComparison:
    """How far VC6 reproduces the shipped bytes for `record`, best of `flags`.

    `flags` is a flag string, or several. THERE IS NO DEFAULT SET HERE, and
    that is the point: which invocation built a given function is not
    something this module can know, and a constant sitting in it would be
    read as the answer. It is not even a property of the BUILD - `/Oy-`
    appears nowhere in this tree's CMake configuration. It is a property of
    the FUNCTION: `/O2` implies `/Oy`, which omits the frame pointer, and the
    shipped image is mixed, so either spelling is wrong for roughly half of
    it. A caller that asks about one invocation has asked half a question,
    and the caller is who knows which half it meant.

    THE SEARCH CANNOT LIVE IN `compiled_asm`. Keeping the best result means
    comparing each candidate against the image, and the compiled side alone
    has never seen the image; this is the one function that holds both, so it
    is the one that can choose.

    A flag set whose compile fails, or whose object does not name the
    subject, is skipped rather than fatal - the question is whether ANY
    invocation reproduces the bytes. If every one fails, the last failure is
    raised, because "nothing compiled" and "nothing matched" are different
    answers and only the first names a defect to fix.
    """
    attempts = (flags,) if isinstance(flags, str) else tuple(flags)
    if not attempts:
        raise ValueError("no flag sets to try")
    original = original_asm(record, exe)
    best, failure = None, None
    for attempt in attempts:
        try:
            compiled = compiled_asm(record, compile_commands, attempt)
        except (ValueError, subprocess.SubprocessError) as error:
            failure = error
            continue
        result = compare_asm(original, compiled)
        if best is None or result.verdict.rank < best.verdict.rank:
            best = result
        if best.verdict is Tier.BYTE_EXACT:
            break                    # nothing beats byte equality
    if best is None:
        raise failure or ValueError(
            f"{record.address_hex}: no flag set produced a comparison")
    return best
