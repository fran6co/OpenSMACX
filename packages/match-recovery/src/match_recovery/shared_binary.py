"""Prove every declared shared body before exposing its reference target.

No duplicate-origin relaxation in the ordinary comparator: each extra emitted
definition gets its own bounded proof. A passing sibling cannot hide a failure.
"""

from .bound_objects import FunctionBinding, check_objects
from .model import Result


def check_shared(bindings, objects, image, external_targets, *, import_targets=None, errors=None):
    symbols = {item.symbol for item in bindings}
    if len(symbols) != len(bindings) or symbols & (external_targets.keys() | (import_targets or {}).keys()):
        raise ValueError("conflicting shared function symbol bindings")
    targets, rows = {}, {}
    pending = list(bindings)
    while pending:
        proven = []
        for binding in pending:
            row = {"interpretation_id": binding.interpretation_id, "entity_id": binding.origin.entity_id,
                   "evidence_entity_id": binding.evidence.entity_id, "symbol": binding.symbol,
                   "address": f"0x{binding.origin.address:08X}", "unit": binding.unit,
                   "source": {"path": binding.unit, "line": binding.line}}
            try:
                if binding.body_error:
                    raise ValueError(binding.body_error)
                if binding.unit in objects and objects[binding.unit].function(binding.symbol).storage != 2:
                    raise ValueError("shared method requires an external owning definition")
                proof, = check_objects([FunctionBinding(binding.origin, (binding.span,), binding.symbol, binding.unit)],
                    objects, image, external_targets={**external_targets, **targets},
                    import_targets=import_targets, errors=errors)
                row.update(proof)
            except ValueError as error:
                row.update(result=Result.UNKNOWN, reason=str(error))
            rows[binding.symbol] = row
            if row["result"] == Result.PASS:
                proven.append(binding)
        if not proven:
            break
        # Publish only after this whole round. Mutual unproved references remain
        # unknown; no convenient ordering or circular address assumptions.
        for binding in proven:
            targets[binding.symbol] = binding.origin.address
            pending.remove(binding)
    return targets, [rows[binding.symbol] for binding in bindings]
