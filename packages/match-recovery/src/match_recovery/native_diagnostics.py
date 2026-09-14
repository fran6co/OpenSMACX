"""Bounded disposable native diagnostics, never verification authority."""

import json


def diagnostic_evidence(report):
    """Bounded display only; never replace the report consumed by admission."""
    try:
        raw = json.dumps(report, ensure_ascii=False, sort_keys=True, allow_nan=False)
    except (TypeError, ValueError, RecursionError) as error:
        # Display failure cannot mint verification evidence or hide the original
        # admission result. Do not repr arbitrary caller objects or walk cycles.
        return ["Diagnostic unavailable (not verification evidence): " + json.dumps({
            "diagnostic_only": True, "reason": "report is not serializable JSON",
            "error": type(error).__name__, "report_type": type(report).__name__[:120]},
            ensure_ascii=False, sort_keys=True)]
    if len(raw) <= 12000:
        return [raw]
    outcome = report.get("result") if isinstance(report, dict) else None
    summary = {"diagnostic_only": True, "result": outcome if isinstance(outcome, str) and outcome in ("pass", "fail", "unknown") else "unknown",
               "result_counts": {}, "reasons": [], "counts": [],
               "omitted_reason_occurrences": 0, "omitted_count_rows": 0,
               "scan_truncated": False}
    reasons = {}
    # Prioritize semantic subtrees over large inventories. Iterate children
    # lazily so a large list cannot itself allocate an unbounded work queue.
    priority = {"application", "linked_output_preservation", "application_residual", "scoped_code", "graph", "code_dependencies",
                "storage_residual", "initializers", "gaps", "errors", "nodes"}

    def children(path, value):
        if isinstance(value, dict):
            for key in sorted(value, key=lambda k: (k not in priority, k)):
                if isinstance(value[key], (dict, list)):
                    yield path + "." + str(key), value[key]
        else:
            for index, child in enumerate(value):
                if isinstance(child, (dict, list)):
                    yield path + "[" + str(index) + "]", child

    pending = [iter((("$", report),))]
    visited = 0
    while pending:
        item = next(pending[-1], None)
        if item is None:
            pending.pop()
            continue
        path, value = item
        visited += 1
        if visited > 200000:
            summary["scan_truncated"] = True
            break
        if isinstance(value, dict):
            result = value.get("result")
            if isinstance(result, str) and result in ("pass", "fail", "unknown"):
                counts = summary["result_counts"]
                counts[result] = counts.get(result, 0) + 1
            reason = value.get("reason")
            if isinstance(reason, str) and reason:
                key = (result if isinstance(result, str) and result in ("pass", "fail", "unknown") else None, reason)
                if key in reasons:
                    reasons[key]["occurrences"] += 1
                else:
                    if len(reasons) >= 64 and result != "pass":
                        # A passing inventory must not consume all refusal slots.
                        discarded = next((k for k, row in reasons.items() if row["result"] == "pass"), None)
                        if discarded is not None:
                            row = reasons.pop(discarded)
                            summary["reasons"].remove(row)
                            summary["omitted_reason_occurrences"] += row["occurrences"]
                    if len(reasons) >= 64:
                        summary["omitted_reason_occurrences"] += 1
                    else:
                        row = {"path": path[:200], "reason": reason[:400],
                               "result": key[0], "occurrences": 1,
                               "text_truncated": len(path) > 200 or len(reason) > 400}
                        for field in ("symbol", "owner", "unit", "check"):
                            if isinstance(value.get(field), str):
                                row[field] = value[field][:120]
                        reasons[key] = row
                        summary["reasons"].append(row)
            if isinstance(value.get("counts"), dict):
                if len(summary["counts"]) < 12:
                    numeric = [(str(k), v) for k, v in value["counts"].items()
                               if type(v) is int]
                    summary["counts"].append({"path": path[:200],
                        "values": {k[:80]: v for k, v in numeric[:8]},
                        "omitted_fields": max(0, len(numeric) - 8)})
                else:
                    summary["omitted_count_rows"] += 1
            pending.append(children(path, value))
        elif isinstance(value, list):
            pending.append(children(path, value))
    # Keep refusals ahead of informational reasons when the summary fills up.
    summary["reasons"].sort(key=lambda row: row["result"] == "pass")
    while True:
        text = json.dumps(summary, ensure_ascii=False, sort_keys=True)
        if len(text) <= 6000:
            break
        if summary["counts"]:
            summary["counts"].pop()
            summary["omitted_count_rows"] += 1
        else:
            summary["omitted_reason_occurrences"] += summary["reasons"].pop()["occurrences"]
    return ["Nested diagnostic summary (not verification evidence): " + text,
            raw[:6000] + "\n[diagnostic truncated]\n" + raw[-6000:]]

