"""Use-derived contracts and original quirks; no original type-spelling claim."""
from dataclasses import replace
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_text
from test_texture_edge_cache_actual_source import _definition, _masked
from test_texture_raster_actual_source import _continuity_fields
from test_texture_sub61f0d0_actual_source import extract_method, translation_unit, build_and_run

ROOT = Path(__file__).resolve().parents[2]
BASE = "fdf7feb74cc492ce3fb2bfb88afff15b95496ffb"
SYMBOLS = {
    0x619D80: "?draw_trans2@Texture@@QAEHPAVBuffer@@PAUVert@@H@Z",
    0x61C630: "?sub_61c630@Texture@@QAEXPAVBuffer@@PAUVert@@PAV1@PAEH1@Z",
    0x61D990: "?sub_61d990@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPAV1@PAEH@Z",
    0x61B780: "?sub_61b780@Texture@@QAEXPAVBuffer@@PAUVert@@PAHH@Z",
    0x61D090: "?sub_61d090@Texture@@QAEXPAVBuffer@@PAUVert@@PAHI@Z",
    0x61F0D0: "?sub_61f0d0@Texture@@QAEXPAVBuffer@@PAUVert@@PAHHHIH@Z",
    0x623F50: "?sub_623f50@Texture@@QAEXPAVBuffer@@PAUVert@@PAV1@PAEH1H@Z",
}


def test_all_original_identities_claims_and_history_survive_type_recovery():
    path = ROOT / "src/texture.cpp"
    prior = subprocess.check_output(["git", "show", BASE + ":src/texture.cpp"],
                                    cwd=ROOT, text=True)
    before = {r.address: r for r in read_text(prior, path)}
    after = {r.address: r for r in read_text(path.read_text(), path)}
    assert set(before) == set(after)
    for address, old in before.items():
        current = after[address]
        assert _continuity_fields(current) == _continuity_fields(old)
        assert current.symbol == SYMBOLS.get(address, old.symbol)
        if address == 0x61F785:
            assert current.function_fragment == replace(
                old.function_fragment, parent_symbol=SYMBOLS[0x61F0D0])
        else:
            assert current.function_fragment == old.function_fragment
    assert after[0x623F50].byte_exact and after[0x61F785].byte_exact


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
    t.sub_61f0d0(b,v,light,0,0,1,0);
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
