"""Scoped native-equality source and historical-identity guard."""

from pathlib import Path

from match_recovery.source import SourceRole, read_text


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "matrix3.cpp"
SCAFFOLD_REASON = (
    "Bounded numerical receiver view only; shared original object ownership, "
    "containing class, full extent, callers and lifetime remain unproved. "
    "Source consolidation and complete arithmetic bodies do not resolve this "
    "structural obligation."
)
EXPECTED_BODY = """bool Matrix3::equals(const float *other) {
    const float *mine = cells;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            // The pinned VC6 FCOMP lowering accepts unordered NaN pairs with
            // masked exceptions. Standard host C++ != rejects them instead.
            // Signaling-NaN, unmasked exceptions and x87 status remain unknown.
            if (mine[row * 3 + col] != other[row * 3 + col]) {
                return false;
            }
        }
    }
    return true;
}"""


def test_native_equals_body_and_original_identity_are_retained() -> None:
    text = SOURCE.read_text()
    assert text.count(EXPECTED_BODY) == 1

    rows = read_text(text, SOURCE.resolve())
    assert len(rows) == 12
    assert len({row.address for row in rows}) == 12
    row = next(row for row in rows if row.address == 0x00634920)
    assert row.name == "sub_634920"
    assert row.image_spans == ((0x00634920, 0x00634975),)
    assert row.kind == ""
    assert row.byte_exact is True
    assert row.semantic is False
    assert row.symbol == "?equals@Matrix3@@QAE_NPBM@Z"
    assert row.source_role is SourceRole.SCAFFOLD
    assert row.source_role_reason == SCAFFOLD_REASON
    assert row.state.value == "implemented"
    assert row.mode.value == "body"

    # Preserve the pinned span, emitted-name hypothesis, role and historical
    # body claim; only the live C++ comparison and its explanatory note change.
    assert "// ORIGINAL: 0x00634920 sub_634920 0x00634920-0x00634975 BYTE_EXACT" in text
    assert "// symbol    ?equals@Matrix3@@QAE_NPBM@Z" in text
    assert "// RECOVERY_ROLE: SCAFFOLD " + SCAFFOLD_REASON in text
