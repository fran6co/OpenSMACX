"""Reading an MSVC mangled name far enough to tell overloads apart."""

import pytest

from decomp.mangled import arity

# The real overload sets this exists for: in each pair the catalogue's
# spelling and what CL emits disagree about the return type, the constness,
# or both - and agree about the COUNT.
FILEMAP = [("?init@Filemap@@QAEHPADH@Z", "?init@Filemap@@QAEPAV1@PBDH@Z", 2),
           ("?init@Filemap@@QAEHPAD@Z", "?init@Filemap@@QAEPAV1@PBD@Z", 1)]


@pytest.mark.parametrize("catalogued,emitted,count", FILEMAP)
def test_the_two_spellings_agree_on_arity(catalogued, emitted, count):
    assert arity(catalogued) == arity(emitted) == count


def test_void_is_no_arguments_not_one():
    assert arity("?start@Time@@QAEHXZ") == 0
    assert arity("?start@Time@@QAEIXZ") == 0


def test_a_wrong_return_type_does_not_move_the_count():
    """`?map_init@@YAXXZ` says void over a body `src/map.cpp` defines as
    `BOOL __cdecl map_init()`."""
    assert arity("?map_init@@YAXXZ") == arity("?map_init@@YAHXZ") == 0


def test_a_back_referenced_argument_still_counts():
    """MSVC never writes the same composite type twice; the catalogue does."""
    written = ("?set_font@Buffer@@QAEHPAVFont@@PAVFont@@PAVFont@@"
               "PAVFont@@@Z")
    assert arity(written) == arity("?set_font@Buffer@@QAEHPAVFont@@000@Z") == 4


def test_a_callback_argument_is_one_argument():
    """`P6AXH@Z` is a function POINTER, not its arguments spilled inline."""
    assert arity("?start@Time@@QAEIP6AXH@ZHII@Z") == 4
    assert arity("?start@Time@@QAEIP6AXHH@ZHHII@Z") == 5


def test_a_static_member_carries_no_cv_code():
    """Reading three characters after the head would swallow the return type
    of every static member and every free function."""
    assert arity("?f@C@@SAXH@Z") == 1        # static: access + convention
    assert arity("?g@@YAXH@Z") == 1          # free function


def test_a_nonstatic_member_does():
    assert arity("?h@C@@QAEXH@Z") == 1


@pytest.mark.parametrize("bad", [
    "", "not a mangled name", "?f@C@@", "??_GAutoSound@@UAEPAXI",
    "?f@C@@ZZZXH@Z",
])
def test_unreadable_names_say_so_rather_than_guess(bad):
    """None means "cannot tell", which every caller treats as ambiguous.
    A tokeniser that guessed would produce a symbol wrong in a NEW way."""
    assert arity(bad) is None


def test_a_constructor_declares_an_arity_like_anything_else():
    """MSVC marks the ABSENCE of a return type with a bare `@` where the
    type would be, and read as a type it made `_types` fail - so every
    `??0`/`??1` name came back None. That is exactly the population where
    arity is needed: a class's constructors are told apart by nothing else,
    and `??0Font@@QAE@XZ` and `??0Font@@QAE@PADHH@Z` are two definitions on
    two lines of one file."""
    assert arity("??0Font@@QAE@XZ") == 0
    assert arity("??0Font@@QAE@PADHH@Z") == 3
    assert arity("??0Text@@QAE@H@Z") == 1
    assert arity("??1Font@@QAE@XZ") == 0


def test_a_method_still_drops_its_return_type():
    """The `@` only stands in for a return type that is absent; where one is
    present it is still the first of the types and still not an argument."""
    assert arity("?on_mouse_move@Win@@QAEXHHIH@Z") == 4
    assert arity("?set_font@Buffer@@QAEHPAVFont@@000@Z") == 4
    assert arity("?close@Text@@QAEXXZ") == 0
