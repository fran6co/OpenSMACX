"""Per-callee proof that a direct callee preserves one 32-bit register.

The path-bounded switch proof (see `associated_binary`) may carry a tracked
register value across a direct `call rel32` only when this module proves,
from the callee's own pinned image bytes, that the callee hands that register
back unchanged. No calling-convention (callee-saved) assumption is made.

A callee preserves register R when, on every state reachable from its entry:

* R is never written while it still holds the caller's value, or
* R is first saved by `push R` and every return happens after the matching
  `pop R` restored it from that exact stack slot.

Stack depth is tracked exactly relative to the callee entry: PUSH/POP,
ADD/SUB ESP,imm, the EBP frame (`mov ebp,esp`, `mov esp,ebp`,
`lea esp,[ebp+d]`, `leave`) and the stack effect of nested direct calls. A
nested callee's effect is the one RET immediate shared by every return
reachable in its own bytes (direct jumps and path-bounded jump tables); any
unresolved code or non-standard ESP write there leaves it unknown. That
nested callee's internal balance is its encoded return contract and is not
re-proved. The one non-standard stack helper admitted is the VC6 stack probe
(`_STACK_PROBE`), recognized only by exact unrelocated byte equality; its
effect (ESP lowered by the caller's EAX, which must be a tracked
`mov eax,imm` constant) is derived below. Indirect calls and unknown effects
leave ESP unknown until an EBP restore re-anchors it.

The save slot must stay live (ESP never rises above it while R is
clobbered) and no fixed-displacement ESP/EBP-relative store may overlap it;
such a store from an unknown frame base refuses. While R holds the caller's
value every nested direct call must itself be proved preserving R
(recursively, depth-bounded; recursion refuses). Once R is clobbered under a
live save, nested calls need only a stack effect.

Indirect jumps are followed only through `jmp [r*4+table]` whose index is
bounded on the path by an unsigned `cmp r,imm` edge (optionally through one
byte selector map); every table cell must carry a HIGHLOW relocation.

Imported Windows API functions (user decision 2026-11-16: "Take the
Windows's functions stack effect from it's documented signature") are the one
exception to "proved from the callee's own bytes". A call is an import call
when it is `call [slot]`, `call reg` where reg was loaded by `mov reg,[slot]`
on the path (and survived since), or a direct call/jump into a routine that
tail-jumps `jmp [slot]`, and the IAT `slot` carries a contract declared for
this image by `import_contracts.declare_import_contracts` (the
convention and argument bytes come from the header-derived decorated import
name in the toolchain's import libraries, never from the caller's code). Such a
call pops the declared bytes and, per the documented Win32 x86 contract that
the same documented signature states, hands back EBX, ESI, EDI and EBP
unchanged while EAX, ECX and EDX are clobbered. That contract applies only to
slots with a declared signature; an import without one refuses as before.

Virtual calls (user decision 2026-11-17) are followed only when the vtable
pointer is provably a constant stored into the analysed function's own frame:
on the path, `mov dword [frame],imm32` stored a HIGHLOW-relocated address of
read-only image data, the slot was then reloaded into the CALL's base register
(`call [r+disp]`), and between the store and the CALL no CALL of any kind ran
and no memory write could overlap the slot (see ASSUMPTIONS (b) and (c)
below for indexed virtual-base stores, pointer stores and calls). The slot cell `vtable+disp` must lie
in read-only initialized image data and carry a HIGHLOW relocation; its value
is the target, proved (and its stack effect taken) exactly like a direct
callee. A frame address may be adjusted by a virtual-base offset only when
that offset is read from read-only image data (no relocation) through a
vbtable pointer that is itself such a frame constant. The resolutions are
returned by `callee_preserves` as evidence records.

Any other virtual call `call [r+disp]` (user decision 2026-11-17, option 1:
image-wide slot proof) is followed through every vtable the image installs.
ASSUMPTION: an object only ever carries a vtable present in the image, and
the image installs each such vtable with a HIGHLOW-relocated
`mov dword [mem],imm32` whose immediate addresses read-only, initialized,
non-executable data holding a relocated pointer into executable code (VC6
constructors store vptrs this way). The candidate targets of slot `disp` are
then the relocated code pointers at `vtable+disp` over every such installed
vtable. The call is followed only when there is at least one candidate, every
candidate has the same known stack effect, and (while R holds the caller's
value) every candidate is proved to preserve R; any unprovable or differing
candidate refuses. This is proved from image bytes alone; the source plays no
part. A `call [r+disp]` through a non-vtable function-pointer table is held
to the same rule, so it only passes when every installed vtable's slot agrees.

Three further ASSUMPTIONS (user decisions 2026-11-17) refine virtual calls:

(a) A call that reaches its vptr through a virtual-base offset (the VC6
chain: vbptr load, vbtable entry `[vbptr+k]`, offset added to the object,
vptr load from the sum) only sees vtables the image installs through indexed
(virtual-base subobject) stores `mov [obj+off+disp],imm32`. Its slot
candidates are taken from those vtables alone.

(b) Objects carry only vbtables the image installs: relocated
`mov dword [mem],imm32` immediates addressing read-only data whose first
dword is an unrelocated multiple of 4 in (-0x10000, 0] (MSVC's vbtable[0],
the vbptr's offset back to its object). When entry k of every such candidate
is an unrelocated non-negative multiple of 4, a frame store
`[frame + r + disp]` whose r was read at vbtable entry k writes only at or
above frame+disp, so frame vptr facts wholly below it survive the store.

(c) A frame object's known vptr changes only in that object's own
constructor or destructor (the C++ object model). The image shows such a call
as one receiving a frame address in ECX whose callee (the direct target,
the resolved virtual target or every slot candidate) itself contains a
vtable/vbtable install store; it forgets the facts at or above that address.
An unresolved call receiving a frame address counts as one. Every other call
keeps the facts, as do stores through non-frame pointers unless they install
a vtable/vbtable themselves; exact frame stores, frame stores at an unknown
index and string stores still forget what they may overlap.

Everything else refuses: undecodable or non-executable bytes, other indirect
calls while R holds the caller's value (`call reg`, ESP/EBP-based and indexed
memory calls),
unbounded indirect jumps, interrupts and other unsupported
transfers, a restore at an unknown or wrong stack depth, recursion and the
search/depth bounds. Stores through computed pointers (including indexed frame
addresses) are outside this model.
"""

from __future__ import annotations

from capstone import (
    CS_AC_WRITE, CS_ARCH_X86, CS_GRP_BRANCH_RELATIVE, CS_GRP_CALL, CS_GRP_INT, CS_GRP_IRET,
    CS_GRP_JUMP, CS_GRP_RET, CS_MODE_32, Cs, x86,
)

from .import_contracts import declared_import_contracts
from .model import Span
from .pe_coff import PeImage


_ENGINE = Cs(CS_ARCH_X86, CS_MODE_32)
_ENGINE.detail = True
_CODE = 0x00000020
_EXECUTE = 0x20000000
PRESERVATION_DEPTH_LIMIT = 8
PRESERVATION_STATE_LIMIT = 20000
_FAMILIES = {
    "al": "eax", "ah": "eax", "ax": "eax", "eax": "eax",
    "bl": "ebx", "bh": "ebx", "bx": "ebx", "ebx": "ebx",
    "cl": "ecx", "ch": "ecx", "cx": "ecx", "ecx": "ecx",
    "dl": "edx", "dh": "edx", "dx": "edx", "edx": "edx",
    "si": "esi", "esi": "esi", "di": "edi", "edi": "edi",
    "bp": "ebp", "ebp": "ebp", "sp": "esp", "esp": "esp",
}
_PRESERVABLE = frozenset(("eax", "ebx", "ecx", "edx", "esi", "edi", "ebp"))
_ORIGINAL = "original"
_UNSUPPORTED = frozenset(("hlt", "ud2", "into", "sysenter", "syscall", "enter",
                          "lcall", "ljmp", "retf", "iretd"))
# VC6 `_chkstk` (position independent, no relocations):
#   push ecx; cmp eax,1000h; lea ecx,[esp+8]; jb last
#   probe: sub ecx,1000h; sub eax,1000h; test [ecx],eax; cmp eax,1000h; jae probe
#   last: sub ecx,eax; mov eax,esp; test [ecx],eax; mov esp,ecx
#         mov ecx,[eax]; mov eax,[eax+4]; push eax; ret
# With E the caller's ESP before the CALL and N its EAX: `lea ecx,[esp+8]`
# is E, the loop lowers ECX and EAX in step, so `mov esp,ecx` sets ESP = E-N;
# ECX and the return address are reloaded from the old frame and
# `push eax; ret` returns with ESP = E-N. Only EAX and flags change.
_STACK_PROBE = bytes.fromhex(
    "513d001000008d4c2408721481e9001000002d0010000085013d0010000073ec"
    "2bc88bc485018be18b088b400450c3")
_STACK_PROBE_PRESERVES = frozenset(("ebx", "ecx", "edx", "esi", "edi", "ebp"))
_PROBE = "probe"
# Documented Win32 x86 contract of every documented API function (stdcall and
# cdecl alike): these registers come back unchanged; EAX, ECX, EDX do not.
_IMPORT_PRESERVES = frozenset(("ebx", "esi", "edi", "ebp"))
_READ_ONLY_DATA = 0x00000040
_MEM_WRITE = 0x80000000
_STRING_STORES = ("stos", "movs", "ins")


class _Refusal(ValueError):
    pass


class _ContextRefusal(_Refusal):
    """A depth/cycle refusal that depends on the query's call chain."""


class _Context:
    """Decoded-instruction, verdict and stack-effect caches for one image."""

    def __init__(self, image: PeImage):
        self.image = image
        self.imports: dict = declared_import_contracts(image)
        self.decoded: dict[int, object] = {}
        self.verdicts: dict[tuple[int, str], str] = {}
        self.effects: dict[int, int | str | None] = {}
        self.evidence: dict[tuple[int, str], frozenset] = {}
        self._vtables: tuple[int, ...] | None = None
        self._indexed: tuple[int, ...] = ()
        self._vbtables: tuple[int, ...] = ()
        self._slots: dict[tuple[int, bool], tuple[int, ...]] = {}
        self._vbase: dict[int, bool] = {}
        self.installers: dict[int, bool] = {}
        self._tables: frozenset[int] | None = None
        self.taint = 0  # count of depth/call-chain refusals met so far

    def _code_region(self, address: int):
        matches = [(region, size, flags) for (region, _, size), flags
                   in zip(self.image.sections, self.image.section_flags)
                   if region.start <= address < region.start + size]
        if len(matches) != 1:
            raise _Refusal(f"callee code {address:#x} is not file-backed image storage")
        region, size, flags = matches[0]
        if not flags & _CODE or not flags & _EXECUTE:
            raise _Refusal(f"callee code {address:#x} is not executable image code")
        return region, size

    def decode(self, address: int):
        cached = self.decoded.get(address)
        if cached is not None:
            return cached
        region, size = self._code_region(address)
        end = min(address + 15, region.start + size)
        decoded = next(_ENGINE.disasm(self.image.read(Span(address, end)), address, 1), None)
        if decoded is None:
            raise _Refusal(f"callee code {address:#x} does not decode")
        self.decoded[address] = decoded
        return decoded

    def read_only_word(self, address: int) -> int | None:
        """A dword of read-only, initialized, non-executable image data."""
        matches = [flags for (region, _, size), flags in zip(self.image.sections, self.image.section_flags)
                   if region.start <= address and address + 4 <= region.start + size]
        if (len(matches) != 1 or not matches[0] & _READ_ONLY_DATA
                or matches[0] & (_MEM_WRITE | _EXECUTE | _CODE)):
            return None
        return int.from_bytes(self.image.read(Span(address, address + 4)), "little")

    def relocated_immediate(self, instruction, value: int) -> bool:
        """Whether a HIGHLOW relocation inside the instruction encodes `value`."""
        return any(site in self.image.relocations
                   and int.from_bytes(instruction.bytes[site - instruction.address:
                                                        site - instruction.address + 4],
                                      "little") == value
                   for site in range(instruction.address, instruction.address + instruction.size - 3))

    def executable(self, address: int) -> bool:
        try:
            self._code_region(address)
        except _Refusal:
            return False
        return True

    def _installs(self) -> None:
        """Scan every relocated `mov dword [mem],imm32` immediate in executable code.

        Its value is a vtable when it addresses read-only data holding a
        relocated pointer into executable code; the vtable is "indexed" when
        some install addresses memory through an index register (the VC6
        virtual-base subobject store `mov [obj+off+disp],imm32`). Its value is
        a vbtable candidate when it addresses read-only data whose first
        dword is unrelocated, a multiple of 4 and in (-0x10000, 0] (MSVC's
        vbtable[0] is the vbptr's offset back to its object start).
        """
        if self._vtables is not None:
            return
        vtables, indexed, vbtables = set(), set(), set()
        for site in sorted(self.image.relocations):
            if not self.executable(site):
                continue
            value = int.from_bytes(self.image.read(Span(site, site + 4)), "little")
            first = self.read_only_word(value)
            if first is None:
                continue
            is_vtable = value in self.image.relocations and self.executable(first)
            is_vbtable = (not any(cell in self.image.relocations for cell in range(value - 3, value + 4))
                          and -0x10000 < _signed(first) <= 0 and not first % 4)
            if not (is_vtable or is_vbtable):
                continue
            # The immediate ends the instruction: `C7 modrm [sib] [disp8|disp32] imm32`.
            for start in range(site - 7, site - 1):
                if not self.executable(start):
                    continue
                code = self.image.read(Span(start, site + 4))
                decoded = next(_ENGINE.disasm(code, start, 1), None)
                if (decoded is not None and decoded.address + decoded.size == site + 4
                        and decoded.mnemonic == "mov" and len(decoded.operands) == 2
                        and decoded.operands[0].type == x86.X86_OP_MEM
                        and decoded.operands[0].size == 4
                        and decoded.operands[1].type == x86.X86_OP_IMM
                        and decoded.operands[1].imm & 0xFFFFFFFF == value):
                    if is_vtable:
                        vtables.add(value)
                        if decoded.operands[0].mem.index:
                            indexed.add(value)
                    else:
                        vbtables.add(value)
                    break
        self._indexed = tuple(sorted(indexed))
        self._vbtables = tuple(sorted(vbtables))
        self._vtables = tuple(sorted(vtables))

    def install_tables(self) -> frozenset[int]:
        """Every installed vtable and vbtable candidate."""
        self._installs()
        if self._tables is None:
            self._tables = frozenset(self._vtables) | frozenset(self._vbtables)
        return self._tables

    def vtables(self, indexed: bool = False) -> tuple[int, ...]:
        """Every vtable the image installs, or only those installed through
        virtual-base (indexed) stores (see the module ASSUMPTIONS)."""
        self._installs()
        return self._indexed if indexed else self._vtables

    def vbase_offsets_nonnegative(self, displacement: int) -> bool:
        """Whether entry `displacement` of every installed vbtable candidate is
        an unrelocated, non-negative multiple of 4 (a virtual-base offset)."""
        cached = self._vbase.get(displacement)
        if cached is not None:
            return cached
        self._installs()
        result = displacement >= 4 and not displacement % 4 and bool(self._vbtables)
        for vbtable in self._vbtables if result else ():
            cell = vbtable + displacement
            word = self.read_only_word(cell)
            if (word is None or any(site in self.image.relocations for site in range(cell - 3, cell + 4))
                    or _signed(word) < 0 or word % 4):
                result = False
                break
        self._vbase[displacement] = result
        return result

    def slot_targets(self, displacement: int, indexed: bool = False) -> tuple[int, ...]:
        """Relocated code pointers at `vtable+displacement` over every installed
        vtable (only the virtual-base-installed ones when `indexed`)."""
        key = (displacement, indexed)
        cached = self._slots.get(key)
        if cached is not None:
            return cached
        targets = set()
        for vtable in self.vtables(indexed):
            cell = (vtable + displacement) & 0xFFFFFFFF
            target = self.read_only_word(cell)
            if target is not None and cell in self.image.relocations and self.executable(target):
                targets.add(target)
        self._slots[key] = tuple(sorted(targets))
        return self._slots[key]

    def is_stack_probe(self, entry: int) -> bool:
        try:
            region, size = self._code_region(entry)
        except _Refusal:
            return False
        end = entry + len(_STACK_PROBE)
        if end > region.start + size:
            return False
        return (self.image.read(Span(entry, end)) == _STACK_PROBE
                and not any(entry <= site < end for site in self.image.relocations))

    def table_targets(self, instruction, bounds: frozenset) -> tuple[int, ...]:
        """Targets of `jmp [r*4 + table]` whose index is bounded on this path."""
        operands = instruction.operands
        if (instruction.id != x86.X86_INS_JMP or len(operands) != 1
                or operands[0].type != x86.X86_OP_MEM or operands[0].size != 4):
            raise _Refusal(f"callee reaches an unbounded indirect jump at {instruction.address:#x}")
        memory = operands[0].mem
        maximum = dict(bounds).get(_family(instruction, memory.index)) if memory.index else None
        if (memory.base or memory.scale != 4 or memory.segment != x86.X86_REG_INVALID
                or maximum is None or maximum > _MAX_TABLE_INDEX):
            raise _Refusal(f"callee reaches an unbounded indirect jump at {instruction.address:#x}")
        table = memory.disp & 0xFFFFFFFF
        targets = []
        for index in range(maximum + 1):
            cell = table + 4 * index
            if cell not in self.image.relocations:
                raise _Refusal(f"callee jump table cell {cell:#x} lacks a HIGHLOW relocation")
            targets.append(int.from_bytes(self.image.read(Span(cell, cell + 4)), "little"))
        return tuple(targets)


_CONTEXTS: dict[int, tuple[PeImage, _Context]] = {}


def _context(image: PeImage) -> _Context:
    cached = _CONTEXTS.get(id(image))
    # A context built under other declared contracts is stale.
    if (cached is not None and cached[0] is image
            and cached[1].imports == declared_import_contracts(image)):
        return cached[1]
    if len(_CONTEXTS) > 8:
        _CONTEXTS.clear()
    context = _Context(image)
    _CONTEXTS[id(image)] = (image, context)
    return context


def _family(instruction, register: int) -> str | None:
    return _FAMILIES.get(instruction.reg_name(register))


def _written_families(instruction) -> set[str]:
    try:
        _, writes = instruction.regs_access()
    except Exception as error:
        raise _Refusal(f"cannot establish register writes at {instruction.address:#x}") from error
    families = set()
    for register in writes:
        family = _family(instruction, register)
        if family is not None:
            families.add(family)
    return families


def _direct_target(instruction) -> int | None:
    operands = instruction.operands
    if len(operands) == 1 and operands[0].type == x86.X86_OP_IMM:
        return operands[0].imm & 0xFFFFFFFF
    return None


def _absolute_slot(operand) -> int | None:
    """The address of a plain `[disp32]` memory operand, if it is one."""
    if operand.type != x86.X86_OP_MEM or operand.size != 4:
        return None
    memory = operand.mem
    if memory.base or memory.index or memory.segment != x86.X86_REG_INVALID:
        return None
    return memory.disp & 0xFFFFFFFF


def _import_slot(context: "_Context", instruction, held: frozenset) -> int | None:
    """The declared import an indirect CALL/JMP transfers to, if any."""
    operands = instruction.operands
    if len(operands) != 1:
        return None
    operand = operands[0]
    if operand.type == x86.X86_OP_REG and operand.size == 4:
        slot = dict(held).get(_family(instruction, operand.reg))
    else:
        slot = _absolute_slot(operand)
    return slot if slot in context.imports else None


def _held_step(context: "_Context", instruction, held: frozenset) -> frozenset:
    """Registers holding a declared import's pointer, loaded by `mov r32,[slot]`."""
    written = _written_families(instruction)
    result = frozenset(item for item in held if item[0] not in written)
    operands = instruction.operands
    if (instruction.mnemonic == "mov" and len(operands) == 2
            and operands[0].type == x86.X86_OP_REG and operands[0].size == 4):
        slot = _absolute_slot(operands[1])
        family = _family(instruction, operands[0].reg)
        if slot in context.imports and family not in (None, "esp"):
            result = result | {(family, slot)}
    return result


def _import_label(context: "_Context", slot: int) -> str:
    contract = context.imports[slot]
    module = getattr(contract, "module", "?")
    name = getattr(contract, "name", None) or f"#{getattr(contract, 'ordinal', '?')}"
    return f"{module}!{name}"


def _frame_store_overlaps(instruction, esp: int | None, ebp: int | None, slot: int) -> str | None:
    """Why an explicit ESP/EBP-relative store may overlap the save slot, if it may."""
    for operand in instruction.operands:
        if operand.type != x86.X86_OP_MEM or not operand.access & CS_AC_WRITE:
            continue
        memory = operand.mem
        base = _family(instruction, memory.base) if memory.base else None
        if base not in ("esp", "ebp") or memory.index:
            # Indexed and other computed addresses are pointer stores,
            # outside this model like every store through a computed pointer.
            continue
        if memory.segment != x86.X86_REG_INVALID:
            return "stores through a segment-relative frame address"
        anchor = esp if base == "esp" else ebp
        if anchor is None:
            return f"stores relative to an unknown {base} depth"
        start = anchor + memory.disp
        if start < slot + 4 and slot < start + operand.size:
            return "overwrites the save slot"
    return None


def _stack_step(instruction, esp: int | None, ebp: int | None) -> tuple[int | None, int | None]:
    """Track ESP/EBP (relative to callee entry) across one non-control instruction."""
    mnemonic = instruction.mnemonic
    operands = instruction.operands
    written = _written_families(instruction)
    new_esp, new_ebp = esp, ebp
    if mnemonic == "leave":
        return (None if ebp is None else ebp + 4), None
    if mnemonic in ("push", "pop") and len(operands) == 1:
        size = operands[0].size if mnemonic == "pop" else -operands[0].size
        new_esp = None if esp is None else esp + size
        if mnemonic == "pop" and "esp" in {_family(instruction, operands[0].reg)
                                           if operands[0].type == x86.X86_OP_REG else None}:
            new_esp = None
    elif mnemonic in ("pushf", "popf", "pushfd", "popfd", "pushal", "pushad", "popal", "popad"):
        size = {"pushf": 2, "popf": 2, "pushfd": 4, "popfd": 4}.get(mnemonic, 32)
        new_esp = None if esp is None else esp + (size if mnemonic.startswith("pop") else -size)
    elif "esp" in written:
        new_esp = None
        if (len(operands) == 2 and operands[0].type == x86.X86_OP_REG
                and _family(instruction, operands[0].reg) == "esp" and operands[0].size == 4):
            source = operands[1]
            if mnemonic in ("add", "sub") and source.type == x86.X86_OP_IMM and esp is not None:
                new_esp = esp + (source.imm if mnemonic == "add" else -source.imm)
            elif (mnemonic == "mov" and source.type == x86.X86_OP_REG
                  and _family(instruction, source.reg) == "ebp"):
                new_esp = ebp
            elif (mnemonic == "lea" and source.type == x86.X86_OP_MEM and source.mem.base
                  and _family(instruction, source.mem.base) == "ebp"
                  and not source.mem.index and ebp is not None):
                new_esp = ebp + source.mem.disp
    if "ebp" in written:
        new_ebp = None
        if (mnemonic == "mov" and len(operands) == 2
                and operands[0].type == x86.X86_OP_REG and operands[1].type == x86.X86_OP_REG
                and _family(instruction, operands[0].reg) == "ebp"
                and _family(instruction, operands[1].reg) == "esp"):
            new_ebp = esp
    return new_esp, new_ebp


_MAX_TABLE_INDEX = 255
_BOUND_RELATIONS = {
    # (taken bound, fallthrough bound) for `cmp r, imm`; "imm" or "imm-1".
    x86.X86_INS_JA: (None, 0), x86.X86_INS_JAE: (None, -1),
    x86.X86_INS_JB: (-1, None), x86.X86_INS_JBE: (0, None),
    x86.X86_INS_JE: (0, None), x86.X86_INS_JNE: (None, 0),
}


def _with_bound(bounds: frozenset, family: str, maximum: int) -> frozenset:
    current = dict(bounds)
    if family in current:
        maximum = min(maximum, current[family])
    current[family] = maximum
    return frozenset(current.items())


def _edge_bounds(instruction, flags, bounds: frozenset) -> tuple[frozenset, frozenset]:
    """Unsigned upper bounds a conditional edge establishes from `cmp r32, imm`."""
    relation = _BOUND_RELATIONS.get(instruction.id)
    if flags is None or relation is None:
        return bounds, bounds
    family, immediate = flags
    result = []
    for adjustment in relation:
        if adjustment is None or immediate + adjustment < 0:
            result.append(bounds)
        else:
            result.append(_with_bound(bounds, family, immediate + adjustment))
    return result[0], result[1]


def _selector_bound(context: _Context, instruction, current: dict) -> int | None:
    """Bound of a byte loaded from a selector map indexed by a bounded register.

    Admits `movzx r32, byte [idx+map]` and `mov r8low, byte [idx+map]` when
    the destination register was bounded by 0xFF (upper bytes zero). The map
    bytes are read from the pinned image and must carry no relocation.
    """
    operands = instruction.operands
    if len(operands) != 2 or operands[0].type != x86.X86_OP_REG or operands[1].type != x86.X86_OP_MEM:
        return None
    source = operands[1]
    memory = source.mem
    if source.size != 1 or memory.segment != x86.X86_REG_INVALID:
        return None
    registers = [register for register in (memory.base, memory.index) if register]
    if len(registers) != 1 or (memory.index and memory.scale != 1):
        return None
    maximum = current.get(_family(instruction, registers[0]))
    if maximum is None or maximum > _MAX_TABLE_INDEX:
        return None
    destination = instruction.reg_name(operands[0].reg)
    if instruction.mnemonic == "movzx" and operands[0].size == 4:
        pass
    elif (instruction.mnemonic == "mov" and destination in ("al", "bl", "cl", "dl")
          and current.get(_family(instruction, operands[0].reg), 0x100) <= 0xFF):
        pass
    else:
        return None
    start = memory.disp & 0xFFFFFFFF
    end = start + maximum + 1
    if any(site in context.image.relocations for site in range(start - 3, end)):
        return None
    try:
        return max(context.image.read(Span(start, end)))
    except ValueError:
        return None


def _value_step(instruction, flags, bounds: frozenset, context: _Context | None = None):
    """Carry the `cmp r32, imm` flag source and per-register unsigned bounds."""
    operands = instruction.operands
    written = _written_families(instruction)
    current = dict(bounds)
    selected = None if context is None else _selector_bound(context, instruction, current)
    copied = None
    if (instruction.mnemonic == "mov" and len(operands) == 2
            and operands[0].type == x86.X86_OP_REG and operands[0].size == 4
            and operands[1].type == x86.X86_OP_REG and operands[1].size == 4):
        copied = current.get(_family(instruction, operands[1].reg))
    zeroed = (instruction.mnemonic == "xor" and len(operands) == 2
              and operands[0].type == x86.X86_OP_REG and operands[1].type == x86.X86_OP_REG
              and operands[0].reg == operands[1].reg and operands[0].size == 4)
    for family in written:
        current.pop(family, None)
    if copied is not None:
        current[_family(instruction, operands[0].reg)] = copied
    if zeroed:
        current[_family(instruction, operands[0].reg)] = 0
    if selected is not None:
        current[_family(instruction, operands[0].reg)] = selected
    if (instruction.mnemonic == "cmp" and len(operands) == 2
            and operands[0].type == x86.X86_OP_REG and operands[0].size == 4
            and operands[1].type == x86.X86_OP_IMM):
        family = _family(instruction, operands[0].reg)
        next_flags = None if family == "esp" else (family, operands[1].imm & 0xFFFFFFFF)
    elif instruction.eflags or written & ({flags[0]} if flags else set()):
        next_flags = None
    else:
        next_flags = flags
    return next_flags, frozenset(current.items())


def _eax_constant(instruction, eax: int | None) -> int | None:
    """The tracked `mov eax, imm32` constant after one instruction."""
    operands = instruction.operands
    if (instruction.mnemonic == "mov" and len(operands) == 2
            and operands[0].type == x86.X86_OP_REG and operands[0].reg == x86.X86_REG_EAX
            and operands[1].type == x86.X86_OP_IMM):
        return operands[1].imm & 0xFFFFFFFF
    return None if "eax" in _written_families(instruction) else eax


def _value_of(values: frozenset, family: str | None):
    for item in values:
        if item[0] == family:
            return item[1:]
    return None


def _frame_address(instruction, memory, esp: int | None, ebp: int | None,
                   values: frozenset):
    """(entry-relative offset, provenance) of a frame memory operand, or None.

    Exactly one component is a frame anchor (ESP/EBP at a known depth or a
    register holding a frame address); every other register component must
    hold a read-only table integer.
    """
    if memory.segment != x86.X86_REG_INVALID:
        return None
    anchor, offset, provenance = None, memory.disp, ()
    for register, scale in ((memory.base, 1), (memory.index, memory.scale)):
        if not register:
            continue
        family = _family(instruction, register)
        if family in ("esp", "ebp"):
            depth = esp if family == "esp" else ebp
            value = None if depth is None else ("frame", depth, ())
        else:
            value = _value_of(values, family)
        if value is None:
            return None
        kind, amount, origin = value
        if kind == "frame" and scale == 1 and anchor is None:
            anchor = amount
        elif kind == "int":
            offset += amount * scale
        else:
            return None
        provenance += origin
    if anchor is None:
        return None
    return anchor + offset, provenance


def _signed(value: int) -> int:
    value &= 0xFFFFFFFF
    return value - (1 << 32) if value & 0x80000000 else value


def _fact_step(context: _Context, instruction, esp: int | None, ebp: int | None,
               facts: frozenset, values: frozenset) -> tuple[frozenset, frozenset]:
    """Carry frame vtable facts and register values across one non-call instruction.

    A fact is (entry-relative slot, constant, provenance) for a dword slot
    that a relocated `mov dword [frame],imm32` set to an address of
    read-only image data. A value is (family, kind, amount, provenance) with
    kind "frame" (an entry-relative frame address), "vptr" (a fact reloaded
    into a register) or "int" (a dword of read-only image data read through
    a vptr value, e.g. a vbtable's virtual-base offset).
    """
    operands = instruction.operands
    mnemonic = instruction.mnemonic
    # Memory writes: each must resolve to an exact frame range or every fact is lost.
    for operand in operands:
        if operand.type != x86.X86_OP_MEM or not operand.access & CS_AC_WRITE:
            continue
        resolved = _frame_address(instruction, operand.mem, esp, ebp, values)
        plain = not (mnemonic == "pop" or mnemonic.startswith(_STRING_STORES)
                     or instruction.prefix[0] in (x86.X86_PREFIX_REP, x86.X86_PREFIX_REPNE))
        if resolved is None and plain:
            floor = _virtual_base_floor(context, instruction, operand.mem, esp, ebp, values)
            if floor is not None:
                # Writes start at or above `floor`: facts wholly below survive.
                facts = frozenset(fact for fact in facts if fact[0] + 4 <= floor)
                continue
        if resolved is None and plain and not _frame_based(instruction, operand.mem, values):
            # ASSUMPTION (c): a store through a non-frame pointer is not a vptr
            # write unless it installs a vtable/vbtable itself.
            if (mnemonic == "mov" and len(operands) == 2 and operands[1].type == x86.X86_OP_IMM
                    and operands[1].imm & 0xFFFFFFFF in context.install_tables()):
                facts = frozenset()
                break
            continue
        if resolved is None or not plain:
            facts = frozenset()
            break
        start = resolved[0]
        facts = frozenset(fact for fact in facts
                          if not (start < fact[0] + 4 and fact[0] < start + operand.size))
    if mnemonic in ("push", "pushf", "pushfd", "pushal", "pushad"):
        size = (operands[0].size if mnemonic == "push" and operands
                else {"pushf": 2, "pushfd": 4}.get(mnemonic, 32))
        if esp is None:
            facts = frozenset()
        else:
            start = esp - size
            facts = frozenset(fact for fact in facts if not (start < fact[0] + 4 and fact[0] < esp))
    if (mnemonic == "mov" and len(operands) == 2 and operands[0].type == x86.X86_OP_MEM
            and operands[0].size == 4 and operands[1].type == x86.X86_OP_IMM):
        constant = operands[1].imm & 0xFFFFFFFF
        resolved = _frame_address(instruction, operands[0].mem, esp, ebp, values)
        if (resolved is not None and context.read_only_word(constant) is not None
                and context.relocated_immediate(instruction, constant)):
            slot = resolved[0]
            origin = resolved[1] + (("store", instruction.address, slot, constant),)
            facts = facts | {(slot, constant, origin)}
    # Register values.
    written = _written_families(instruction)
    previous = values
    values = frozenset(item for item in values if item[0] not in written)
    if len(operands) != 2 or operands[0].type != x86.X86_OP_REG or operands[0].size != 4:
        return facts, values
    destination = _family(instruction, operands[0].reg)
    if destination in (None, "esp", "ebp"):
        return facts, values
    source = operands[1]
    value = None
    if mnemonic == "lea" and source.type == x86.X86_OP_MEM:
        resolved = _frame_address(instruction, source.mem, esp, ebp, previous)
        if resolved is not None:
            value = ("frame", resolved[0], resolved[1])
    elif mnemonic == "mov" and source.type == x86.X86_OP_REG and source.size == 4:
        value = _value_of(previous, _family(instruction, source.reg))
        if value is None and _family(instruction, source.reg) in ("esp", "ebp"):
            depth = esp if _family(instruction, source.reg) == "esp" else ebp
            value = None if depth is None else ("frame", depth, ())
    elif mnemonic == "mov" and source.type == x86.X86_OP_MEM and source.size == 4:
        memory = source.mem
        resolved = _frame_address(instruction, memory, esp, ebp, previous)
        if resolved is not None:
            slot = resolved[0]
            fact = next((fact for fact in facts if fact[0] == slot), None)
            if fact is not None:
                value = ("vptr", fact[1], resolved[1] + fact[2])
        elif (memory.base and not memory.index and memory.segment == x86.X86_REG_INVALID
              and (_value_of(previous, _family(instruction, memory.base)) or ("",))[0] == "vptr"):
            base = _value_of(previous, _family(instruction, memory.base))
            if base is not None and base[0] == "vptr":
                cell = (base[1] + memory.disp) & 0xFFFFFFFF
                word = context.read_only_word(cell)
                if word is not None and not any(site in context.image.relocations
                                                for site in range(cell - 3, cell + 4)):
                    value = ("int", _signed(word), base[2] + (("table", cell, _signed(word)),))
    elif mnemonic == "add" and source.type == x86.X86_OP_REG and source.size == 4:
        left = _value_of(previous, destination)
        right = _value_of(previous, _family(instruction, source.reg))
        if left is not None and right is not None:
            kinds = {left[0], right[0]}
            if kinds == {"frame", "int"}:
                value = ("frame", left[1] + right[1], left[2] + right[2])
    if (value is None and mnemonic == "mov" and source.type == x86.X86_OP_MEM
            and source.size == 4 and source.mem.disp == 0 and source.mem.base
            and not source.mem.index and source.mem.segment == x86.X86_REG_INVALID):
        base = _value_of(previous, _family(instruction, source.mem.base))
        if base is not None and base[0] == "object":
            value = ("vptr", base[1], base[2])
    if value is None:
        value = _virtual_base_value(instruction, previous)
    if value is not None:
        values = values | {(destination, *value)}
    return facts, values


def _installs_vptr(context: _Context, entry: int) -> bool:
    """Whether a callee's own reachable body installs a vtable or vbtable.

    Scans the instructions reachable from `entry` through fallthrough, direct
    jumps and path-bounded jump tables (calls fall through, like `_effect`)
    for a `mov dword [mem],imm32` whose immediate is an installed vtable or
    vbtable candidate: the vptr/vbptr store of a VC6 constructor, or the vptr
    re-install a VC6 destructor makes before tearing down. Anything that
    cannot be scanned counts as installing.
    """
    cached = context.installers.get(entry)
    if cached is not None:
        return cached
    tables = context.install_tables()
    pending = [(entry, None, frozenset())]
    seen = set()
    result = False
    try:
        while pending and not result:
            state = pending.pop()
            if state in seen:
                continue
            seen.add(state)
            if len(seen) > PRESERVATION_STATE_LIMIT:
                raise _Refusal("installer scan exceeds its bound")
            address, flags, bounds = state
            instruction = context.decode(address)
            following = address + instruction.size
            operands = instruction.operands
            if (instruction.mnemonic == "mov" and len(operands) == 2
                    and operands[0].type == x86.X86_OP_MEM and operands[0].size == 4
                    and operands[1].type == x86.X86_OP_IMM
                    and operands[1].imm & 0xFFFFFFFF in tables):
                result = True
                break
            if instruction.group(CS_GRP_RET):
                continue
            if instruction.group(CS_GRP_CALL):
                pending.append((following, None, frozenset()))
                continue
            if instruction.group(CS_GRP_JUMP) or instruction.group(CS_GRP_BRANCH_RELATIVE):
                target = _direct_target(instruction)
                if target is None:
                    if _import_slot(context, instruction, frozenset()) is not None:
                        continue
                    pending.extend((item, None, bounds)
                                   for item in context.table_targets(instruction, bounds))
                    continue
                taken, fallthrough = _edge_bounds(instruction, flags, bounds)
                pending.append((target, flags, taken))
                if instruction.id != x86.X86_INS_JMP:
                    pending.append((following, flags, fallthrough))
                continue
            next_flags, next_bounds = _value_step(instruction, flags, bounds, context)
            pending.append((following, next_flags, next_bounds))
    except _Refusal:
        result = True
    context.installers[entry] = result
    return result


def _facts_across_call(context: _Context, targets, values: frozenset,
                       facts: frozenset, address: int = 0, depth: int = 0,
                       active: frozenset[int] = frozenset()) -> frozenset:
    """Frame vtable facts that survive one CALL (ASSUMPTION (c)).

    A frame object's vptr changes only in that object's own constructor or
    destructor. The image shows such a call as one that receives the object's
    address in ECX (VC6 thiscall) and whose callee installs a vtable or
    vbtable (`_installs_vptr`). Such a call forgets every fact at or above
    the address in ECX; an unknown callee receiving a frame address counts
    as one. Every other call, including one that receives the address of an
    object whose vptr is known, keeps the facts.
    """
    if not facts:
        return facts
    this = _value_of(values, "ecx")
    if this is None or this[0] != "frame":
        return facts
    if targets is not None and not any(_installs_vptr(context, target) for target in targets):
        return facts
    kept = frozenset(fact for fact in facts if fact[0] < this[1])
    # ASSUMPTION (e1): after the object's own constructor its vptr is the last
    # vtable that constructor installs at [this], read from its bytes.
    if targets is not None and len(targets) == 1:
        constructed = _constructed_vptr(context, targets[0], depth, active)
        if constructed is not None:
            kept = kept | {(this[1], constructed, (("store", address, this[1], constructed),))}
    return kept


def _receiver(values: frozenset, facts: frozenset) -> int | None:
    """The known vptr of the object a CALL passes in ECX (ASSUMPTION (e2))."""
    this = _value_of(values, "ecx")
    if this is None:
        return None
    if this[0] == "object":
        return this[1]
    if this[0] == "frame":
        return next((fact[1] for fact in facts if fact[0] == this[1]), None)
    return None


def _objects_across_call(context: _Context, targets, values: frozenset, depth: int,
                         active: frozenset[int]) -> frozenset:
    """Object values (`this` with a known vptr) that survive a CALL.

    A value survives in a register the callee is proved to hand back, unless
    the call is that object's own constructor/destructor (ASSUMPTION (c)).
    """
    objects = [item for item in values if item[1] == "object"]
    if not objects or targets is None:
        return frozenset()
    this = _value_of(values, "ecx")
    kept = set()
    for item in objects:
        if item[0] == "eax":
            continue
        if (this is not None and this[0] == "object" and this[1] == item[2]
                and any(_installs_vptr(context, target) for target in targets)):
            continue
        if all(_preserves_quietly(context, target, item[0], depth, active) for target in targets):
            kept.add(item)
    return frozenset(kept)


def _constructed_vptr(context: _Context, entry: int, depth: int = 0,
                      active: frozenset[int] = frozenset()) -> int | None:
    """The vtable a constructor leaves at [this] (ASSUMPTION (e1)), or None.

    Tracks the registers holding `this` (ECX at entry, register copies; a
    callee-saved copy survives a nested call only when that callee is proved
    to preserve it). A `mov dword [this],vtable` sets the result; a nested
    call receiving `this` whose callee installs (a base constructor) sets it
    to that callee's own result. Every return must agree. A vtable store
    through any other register at displacement 0 or an unscannable path
    gives None.
    """
    key = ("constructed", entry)
    if key in context.installers:
        return context.installers[key]
    context.installers[key] = None  # recursion guard
    if depth > PRESERVATION_DEPTH_LIMIT or entry in active:
        return None
    active = active | {entry}
    tables = frozenset(context.vtables())
    # `this` holds (family, offset) pairs: registers holding this+offset.
    pending = [(entry, frozenset({("ecx", 0)}), None, None, frozenset())]
    seen = set()
    results = set()
    try:
        while pending:
            state = pending.pop()
            if state in seen:
                continue
            seen.add(state)
            if len(seen) > PRESERVATION_STATE_LIMIT:
                raise _Refusal("constructor scan exceeds its bound")
            address, this, last, flags, bounds = state
            instruction = context.decode(address)
            following = address + instruction.size
            operands = instruction.operands
            if instruction.group(CS_GRP_RET):
                results.add(last)
                continue
            if instruction.group(CS_GRP_CALL):
                target = _direct_target(instruction)
                if target is None and _import_slot(context, instruction, frozenset()) is not None:
                    pending.append((following, frozenset(item for item in this
                                                         if item[0] in _IMPORT_PRESERVES),
                                    last, None, frozenset()))
                    continue
                if target is None or instruction.bytes[0] != 0xE8:
                    raise _Refusal("unresolved call in a constructor")
                if ("ecx", 0) in this and _installs_vptr(context, target):
                    last = _constructed_vptr(context, target, depth + 1, active)
                    if last is None:
                        raise _Refusal("a base constructor's result is unknown")
                kept = frozenset(item for item in this if item[0] not in ("eax", "ecx", "edx")
                                 and _preserves_quietly(context, target, item[0], depth, active))
                pending.append((following, kept, last, None, frozenset()))
                continue
            if instruction.group(CS_GRP_JUMP) or instruction.group(CS_GRP_BRANCH_RELATIVE):
                target = _direct_target(instruction)
                if target is None:
                    pending.extend((item, this, last, None, bounds)
                                   for item in context.table_targets(instruction, bounds))
                    continue
                taken, fallthrough = _edge_bounds(instruction, flags, bounds)
                pending.append((target, this, last, flags, taken))
                if instruction.id != x86.X86_INS_JMP:
                    pending.append((following, this, last, flags, fallthrough))
                continue
            if (instruction.mnemonic == "mov" and len(operands) == 2
                    and operands[0].type == x86.X86_OP_MEM and operands[0].size == 4
                    and operands[1].type == x86.X86_OP_IMM
                    and operands[1].imm & 0xFFFFFFFF in tables):
                memory = operands[0].mem
                offsets = dict(this)
                base = _family(instruction, memory.base) if memory.base else None
                if (base in offsets and not memory.index
                        and memory.segment == x86.X86_REG_INVALID):
                    if offsets[base] + memory.disp == 0:
                        last = operands[1].imm & 0xFFFFFFFF
                elif memory.disp == 0 or not memory.base:
                    raise _Refusal("a vtable store through an untracked pointer")
            written = _written_families(instruction)
            offsets = dict(this)
            copied = None
            if (instruction.mnemonic == "mov" and len(operands) == 2
                    and operands[0].type == x86.X86_OP_REG and operands[1].type == x86.X86_OP_REG
                    and operands[0].size == 4 and _family(instruction, operands[1].reg) in offsets):
                copied = (_family(instruction, operands[0].reg),
                          offsets[_family(instruction, operands[1].reg)])
            elif (instruction.mnemonic == "lea" and len(operands) == 2
                    and operands[0].type == x86.X86_OP_REG and operands[0].size == 4
                    and operands[1].mem.base and not operands[1].mem.index
                    and _family(instruction, operands[1].mem.base) in offsets):
                copied = (_family(instruction, operands[0].reg),
                          offsets[_family(instruction, operands[1].mem.base)] + operands[1].mem.disp)
            this = frozenset(item for item in this if item[0] not in written)
            if copied is not None:
                this = this | {copied}
            next_flags, next_bounds = _value_step(instruction, flags, bounds, context)
            pending.append((following, this, last, next_flags, next_bounds))
    except _Refusal:
        results = {None}
    result = next(iter(results)) if len(results) == 1 else None
    context.installers[key] = result
    return result


def _virtual_base_value(instruction, previous: frozenset):
    """Shape tags of the VC6 virtual-base access chain, for one register write.

    "word": any dword loaded from memory (e.g. an object's vbptr).
    "vbofs": `[word+disp]`, disp >= 4: a candidate virtual-base offset read
    from vbtable entry disp/4 (amount = disp).
    "vbsub": a register sum (`add`, `lea [a+b]`) with a "vbofs" term: an
    adjusted virtual-base subobject address.
    "vbvptr": `[vbsub]` or `[a+b]` with a "vbofs" term and no displacement:
    the vptr of a virtual-base subobject.
    """
    operands = instruction.operands
    mnemonic = instruction.mnemonic
    source = operands[1]

    def kind(register):
        value = _value_of(previous, _family(instruction, register)) if register else None
        return None if value is None else value[0]

    if mnemonic == "mov" and source.type == x86.X86_OP_MEM and source.size == 4:
        memory = source.mem
        if memory.segment != x86.X86_REG_INVALID:
            return ("word", 0, ())
        kinds = {kind(memory.base), kind(memory.index)}
        if memory.disp == 0 and memory.index and memory.scale == 1 and "vbofs" in kinds:
            return ("vbvptr", 0, ())
        if memory.disp == 0 and not memory.index and kind(memory.base) == "vbsub":
            return ("vbvptr", 0, ())
        if (memory.base and not memory.index and kind(memory.base) == "word"
                and memory.disp >= 4 and not memory.disp % 4):
            return ("vbofs", memory.disp, ())
        return ("word", 0, ())
    if mnemonic == "add" and source.type == x86.X86_OP_REG and source.size == 4:
        if "vbofs" in {kind(operands[0].reg), kind(source.reg)}:
            return ("vbsub", 0, ())
    if mnemonic == "lea" and source.type == x86.X86_OP_MEM:
        memory = source.mem
        if memory.index and memory.scale == 1 and "vbofs" in {kind(memory.base), kind(memory.index)}:
            return ("vbsub", 0, ())
    return None


def _frame_based(instruction, memory, values: frozenset) -> bool:
    """Whether a memory operand may address the frame: ESP/EBP or a register
    holding a frame address appears in it (any other base is a pointer)."""
    for register in (memory.base, memory.index):
        if not register:
            continue
        family = _family(instruction, register)
        if family in ("esp", "ebp"):
            return True
        value = _value_of(values, family)
        if value is not None and value[0] == "frame":
            return True
    return not (memory.base or memory.index)  # an absolute address: be conservative


def _virtual_base_floor(context: _Context, instruction, memory, esp: int | None,
                        ebp: int | None, values: frozenset) -> int | None:
    """Lowest entry-relative frame offset an indexed virtual-base store can write.

    The store is `[frame + r + disp]` (scale 1) where r holds a "vbofs"
    value read at vbtable entry k; when every installed vbtable candidate's
    entry k is non-negative (ASSUMPTION (b)), the write lands at or above
    frame+disp. Any other shape returns None.
    """
    if memory.segment != x86.X86_REG_INVALID or not memory.base or not memory.index or memory.scale != 1:
        return None
    anchor, offset = None, None
    for first, second in ((memory.base, memory.index), (memory.index, memory.base)):
        family = _family(instruction, first)
        if family in ("esp", "ebp"):
            depth = esp if family == "esp" else ebp
        else:
            value = _value_of(values, family)
            depth = value[1] if value is not None and value[0] == "frame" else None
        other = _value_of(values, _family(instruction, second))
        if depth is not None and other is not None and other[0] == "vbofs":
            anchor, offset = depth, other[1]
            break
    if anchor is None or not context.vbase_offsets_nonnegative(offset):
        return None
    return anchor + memory.disp


def _virtual_target(context: _Context, instruction, values: frozenset):
    """(target, evidence) of `call [r+disp]` through a frame-constant vtable, or None."""
    operands = instruction.operands
    if len(operands) != 1 or operands[0].type != x86.X86_OP_MEM or operands[0].size != 4:
        return None
    memory = operands[0].mem
    if not memory.base or memory.index or memory.segment != x86.X86_REG_INVALID:
        return None
    value = _value_of(values, _family(instruction, memory.base))
    if value is None or value[0] != "vptr":
        return None
    cell = (value[1] + memory.disp) & 0xFFFFFFFF
    target = context.read_only_word(cell)
    if target is None or cell not in context.image.relocations:
        return None  # not a relocated read-only vtable slot: an unprovable call
    stores = [{"address": item[1], "frame_offset": item[2], "constant": item[3]}
              for item in value[2] if item[0] == "store"]
    tables = [{"cell": item[1], "value": item[2]} for item in value[2] if item[0] == "table"]
    record = (("call", instruction.address), ("vtable", value[1]), ("slot", cell),
              ("target", target),
              ("stores", tuple(tuple(sorted(item.items())) for item in stores)),
              ("table_reads", tuple(tuple(sorted(item.items())) for item in tables)))
    return target, record


def _slot_call(context: _Context, instruction,
               values: frozenset = frozenset()) -> tuple[int, tuple[int, ...], bool] | None:
    """(slot displacement, candidate targets, narrowed) of a `call [r+disp]` virtual call, or None.

    A call whose base register holds a virtual-base subobject's vptr
    ("vbvptr") considers only vtables installed through virtual-base
    (indexed) stores (ASSUMPTION (a)).
    """
    operands = instruction.operands
    if len(operands) != 1 or operands[0].type != x86.X86_OP_MEM or operands[0].size != 4:
        return None
    memory = operands[0].mem
    if (not memory.base or memory.index or memory.segment != x86.X86_REG_INVALID
            or _family(instruction, memory.base) in ("esp", "ebp")):
        return None
    displacement = memory.disp
    if displacement < 0 or displacement % 4:
        return None
    base = _value_of(values, _family(instruction, memory.base))
    indexed = base is not None and base[0] == "vbvptr"
    return displacement, context.slot_targets(displacement, indexed), indexed


def _slot_effect(context: _Context, candidates: tuple[int, ...]) -> int | None:
    """The one known stack effect every slot candidate shares, else None."""
    effects = {_effect(context, candidate) for candidate in candidates}
    if len(effects) != 1:
        return None
    effect = next(iter(effects))
    return effect if isinstance(effect, int) else None


def _analyse(context: _Context, entry: int, family: str, depth: int,
             active: frozenset[int], receiver: int | None = None) -> None:
    """Prove `family` preserved on every reachable state of one callee.

    The register status is ORIGINAL (the register holds the caller's value
    and no save is live) or (slot, clobbered) after `push R` stored the
    caller's value at the ESP-relative `slot`.
    """
    if depth > PRESERVATION_DEPTH_LIMIT:
        raise _ContextRefusal(f"callee {entry:#x} exceeds the preservation depth bound")
    if entry in active:
        raise _ContextRefusal(f"callee {entry:#x} is recursive; preservation is not proved")
    active = active | {entry}
    empty = frozenset()
    # ASSUMPTION (e2): a caller's known receiver vptr arrives with `this` (ECX).
    initial = (frozenset({("ecx", "object", receiver, (("receiver", entry, receiver),))})
               if receiver is not None else empty)
    stack = [(entry, 0, None, None, _ORIGINAL, None, empty, empty, empty, initial)]
    seen = set()
    returns = 0
    evidence = set()
    while stack:
        state = stack.pop()
        if state in seen:
            continue
        seen.add(state)
        if len(seen) > PRESERVATION_STATE_LIMIT:
            raise _Refusal(f"callee {entry:#x} exceeds the preservation search bound")
        address, esp, ebp, eax, status, flags, bounds, held, facts, values = state
        instruction = context.decode(address)
        following = address + instruction.size
        if (instruction.group(CS_GRP_INT) or instruction.group(CS_GRP_IRET)
                or instruction.mnemonic in _UNSUPPORTED):
            raise _Refusal(f"callee {entry:#x} reaches an unsupported transfer at {address:#x}")
        if any(operand.type == x86.X86_OP_MEM and operand.mem.segment == x86.X86_REG_SS
               for operand in instruction.operands):
            raise _Refusal(f"callee {entry:#x} uses an explicit stack segment at {address:#x}")
        saved = status != _ORIGINAL
        overlap = _frame_store_overlaps(instruction, esp, ebp, status[0]) if saved else None
        if overlap:
            raise _Refusal(f"callee {entry:#x} {overlap} while {family} is saved, at {address:#x}")
        if instruction.group(CS_GRP_RET):
            if saved and status[1]:
                raise _Refusal(f"callee {entry:#x} returns at {address:#x} without restoring {family}")
            returns += 1
            continue
        if instruction.group(CS_GRP_JUMP) and _direct_target(instruction) is None:
            slot = _import_slot(context, instruction, held)
            if slot is not None:
                # Tail jump into a declared import: it returns to our caller.
                if saved:
                    raise _Refusal(f"callee {entry:#x} tail-jumps to {_import_label(context, slot)} "
                                   f"with {family} saved, at {address:#x}")
                if family not in _IMPORT_PRESERVES:
                    raise _Refusal(f"callee {entry:#x} tail-jumps to {_import_label(context, slot)}, "
                                   f"which does not preserve {family}, at {address:#x}")
                returns += 1
                continue
        if instruction.group(CS_GRP_CALL):
            target = _direct_target(instruction)
            next_status = status
            slot = None if target is not None else _import_slot(context, instruction, held)
            if slot is not None:
                if family not in _IMPORT_PRESERVES:
                    if not saved:
                        raise _Refusal(f"callee {entry:#x} calls {_import_label(context, slot)}, which "
                                       f"does not preserve {family}, at {address:#x}")
                    next_status = (status[0], True)
                next_esp = None if esp is None else esp + context.imports[slot].popped
                next_held = frozenset(item for item in held if item[0] in _IMPORT_PRESERVES)
                # An imported API is never a frame object's constructor/destructor.
                kept = frozenset(item for item in values
                                 if item[1] == "object" and item[0] in _IMPORT_PRESERVES)
                stack.append((following, next_esp, ebp, None, next_status, None, empty, next_held,
                              facts, kept))
                continue
            next_held = frozenset()
            virtual = None if target is not None else _virtual_target(context, instruction, values)
            if virtual is not None:
                target = virtual[0]
                evidence.add(virtual[1])
            slots = None
            if target is None and virtual is None:
                slots = _slot_call(context, instruction, values)
                # No candidate, or candidates without one shared known effect:
                # the call stays unprovable (handled below).
                if slots is not None and (not slots[1] or _slot_effect(context, slots[1]) is None):
                    slots = None
            if slots is not None:
                displacement, candidates = slots[:2]
                effect = _slot_effect(context, candidates)
                if not (saved and status[1]):
                    try:
                        for candidate in candidates:
                            _verdict(context, candidate, family, depth + 1, active)
                            evidence |= context.evidence.get((candidate, family), empty)
                    except _Refusal:
                        if not saved:
                            raise
                        next_status = (status[0], True)
                narrowed = slots[2]
                evidence.add((("call", address), ("kind", "image_wide_slot"),
                              ("slot_offset", displacement), ("targets", candidates),
                              ("vtables", len(context.vtables(narrowed))),
                              ("virtual_base", narrowed), ("stores", ()),
                              ("table_reads", ())))
            elif target is None or (virtual is None and instruction.bytes[0] != 0xE8):
                if not saved:
                    raise _Refusal(f"callee {entry:#x} makes an unprovable call at {address:#x}")
                effect = None
                if saved:
                    next_status = (status[0], True)
            else:
                effect = _effect(context, target)
                receiver_in = _receiver(values, facts)
                if not (saved and status[1]):
                    try:
                        _verdict(context, target, family, depth + 1, active, receiver_in)
                        evidence |= context.evidence.get(_key(target, family, receiver_in), empty)
                    except _Refusal:
                        if not saved:
                            raise
                        # The save is live; a nested clobber only means the
                        # POP must restore the register.
                        next_status = (status[0], True)
                next_held = frozenset(item for item in held
                                      if _preserves_quietly(context, target, item[0], depth, active))
                for item in next_held:
                    evidence |= context.evidence.get((target, item[0]), empty)
            if effect == _PROBE:
                next_esp = None if esp is None or eax is None else esp - eax
            else:
                next_esp = None if esp is None or effect is None else esp + effect
            # ASSUMPTION (c): only the object's own constructor/destructor changes its vptr.
            possible = ((target,) if target is not None
                        else slots[1] if slots is not None else None)
            next_facts = _facts_across_call(context, possible, values, facts, address, depth, active)
            next_values = _objects_across_call(context, possible, values, depth, active)
            stack.append((following, next_esp, ebp, None, next_status, None, empty, next_held,
                          next_facts, next_values))
            continue
        if instruction.group(CS_GRP_JUMP) or instruction.group(CS_GRP_BRANCH_RELATIVE):
            target = _direct_target(instruction)
            if target is None:
                for target in context.table_targets(instruction, bounds):
                    stack.append((target, esp, ebp, eax, status, None, bounds, held, facts, values))
                continue
            taken, fallthrough = _edge_bounds(instruction, flags, bounds)
            stack.append((target, esp, ebp, eax, status, flags, taken, held, facts, values))
            if instruction.id != x86.X86_INS_JMP:
                stack.append((following, esp, ebp, eax, status, flags, fallthrough, held, facts, values))
            continue
        next_status = status
        operands = instruction.operands
        register_operand = (len(operands) == 1 and operands[0].type == x86.X86_OP_REG
                            and _family(instruction, operands[0].reg) == family
                            and operands[0].size == 4)
        if instruction.mnemonic == "push" and register_operand:
            if not saved and esp is not None:
                next_status = (esp - 4, False)
        elif instruction.mnemonic == "pop" and register_operand:
            if not saved:
                raise _Refusal(f"callee {entry:#x} overwrites {family} at {address:#x}")
            if esp is None:
                raise _Refusal(f"callee {entry:#x} restores {family} at an unknown stack depth")
            next_status = _ORIGINAL if esp == status[0] else (status[0], True)
        elif family in _written_families(instruction):
            if not saved:
                raise _Refusal(f"callee {entry:#x} overwrites {family} at {address:#x}")
            next_status = (status[0], True)
        next_esp, next_ebp = _stack_step(instruction, esp, ebp)
        if next_status != _ORIGINAL and next_esp is not None and next_esp > next_status[0]:
            if next_status[1]:
                raise _Refusal(f"callee {entry:#x} releases its saved {family} slot at {address:#x}")
            next_status = _ORIGINAL  # released while the register still holds the value
        next_flags, next_bounds = _value_step(instruction, flags, bounds, context)
        next_facts, next_values = _fact_step(context, instruction, esp, ebp, facts, values)
        stack.append((following, next_esp, next_ebp, _eax_constant(instruction, eax), next_status,
                      next_flags, next_bounds, _held_step(context, instruction, held),
                      next_facts, next_values))
    if not returns:
        raise _Refusal(f"callee {entry:#x} has no reachable return")
    return frozenset(evidence)


def _preserves_quietly(context: _Context, target: int, family: str, depth: int,
                       active: frozenset[int], receiver: int | None = None) -> bool:
    """Whether a nested direct callee is proved to hand `family` back."""
    try:
        _verdict(context, target, family, depth + 1, active, receiver)
    except _Refusal:
        return False
    return True


def _standard_stack_write(instruction) -> bool:
    """ESP writes whose effect `_stack_step` derives exactly from the frame."""
    if instruction.mnemonic in ("push", "pop", "leave", "pushf", "popf", "pushfd", "popfd",
                                "pushal", "pushad", "popal", "popad"):
        return not (instruction.mnemonic == "pop" and instruction.operands
                    and instruction.operands[0].type == x86.X86_OP_REG
                    and _family(instruction, instruction.operands[0].reg) == "esp")
    operands = instruction.operands
    if (len(operands) != 2 or operands[0].type != x86.X86_OP_REG
            or _family(instruction, operands[0].reg) != "esp" or operands[0].size != 4):
        return False
    source = operands[1]
    return ((instruction.mnemonic in ("add", "sub") and source.type == x86.X86_OP_IMM)
            or (instruction.mnemonic == "mov" and source.type == x86.X86_OP_REG
                and _family(instruction, source.reg) == "ebp")
            or (instruction.mnemonic == "lea" and source.type == x86.X86_OP_MEM
                and bool(source.mem.base) and _family(instruction, source.mem.base) == "ebp"
                and not source.mem.index))


def _effect(context: _Context, entry: int) -> int | str | None:
    """A nested callee's declared stack effect, `_PROBE`, or None (unknown).

    The declared effect is the one RET immediate shared by every return
    reachable from `entry` (fallthrough, direct jumps and path-bounded jump
    tables; calls fall through). It is unknown when any reachable code is
    unresolved or writes ESP other than by the standard frame forms. The
    nested callee's own internal balance is its encoded contract and is not
    re-proved; the stack probe is the only admitted non-standard helper.
    """
    if entry in context.effects:
        return context.effects[entry]
    if context.is_stack_probe(entry):
        context.effects[entry] = _PROBE
        return _PROBE
    effect = None
    pending = [(entry, None, frozenset())]
    seen = set()
    immediates = set()
    try:
        while pending:
            state = pending.pop()
            if state in seen:
                continue
            seen.add(state)
            if len(seen) > PRESERVATION_STATE_LIMIT:
                raise _Refusal("stack-effect scan exceeds its bound")
            address, flags, bounds = state
            instruction = context.decode(address)
            following = address + instruction.size
            if (instruction.group(CS_GRP_INT) or instruction.group(CS_GRP_IRET)
                    or instruction.mnemonic in _UNSUPPORTED):
                raise _Refusal("unsupported transfer")
            if instruction.group(CS_GRP_RET):
                immediates.add(instruction.operands[0].imm if instruction.operands else 0)
                continue
            if instruction.group(CS_GRP_CALL):
                pending.append((following, None, frozenset()))
                continue
            if instruction.group(CS_GRP_JUMP) or instruction.group(CS_GRP_BRANCH_RELATIVE):
                target = _direct_target(instruction)
                slot = None if target is not None else _import_slot(context, instruction, frozenset())
                if slot is not None:
                    immediates.add(context.imports[slot].popped)  # tail jump into an import
                    continue
                if target is None:
                    pending.extend((item, None, bounds)
                                   for item in context.table_targets(instruction, bounds))
                    continue
                taken, fallthrough = _edge_bounds(instruction, flags, bounds)
                pending.append((target, flags, taken))
                if instruction.id != x86.X86_INS_JMP:
                    pending.append((following, flags, fallthrough))
                continue
            if "esp" in _written_families(instruction) and not _standard_stack_write(instruction):
                raise _Refusal("non-standard stack pointer write")
            pending.append((following, *_value_step(instruction, flags, bounds, context)))
        if len(immediates) == 1:
            effect = immediates.pop()
    except _Refusal:
        effect = None
    context.effects[entry] = effect
    return effect


def _key(entry: int, family: str, receiver: int | None) -> tuple:
    return (entry, family) if receiver is None else (entry, family, receiver)


def _verdict(context: _Context, entry: int, family: str, depth: int,
             active: frozenset[int], receiver: int | None = None) -> None:
    key = _key(entry, family, receiver)
    cached = context.verdicts.get(key)
    if isinstance(cached, str) and cached != "proved":
        raise _Refusal(cached)
    if cached is not None:
        return
    if context.is_stack_probe(entry):
        if family not in _STACK_PROBE_PRESERVES:
            context.verdicts[key] = f"stack probe {entry:#x} overwrites {family}"
            raise _Refusal(context.verdicts[key])
        context.verdicts[key] = "proved"
        return
    taint = context.taint
    try:
        resolved = _analyse(context, entry, family, depth, active, receiver)
    except _ContextRefusal:
        context.taint += 1
        raise
    except _Refusal as error:
        if context.taint == taint:
            context.verdicts[key] = str(error)
        raise
    context.evidence[key] = resolved
    context.verdicts[key] = "proved"


def callee_preserves(image: PeImage, entry: int, family: str) -> list[dict]:
    """Raise ValueError unless the callee at `entry` provably preserves `family`.

    Returns the virtual calls the proof resolved (in this callee or any
    nested callee whose proof it relied on): for each, the CALL address,
    the frame-constant vtable, the slot cell read, the target proved, the
    constant stores (address, entry-relative frame offset, constant) and any
    read-only table reads (virtual-base offsets) the resolution used.
    """
    if type(image) is not PeImage:
        raise ValueError("callee preservation requires a pinned parsed PE image")
    if family not in _PRESERVABLE:
        raise ValueError(f"callee preservation does not model register {family!r}")
    if type(entry) is not int or not 0 <= entry <= 0xFFFFFFFF:
        raise ValueError("callee entry is outside i386 address space")
    context = _context(image)
    _verdict(context, entry, family, 0, frozenset())
    records = []
    for record in sorted(context.evidence.get((entry, family), frozenset())):
        item = dict(record)
        item["stores"] = [dict(store) for store in item["stores"]]
        item["table_reads"] = [dict(read) for read in item["table_reads"]]
        records.append(item)
    return records
