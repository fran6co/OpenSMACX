from match_recovery.source.reader import function_line
import pytest


def test_large_translation_unit_text_is_not_treated_as_a_filename():
    text = "// source context\n" * 10000 + "void target() { }\n"
    assert function_line(text, "target") == 10001


def test_large_single_line_source_is_not_treated_as_a_filename():
    text = " " * 10000 + "void target() { }"
    assert function_line(text, "target") == 1


def test_existing_path_string_still_reads_the_file(tmp_path):
    path = tmp_path / "unit.cpp"
    path.write_text("// context\nvoid target() { }\n")
    assert function_line(str(path), "target") == 2


def test_bounded_overload_lookup_keeps_full_file_lexical_state():
    text = ('/* misleading definition: void Owner::fill() { } */\n'
            'void Owner::fill(int) { }\n'
            '/* a comment begins before the mapped marker\n'
            '// ORIGINAL: 0x00401000 _fill 0x00401000-0x00401001\n'
            'Owner::fill() { const char *s = R"tag({ })tag"; }\n'
            '*/\n'
            'void Owner::fill(double) { const char *s = R"tag({ })tag"; }\n'
            'void Owner::fill(char) { }\n')
    assert function_line(text, "Owner::fill", after_line=6, before_line=8) == 7
    with pytest.raises(ValueError, match="3 definitions"):
        function_line(text, "Owner::fill")
    with pytest.raises(ValueError, match="0 definitions"):
        function_line(text, "Owner::fill", after_line=3, before_line=6)


def test_bounded_lookup_refuses_duplicate_definitions_and_invalid_bounds():
    text = "void Owner::fill(int) {}\nvoid Owner::fill(double) {}\n"
    with pytest.raises(ValueError, match="2 definitions"):
        function_line(text, "Owner::fill", after_line=0, before_line=3)
    with pytest.raises(ValueError, match="2 definitions"):
        function_line("void Owner::fill(int) {} void Owner::fill(double) {}",
                      "Owner::fill", after_line=0, before_line=2)
    with pytest.raises(ValueError, match="line bounds"):
        function_line(text, "Owner::fill", after_line=1, before_line=2)
