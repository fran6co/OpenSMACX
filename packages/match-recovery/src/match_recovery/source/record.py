"""Pure transitions of historical source claims; no compiler or file writes.

Only a measuring coordinator may publish a new claim. This function describes
the annotation transition and is also exercised in memory by the source-integrity
checker. A tier supplied by a caller is not itself verification evidence.
"""

from dataclasses import replace

from .model import DecompilationState, Tier


def stamped(record: DecompilationState, tier: Tier,
            demote: bool = False) -> DecompilationState:
    """Prepare a legacy annotation without writing it or discarding obligations.

    Historical LEVER/TRIED text follows the existing transition convention.
    Demotion must be explicit; this API does not authorize it. Current recovery
    preserves matching requirements while structurally justified edits proceed.
    """
    if not isinstance(tier, Tier) or type(demote) is not bool:
        raise ValueError("claim transition requires a Tier and boolean demotion decision")
    if tier is Tier.BYTE_EXACT:
        return replace(record, byte_exact=True, semantic=False, ruled_out=())
    if not record.byte_exact:
        # An unfinished body's lever can describe partial progress, not a match.
        # Converting it to a failed attempt would change its recorded meaning.
        return replace(record, byte_exact=False)
    return replace(record, byte_exact=not demote, levers=(),
                   ruled_out=record.ruled_out
                   + tuple(f"{key} {prose}" for key, prose in record.levers))
