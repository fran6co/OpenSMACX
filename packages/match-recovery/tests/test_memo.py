from match_recovery.memo import memoized
from match_recovery.model import digest


def test_memo_hit_corruption_and_deletion(tmp_path):
    calls = []

    def compute():
        calls.append(True)
        return {"parsed": [1, "two"]}

    key = digest("input")
    assert memoized(tmp_path, key, compute) == compute()
    calls.clear()
    assert memoized(tmp_path, key, compute) == {"parsed": [1, "two"]}
    assert not calls
    path = next(tmp_path.rglob("*.json"))
    path.write_text(path.read_text().replace('"two"', '"bad"'))
    assert memoized(tmp_path, key, compute) == {"parsed": [1, "two"]}
    assert len(calls) == 1
    path.unlink()
    memoized(tmp_path, key, compute)
    assert len(calls) == 2


def test_unavailable_and_disabled_cache_recompute(tmp_path):
    root = tmp_path / "not-a-directory"
    root.write_text("occupied")
    for cache in (root, None):
        assert memoized(cache, digest("input"), lambda: {"ok": True}) == {"ok": True}
