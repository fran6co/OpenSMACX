"""The pinned sound hook is an out-of-line zero-returning function."""

from pathlib import Path

from match_recovery.source import read_file, read_text, write_body


ROOT = Path(__file__).resolve().parents[2]
ADDRESS = 0x5FD2B0


def test_do_sound_has_one_real_out_of_line_definition_and_retained_binding():
    cpp_path = ROOT / "src/autosound.cpp"
    header_path = ROOT / "src/autosound.h"
    cpp = cpp_path.read_text()
    header = header_path.read_text()

    record, = (item for item in read_file(cpp_path) if item.address == ADDRESS)
    assert record.name == "?do_sound@@YAHXZ"
    assert record.image_spans == ((ADDRESS, ADDRESS + 3),)
    assert record.byte_exact
    assert record.body == "src/autosound.cpp"
    assert cpp.count("int __cdecl do_sound() {") == 1
    assert "int __cdecl do_sound();" in header
    assert "MEASURED inline int __cdecl do_sound()" not in header
    assert "int __cdecl do_sound() {\n    return 0;\n}" in cpp


def test_body_path_transition_uses_guarded_native_source_writer():
    cpp_path = ROOT / "src/autosound.cpp"
    current = cpp_path.read_bytes()
    current_record, = (item for item in read_file(cpp_path) if item.address == ADDRESS)
    assert current_record.body == "src/autosound.cpp"

    old = current.replace(b"// body      src/autosound.cpp",
                          b"// body      src/autosound.h", 1)
    assert old != current
    old_record, = (item for item in read_text(
        old.decode("utf-8"), current_record.path) if item.address == ADDRESS)
    rewritten = write_body(old, old_record, "src/autosound.cpp")
    assert rewritten == current


def test_original_direct_call_sites_do_not_use_sound_forwarders():
    win = (ROOT / "src/win.cpp").read_text()
    net = (ROOT / "src/net_class.cpp").read_text()
    assert "redraw_sound_call" not in win
    assert "static int __cdecl voice_sound_call()" not in net
    assert "voice_sound_call();" not in net
    assert "do_sound();" in win
    assert "do_sound();" in net
