"""Typed facts for the small, bounded VC6 unwind-action grammar."""

from __future__ import annotations

from dataclasses import dataclass, replace
from enum import StrEnum

class EHActionKind(StrEnum):
    FRAME_TRANSFER = "frame_transfer"
    # The frame slot's source-level meaning is not encoded by this action kind.
    FRAME_SLOT_CALL_CLEANUP = "frame_slot_call_cleanup"
    ARRAY_DESTRUCTOR = "array_destructor"
    # ``lea eax,[ebp+d]``: the array is itself a frame local, not a pointer
    # loaded from a frame slot.
    FRAME_ARRAY_DESTRUCTOR = "frame_array_destructor"
    # A subobject reached through a possibly-null pointer conversion: VC6
    # materializes ``p ? p - adjust : 0`` in a frame temporary first.
    NULL_CHECKED_FRAME_TRANSFER = "null_checked_frame_transfer"
    # A cdecl call of several frame-loaded words, popped by ``add esp,4n``:
    # VC6's constructor-throw cleanup of a placement ``new`` calls the
    # matching ``operator delete(void *, extra...)`` this way.
    FRAME_ARGUMENTS_CALL_CLEANUP = "frame_arguments_call_cleanup"


class EHTransferRole(StrEnum):
    CLEANUP = "cleanup"
    CALLBACK = "callback"
    HELPER = "helper"


@dataclass(frozen=True)
class EHTransferFact:
    """One bounded action operand and its independently resolved target."""

    role: EHTransferRole
    operand_offset: int
    width: int
    encoding: str
    relocation_kind: str | None
    target: int | str | None
    addend: int | None


@dataclass(frozen=True)
class EHFrameGuardFact:
    """A frame-word bit test that returns without running the cleanup."""

    frame_displacement: int
    mask: int
    scratch_register: str


@dataclass(frozen=True)
class EHNullConversionFact:
    """Constants of one null-checked pointer conversion before a cleanup."""

    null_test_adjustment: int
    conversion_adjustment: int
    temporary_displacement: int


@dataclass(frozen=True)
class EHFrameArgumentFact:
    """One pushed word: a frame slot followed by zero or more dereferences.

    ``registers`` names the destination of each load in order; the last one
    is pushed. Offsets are signed displacements.
    """

    frame_displacement: int
    dereferences: tuple[int, ...]
    registers: tuple[str, ...]


@dataclass(frozen=True)
class EHActionFact:
    """Decoded action boundary and the operands needed for physical binding."""

    kind: EHActionKind
    start: int
    end: int
    transfers: tuple[EHTransferFact, ...]
    element_count: int | None = None
    element_size: int | None = None
    frame_displacement: int | None = None
    base_adjustment: int | None = None
    scratch_register: str | None = None
    guard: EHFrameGuardFact | None = None
    conversion: EHNullConversionFact | None = None
    # Pushed words in push order (last argument first) for the
    # frame-arguments call cleanup.
    arguments: tuple[EHFrameArgumentFact, ...] | None = None


def resolve_transfer(action: EHActionFact, *, role: EHTransferRole,
                     relocation_kind: str, target: int | str,
                     addend: int) -> EHActionFact:
    """Return an action with one transfer resolved from independent COFF facts."""
    transfers = tuple(replace(item, relocation_kind=relocation_kind,
                              target=target, addend=addend)
                      if item.role is role else item for item in action.transfers)
    if len(transfers) == len(action.transfers) and all(
            item.role is not role for item in action.transfers):
        raise ValueError("EH action is missing the requested transfer role")
    return replace(action, transfers=transfers)
