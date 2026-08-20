"""The read -> write -> read loop.

Fixtures exercise every feature of the annotation grammar, and the last
test closes the loop over the whole tree. Everything is proved against
`src/` and purpose-built text alone - the package is NOT compared against
the `tools/` copies here; that comparison is the drift check in
`python -m decomp`.

Run with `uv run pytest decomp/tests`.
"""

from pathlib import Path

import ast
import sys

import pytest

import decomp

from decomp import (DecompilationState, Mode, State, function_line, grammar,
                    read,
                    read_file, read_text, reader, remove, remove_file,
                    write, write_file)

# This checkout, answered by the test rather than by the package.
REPO_ROOT = Path(__file__).resolve().parents[2]

FIXTURE = Path("fixture.cpp")


def record(address, line, mode=Mode.BODY, state=State.IMPLEMENTED,
           name="", spans=(), **kwargs):
    return DecompilationState(address=address, mode=mode, state=state,
                              path=FIXTURE, line=line, name=name,
                              image_spans=spans, **kwargs)


def loop(text):
    """read -> write -> read; the records the loop must preserve."""
    records = read_text(text, FIXTURE)
    return records, read_text(write(text, records), FIXTURE)


def keys(records):
    return [_key(r) for r in records]


# ------------------------------------------------------------- round trips

BODY = """// ORIGINAL: 0x00401000
void __cdecl f() {
    return;
}
"""


def test_body_marker():
    records, reread = loop(BODY)
    assert len(records) == 1
    assert records[0].address == 0x00401000
    assert records[0].mode is Mode.BODY
    assert records[0].state is State.IMPLEMENTED
    assert keys(records) == keys(reread)
    # Canonical in, byte-identical out.
    assert write(BODY, records) == BODY


FILE_TEXT = """// ORIGINAL: 0x00402000 FILE
// BODY GOES HERE.
"""


def test_file_mode():
    records, reread = loop(FILE_TEXT)
    assert len(records) == 1
    assert records[0].mode is Mode.FILE
    assert records[0].state is State.PLACEHOLDER   # derived from the sentinel
    assert keys(records) == keys(reread)
    assert write(FILE_TEXT, records) == FILE_TEXT


EXCLUDED_TEXT = """// ORIGINAL: 0x00403000 EXCLUDED S1
"""


def test_excluded():
    records, reread = loop(EXCLUDED_TEXT)
    assert records[0].state is State.EXCLUDED
    assert records[0].exclusion == "S1"
    assert keys(records) == keys(reread)
    assert write(EXCLUDED_TEXT, records) == EXCLUDED_TEXT


MATCHED_TEXT = """// ORIGINAL: 0x00404000 BYTE_EXACT
void __cdecl g() {
}
"""


def test_byte_exact():
    records, reread = loop(MATCHED_TEXT)
    assert records[0].byte_exact is True
    assert records[0].state is State.IMPLEMENTED
    assert keys(records) == keys(reread)
    assert write(MATCHED_TEXT, records) == MATCHED_TEXT


LESSON_TEXT = """// ORIGINAL: 0x00405000 ?f@@YAXXZ 0x00405000-0x00405040
// LEVER: fp-1234 the cast that landed it
// RULED-OUT: plain ret 4 leaves ECX live
// UNRECOVERABLE: reads three registers live-in
// DEFERRED: needs the vtable order first
void __cdecl h() {
}
"""


def test_lessons():
    records, reread = loop(LESSON_TEXT)
    assert records[0].levers == (("fp-1234", "the cast that landed it"),)
    assert records[0].ruled_out == ("plain ret 4 leaves ECX live",)
    assert records[0].unrecoverable == ("reads three registers live-in",)
    assert records[0].deferred == ("needs the vtable order first",)
    assert keys(records) == keys(reread)
    assert write(LESSON_TEXT, records) == LESSON_TEXT


WRAPPED_LESSON = """// ORIGINAL: 0x00405100 ?g@@YAXXZ 0x00405100-0x00405140
// RULED-OUT: plain ret 4 leaves ECX live,
//            so the stub must zero it first
void __cdecl h() {
}
"""


def test_wrapped_lesson_canonicalises():
    records = read_text(WRAPPED_LESSON, FIXTURE)
    # The reader joins the continuation with a space.
    assert records[0].ruled_out == \
        ("plain ret 4 leaves ECX live, so the stub must zero it first",)
    rewritten = write(WRAPPED_LESSON, records)
    assert rewritten != WRAPPED_LESSON          # one canonical line now
    reread = read_text(rewritten, FIXTURE)
    assert keys(records) == keys(reread)
    # The fixed point: canonical is stable.
    assert write(rewritten, reread) == rewritten


MULTI = """// ORIGINAL: 0x00406000 ?a@@YAXXZ 0x00406000-0x00406008
void __cdecl a() {
}

// ORIGINAL: 0x00406010 ?b@@YAXXZ 0x00406010-0x00406018
void __cdecl b() {
}
"""


def test_several_markers():
    records, reread = loop(MULTI)
    assert [r.address for r in records] == [0x00406000, 0x00406010]
    assert keys(records) == keys(reread)
    assert write(MULTI, records) == MULTI


def test_read_directory_globs_recursively(tmp_path):
    nested = tmp_path / "nested"
    nested.mkdir()
    (tmp_path / "a.cpp").write_text(BODY)
    (nested / "b.cpp").write_text(MATCHED_TEXT)
    records = read(tmp_path)
    assert [r.address for r in records] == [0x00401000, 0x00404000]
    assert all(r.path.is_absolute() for r in records)


# ------------------------------------------- write trusts the records it gets

TWO_WITH_LESSONS = """// ORIGINAL: 0x00407000
// LEVER: fp-1 what landed A
void __cdecl a() {
}

// ORIGINAL: 0x00407010
// RULED-OUT: the spelling that failed B
void __cdecl b() {
}
"""


def test_unclaimed_markers_pass_through():
    records = read_text(TWO_WITH_LESSONS, FIXTURE)
    described = [r for r in records if r.address == 0x00407000]
    rewritten = write(TWO_WITH_LESSONS, described)
    reread = read_text(rewritten, FIXTURE)
    # A is rewritten canonically; B was not described, so it is untouched.
    assert [r.address for r in reread] == [0x00407000, 0x00407010]
    assert reread[0].levers == (("fp-1", "what landed A"),)
    assert "0x00407010" in rewritten
    assert "the spelling that failed B" in rewritten


def test_empty_records_leave_the_text_untouched():
    assert write(TWO_WITH_LESSONS, []) == TWO_WITH_LESSONS


def test_append_past_the_last_line():
    text = "void __cdecl f() {\n}\n"
    made = [record(0x00408100, line=len(text.splitlines()) + 1,
                   name="?g@@YAXXZ", spans=((0x00408100, 0x00408104),))]
    rewritten = write(text, made)
    assert rewritten.splitlines()[-1] == \
        "// ORIGINAL: 0x00408100 ?g@@YAXXZ 0x00408100-0x00408104"


# ------------------------------------------------- remove is a statement


def test_remove_deletes_named_records():
    records = read_text(TWO_WITH_LESSONS, FIXTURE)
    drop = [r for r in records if r.address == 0x00407010]
    rewritten = remove(TWO_WITH_LESSONS, drop)
    reread = read_text(rewritten, FIXTURE)
    assert [r.address for r in reread] == [0x00407000]
    # B's marker and lesson run are gone; B's code and all of A remain.
    assert "0x00407010" not in rewritten
    assert "the spelling that failed B" not in rewritten
    assert "void __cdecl b() {" in rewritten
    assert "// ORIGINAL: 0x00407000" in rewritten
    assert "what landed A" in rewritten


def test_remove_all_leaves_only_the_code():
    records = read_text(TWO_WITH_LESSONS, FIXTURE)
    rewritten = remove(TWO_WITH_LESSONS, records)
    assert "ORIGINAL:" not in rewritten
    assert "LEVER:" not in rewritten
    assert "RULED-OUT:" not in rewritten
    assert "void __cdecl a() {" in rewritten
    assert "void __cdecl b() {" in rewritten


def test_remove_refusals():
    with pytest.raises(ValueError, match="past the end"):
        remove("one line\n", [record(0x409000, line=99)])


# ----------------------------------------------------------------- refusals


def test_a_line_number_is_1_based_and_the_record_says_so():
    """Refused at construction, not at the two entry points that remembered
    to ask: `lines[line - 1]` on a zero silently addresses the LAST line, so
    a record that could do that must not exist in the first place."""
    with pytest.raises(ValueError, match="1-based"):
        record(0x409000, line=0)
    with pytest.raises(ValueError, match="1-based"):
        record(0x409000, line=-1)


def test_refuses_file_and_excluded_together():
    bad = record(0x409000, line=1, mode=Mode.FILE, state=State.EXCLUDED)
    with pytest.raises(ValueError, match="mutually exclusive"):
        write("anything\n", [bad])


def test_refuses_a_line_past_the_end():
    with pytest.raises(ValueError, match="past the end"):
        write("one line\n", [record(0x409000, line=99)])


def test_refuses_two_records_on_one_line():
    with pytest.raises(ValueError, match="claimed by another"):
        write("anything\n",
              [record(0x409000, line=1), record(0x409001, line=1)])


def test_write_file_uses_the_paths_records_carry(tmp_path):
    path = tmp_path / "unit.cpp"
    path.write_text(WRAPPED_LESSON)
    write_file(read_file(path))
    # The wrapped lesson is canonicalised on disk, one line.
    assert ("// RULED-OUT: plain ret 4 leaves ECX live, "
            "so the stub must zero it first") in path.read_text()


def test_write_file_groups_records_by_path(tmp_path):
    a, b = tmp_path / "a.cpp", tmp_path / "b.cpp"
    a.write_text(WRAPPED_LESSON)
    b.write_text(BODY)
    write_file(read_file(a) + read_file(b))
    assert "so the stub must zero it first" in a.read_text()
    assert b.read_text() == BODY


def test_remove_file(tmp_path):
    path = tmp_path / "unit.cpp"
    path.write_text(BODY)
    remove_file(read_file(path))
    assert "ORIGINAL:" not in path.read_text()
    assert "void __cdecl f() {" in path.read_text()


# ------------------------------------------------- new annotations: insert


PLAIN = """void __cdecl alpha() {
    step_one();
}

void __cdecl beta(int x) {
    step_two(x);
}
"""


def test_insert_new_annotation_above_a_function():
    made = [record(0x00409000, line=1, name="?alpha@@YAXXZ",
                   spans=((0x00409000, 0x00409008),))]
    rewritten = write(PLAIN, made)
    lines = rewritten.splitlines()
    # The annotation lands complete: the marker names the piece and says
    # where it ends in the image.
    assert lines[0] == "// ORIGINAL: 0x00409000 ?alpha@@YAXXZ 0x00409000-0x00409008"
    assert lines[1] == "void __cdecl alpha() {"   # definition untouched
    reread = read_text(rewritten, FIXTURE)
    assert len(reread) == 1
    assert reread[0].address == 0x00409000
    assert reread[0].line == 1
    assert reread[0].state is State.IMPLEMENTED
    assert reread[0].name == "?alpha@@YAXXZ"
    assert reread[0].image_spans == ((0x00409000, 0x00409008),)
    # The fixed point holds for inserted annotations too.
    assert write(rewritten, reread) == rewritten


def test_insert_refuses_an_annotation_without_identity():
    with pytest.raises(ValueError, match="name and its spans"):
        write(PLAIN, [record(0x00409000, line=1)])


MARKED = """// ORIGINAL: 0x00400100 ?alpha@@YAXXZ 0x00400100-0x00400108
void __cdecl alpha() {
}

void __cdecl beta() {
}

// ORIGINAL: 0x00400300 ?gamma@@YAXXZ 0x00400300-0x00400308
void __cdecl gamma() {
}
"""


def test_insert_between_existing_annotations():
    records = read_text(MARKED, FIXTURE)
    assert [r.address for r in records] == [0x00400100, 0x00400300]
    new = record(0x00400200, line=5, name="?beta@@YAXXZ",
                 spans=((0x00400200, 0x00400208),))
    rewritten = write(MARKED, records + [new])
    reread = read_text(rewritten, FIXTURE)
    assert [r.address for r in reread] == [0x00400100, 0x00400200, 0x00400300]
    assert reread[1].line == 5
    assert reread[1].name == "?beta@@YAXXZ"
    assert "void __cdecl beta() {" in rewritten


def test_one_call_mixes_replacement_and_insertion():
    records = read_text(MARKED, FIXTURE)
    first = next(r for r in records if r.address == 0x00400100)
    first.byte_exact = True
    new = record(0x00400200, line=5, name="?beta@@YAXXZ",
                 spans=((0x00400200, 0x00400208),))
    rewritten = write(MARKED, records + [new])
    assert ("// ORIGINAL: 0x00400100 ?alpha@@YAXXZ 0x00400100-0x00400108 "
            "BYTE_EXACT") in rewritten
    assert "// ORIGINAL: 0x00400200 ?beta@@YAXXZ 0x00400200-0x00400208" \
        in rewritten
    reread = read_text(rewritten, FIXTURE)
    assert [r.address for r in reread] == [0x00400100, 0x00400200, 0x00400300]
    assert reread[0].byte_exact is True


# ----------------------------------------------------- function_line

FIND_ME = """// a line comment mentioning find_me( in prose
/* find_me( inside a block comment
   still inside */
void find_me(int a);

void caller() {
    find_me(1);
    if (find_me(2)) {
        other();
    }
}

void find_me(int a) {
    const char *s = "find_me( in a string";
}
"""


def test_function_line_skips_comments_calls_and_declarations():
    assert function_line(FIND_ME, "find_me") == 13


QUALIFIED = """int StringStruct::close(int x) {
    return 0;
}

BasePop::BasePop() {
    init();
}
"""


def test_function_line_finds_members_and_constructors():
    assert function_line(QUALIFIED, "StringStruct::close") == 1
    assert function_line(QUALIFIED, "BasePop::BasePop") == 5


def test_function_line_refuses_ambiguity_and_absence():
    two = "void dup() {\n}\nvoid dup() {\n}\n"
    with pytest.raises(ValueError, match="2 definitions"):
        function_line(two, "dup")
    with pytest.raises(ValueError, match="0 definitions"):
        function_line(two, "absent")


def test_annotate_by_name_end_to_end(tmp_path):
    path = tmp_path / "unit.cpp"
    path.write_text(QUALIFIED)
    line = function_line(path, "StringStruct::close")
    made = DecompilationState(address=0x00401060, mode=Mode.BODY,
                              state=State.IMPLEMENTED,
                              path=path.resolve(), line=line,
                              name="?close@StringStruct@@QAEXXZ",
                              image_spans=((0x00401060, 0x004010F9),))
    write_file([made])
    reread = read_file(path)
    assert len(reread) == 1
    assert reread[0].address == 0x00401060
    assert reread[0].line == 1
    assert reread[0].name == "?close@StringStruct@@QAEXXZ"
    assert reread[0].image_spans == ((0x00401060, 0x004010F9),)
    lines = path.read_text().splitlines()
    assert lines[0] == ("// ORIGINAL: 0x00401060 ?close@StringStruct@@QAEXXZ "
                        "0x00401060-0x004010F9")
    assert lines[1] == "int StringStruct::close(int x) {"


# ------------------------------------------------------------- the loop


def test_tree_roundtrip():
    """Every annotated file in src/ survives read -> write -> read.

    Measured zero skips when this landed: a new skip is an event worth
    investigating - the writer and the reader disagreeing about what an
    annotation owns.
    """
    looped, skipped = roundtrip_tree(REPO_ROOT / "src")
    assert looped > 3000, looped
    assert skipped == 0, skipped


# ------------------------------------------------------- the layering check


def test_layering_accepts_the_package_as_it_stands():
    assert layering() >= 5


def test_layering_refuses_a_reader_that_grows_a_dependency(tmp_path):
    """The check has teeth: a stdlib-only reader importing capstone fails.

    Written because a check nobody has seen fail is a comment. This is the
    exact edit the rule exists to stop - `reader` reaching for something that
    must be installed - and it must be reported, not tolerated.
    """
    (tmp_path / "reader.py").write_text("import capstone\n")
    with pytest.raises(AssertionError, match="standard library"):
        layering(tmp_path)


def test_layering_refuses_an_undeclared_dependency(tmp_path):
    """A module outside the reader set still may not import anything it likes."""
    (tmp_path / "asm.py").write_text("import pefile\n")
    with pytest.raises(AssertionError, match="_MAY_IMPORT"):
        layering(tmp_path)


def test_layering_allows_what_it_declares(tmp_path):
    (tmp_path / "asm.py").write_text("import capstone\nimport struct\n")
    assert layering(tmp_path) == 1


def test_reading_a_text_and_reading_a_path_are_different_calls(tmp_path):
    """The hazard this replaces: `read(source, path)` switched on argument
    COUNT, so `read(path, path)` scanned the FILENAME as source, matched no
    marker and returned `[]`. Overloading on arity cannot be shown to a type
    checker, so it needed a runtime guard; two names need none.
    """
    source = tmp_path / "x.cpp"
    source.write_text(
        "// ORIGINAL: 0x00401000 ?f@@YAXXZ 0x00401000-0x00401004\n")
    assert len(read(source)) == 1
    assert len(read_text(source.read_text(), source)) == 1


# ------------------------------------------- what `python -m decomp` was
#
# These were `decomp/__main__.py`, run as `uv run python -m decomp`. Every
# one of them is an assertion about the package, and this module already
# imported three of them - a test suite reaching into an entry point for its
# own checks, which is the wrong way round. There is one way to run the
# package's checks now, and it is the one that was always running them.

def test_the_tree_parses_to_the_shape_consumers_read():
    """The floor, and the failure this package can actually have in
    isolation: SILENCE. A reader that finds nothing returns `[]`, and every
    count computed from it comes out zero looking like an answer.
    """
    records = read(REPO_ROOT / "src")
    assert len(records) > 5000, len(records)
    assert any(record.state is State.IMPLEMENTED for record in records)
    assert all(isinstance(record.path, Path) and record.path.is_absolute()
               for record in records)
    # A marker names the piece it carries: the migration put a name and
    # image spans on every one, and a fresh annotation must supply both.
    unnamed = [r for r in records if not r.name or not r.image_spans]
    assert not unnamed, \
        f"{len(unnamed)} records without name/spans, first at " \
        f"{unnamed[0].path}:{unnamed[0].line}"


def _code_only(region: str) -> str:
    """The region with comments and blank lines removed."""
    kept, in_block = [], False
    for line in region.splitlines():
        text = line.strip()
        if in_block:
            if "*/" in text:
                in_block = False
                text = text.split("*/", 1)[1].strip()
            else:
                continue
        if "//" in text:
            text = text.split("//", 1)[0].strip()
        if text.startswith("/*"):
            if "*/" not in text:
                in_block = True
            continue
        if not text:
            continue
        kept.append(text)
    return "\n".join(kept)


def _region_code(region: str) -> str:
    """The region's code with the annotation layer removed.

    A region can start mid-comment - a bare marker inside a doc block - and
    `_code_only` cannot know that: it keeps the bare marker line, which is
    annotation, and drops the `//`-prefixed one the writer emits, so the two
    spellings of the SAME layer would compare as different code. Marker
    lines are removed from both sides first; the code underneath is what a
    round trip must preserve.
    """
    lines = [line for line in region.splitlines()
             if not grammar.MARKER.search(line)]
    return _code_only("\n".join(lines))


def _key(record: DecompilationState) -> tuple[object, ...]:
    """Everything a round trip must preserve.

    `line` is NOT in it: it is a position in one text, and a rewrite that
    canonicalises the annotation layer - a wrapped lesson re-emitted on one
    line - legitimately moves every line below. The region is compared
    code-only for the same reason, and the record fields beside it already
    prove the annotations survived. What must never change is the code.
    """
    return (record.address, record.mode, record.state,
            _region_code(record.region), record.byte_exact,
            record.exclusion, record.extract_error, record.recipe,
            record.levers, record.ruled_out, record.unrecoverable,
            record.deferred, record.name, record.image_spans,
            record.symbol)


def roundtrip_tree(root: Path) -> tuple[int, int]:
    """(looped, skipped): files whose annotations survive write -> read.

    Every file with annotations is read, rewritten in memory from its own
    records, and read again; the two parses must agree field for field.
    A skip means `write` refused a record - measured zero since the
    migration gave every annotation an explicit marker, so a new skip is
    an event worth investigating.
    """
    by_path: dict[Path, list[DecompilationState]] = {}
    for record in read(root):
        by_path.setdefault(record.path, []).append(record)
    looped = skipped = 0
    for path, records in by_path.items():
        try:
            rewritten = write(path.read_text(), records)
        except ValueError:
            skipped += 1
            continue
        reread = reader.read_text(rewritten, path)
        if [_key(r) for r in records] != [_key(r) for r in reread]:
            skipped += 1
            continue
        # THE FIXED POINT. The first write may canonicalise - one line per
        # lesson, keywords in one order - and shift every line below; the
        # second write, from the records of the text it produced, must
        # change nothing. Canonical is stable or it is not canonical.
        if write(rewritten, reread) != rewritten:
            skipped += 1
            continue
        looped += 1
    return looped, skipped


# WHAT EACH MODULE MAY IMPORT, and the reason the table exists rather than a
# sentence in `pyproject.toml`. The package used to claim it was standard
# library throughout; `asm` disassembles, so that claim was already false and
# capstone is a declared dependency now. The invariant worth keeping is
# NARROWER and is the one people actually rely on: reading an annotation costs
# nothing but Python. A promise nobody checks decays into a comment; this is
# the check.
_STDLIB_ONLY = ("model", "grammar", "reader", "writer")
# `calls` reads the operands capstone decoded, so it needs the same
# dependency for the type constants and nothing else.
_MAY_IMPORT = {"asm": frozenset({"capstone"}),
               "calls": frozenset({"capstone"})}


def layering(root: Path | None = None) -> int:
    """Every module's module-level imports, held to what it may depend on.

    Returns the number of modules checked. Two rules, and the second is the
    one that keeps the first honest.

    WHAT a module may import comes from `_MAY_IMPORT`, with the readers held
    to the standard library so that reading an annotation needs nothing
    installed.

    WHERE it may import it is: at module level, always. A lazy import is a
    dependency that does not show up in the import block, so it escapes the
    rule above and moves its own failure from startup into the middle of a
    measurement - `asm` hid capstone in `_disasm` for exactly as long as the
    package claimed to be standard library throughout. Declared dependencies
    are imported like anything else.
    """
    checked = 0
    root = root or Path(decomp.__file__).resolve().parent
    for path in sorted(root.glob("*.py")):
        module = path.stem
        if module == "__main__":
            continue
        allowed = _MAY_IMPORT.get(module, frozenset())
        tree = ast.parse(path.read_text())
        top = {id(node) for node in ast.walk(tree)
               if isinstance(node, (ast.Import, ast.ImportFrom))
               and any(node is child for child in tree.body)}
        for node in ast.walk(tree):
            if not isinstance(node, (ast.Import, ast.ImportFrom)):
                continue
            assert id(node) in top, (
                f"{path.name} line {node.lineno} imports inside a function; "
                f"every import in this package belongs at module level, so "
                f"that what a module costs to import is what it says it does")

            if isinstance(node, ast.Import):
                roots = [alias.name.split(".")[0] for alias in node.names]
            elif isinstance(node, ast.ImportFrom):
                if node.level:            # `from .model import ...`
                    continue
                roots = [(node.module or "").split(".")[0]]
            else:
                continue
            for name in roots:
                if name in sys.stdlib_module_names or name in allowed:
                    continue
                assert module not in _STDLIB_ONLY, (
                    f"{path.name} imports {name!r} at module level; "
                    f"{module} is one of the readers and must stay standard "
                    f"library so that reading an annotation needs nothing "
                    f"installed")
                raise AssertionError(
                    f"{path.name} imports {name!r} at module level, which "
                    f"_MAY_IMPORT does not allow for {module}")
        checked += 1
    assert checked, "no modules checked - the glob found nothing"
    return checked


def test_a_body_fact_must_name_a_path_not_a_word():
    """`FACT_LINE`'s own comment records this lesson for `calls`: a wrapped
    sentence re-flowed onto its own line begins `// ` plus a word, and the
    key alone cannot tell prose from a fact. `body needs.` is one token and
    is the tail of a sentence in src/recovered/units/005e8fa5.cpp."""
    prose = reader.read_text(
        "// ORIGINAL: 0x00401000 ?f@@YAXXZ 0x00401000-0x00401008\n"
        "// body needs.\n", Path("x.cpp"))
    assert prose[0].body == ""
    real = reader.read_text(
        "// ORIGINAL: 0x00401000 ?f@@YAXXZ 0x00401000-0x00401008\n"
        "// body      src/thing.h\n", Path("x.cpp"))
    assert real[0].body == "src/thing.h"


# ------------------------------------------------- qualified_name

@pytest.mark.parametrize("mangled, expected", [
    ("?get_rgbquad@Palette@@QAEHPAURGBQUAD@@HH@Z", "Palette::get_rgbquad"),
    ("?init_class@Win@@SAHPAD@Z", "Win::init_class"),
    ("??0Buffer@@QAE@XZ", "Buffer::Buffer"),
    ("??1Buffer@@QAE@XZ", "Buffer::~Buffer"),
    ("??0Inner@Outer@@QAE@XZ", "Outer::Inner::Inner"),
    ("??1Inner@Outer@@QAE@XZ", "Outer::Inner::~Inner"),
    ("??_GBuffer@@UAEPAXI@Z", "Buffer::`scalar deleting destructor'"),
    ("??__Eg_PALETTE1@@YAXXZ", "`dynamic initializer for 'g_PALETTE1''"),
    ("??__FTxt@@YAXXZ", "`dynamic atexit destructor for 'Txt''"),
    ("?f@A@B@@QAEXXZ", "B::A::f"),
    ("_WinMain@16", "WinMain"),
    ("sub_628180", "sub_628180"),
])
def test_qualified_name_reads_the_scope_chain(mangled, expected):
    from decomp.mangled import qualified_name
    assert qualified_name(mangled) == expected


@pytest.mark.parametrize("mangled", [
    "??2@YAPAXI@Z",          # operator new: a form this image does not carry
    "?1bad@@YAXXZ",          # a piece that is neither a name nor a digit
    "",
])
def test_qualified_name_says_it_cannot_tell(mangled):
    """The module's contract everywhere: "" rather than a guess. MSVC's
    operator table is real and simply does not appear in this image, so
    inventing entries for it would be untested code that reads as fact."""
    from decomp.mangled import qualified_name
    assert qualified_name(mangled) == ""
