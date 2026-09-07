"""Every class in a batch needs both complete controls; objects are synthetic."""

import re

import pytest

from match_recovery import layout_witness


@pytest.mark.parametrize("damage", [None, "missing_second_class", "wrong_second_width", "unchanged_second_body", "second_compile"])
def test_batch_never_returns_partial_or_borrowed_control_results(monkeypatch, damage):
    calls, indexes, lookups, signatures = [], [], [], []
    names = ["First", "Second"]
    probes = [(name, "match_recovery_" + name, "") for name in names]

    def compile(contents):
        assert contents.startswith(b"original source\n")
        width = int(re.search(rb"_sentinel\[(\d+)\]", contents)[1])
        calls.append(width)
        assert all(("struct " + name).encode() in contents for name in names)
        if damage == "second_compile" and width == 29:
            raise ValueError("second complete compilation unavailable")
        return width

    class Index:
        def __init__(self, width):
            self.width = width
            indexes.append(width)

        def layout(self, name):
            lookups.append((name, self.width))
            if damage == "missing_second_class" and name == "Second" and self.width == 29:
                raise ValueError("one definition missing")
            member = "match_recovery_" + name + "_sentinel"
            return {"size": 17 if damage == "wrong_second_width" and name == "Second" else self.width,
                    "members": {member: 0}, "bases": [], "member_type_kinds": {member: "array"}}

    def signature(width, symbol):
        signatures.append((symbol, width))
        return (symbol, 17 if damage == "unchanged_second_body" and symbol.endswith("Second") else width)

    monkeypatch.setattr(layout_witness, "ClassLayouts", Index)
    monkeypatch.setattr(layout_witness, "function_signature", signature)
    if damage:
        with pytest.raises(ValueError):
            layout_witness._absence_witnesses(compile, b"original source\n", probes)
    else:
        rows = layout_witness._absence_witnesses(compile, b"original source\n", probes)
        assert all(layout_witness.declaration_absent(row) for row in rows)
        assert [row["probe_symbol"] for row in rows] == ["_match_recovery_" + name for name in names]
        assert lookups == [(name, width) for width in (17, 29) for name in names]
        assert signatures == [("_match_recovery_" + name, width) for width in (17, 29) for name in names]
    assert calls == [17, 29]
    assert indexes == ([17] if damage == "second_compile" else [17, 29])
