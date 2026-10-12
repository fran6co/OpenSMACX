"""Reading an MSVC mangled name far enough to tell overloads apart."""

import pytest

from match_recovery.source.mangled import arity

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


@pytest.mark.parametrize("path", [
    "src/unit.cpp", "other/unit.cpp", "unit.cpp", "src/../unit.cpp",
    "src/unit.cpp123", "src/unit.cpp456", r"C:\project\unit.cpp",
])
@pytest.mark.parametrize("encoding", ["EA", "HA", "UPayload@1@A", "VBuffer@@A"])
def test_vc6_anonymous_data_keeps_the_entire_encoded_scope(path, encoding):
    from match_recovery.source.mangled import qualified_name
    assert qualified_name("?Global@?%" + path + "@@3" + encoding) == "?%" + path + "::Global"


def test_vc6_anonymous_data_does_not_merge_path_or_nonce_spellings():
    from match_recovery.source.mangled import qualified_name
    paths = ["src/unit.cpp", "other/unit.cpp", "src/../unit.cpp",
             "unit.cpp", "unit.cpp123", "unit.cpp456", r"src\unit.cpp"]
    assert len({qualified_name("?Global@?%" + path + "@@3HA") for path in paths}) == len(paths)


@pytest.mark.parametrize("symbol", [
    "?Global@?%@@3HA", "?Global@?%unit@@3HA", "?Global@?%src/@@3HA",
    "?Global@?%src/unit.cpp@3HA", "?Global@?%src/unit.cpp@@",
    "?Global@?%src/unit.cpp@@3", "?Global@?%src/unit.cpp@@3A",
    "?Global@?%src/unit.cpp@@3@@A", "?Global@?%src/unit.cpp@@3?@@A",
    "?Global@?%src/unit.cpp@@3$@@A", "?Global@?%src/unit.cpp@@30A",
    "?Global@?%src/unit.cpp@@3_NA", "?Global@?%src/unit.cpp@@3DA",
    "?Global@?%src/unit.cpp@@3U@@A", "?Global@?%src/unit.cpp@@3V?@@A",
    "?Global@?%src/unit.cpp@@3H@@A", "?Global@?%src/unit.cpp@@3EPayload@@A",
    "?Global@?%src/unit.cpp@@3UPayload@2@A",
    "?Global@?%src/unit.cpp@@3U?$Payload@H@@A",
    "?Global@?%src/unit.cpp@@3HAjunk", "?Global@?%src/unit.cpp@@3H\nA",
    "?Global@?%src/un it.cpp@@3HA", "?Global@?%src/unit%.cpp@@3HA",
    "?Global@?%src/unit.cpp@Outer@@3HA",
    "?Global@Inner@?%src/unit.cpp@@3HA",
    "??$Global@H@?%src/unit.cpp@@3HA",
    "?Global@?$Owner@H@?%src/unit.cpp@@3HA",
    "?Global@?%src/unit.cpp@@2HA",
])
def test_unsupported_anonymous_data_shapes_still_have_no_qualified_name(symbol):
    from match_recovery.source.mangled import qualified_name
    assert qualified_name(symbol) == ""


@pytest.mark.parametrize("symbol", [
    "?function@?%src/unit.cpp@@YAHXZ",
    "?method@Class@?%src/unit.cpp@@QAEXXZ",
    "??0Class@?%src/unit.cpp@@QAE@XZ",
    "??1Class@?%src/unit.cpp@@QAE@XZ",
    "??_GClass@?%src/unit.cpp@@UAEPAXI@Z",
    "??__EGlobal@?%src/unit.cpp@@YAXXZ",
    "??0?$Owner@H@?%src/unit.cpp@@QAE@XZ",
])
def test_anonymous_function_class_and_template_eligibility_is_unchanged(symbol):
    from match_recovery.function_witness import _expression
    from match_recovery.ownership_source import class_hint
    from match_recovery.source.mangled import qualified_name
    assert qualified_name(symbol) == ""
    assert class_hint(symbol) is None
    with pytest.raises(ValueError):
        _expression(symbol, "msvc")


def test_recognized_anonymous_data_cannot_become_a_function_or_class_witness():
    from match_recovery.function_witness import _expression
    from match_recovery.ownership_source import class_hint
    symbol = "?Global@?%src/unit.cpp@@3HA"
    assert class_hint(symbol) is None
    with pytest.raises(ValueError):
        _expression(symbol, "msvc")


@pytest.mark.parametrize("symbol", [
    "_?saved@?1??work@@YAHHHH@Z@4PAVBuffer@@A",
    "_?redrawn@?1??work@@YAHHHH@Z@4PAVBuffer@@A",
    "_?animation@?1??work@@YAHHHH@Z@4VAnimation@@A",
    "_?working@?1??work@@YAHHHH@Z@4VBuffer@@A",
    "_?saved@?2??work@@YAHHHH@Z@4PAVBuffer@@A",
    "_?saved@?1??work@@YAHMH@Z@4PAVBuffer@@A",
])
def test_vc6_local_data_retains_the_complete_encoded_identity(symbol):
    from match_recovery.function_witness import _expression
    from match_recovery.ownership_source import class_hint
    from match_recovery.source.mangled import qualified_name
    assert qualified_name(symbol) == symbol
    assert class_hint(symbol) is None
    with pytest.raises(ValueError):
        _expression(symbol, "msvc")


def test_local_data_block_overload_name_and_trailer_are_distinct():
    from match_recovery.source.mangled import qualified_name
    symbols = [
        "_?saved@?1??work@@YAHHHH@Z@4PAVBuffer@@A",
        "_?saved@?2??work@@YAHHHH@Z@4PAVBuffer@@A",
        "_?saved@?1??work@@YAHMH@Z@4PAVBuffer@@A",
        "_?saved@?1??other@@YAHHHH@Z@4PAVBuffer@@A",
        "_?saved@?1??work@@YAHHHH@Z@4VBuffer@@A",
        "_?saved@?1??work@@YAHHHH@Z@4PAVOther@@A",
    ]
    assert {qualified_name(symbol) for symbol in symbols} == set(symbols)


@pytest.mark.parametrize("symbol", [
    "?saved@?1??work@@YAHHHH@Z@4PAVBuffer@@A",  # missing exact prefix
    "__?saved@?1??work@@YAHHHH@Z@4PAVBuffer@@A",
    "_?saved@???work@@YAHHHH@Z@4PAVBuffer@@A",
    "_?saved@?0??work@@YAHHHH@Z@4PAVBuffer@@A",
    "_?saved@?01??work@@YAHHHH@Z@4PAVBuffer@@A",
    "_?saved@?A@??work@@YAHHHH@Z@4PAVBuffer@@A",
    "_?saved@?1??work@@YAHHHH@Z@3PAVBuffer@@A",
    "_?saved@?1??work@@YAHHHH@Z@4PAVBuffer@@",
    "_?saved@?1??work@@YAHHHH@Z@4PAVBuffer@@Aextra",
    "_?saved@?1??work@@YAHHHH@Z@4PAVBuffer@@A\nextra",
    "_?saved@?1??work@@YAHHHH@Z@4UBuffer@@A",
    "_?saved@?1??work@@YAHHHH@Z@4HA",
    "_?saved@?1??work@@YAHW@Z@4VBuffer@@A",  # unreadable enclosing signature
    "_?saved@?1??work@@YAHHHH@Z",            # function, not local data
    "_?saved@?1??work@Class@@QAEHH@Z@4VBuffer@@A",
    "_?saved@?1???0Class@@QAE@XZ@4VBuffer@@A",
    "_?saved@?1???$work@H@@YAHHHH@Z@4VBuffer@@A",
    "_?saved@?1??work@@YAHHHH@Z@4V?$Owner@H@@A",
    "_?$S1@?1??work@@YAHHHH@Z@4EA",        # generated guard, not ordinary local
])
def test_unsupported_local_prefix_block_function_and_data_forms_refuse(symbol):
    from match_recovery.function_witness import _expression
    from match_recovery.ownership_source import class_hint
    from match_recovery.source.mangled import qualified_name
    assert qualified_name(symbol) == ""
    assert class_hint(symbol) is None
    with pytest.raises(ValueError):
        _expression(symbol, "msvc")
