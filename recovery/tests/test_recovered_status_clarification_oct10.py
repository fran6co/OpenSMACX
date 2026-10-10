"""Classification is not a body recovery or a fresh matching claim."""
from dataclasses import fields
from pathlib import Path
import subprocess

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_text, write
from test_netwin_loaded_path_member_oct10 import restore_loaded_path_member_migration

ROOT = Path(__file__).resolve().parents[2]
BASE = "71569c1962d1d0438cf97a0db7e6da5a883beb2e"


def cpp_without_comments(source):
    # Keep literals intact: changing strings is not a comment-only migration.
    return COMMENTS_OR_STRINGS.sub(
        lambda m: " " if m[0].startswith(("//", "/*")) else m[0], source).split()


def test_only_comments_and_one_role_change_without_losing_original_evidence():
    for relative in ("src/game.cpp", "src/movie_media.cpp"):
        path = ROOT / relative
        current = path.read_text()
        before = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT).decode()
        # Later verified typed-path callers change spelling, not this status step.
        assert cpp_without_comments(restore_loaded_path_member_migration(current)) == cpp_without_comments(before)
        prior = {r.address: r for r in read_text(before, path)}
        rows = read_text(current, path)
        assert {r.address for r in rows} == set(prior)
        assert write(current, rows) == current
        for row in rows:
            exceptions = {"line", "region", "origin"}
            if relative == "src/game.cpp" and row.address == 0x524370:
                assert row.source_role == "scaffold"
                assert prior[row.address].source_role == "unclassified"
                exceptions |= {"source_role", "source_role_reason"}
            for field in fields(row):
                if field.name not in exceptions:
                    assert getattr(row, field.name) == getattr(prior[row.address], field.name)
            def lessons(r):
                return [(x.kind,x.key,x.prose,tuple(t for _,t in x.lines)) for x in r.origin]
            assert lessons(row) == lessons(prior[row.address])


def test_movie_current_status_names_real_implemented_owners_not_acceptance():
    movie = (ROOT / "src/movie_media.cpp").read_text()
    decoder = (ROOT / "src/movie_media_decoder.cpp").read_text()
    frame = (ROOT / "src/movie_decoder_frame.cpp").read_text()
    assert 'extern "C" void __cdecl sub_5cdfa0(void *argument) {' in movie
    assert "int __fastcall sub_5d35b0(MediaRuntime *runtime," in decoder
    assert "movie_decoder_frame::decode_frame(args)" in decoder
    assert "int decode_frame(" in frame
    assert "5D35B0 and its separate downstream renderer remain unfinished" not in movie
    assert "native codec equivalence or working playback" in movie
