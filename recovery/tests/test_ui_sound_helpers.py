"""Actual-source helper controls; neither matching nor full-game acceptance."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from recovery.tests.test_pact_ascent import ROOT, definition


def product_control():
    fixture = Path(__file__).with_name("ui_sound_helpers_control.cpp").read_text()
    datalink = (ROOT / "src/datalink.cpp").read_text()
    sound = (ROOT / "src/sound.cpp").read_text()
    blocks = {
        "// @@CALLBACK@@": definition(datalink, r"void __cdecl discard_technology_help_text\([^)]*\)"),
        "// @@HELP_WRAPPER@@": definition(datalink, r"void __cdecl technology_help_without_output\([^)]*\)"),
        "// @@SOUND_CHAIN@@": definition(sound, r"int __cdecl unload_active_sound_chain\(\)"),
    }
    for token, block in blocks.items():
        assert fixture.count(token) == 1
        assert "BODY GOES HERE" not in block
        fixture = fixture.replace(token, block)
    return fixture


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_typed_callback_and_virtual_chain_reload(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    source = product_control()
    variants = {
        "actual": source,
        "swapped_help_arguments": source.replace("technology, faction, -1);", "faction, technology, -1);", 1),
        "wrong_link_mode": source.replace("technology, faction, -1);", "technology, faction, 0);", 1),
        "wrong_callback_identity": source.replace("help_tech_info(discard_technology_help_text,", "help_tech_info(+[](char *) {},", 1),
        "one_node_only": source.replace("while (SoundChainHead != 0)", "if (SoundChainHead != 0)", 1),
        "cached_next_instead_of_live_head": source.replace("SoundChainHead->unload();", "Sound *old_next = SoundChainHead->next_; SoundChainHead->unload(); SoundChainHead = old_next;", 1),
        "wrong_return": source.replace("return 0x0B;", "return 0;", 1),
    }
    # A distinct C++98 callback supplies the callback-identity damage control.
    variants["wrong_callback_identity"] = variants["wrong_callback_identity"].replace(
        "+[](char *) {}", "wrong_callback").replace(
        "void __cdecl discard_technology_help_text", "void wrong_callback(char *) {}\nvoid __cdecl discard_technology_help_text", 1)
    with tempfile.TemporaryDirectory(prefix="ui-sound-actual-source-") as temp:
        directory = Path(temp)
        for name, text in variants.items():
            if name != "actual":
                assert text != source, name
            cpp, executable = directory / (name + ".cpp"), directory / name
            cpp.write_text(text)
            build = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)],
                                   capture_output=True, text=True, timeout=45)
            assert build.returncode == 0, build.stdout + build.stderr
            run = subprocess.run([str(executable)], capture_output=True, text=True, timeout=15)
            assert (run.returncode == 0) == (name == "actual"), name + run.stdout + run.stderr
