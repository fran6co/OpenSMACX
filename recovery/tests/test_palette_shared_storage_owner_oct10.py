"""Source-owned palette bindings and actual shared-storage consumer controls.

Start identities do not assert original allocation bounds or initializer equality.
Host controls are not COM/GDI runtime, native layout or full-game acceptance.
"""
from pathlib import Path
import re

import pytest

from match_recovery.object_source import parse_objects
from test_texture_sub61f0d0_actual_source import build_and_run, extract_method

ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/jackal/palette.cpp").is_file())
SOURCE = ROOT / "src/jackal/palette.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
OBJECTS = {
    0x9B8178: ("PaletteInitialized", "?PaletteInitialized@@3PAUHPALETTE__@@A"),
    0x9B8188: ("PaletteUsesSystemColours", "?PaletteUsesSystemColours@@3HA"),
    0x9B8184: ("PaletteSeedCache", "?PaletteSeedCache@@3HA"),
    0x9BC4A0: ("DirectDrawPalette", "?DirectDrawPalette@@3PAUIDirectDrawPalette@@A"),
}


def test_shared_objects_have_unique_start_only_owner_bindings():
    text = SOURCE.read_text()
    rows = parse_objects(text, "src/jackal/palette.cpp", PIN, address_space="pe:va")
    for address, (name, symbol) in OBJECTS.items():
        selected = [row for row in rows if row.origin.address == address]
        assert len(selected) == 1
        row = selected[0]
        assert row.symbol == symbol and row.span is None and row.evidence is None
        assert row.parent_symbol is None and row.target_symbol is None
        assert name in text.splitlines()[row.line]


HOST = r"""
#include <cassert>
#define nullptr 0
struct HPALETTE__ { int value; };
typedef HPALETTE__ *HPALETTE;
class Palette;
static int syncs,animations,deletes,dd_calls;
static Palette *expected;
struct IDirectDrawPalette {
    void SetEntries(int flags,int start,int count,int *entries);
};
// STORAGE
static Palette *PaletteActive;
static int BufferDirectDraw;
class Palette {
public:
    int seed_,entries_[256];
    int set();
    static void close_palette_class();
};
static HPALETTE__ token;
static IDirectDrawPalette direct;
struct Buffer {
    void sync_to_palette(Palette *p) {
        assert(p==expected && PaletteActive==p); ++syncs;
    }
};
struct Owner { Buffer value; Buffer &buffer() { return value; } };
static Owner ScreenBufferOwner;
void IDirectDrawPalette::SetEntries(int flags,int start,int count,int *entries) {
    assert(this==&direct && flags==0 && start==0 && count==256);
    assert(entries==expected->entries_); ++dd_calls;
}
static void AnimatePalette(HPALETTE handle,int start,int count,int *entries) {
    assert(handle==&token && start==10 && count==236);
    assert(entries==expected->entries_+10); ++animations;
}
static void DeleteObject(HPALETTE handle) {
    assert(handle==&token && PaletteInitialized==handle); ++deletes;
}
// BODIES
int main() {
    assert(PaletteInitialized==0 && PaletteUsesSystemColours==0);
    assert(PaletteSeedCache==0 && DirectDrawPalette==0);
    Palette p; p.seed_=29; expected=&p;
    BufferDirectDraw=1; DirectDrawPalette=&direct;
    assert(p.set()==0 && dd_calls==1 && syncs==0 && animations==0);
    DirectDrawPalette=0;
    assert(p.set()==0 && dd_calls==1 && PaletteActive==0);
    BufferDirectDraw=0;
    assert(p.set()==7 && syncs==0 && PaletteActive==0);
    PaletteInitialized=&token; PaletteUsesSystemColours=1;
    assert(p.set()==0 && syncs==1 && animations==0 && PaletteSeedCache==0);
    PaletteUsesSystemColours=0;
    assert(p.set()==0 && syncs==2 && animations==1 && PaletteSeedCache==29);
    assert(p.set()==0 && syncs==3 && animations==1);
    DirectDrawPalette=&direct; // independent COM palette is not GDI handle storage
    Palette::close_palette_class();
    assert(deletes==1 && PaletteInitialized==0 && DirectDrawPalette==&direct);
    Palette::close_palette_class(); assert(deletes==1);
}
"""


def unit(damage=None):
    text = SOURCE.read_text()
    definitions = []
    for name, _ in OBJECTS.values():
        selected = re.findall(r"^([^\n;]*\b" + name + r";)\s*//", text, re.M)
        assert len(selected) == 1, name
        definitions.append(selected[0])
    bodies = "\n".join(extract_method(text, signature) for signature in
                       ("int Palette::set()", "void Palette::close_palette_class()"))
    if damage:
        old, new = damage
        assert old in bodies
        bodies = bodies.replace(old, new, 1)
    return HOST.replace("// STORAGE", "\n".join(definitions)).replace("// BODIES", bodies)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_palette_storage_consumers(compiler):
    build_and_run(unit(), compiler, expect_success=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("old,new", (
    ("SetEntries(0, 0, 256, entries_)", "SetEntries(0, 0, 255, entries_)"),
    ("PaletteActive = this;", "PaletteActive = 0;"),
    ("PaletteSeedCache = seed_;", "PaletteSeedCache = 0;"),
    ("PaletteInitialized = nullptr;", "(void)0;"),
))
def test_rejects_shared_storage_consumer_damage(compiler, old, new):
    build_and_run(unit((old, new)), compiler, expect_success=False)
