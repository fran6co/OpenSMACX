"""Prove every declared shared body before exposing its reference target.

No duplicate-origin relaxation in the ordinary comparator: each extra emitted
definition gets its own bounded proof. A passing sibling cannot hide a failure,
except through an observed link (weak_selection.LinkObservation): when its MAP
places the symbol from exactly one owner object, that owner is uniquely one
compiled unit whose own copy passes, and every failing copy is a COMDAT the
linker therefore discarded, the failing copies are reported as discarded
rather than failing the binding. Unknown copies still refuse.
"""

from pathlib import PurePosixPath

from .bound_objects import FunctionBinding, check_objects
from .model import Result


def _link_selected_unit(symbol, objects, link_observation):
    """The compiled unit whose COMDAT copy the observed link placed, or None."""
    if link_observation is None:
        return None
    rows = link_observation.publics.get(symbol, ())
    if len(rows) != 1 or not rows[0].function or not rows[0].segment or "(" in rows[0].owner:
        return None
    owner = rows[0].owner.casefold()
    if link_observation.explicit_owner_counts.get(owner) != 1:
        return None
    units = [unit for unit in objects if (PurePosixPath(unit).name + ".obj").casefold() == owner]
    return units[0] if len(units) == 1 else None


def _comdat_copy(obj, symbol):
    definitions = [row for row in obj.symbols.values()
                   if row.name == symbol and row.storage == 2 and row.section > 0]
    return (len(definitions) == 1 and 1 <= definitions[0].section <= len(obj.sections)
            and bool(obj.sections[definitions[0].section - 1].flags & 0x1000))


def check_shared(bindings, objects, image, external_targets, *, import_targets=None, literal_targets=None, errors=None,
                 merged_literal_names=(), link_observation=None):
    symbols = {item.symbol for item in bindings}
    if len(symbols) != len(bindings) or symbols & (external_targets.keys() | (import_targets or {}).keys()):
        raise ValueError("conflicting shared function symbol bindings")
    targets, rows = {}, {}
    pending = list(bindings)
    while pending:
        proven = []
        round_rows = {}
        for binding in pending:
            carrier = binding.object_unit
            base = {"interpretation_id": binding.interpretation_id, "entity_id": binding.origin.entity_id,
                    "evidence_entity_id": binding.evidence.entity_id, "symbol": binding.symbol,
                    "interpretation_kind": binding.interpretation["kind"],
                    "address": f"0x{binding.origin.address:08X}",
                    "source": {"path": binding.unit, "line": binding.line}}
            copies, copy_proofs = [], []
            for unit, obj in objects.items():
                definitions = [symbol for symbol in obj.symbols.values()
                               if symbol.name == binding.symbol and symbol.storage == 2 and symbol.section > 0]
                if definitions:
                    copies.append(unit)
            for unit in sorted(copies):
                try:
                    proof, = check_objects(
                        [FunctionBinding(binding.origin, (binding.span,), binding.symbol, unit)],
                        objects, image, external_targets={**external_targets, **targets},
                        import_targets=import_targets, literal_targets=literal_targets, errors=errors,
                        merged_literal_names=merged_literal_names)
                    copy_proofs.append({key: value for key, value in proof.items()
                                        if key not in {"interpretation_id", "entity_id", "symbol"}})
                except ValueError as error:
                    copy_proofs.append({"unit": unit, "result": Result.UNKNOWN, "reason": str(error)})
            copy_by_unit = {row["unit"]: row for row in copy_proofs}
            carrier_proof = copy_by_unit.get(carrier) if carrier is not None else None
            source_reasons = [reason for reason in (binding.body_error, binding.context_error) if reason]
            if carrier is None and not source_reasons:
                source_reasons.append("shared definition has no resolved compiler context")
            if carrier is not None and carrier_proof is None:
                source_reasons.append("resolved shared compiler context has no observed external definition")
            results = [row["result"] for row in copy_proofs]
            selected_unit = (_link_selected_unit(binding.symbol, objects, link_observation)
                             if Result.FAIL in results else None)
            if (selected_unit is not None
                    and copy_by_unit.get(selected_unit, {}).get("result") == Result.PASS
                    and (carrier is None or copy_by_unit.get(carrier, {}).get("result") == Result.PASS)
                    and all(_comdat_copy(objects[row["unit"]], binding.symbol)
                            for row in copy_proofs if row["result"] == Result.FAIL)):
                for row in copy_proofs:
                    if row["result"] == Result.FAIL:
                        row["observed_link"] = {"selected_unit": selected_unit,
                                                "disposition": "discarded_comdat_copy",
                                                "evidence": dict(link_observation.evidence)}
                results = [Result.PASS if row.get("observed_link") else row["result"] for row in copy_proofs]
            if any(result == Result.FAIL for result in results):
                result, reason = Result.FAIL, "one or more observed external definitions failed"
            elif source_reasons or not results or any(item != Result.PASS for item in results):
                result = Result.UNKNOWN
                reason = "; ".join(source_reasons) or "one or more observed external definitions remain unproved"
            elif any(row.get("observed_link") for row in copy_proofs):
                result, reason = Result.PASS, ("the observed link's selected copy and the evidence carrier passed; "
                                               "differing COMDAT copies were discarded by that link")
            else:
                result, reason = Result.PASS, "all observed external definitions and the evidence carrier passed"
            aggregate = {**base, "unit": carrier or binding.unit, "compiler_context": carrier,
                         "result": result, "reason": reason, "copies": copy_proofs,
                         "observed_external_copies": len(copy_proofs)}
            for dimension in ("boundary", "bytes", "references"):
                values = [row.get(dimension, Result.UNKNOWN) for row in copy_proofs
                          if not row.get("observed_link")]
                if source_reasons or carrier is not None and carrier_proof is None or not values:
                    values.append(Result.UNKNOWN)
                aggregate[dimension] = (Result.FAIL if Result.FAIL in values else Result.UNKNOWN
                                        if Result.UNKNOWN in values else Result.PASS)
            round_rows[binding.symbol] = [aggregate]
            if result == Result.PASS:
                proven.append(binding)
        rows.update(round_rows)
        if not proven:
            break
        # Publish only after this whole round. Mutual unproved references remain
        # unknown; no convenient ordering or circular address assumptions.
        for binding in proven:
            targets[binding.symbol] = binding.origin.address
            pending.remove(binding)
    return targets, [rows[binding.symbol][0] for binding in bindings]
