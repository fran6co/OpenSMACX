"""Display-only differences between a saved check-matches report and a new one.

The comparison never changes a verdict or the new report's result; it only
says which rows moved. Rows are keyed by (address, path); in a scoped report
only rows the new report measured are compared.
"""


def _key(row):
    return row["address"], row["path"]


def _claimed(row):
    return bool(row.get("byte_obligation") or row.get("shape_obligation"))


def _brief(row):
    return {key: row.get(key) for key in ("address", "path", "name", "verdict", "result",
                                          "matching_instructions", "original_instructions")}


def compare_match_reports(baseline: dict, current: dict) -> dict:
    """Return newly regressed/restored claims, free/lost-free rows and verdict changes."""
    before = {_key(row): row for row in baseline.get("measurements", [])}
    after = {_key(row): row for row in current.get("measurements", [])}
    newly_regressed, restored, newly_free, lost_free, changed, appeared = [], [], [], [], [], []
    for key, row in sorted(after.items()):
        old = before.get(key)
        if old is None:
            appeared.append(_brief(row))
            continue
        if _claimed(row):
            if row.get("result") == "fail" and old.get("result") != "fail":
                newly_regressed.append(_brief(row))
            elif old.get("result") == "fail" and row.get("result") == "pass":
                restored.append(_brief(row))
        else:
            if row.get("verdict") == "BYTE_EXACT" and old.get("verdict") != "BYTE_EXACT":
                newly_free.append(_brief(row))
            elif old.get("verdict") == "BYTE_EXACT" and row.get("verdict") != "BYTE_EXACT":
                lost_free.append(_brief(row))
        if (row.get("verdict") != old.get("verdict")
                or row.get("matching_instructions") != old.get("matching_instructions")):
            changed.append({**_brief(row), "before": {"verdict": old.get("verdict"),
                                                       "matching_instructions": old.get("matching_instructions")}})
    scoped = current.get("scope") is not None
    disappeared = [] if scoped else [_brief(row) for key, row in sorted(before.items()) if key not in after]
    return {"baseline_fingerprint": baseline.get("input_fingerprint"),
            "newly_regressed": newly_regressed, "restored": restored,
            "newly_free": newly_free, "lost_free": lost_free,
            "changed": changed, "appeared": appeared, "disappeared": disappeared,
            "compared_rows": sum(1 for key in after if key in before)}
