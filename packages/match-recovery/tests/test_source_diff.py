import pytest
import json
from pathlib import Path

from match_recovery.source_diff import generate_diff
from match_recovery.source_listing import listing_source_texts
from match_recovery.source_listing import listing_lines
from test_recording import project, compiler, calls
from source.test_local_asm import local_coff


def configure_listing(root, *, damage=False, effect=""):
    (root / 'recovery/diff.json').write_text(json.dumps({
        'schema_version': 1, 'format': 'msvc-assembly',
        'flags': ['listing', 'listing-path={listing}']}))
    listing = '_f PROC NEAR\n; 3 : void f() {}\n  00000\t' + ('cc' if damage else 'c3') + '\t ret\n_f ENDP\n'
    compiler(root, raw_object=local_coff(b'\xc3', name='_f'), effect=(
        "for flag in r['flags']:\n"
        "    if flag.startswith('listing-path='):\n"
        f"        Path(flag.split('=', 1)[1]).write_text({listing!r})\n" + effect))


def test_cli_fresh_view_and_regeneration_preserve_source(project, capsys):
    from match_recovery.cli import main
    configure_listing(project)
    before = (project / 'engine/family.cc').read_bytes()
    args = ['diff', '_f', '--root', str(project), '--image', 'original/input.exe']
    assert main(args) == 0
    report = json.loads(capsys.readouterr().out)
    output = Path(report['output'])
    assert output.is_file() and 'void f()' in output.read_text()
    assert report['comparison']['verdict'] == 'BYTE_EXACT'
    assert len(calls(project)) == 2
    assert main(args) == 0
    capsys.readouterr()
    assert len(calls(project)) == 4
    assert (project / 'engine/family.cc').read_bytes() == before


@pytest.mark.parametrize('damage,effect', [
    (True, ''),
    (False, "Path('engine/types.hpp').write_text('changed')"),
])
def test_invalid_or_changed_inputs_never_replace_existing_view(project, damage, effect):
    configure_listing(project, damage=damage, effect=effect)
    output = project / 'build/old.html'
    output.parent.mkdir(exist_ok=True)
    output.write_text('previous view')
    with pytest.raises(ValueError):
        generate_diff(project, '_f', image=Path('original/input.exe'), output=output)
    assert output.read_text() == 'previous view'


def test_listing_wraps_source_runs_and_checks_complete_machine_bytes():
    text = '''other PROC NEAR
  00000\tcc\t int 3
other ENDP
example PROC NEAR
; 12 : int example() {
  00000\t68 01 00\t
\t00 00\t push 1
; 14 : return;
  00005\tc3\t ret
example ENDP
'''
    assert listing_lines(text, "example", bytes.fromhex("68 01 00 00 00 c3")) == {0: 12, 5: 14}
    with pytest.raises(ValueError, match="do not match"):
        listing_lines(text, "example", bytes.fromhex("68 02 00 00 00 c3"))
    with pytest.raises(ValueError, match="completely cover"):
        listing_lines(text.replace("  00005\tc3\t ret", ""), "example", bytes.fromhex("68 01 00 00 00 c3"))


def test_listing_rejects_duplicate_symbol_and_missing_source_anchors():
    fragment = 'x PROC NEAR\n; 1 : x\n  00000\tc3\t ret\nx ENDP\n'
    with pytest.raises(ValueError, match="completely cover"):
        listing_lines(fragment * 2, "x", b"\xc3")
    with pytest.raises(ValueError, match="completely cover"):
        listing_lines(fragment.replace('; 1 : x\n', ''), "x", b"\xc3")


def test_associated_cold_section_offsets_do_not_overwrite_root_source_anchors():
    text = '''_TEXT SEGMENT
x PROC NEAR
; 12 : source
  00000\tc3\t ret
_TEXT ENDS
text$x SEGMENT
  00000\tcc\t int 3
text$x ENDS
x ENDP
'''
    assert listing_lines(text, 'x', b'\xc3') == {0: 12}
    assert listing_lines(text, 'x', b'\xc3\x90\xcc') == {0: 12, 1: None}
    with pytest.raises(ValueError, match='completely cover'):
        listing_lines(text, 'x', b'\xc3\x01')


def test_header_lines_are_not_attributed_to_cpp_host_and_ambiguity_refuses(tmp_path):
    header = tmp_path / 'owner.hpp'
    host = tmp_path / 'owner.cpp'
    header.write_text('inline int f() {return 1;}\n')
    host.write_text('#include "owner.hpp"\n')
    text = '_f PROC NEAR\n; 1 : inline int f() {return 1;}\n_f ENDP\n'
    assert listing_source_texts(text, '_f', {header: header.read_text(), host: host.read_text()}, {1}) == (header, header.read_text())
    host.write_text(header.read_text())
    with pytest.raises(ValueError, match='identify one'):
        listing_source_texts(text, '_f', {header: header.read_text(), host: host.read_text()}, {1})


def test_captured_source_texts_return_only_unique_canonical_header(tmp_path):
    header = (tmp_path / 'include' / 'owner.hpp').resolve()
    host = (tmp_path / 'src' / 'owner.cpp').resolve()
    text = '_f@4 PROC NEAR\n; 1 : int f(int) { return 1; }\n_f@4 ENDP\n'
    inferred, source = listing_source_texts(
        text, '_f@4', {header: 'int f(int) { return 1; }\n',
                       host: '#include "owner.hpp"\n'}, {1})
    assert inferred == header
    assert source == 'int f(int) { return 1; }\n'
    # A compiler-selected exact symbol can still be rejected by its consumer
    # when the uniquely inferred source path is not the declared body path.
    assert inferred != host


def test_captured_source_texts_reject_conflicting_quotes_and_missing_anchors(tmp_path):
    header = (tmp_path / 'owner.hpp').resolve()
    conflict = ('_f PROC NEAR\n; 1 : int f() {\n'
                '; 1 : int f() { return 0; }\n_f ENDP\n')
    with pytest.raises(ValueError, match='conflicting source quotes'):
        listing_source_texts(conflict, '_f', {header: 'int f() { return 0; }\n'}, {1})
    one_quote = '_f PROC NEAR\n; 1 : int f() { return 0; }\n_f ENDP\n'
    with pytest.raises(ValueError, match='every selected anchor'):
        listing_source_texts(one_quote, '_f', {header: 'int f() { return 0; }\n'}, {1, 2})


def test_captured_source_labels_never_resolve_or_reopen_live_paths(monkeypatch):
    def forbidden(*_args, **_kwargs):
        raise AssertionError('captured source matching must not read the filesystem')
    monkeypatch.setattr(Path, 'resolve', forbidden)
    monkeypatch.setattr(Path, 'read_text', forbidden)
    listing = '_f PROC NEAR\n; 1 : void f() {}\n_f ENDP\n'
    path, text = listing_source_texts(listing, '_f', {'/abs/captured/header.h': 'void f() {}\n'}, {1})
    assert path == Path('/abs/captured/header.h') and text == 'void f() {}\n'


def test_listing_lines_selects_exact_overload_symbol_and_checks_its_bytes():
    listing = ('?f@@YAHH@Z PROC NEAR\n; 1 : int f(int) { return 1; }\n'
               '  00000\tc3\t ret\n?f@@YAHH@Z ENDP\n'
               '?f@@YAHXZ PROC NEAR\n; 2 : int f() { return 0; }\n'
               '  00000\t90\t nop\n?f@@YAHXZ ENDP\n')
    assert listing_lines(listing, '?f@@YAHH@Z', b'\xc3') == {0: 1}
    with pytest.raises(ValueError, match='completely cover'):
        listing_lines(listing, '?f@@YAHHH@Z', b'\xc3')
    with pytest.raises(ValueError, match='do not match'):
        listing_lines(listing, '?f@@YAHH@Z', b'\x90')
