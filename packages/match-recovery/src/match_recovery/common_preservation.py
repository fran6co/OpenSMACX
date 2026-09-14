"""Cross-build common storage observations, not generated-effect permissions.

An extra uniform COFF request can leave the selected common allocation unchanged.
That fact says nothing about writes, initialization ordering, callbacks or other
linked data. Native consumers must separately bind any use to a live task/trial.
"""

from .common_allocation import verify_common_allocations
from .linked_initializers import _artifacts
from .model import Result


def _result(values):
    return (Result.FAIL if Result.FAIL in values else Result.UNKNOWN
            if any(value != Result.PASS for value in values) else Result.PASS)


def _symbols(symbols):
    if (not isinstance(symbols, (tuple, list)) or not 0 < len(symbols) <= 4096
            or any(not isinstance(name, str) or not name or
                   any(ord(c) < 33 or ord(c) >= 127 for c in name) for name in symbols)
            or len(set(symbols)) != len(symbols)):
        raise ValueError("common comparison requires 1..4096 unique nonempty symbol names")
    return tuple(symbols)


def _comparison(observations):
    """Pure private algebra; neither supplied observations nor PASS grant scope."""
    rows = []
    for old, new in zip(observations["baseline"]["allocations"],
                        observations["current"]["allocations"], strict=True):
        if old["symbol"] != new["symbol"]:
            raise ValueError("common comparison identities differ")
        result = _result([old["result"], new["result"]])
        row = {"symbol": old["symbol"], "result": result}
        if result == Result.PASS:
            # Actual addresses may move. Both complete native observations
            # must independently prove the same bounded zero allocation.
            if any(type(item["size"]) is not int or item["size"] <= 0
                   or type(item["address"]) is not int or item["address"] < 0
                   or item["initialization"] != Result.PASS for item in (old, new)):
                raise ValueError("common comparison requires complete typed allocation observations")
            same = old["size"] == new["size"]
            row.update(result=Result.PASS if same else Result.FAIL,
                       baseline_address=old["address"], current_address=new["address"],
                       baseline_size=old["size"], current_size=new["size"],
                       initialization=Result.PASS,
                       basis="independent_uniform_selected_common_allocations")
            if not same:
                row["reason"] = "selected common extent changed"
        else:
            row["reason"] = "both native common allocation observations must pass"
        rows.append(row)
    return {**observations, "allocations": rows, "allocation_isolation": Result.UNKNOWN,
            "remaining_linked_effects": Result.UNKNOWN, "ready_for_collection": False,
            "result": _result([observations["baseline"]["result"], observations["current"]["result"],
                               *[row["result"] for row in rows]]) if rows else Result.UNKNOWN}


def _compare_linked(baseline, current, symbols):
    """Native caller keeps both parsed link epochs live throughout consumption."""
    def fresh():
        try:
            baseline.check_current()
        finally:
            current.check_current()
    try:
        fresh()
        names = _symbols(symbols)
        return _comparison({"baseline": baseline.common(names), "current": current.common(names)})
    finally:
        fresh()


def compare_common_builds(baseline, current, symbols):
    """Observe exact requested identities in two still-live application builds.

    No saved report, numerical address correspondence or neighbor-derived extent
    is accepted as an input. The enclosing native caller owns task authority;
    this result alone cannot authorize residual projection or collection.
    """
    def fresh():
        try:
            baseline.check_current()
        finally:
            current.check_current()

    report = {"check": "application_common_preservation", "result": Result.UNKNOWN,
              "allocations": [], "remaining_linked_effects": Result.UNKNOWN,
              "allocation_isolation": Result.UNKNOWN,
              "original_verification": Result.UNKNOWN, "runtime_verification": Result.UNKNOWN,
              "persisted": False, "ready_for_collection": False,
              "limitations": ["requested common identities, uniform extent and pre-CRT zero storage only",
                              "earlier overlapping allocations and private aliases require separate isolation proof",
                              "no exemption for guard writes, helpers, directives, imports or other linked data",
                              "no generated-change permission, trial coverage or original source binding"]}
    try:
        fresh()
        names = _symbols(symbols)
        observations = {}
        for version, context in (("baseline", baseline), ("current", current)):
            fresh()
            artifacts = _artifacts(context)
            observations[version] = verify_common_allocations(
                artifacts["link-input-files"], artifacts["msvc-link-log"],
                artifacts["pe32-i386"], artifacts["msvc-map"], names)
        report.update(_comparison(observations))
    except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
        report.update(result=Result.UNKNOWN, reason=str(error))
    finally:
        # A refusal is still subject to both original source/compiler epochs.
        # Changed artifacts cannot be converted to an ordinary unknown result.
        fresh()
    return report
