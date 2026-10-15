from match_recovery.source.reader import function_line


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
