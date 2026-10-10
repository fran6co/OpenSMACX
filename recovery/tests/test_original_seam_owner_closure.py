"""Lexical caller/annotation controls only; not original behavior or native ABI proof."""

from pathlib import Path


ROOT = next(parent for parent in Path(__file__).resolve().parents if (parent / "src").is_dir())


def test_sound_internal_slots_use_real_virtual_methods_and_keep_external_slots():
    source = (ROOT / "src/sound.cpp").read_text()
    assert "if (set_fade(a1) == 0)" in source
    assert "if (set_fade_in(a1) == 0)" in source
    assert "            attach();" in source
    assert "    detach();" in source
    assert "vtable_method<device_load_fn>(device_, 0x10)" in source
    assert "original_slot<device_loop_fn>" in source
    header = (ROOT / "src/sound.h").read_text()
    for signature in (
        "virtual int set_fade(unsigned long fade)",
        "virtual int fade()",
        "virtual int set_fade_in(unsigned int fade_in)",
        "virtual int attach()",
        "virtual int detach()",
    ):
        assert signature in header


def test_wave_device_uses_owned_slots_but_keeps_external_device_dispatch():
    source = (ROOT / "src/wave_device.cpp").read_text()
    body = source.split("int Wave_Device::init(void *group_id, unsigned long flags) {", 1)[1]
    body = body.split("\n}\n", 1)[0]
    assert "const int staged = create_device(flags);" in body
    assert "device_14_" in body
    assert "original_slot<device_init_fn>" not in body
    assert "delete_device();" in body
    assert "original_slot<device_init_fn>" in source or "device_init_fn" in source
    header = (ROOT / "src/wave_device.h").read_text()
    assert "virtual int create_device(unsigned long a1);   // slot 0" in header
    assert "virtual int delete_device();                   // slot 1" in header


def test_netwin_calls_typed_owner_methods_at_the_original_sites():
    source = (ROOT / "src/netwin.cpp").read_text()
    assert "#include \"original_seam.h\"" not in source
    assert "flatButton1_.close();" in source
    assert "flatButton1_.redraw();" in source
    assert "on_redraw();" in source
    assert "return editBox_.on_key_down(key);" in source
    assert "release_modal();" in source
    assert "vtable_slot<" not in source
    custom_rules = source.split("void NetWin::draw_custom_rules()", 1)[1]
    custom_rules = custom_rules.split("\n}\n", 1)[0]
    assert "strcat(StringTemp," in custom_rules
    assert "CharUpperA(StringTemp);" in custom_rules
    header = (ROOT / "src/netwin.h").read_text()
    assert "FlatButton flatButton1_" in header
    assert "EditBox editBox_" in header


def test_diplowin_redraw_and_listbox_slot54_use_real_virtual_apis():
    diplo = (ROOT / "src/diplowin.cpp").read_text()
    assert "func_diplo_win_slot062" not in diplo
    assert "vtable_slot<func_diplo_win_slot062>" not in diplo
    assert diplo.count("        redraw();") >= 3

    listbox = (ROOT / "src/listbox.cpp").read_text()
    assert "target->on_selected(" in listbox
    assert "slot054_fn" not in listbox
    # Keep the distinct unknown child receiver/slot contract rather than
    # incorrectly treating it as Win's two-coordinate slot 23.
    assert "Sound *child = target->legacy_sound_36();" in listbox
    assert "child->is_playing()" in listbox
    assert "child->play();" in listbox
    win = (ROOT / "src/win.h").read_text()
    assert "virtual void on_selected(int)" in win


def test_original_mapping_comments_survive_the_caller_closure():
    for filename, marker in (
        ("sound.cpp", "// ORIGINAL: 0x004C6600 ?fade@Sound@@QAEXK@Z"),
        ("sound.cpp", "// ORIGINAL: 0x004C6280 ?load@Sound@@QAEHPBD@Z"),
        ("sound.cpp", "// ORIGINAL: 0x004C6660 ?fade_in@Sound@@QAEXI@Z"),
        ("sound.cpp", "// ORIGINAL: 0x004C6440 ?unload@Sound@@QAEHXZ"),
        ("wave_device.cpp", "// ORIGINAL: 0x004C4F40 ?init@Wave_Device@@QAEHPAXK@Z"),
        ("netwin.cpp", "// ORIGINAL: 0x00480370 ?draw_player_names@NetWin@@QAEXXZ"),
        ("netwin.cpp", "// ORIGINAL: 0x0047F960 ?draw_custom_rules@NetWin@@QAEXXZ"),
        ("diplowin.cpp", "// ORIGINAL: 0x004413F0 ?kill_offer@DiploWin@@QAEXHH@Z"),
        ("listbox.cpp", "// ORIGINAL: 0x0060D030 sub_60d030"),
    ):
        assert marker in (ROOT / "src" / filename).read_text()
