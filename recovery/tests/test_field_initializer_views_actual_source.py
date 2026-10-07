"""Actual-source controls for the migrated 0x4C8090 and 0x4C8100 methods."""
from pathlib import Path
import shutil

import pytest
from match_recovery.source import read_file
from test_word_state_actual_source import (
    ROOT, SOURCE, assert_nine_store_order, compile_run, method_body,
)


def test_migrated_methods_keep_source_roles_order_and_complete_spans():
    source = SOURCE.read_text(encoding="utf-8")
    rows = {row.address: row for row in read_file(SOURCE)}
    for address in (0x4C8090, 0x4C8100):
        assert rows[address].source_role == "scaffold"
        assert rows[address].byte_exact
    first = method_body(source, "NineWordSentinelState *NineWordSentinelState::reset()")
    assert_nine_store_order(first)
    second = method_body(source, "PairWordState *PairWordState::reset_tail()")
    assert second.index("word_04_ = 0;") < second.index("word_08_ = 0;")
    assert second.index("word_08_ = 0;") < second.index("return this;")
    assert "// ORIGINAL: 0x004C8090 sub_4c8090 0x004C8090-0x004C80B3 BYTE_EXACT" in source
    assert "// ORIGINAL: 0x004C8100 sub_4c8100 0x004C8100-0x004C810B BYTE_EXACT" in source


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "last_constant", "omit_store", "clear_prefix"))
def test_actual_migrated_initializer_methods(compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    if damage == "last_constant":
        source = source.replace("word_20_ = 0xFFFFFFFFU;", "word_20_ = 0U;", 1)
    elif damage == "omit_store":
        source = source.replace("word_14_ = 0;", "", 1)
    elif damage == "clear_prefix":
        source = source.replace(
            "PairWordState *PairWordState::reset_tail() {\n    word_04_ = 0;",
            "PairWordState *PairWordState::reset_tail() {\n    word_00_ = 0;", 1)
    compile_run(compiler, source, expected=damage is None)
