"""Use-derived contracts and original quirks; no original type-spelling claim."""
from dataclasses import replace
from pathlib import Path
import re
import shutil
import subprocess
from collections import defaultdict

import pytest
from match_recovery.binding_source import parse as parse_bindings
from match_recovery.source import read_file, read_text
from match_recovery.source_config import SourceConfiguration
from test_texture_edge_cache_actual_source import _definition, _masked
from test_texture_raster_actual_source import _continuity_fields
from test_texture_sub61f0d0_actual_source import extract_method, translation_unit, build_and_run

ROOT = Path(__file__).resolve().parents[2]
BASE = "fdf7feb74cc492ce3fb2bfb88afff15b95496ffb"
BYTE_EXACT_PROMOTIONS = frozenset({
    0x0044E5F0, 0x0044E620, 0x0044E650, 0x0044E6A0, 0x0044E6F0,
    0x0044E720, 0x0044E770, 0x0044E7C0, 0x0044E810, 0x0044E840,
    0x0044E890, 0x0044E8E0, 0x0044E930, 0x0044E980, 0x0044E9D0,
    0x0044EA20, 0x0044EA70, 0x0044EAC0, 0x0044EB10, 0x0044EB60,
})
# Recorded initializer claims: 1bf9c1634. Typed free-helper ABI: fff91bbe8.
SYMBOLS = {
    0x619D80: "?draw_trans2@Texture@@QAEHPAVBuffer@@PAUVert@@H@Z",
    0x61C630: "?sub_61c630@Texture@@QAEXPAVBuffer@@PAUVert@@PAV1@PAEH1@Z",
    0x61D990: "?sub_61d990@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPAV1@PAEH@Z",
    0x61B780: "?sub_61b780@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH@Z",
    0x61D090: "?sub_61d090@Texture@@QAEXPAVBuffer@@PAUVert@@PAHI@Z",
    0x61F0D0: "?sub_61f0d0@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPBXPBIPBEH@Z",
    0x623F50: "?sub_623f50@Texture@@QAEXPAVBuffer@@PAUVert@@PAV1@PAEH1H@Z",
}
CURRENT_FREE_HELPER_SYMBOLS = {
    0x61F800: "?sub_61f800@@YAXPAUEdgeScan@@0@Z",
    0x6202E0: "?sub_6202e0@@YAXPAUEdgeScan@@0@Z",
    0x61FE70: "?sub_61fe70@@YAXPAUEdgeScan@@0@Z",
    0x620640: "?sub_620640@@YAXPAUEdgeScan@@0@Z",
}


def test_all_original_identities_claims_and_history_survive_type_recovery():
    path = ROOT / "src/texture.cpp"
    prior = subprocess.check_output(["git", "show", BASE + ":src/texture.cpp"],
                                    cwd=ROOT, text=True)
    before_rows = read_text(prior, path)
    before = _unique_texture_family(before_rows,
                                    {row.address for row in before_rows})
    configuration = SourceConfiguration.read(ROOT)
    configured_rows = [row for source_path in configuration.files(ROOT)
                       for row in read_file(source_path)]
    after = _unique_texture_family(configured_rows, set(before))
    assert set(before) == set(after)
    _assert_all_relocated_bindings_are_source_owned(after, configured_rows)
    promoted = set()
    for address, old in before.items():
        current = after[address]
        # The only changed historical claims are the twenty recorded FILE
        # initializer matches. Normalize that one monotonic field while
        # comparing every other annotation field and lesson byte-for-byte.
        _assert_annotation_continuity(old, current)
        if current.byte_exact and not old.byte_exact:
            promoted.add(address)
        if address in SYMBOLS:
            assert current.symbol == SYMBOLS[address]
        elif address in CURRENT_FREE_HELPER_SYMBOLS:
            assert current.symbol == CURRENT_FREE_HELPER_SYMBOLS[address]
        elif current.path != old.path:
            # The ORIGINAL name remains the identity. This adjacent field is
            # the emitted local lifetime symbol supplied by the real owner.
            assert old.symbol == ""
            assert current.symbol.startswith("_$E")
        else:
            assert current.symbol == old.symbol
        if address == 0x61F785:
            assert current.function_fragment == replace(
                old.function_fragment, parent_symbol=SYMBOLS[0x61F0D0])
        else:
            assert current.function_fragment == old.function_fragment
    _assert_recorded_promotions(promoted)
    assert after[0x623F50].byte_exact and after[0x61F785].byte_exact


def _unique_texture_family(records, expected_addresses):
    selected = defaultdict(list)
    for record in records:
        if record.address in expected_addresses:
            selected[record.address].append(record)
    duplicates = {address: rows for address, rows in selected.items()
                  if len(rows) != 1}
    missing = set(expected_addresses) - set(selected)
    assert not duplicates, "original Texture identity has duplicate current owners"
    assert not missing, "original Texture identity is missing from configured source selection"
    return {address: rows[0] for address, rows in selected.items()}


def _assert_all_relocated_bindings_are_source_owned(family, all_records):
    configured = SourceConfiguration.read(ROOT)
    configured_paths = set(configured.files(ROOT))
    assert all(record.path in configured_paths for record in family.values())
    relocated = [record for record in family.values()
                 if record.path != ROOT / "src/texture.cpp"]
    relocated_paths = {record.path for record in relocated}
    rows_by_path = defaultdict(list)
    for record in all_records:
        if record.path in relocated_paths:
            rows_by_path[record.path].append(record)
    bindings = set()
    for source_path, rows in rows_by_path.items():
        text = source_path.read_text(encoding="utf-8")
        bindings.update((source_path, line)
                        for line in parse_bindings(text, rows))
    assert len(relocated) == 40
    for record in relocated:
        assert (record.path, record.line) in bindings, \
            f"relocated identity 0x{record.address:08X} lacks adjacent native binding evidence"
        assert record.symbol.startswith("_$E"), \
            f"relocated identity 0x{record.address:08X} lacks its emitted lifetime symbol fact"
        assert record.name.startswith("_$E"), \
            f"relocated identity 0x{record.address:08X} lost its original identity name"


def test_identity_regression_rejects_missing_duplicate_span_and_claim_demotion():
    path = ROOT / "src/texture.cpp"
    prior = subprocess.check_output(["git", "show", BASE + ":src/texture.cpp"],
                                    cwd=ROOT, text=True)
    baseline_rows = read_text(prior, path)
    before = _unique_texture_family(baseline_rows,
                                    {row.address for row in baseline_rows})
    configuration = SourceConfiguration.read(ROOT)
    current_rows = [row for source_path in configuration.files(ROOT)
                    for row in read_file(source_path)]
    after = _unique_texture_family(current_rows, set(before))

    missing_address = next(iter(before))
    with pytest.raises(AssertionError, match="missing from configured source selection"):
        _unique_texture_family([row for row in current_rows
                                if row.address != missing_address], set(before))
    with pytest.raises(AssertionError, match="duplicate current owners"):
        _unique_texture_family([*current_rows, after[missing_address]], set(before))

    changed_span = next(address for address, row in after.items()
                        if row.image_spans and row.image_spans[0][1] -
                        row.image_spans[0][0] > 1)
    current = after[changed_span]
    low, high = current.image_spans[0]
    with pytest.raises(AssertionError):
        _assert_annotation_continuity(before[changed_span],
                                      replace(current, image_spans=((low, high - 1),)))

    retained_claim = next(address for address, row in before.items()
                          if row.byte_exact and address not in BYTE_EXACT_PROMOTIONS)
    with pytest.raises(AssertionError, match="was demoted"):
        _assert_annotation_continuity(
            before[retained_claim], replace(after[retained_claim], byte_exact=False))


def _assert_annotation_continuity(old, current):
    assert _continuity_fields(replace(current, byte_exact=old.byte_exact)) == \
        _continuity_fields(old)
    assert not old.byte_exact or current.byte_exact, \
        f"historical BYTE_EXACT claim was demoted at 0x{old.address:08X}"


def _assert_recorded_promotions(promoted):
    assert promoted == BYTE_EXACT_PROMOTIONS, "historical claim promotions differ from recorded evidence"


def test_claim_promotions_reject_unrecorded_addition_and_missing_recorded_claim():
    with pytest.raises(AssertionError, match="differ from recorded evidence"):
        _assert_recorded_promotions(BYTE_EXACT_PROMOTIONS | {0x619650})
    with pytest.raises(AssertionError, match="differ from recorded evidence"):
        _assert_recorded_promotions(BYTE_EXACT_PROMOTIONS - {0x44E5F0})


def run_cpp(tmp_path, source):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    path, binary = tmp_path / "contract.cpp", tmp_path / "contract"
    path.write_text(source)
    result = subprocess.run([compiler, "-std=c++11", str(path), "-o", str(binary)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    return subprocess.run([str(binary)], capture_output=True, timeout=30)


def test_actual_header_contracts_compile_without_integer_pointer_apis(tmp_path):
    header = _masked((ROOT / "src/texture.h").read_text())
    names = [symbol[1:symbol.index("@")] for symbol in SYMBOLS.values()]
    declarations = []
    for name in names:
        rows = re.findall(r"(?:void|int)\s+" + name + r"\([^;]*\);", header)
        assert len(rows) == 1
        declarations.append(rows[0])
    source = """
#include <stdint.h>
class Buffer; struct Vert { int x,y; }; class TextureStore;
class Texture { public:
""" + "\n".join(declarations) + """
};
void probe(Texture &t, Buffer *b, Vert *v, int *light, unsigned char *remap) {
    t.draw_trans2(b,v,0);
    t.sub_61c630(b,v,&t,remap,4,v);
    t.sub_61d990(b,v,light,&t,remap,0);
    t.sub_61b780(b,v,light,0);
    t.sub_61d090(b,v,light,0);
    t.sub_61f0d0(b,v,light,0,0,remap,0);
    t.sub_623f50(b,v,&t,remap,4,v,-1);
}
int main() { return 0; }
"""
    # A syntax-only check uses the actual declarations without linking doubles.
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    path = tmp_path / "header.cpp"
    path.write_text(source)
    result = subprocess.run([compiler, "-std=c++11", "-fsyntax-only", str(path)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("swap", (False, True))
def test_actual_shaded_sentinel_wrapper_preserves_both_pointer_channels(tmp_path, swap):
    source = (ROOT / "src/texture.cpp").read_text()
    body = _definition(source, "void Texture::sub_623f50(")
    if swap:
        old = "sub_61c630(a1, a2, a3, a4, a5, a6);"
        assert body.count(old) == 1
        body = body.replace(old, "sub_61c630(a1, a6, a3, a4, a5, a2);", 1)
    harness = """
#include <cassert>
#include <stdint.h>
struct Vert { int x,y; }; struct Buffer {};
class Texture { public:
    int calls=0; Vert *geometry=0, *uv=0;
    Buffer *buffer=0; Texture *shade=0; uint8_t *remap=0; int count=0;
    void sub_61c630(Buffer *b, Vert *g, Texture *s, uint8_t *r, int n, Vert *v) {
        ++calls; buffer=b; geometry=g; shade=s; remap=r; count=n; uv=v;
    }
    void sub_623f50(Buffer *,Vert *,Texture *,uint8_t *,int,Vert *,int);
};
""" + body + """
int main() {
    Texture t, shade; Buffer b; Vert g[4]={}, uv[4]={}; uint8_t remap[256]={};
    t.sub_623f50(&b,g,&shade,remap,4,uv,0);
    t.sub_623f50(&b,g,&shade,remap,4,uv,7); assert(t.calls==0);
    t.sub_623f50(&b,g,&shade,remap,4,uv,-1);
    assert(t.calls==1 && t.buffer==&b && t.geometry==g && t.uv==uv);
    assert(t.shade==&shade && t.remap==remap && t.count==4);
}
"""
    result = run_cpp(tmp_path, harness)
    assert (result.returncode == 0) == (not swap)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_index_zero_fix_is_rejected_by_original_high_y_control(compiler):
    body = extract_method((ROOT / "src/texture.cpp").read_text())
    old = "int top_index = static_cast<int>(texture_pointer_address(param_3));"
    assert body.count(old) == 1
    changed = body.replace(old, "int top_index = 0;", 1)
    build_and_run(translation_unit(changed), compiler, expect_success=False)
