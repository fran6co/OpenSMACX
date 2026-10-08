"""Source-owner and configuration checks for final pending-TU retirement.

These controls verify ownership continuity and removed call seams. They do not
establish native CRT selection, exception-unwind equivalence, byte matching,
or complete game behavior.
"""
import re
from pathlib import Path

from match_recovery.source import Mode, read_file
from owner_transfer_test_support import assert_current_owner, assert_leaf_collection_retired

ROOT = Path(__file__).resolve().parents[2]


def _definition(source, signature):
    match = re.search(signature + r"\s*\{", source)
    assert match, "missing source definition: " + signature
    opening = source.index("{", match.start())
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[match.start():end + 1]
    raise AssertionError("unterminated source definition: " + signature)


def test_pending_translation_unit_is_retired_only_after_owners_are_present():
    assert not (ROOT / "src/pending_bodies.cpp").exists()

    assert_leaf_collection_retired(ROOT)
    assert_current_owner(ROOT, 0x006456E4, "src/vendor/vc6-runtime.recovery.h")
    assert_current_owner(ROOT, 0x006457C2, "src/vendor/vc6-runtime.recovery.h")
    runtime = {row.address: row for row in read_file(ROOT / "src/vendor/vc6-runtime.recovery.h")}
    for address, name, spans in (
        (0x006456E4, "??_M@YGXPAXIHP6EX0@Z@Z", ((0x006456E4, 0x00645764),)),
        (0x006457C2, "??_L@YGXPAXIHP6EX0@Z1@Z", ((0x006457C2, 0x00645844),)),
    ):
        row = runtime[address]
        assert row.mode is Mode.FILE and row.name == name
        assert row.image_spans == spans and not row.byte_exact and not row.semantic
        assert not row.body and not row.symbol and row.kind == "library"
    assert "MISMATCH #12" in runtime[0x006456E4].ruled_out[0]
    runtime_text = (ROOT / "src/vendor/vc6-runtime.recovery.h").read_text()
    assert runtime_text.count("Historical project-loop location: git:4ba53acef56137f1aa04151f648ea360681ee511:src/vector_teardown.cpp") == 2
    assert not (ROOT / "src/array_initialization.cpp").exists()
    assert not (ROOT / "src/array_initialization.h").exists()
    netwin = {row.address: row for row in read_file(ROOT / "src/netwin.cpp")}
    assert netwin[0x00402EE0].mode is Mode.FILE
    assert netwin[0x00402EE0].symbol == "??_H@YGXPAXIHP6EX0@Z@Z"
    assert netwin[0x00402EE0].byte_exact

    group_rows = {row.address: row for row in read_file(ROOT / "src/wave_device.cpp")}
    for address, span in ((0x004C5490, (0x004C5490, 0x004C54AB)),
                         (0x004C5B80, (0x004C5B80, 0x004C5BEF))):
        row = group_rows[address]
        assert row.mode is Mode.BODY and row.image_spans == (span,)
        assert not row.symbol and row.kind == "game"
        assert not row.byte_exact and not row.semantic
    wave = [row for row in read_file(ROOT / "src/wave_device.cpp")
            if row.address == 0x004C5BF0]
    assert len(wave) == 1 and wave[0].mode is Mode.BODY
    row, = wave
    assert row.name == "sub_4c5bf0"
    assert row.symbol == "?insert@WaveGroupList@@QAEXPAVWave@@@Z"
    assert row.image_spans == ((0x004C5BF0, 0x004C5C4A),)
    assert row.byte_exact and not row.semantic and row.kind == "game"
    assert row.levers and row.levers[0][0].startswith("`WaveGroupNode")
    wave_source = (ROOT / "src/wave_device.cpp").read_text()
    append = _definition(wave_source, r"void WaveGroupList::insert\(Wave \*a1\)")
    assert "wave_group_insert_redirect" not in wave_source
    assert "WaveOperatorNew" not in wave_source
    allocation = append.index("operator new(0xC)")
    initialize = append.index("node->prev = nullptr;")
    tail_test = append.index("if (tail)")
    old_next = append.index("tail->next")
    reread = append.index("this->tail", old_next)
    assert allocation < initialize < tail_test < old_next < reread
    assert "else {" in append and "node = 0;" in append

    jackal_source = (ROOT / "src/jackal.cpp").read_text()
    jackal = [row for row in read_file(ROOT / "src/jackal.cpp")
              if row.address == 0x0062D500]
    assert len(jackal) == 1 and jackal[0].mode is Mode.BODY
    close = _definition(jackal_source, r"void __cdecl jackal_close\(\)")
    assert "FileWin::close_class();" in close
    assert "BasePop::close_class();" in close
    assert "filewin_close_class" not in jackal_source and "basepop_close_class" not in jackal_source
    general_header = (ROOT / "src/general.h").read_text()
    assert "filewin_close_class" not in general_header
    assert "basepop_close_class" not in general_header
    assert "static void __cdecl close_class();" in (ROOT / "src/filewin.h").read_text()
    filewin = [row for row in read_file(ROOT / "src/filewin.cpp")
               if row.address == 0x00614E30]
    assert len(filewin) == 1
    row, = filewin
    assert row.name == "?close_class@FileWin@@QAAXXZ"
    assert row.symbol == "?close_class@FileWin@@SAXXZ"
    assert row.image_spans == ((0x00614E30, 0x00614E4D),)
    assert row.byte_exact and not row.semantic and row.kind == "game"

    for rel in ("src/wave.cpp", "src/sound.cpp", "src/wave_device.cpp"):
        text = (ROOT / rel).read_text()
        assert "WaveOperatorNew" not in text
        assert "operator new(" in text


def test_no_build_audit_or_project_macro_still_names_the_retired_unit():
    cmake = (ROOT / "CMakeLists.txt").read_text()
    project = (ROOT / "recovery/project.json").read_text()
    audits = (ROOT / "recovery/audits.json").read_text()
    assert "src/pending_bodies.cpp" not in cmake
    assert '"forwarder_macros": []' in project
    assert "PENDING_BODY" not in project and "PENDING_BODY" not in audits
