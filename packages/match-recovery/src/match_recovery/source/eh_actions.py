"""Typed facts for the small, bounded VC6 unwind-action grammar."""

from __future__ import annotations

from dataclasses import dataclass, replace
from enum import StrEnum

class EHActionKind(StrEnum):
    FRAME_TRANSFER = "frame_transfer"
    # The frame slot's source-level meaning is not encoded by this action kind.
    FRAME_SLOT_CALL_CLEANUP = "frame_slot_call_cleanup"
    ARRAY_DESTRUCTOR = "array_destructor"


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
