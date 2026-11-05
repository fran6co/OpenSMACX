"""Pure-byte controls for the generic object annotation API."""

from dataclasses import replace

import pytest

from match_recovery.model import Origin, Span
from match_recovery.object_source import ObjectBinding, parse_objects
from match_recovery.object_writer import replace_object_records, write_object_bytes


PIN = "a" * 64
SPACE = "pe:va"
PATH = "src/data.cpp"


def binding(address: int, symbol: str, line: int, *, end: int | None = None,
            evidence: int | None = None) -> ObjectBinding:
    origin = Origin(PIN, SPACE, address)
    if end is None:
        return ObjectBinding(origin, symbol, PATH, line)
    return ObjectBinding(origin, symbol, PATH, line, Span(address, end),
                         Origin(PIN, SPACE, evidence if evidence is not None else address))


def current(raw: bytes) -> tuple[ObjectBinding, ...]:
    return parse_objects(raw.decode("utf-8", errors="surrogateescape"), PATH,
                         PIN, address_space=SPACE)


def test_add_v1_unknown_extent_and_v2_explicit_extent_round_trip_without_io():
    raw = b"int first;\r\nint second;\r\n"
    first = binding(0x007D3B74, "_fixture_arid", 1)
    second = binding(0x00685684, "_fixture_direction", 2,
                     end=0x00685779, evidence=0x005927D0)
    written = write_object_bytes(raw, expected=(), additions=(first, second),
                                 path=PATH, pin=PIN, address_space=SPACE)
    parsed = current(written)
    assert len(parsed) == 2
    assert {row.symbol: row.span for row in parsed} == {
        first.symbol: None,
        second.symbol: Span(0x00685684, 0x00685779),
    }
    assert all(row.evidence is None for row in parsed if row.span is None)
    assert b"\r\n" in written and b"\nint first;\r\n" in written
    assert raw.endswith(b"int second;\r\n")


@pytest.mark.parametrize("symbol", ["x\n// injected", "x//comment", "x\\\nnext", "bad token", "x\x7f"])
def test_rejects_symbol_injection(symbol):
    raw = b"int value;\n"
    proposed = binding(0x401000, symbol, 1)
    with pytest.raises(ValueError, match="annotation-safe"):
        write_object_bytes(raw, expected=(), additions=(proposed,), path=PATH,
                           pin=PIN, address_space=SPACE)


def test_rejects_stale_current_snapshot_and_existing_identity_conflicts():
    raw = b"// RECOVERY_OBJECT: v1 0x00401000 _old\nint old;\n"
    add = binding(0x00402000, "_new", 2)
    with pytest.raises(ValueError, match="current complete source snapshot"):
        write_object_bytes(raw, expected=(), additions=(add,), path=PATH,
                           pin=PIN, address_space=SPACE)
    existing = current(raw)
    same_address = binding(0x00401000, "_other", 2)
    with pytest.raises(ValueError, match="conflicts"):
        write_object_bytes(raw, expected=existing, additions=(same_address,),
                           path=PATH, pin=PIN, address_space=SPACE)
    same_symbol = binding(0x00402000, "_old", 2)
    with pytest.raises(ValueError, match="conflicts"):
        write_object_bytes(raw, expected=existing, additions=(same_symbol,),
                           path=PATH, pin=PIN, address_space=SPACE)


def test_rejects_wrong_owner_image_or_insertion_anchor():
    raw = b"int value;\n"
    candidate = binding(0x401000, "_new", 2)
    with pytest.raises(ValueError, match="another source unit"):
        write_object_bytes(raw, expected=(), additions=(replace(candidate, unit="src/other.cpp"),),
                           path=PATH, pin=PIN, address_space=SPACE)
    wrong_origin = replace(candidate, origin=Origin("b" * 64, SPACE, 0x401000))
    with pytest.raises(ValueError, match="another image"):
        write_object_bytes(raw, expected=(), additions=(wrong_origin,), path=PATH,
                           pin=PIN, address_space=SPACE)
    with pytest.raises(ValueError, match="outside the source"):
        write_object_bytes(raw, expected=(), additions=(replace(candidate, line=3),),
                           path=PATH, pin=PIN, address_space=SPACE)


def test_rejects_intra_batch_duplicates_overlap_and_existing_marker_shadow():
    raw = b"int first;\nint second;\n"
    same_address = (binding(0x400FF0, "_same_a", 1),
                    binding(0x400FF0, "_same_b", 2))
    with pytest.raises(ValueError, match="same original start"):
        write_object_bytes(raw, expected=(), additions=same_address, path=PATH,
                           pin=PIN, address_space=SPACE)
    one = binding(0x401000, "_one", 1, end=0x401010)
    duplicate = binding(0x40100F, "_two", 2, end=0x401020)
    with pytest.raises(ValueError, match="overlap"):
        write_object_bytes(raw, expected=(), additions=(one, duplicate), path=PATH,
                           pin=PIN, address_space=SPACE)
    conflict = replace(duplicate, origin=Origin(PIN, SPACE, 0x401008),
                       span=Span(0x401008, 0x401020))
    with pytest.raises(ValueError, match="overlap"):
        write_object_bytes(raw, expected=(), additions=(one, conflict), path=PATH,
                           pin=PIN, address_space=SPACE)
    marker_raw = b"// RECOVERY_OBJECT: v1 0x00403000 _old\nint old;\n"
    marker_binding = binding(0x00404000, "_new", 1)
    with pytest.raises(ValueError, match="shadow"):
        write_object_bytes(marker_raw, expected=current(marker_raw),
                           additions=(marker_binding,), path=PATH, pin=PIN,
                           address_space=SPACE)


@pytest.mark.parametrize("child_first", [False, True])
def test_bounded_parent_and_same_start_unbounded_member_round_trip_in_either_order(child_first):
    start = 0x00401000
    origin = Origin(PIN, SPACE, start)
    parent = ObjectBinding(origin, "_parent", PATH, 1, Span(start, start + 16), origin)
    child = ObjectBinding(origin, "old_first", PATH, 1,
                          parent_symbol="_parent", member_path=("first",))
    additions = (child, parent) if child_first else (parent, child)
    written = write_object_bytes(b"int aggregate;\n", expected=(), additions=additions,
                                 path=PATH, pin=PIN, address_space=SPACE)
    parsed = current(written)
    assert [row.symbol for row in parsed] == [row.symbol for row in additions]
    assert parsed[0].origin == parsed[1].origin == origin
    assert next(row for row in parsed if row.symbol == "_parent").span == Span(start, start + 16)


@pytest.mark.parametrize("damage", ["wrong_owner", "wrong_unit", "wrong_domain",
                                     "missing_bounds", "two_children", "target_alias",
                                     "bounded_child"])
def test_same_start_parent_member_exception_rejects_every_noncanonical_pair(damage):
    start = 0x00401000
    origin = Origin(PIN, SPACE, start)
    parent = ObjectBinding(origin, "_parent", PATH, 1, Span(start, start + 16), origin)
    child = ObjectBinding(origin, "old_first", PATH, 2,
                          parent_symbol="_parent", member_path=("first",))
    if damage == "wrong_owner":
        child = replace(child, parent_symbol="_other")
    elif damage == "wrong_unit":
        child = replace(child, unit="src/other.cpp")
    elif damage == "wrong_domain":
        child = replace(child, origin=Origin("b" * 64, SPACE, start))
    elif damage == "missing_bounds":
        parent = binding(start, "_parent", 1)
    elif damage == "two_children":
        second = replace(child, symbol="old_second", member_path=("second",))
        with pytest.raises(ValueError, match="same original start|parent member|unit|image"):
            write_object_bytes(b"int aggregate;\n", expected=(),
                               additions=(parent, child, second), path=PATH,
                               pin=PIN, address_space=SPACE)
        return
    elif damage == "target_alias":
        parent = ObjectBinding(origin, "old_parent", PATH, 1,
                               Span(start, start + 16), origin, target_symbol="_parent")
    elif damage == "bounded_child":
        child = ObjectBinding(origin, "old_first", PATH, 2,
                              Span(start, start + 4), origin,
                              parent_symbol="_parent", member_path=("first",))
    with pytest.raises(ValueError):
        write_object_bytes(b"int aggregate;\n", expected=(), additions=(parent, child),
                           path=PATH, pin=PIN, address_space=SPACE)


def test_round_trip_keeps_existing_record_and_adds_start_only_array():
    raw = b"// RECOVERY_OBJECT: v1 0x00401000 _existing\nint existing;\n"
    parsed = current(raw)
    add = binding(0x007F65D0, "_fixture_moist", 2)
    written = write_object_bytes(raw, expected=parsed, additions=(add,),
                                 path=PATH, pin=PIN, address_space=SPACE)
    result = current(written)
    assert [row.symbol for row in result] == [parsed[0].symbol, add.symbol]
    assert result[0].origin == parsed[0].origin
    assert result[0].line == 1 and result[1].line == 2

@pytest.mark.parametrize("newline", [b"\n", b"\r\n", b"\r"])
def test_eof_without_terminal_newline_keeps_source_and_separates_marker(newline):
    raw = b"int first;" + newline + b"int last;"
    result = write_object_bytes(raw, expected=(), additions=(binding(0x403000, "_new", 3),),
                                path=PATH, pin=PIN, address_space=SPACE)
    assert result.startswith(raw + newline)
    assert current(result)[0].symbol == "_new"


@pytest.mark.parametrize("raw", [b"/* comment\nint value;\n*/\n", b"/* unterminated\n"])
def test_comment_anchors_refuse_even_when_parser_would_read_the_inserted_marker(raw):
    with pytest.raises(ValueError, match="block comment"):
        write_object_bytes(raw, expected=(), additions=(binding(0x403000, "_new", 2),),
                           path=PATH, pin=PIN, address_space=SPACE)


@pytest.mark.parametrize("raw", [b"int x; \\\r\nint y;\r\n", b"int x; ??/\nint y;\n"])
def test_line_splices_refuse_including_crlf_and_trigraphs(raw):
    with pytest.raises(ValueError, match="line-spliced"):
        write_object_bytes(raw, expected=(), additions=(binding(0x403000, "_new", 2),),
                           path=PATH, pin=PIN, address_space=SPACE)


@pytest.mark.parametrize("newline", [b"\n", b"\r\n", b"\r"])
@pytest.mark.parametrize("splice", [b"\\", b"??/", b"\\ \t"])
def test_add_before_untouched_spliced_suffix_preserves_all_bytes(newline, splice):
    prefix = b"int first;" + newline + b"int second;" + newline
    suffix = b"#define VALUE " + splice + newline + b"  42" + newline
    raw = prefix + suffix
    additions = (binding(0x403000, "_first", 1), binding(0x403004, "_second", 2))
    written = write_object_bytes(raw, expected=current(raw), additions=additions,
                                 path=PATH, pin=PIN, address_space=SPACE)
    assert written.endswith(suffix)
    assert len(current(written)) == 2
    without_markers = b"".join(line for line in written.splitlines(keepends=True)
                                if not line.startswith(b"// RECOVERY_OBJECT:"))
    assert without_markers == raw


@pytest.mark.parametrize("anchor", [3, 4, 5])
def test_spliced_suffix_and_eof_anchors_still_refuse(anchor):
    raw = b"int first;\nint second;\n#define VALUE \\\n  42\n"
    # One safe insertion cannot make an unsafe batch peer acceptable.
    additions = (binding(0x403000, "_first", 1), binding(0x403004, "_unsafe", anchor))
    with pytest.raises(ValueError, match="line-spliced"):
        write_object_bytes(raw, expected=current(raw), additions=additions,
                           path=PATH, pin=PIN, address_space=SPACE)


def test_prefix_comment_anchors_still_refuse_with_later_splice():
    raw = b"/* comment\nint first;\n*/\n#define VALUE \\\n  42\n"
    with pytest.raises(ValueError, match="block comment"):
        write_object_bytes(raw, expected=current(raw),
                           additions=(binding(0x403000, "_first", 2),),
                           path=PATH, pin=PIN, address_space=SPACE)


def test_unbounded_start_cannot_overlap_an_independently_bounded_object():
    bounded = binding(0x403000, "_bounded", 1, end=0x403020)
    start = binding(0x403010, "_start", 2)
    with pytest.raises(ValueError, match="inside another"):
        write_object_bytes(b"int first;\nint second;\n", expected=(), additions=(bounded, start),
                           path=PATH, pin=PIN, address_space=SPACE)


def test_explicit_unbounded_member_start_may_be_inside_its_direct_parent_only():
    parent_symbol = "?InterfaceRegistry@@3PAPAVSubInterface@@A"
    parent_origin = Origin(PIN, SPACE, 0x007AE7F0)
    parent = ObjectBinding(parent_origin, parent_symbol, PATH, 1,
                           Span(0x007AE7F0, 0x007AE820), parent_origin)
    child = ObjectBinding(Origin(PIN, SPACE, 0x007AE818),
                          "?BattleWinInitReceiver@@3PAVBattleWin@@A", PATH, 2,
                          parent_symbol=parent_symbol, member_path=("[10]",))
    result = write_object_bytes(b"SubInterface *InterfaceRegistry[12];\nint x;\n",
                                expected=(), additions=(parent, child), path=PATH,
                                pin=PIN, address_space=SPACE)
    parsed = current(result)
    assert [(row.symbol, row.parent_symbol, row.member_path) for row in parsed] == [
        (parent_symbol, None, ()), (child.symbol, parent_symbol, ("[10]",))]
    assert parsed[1].span is None


def test_member_inside_unrelated_parent_does_not_receive_overlap_exception():
    owner = binding(0x007AE7F0, "_owner", 1, end=0x007AE820)
    child = ObjectBinding(Origin(PIN, SPACE, 0x007AE818), "_old", PATH, 2,
                          parent_symbol="_different_owner", member_path=("[10]",))
    with pytest.raises(ValueError, match="inside another"):
        write_object_bytes(b"int owner;\nint child;\n", expected=(),
                           additions=(owner, child), path=PATH, pin=PIN,
                           address_space=SPACE)


@pytest.mark.parametrize("raw", [b'const char *s = R"(literal)";\n', b"\xef\xbb\xbfint x;\n"])
def test_unsupported_lexical_forms_refuse(raw):
    with pytest.raises(ValueError):
        write_object_bytes(raw, expected=(), additions=(binding(0x403000, "_new", 1),),
                           path=PATH, pin=PIN, address_space=SPACE)


def test_ordinary_literal_with_raw_like_text_is_not_a_raw_string():
    raw = b'const char *s = "FATAL ERROR";\n'
    written = write_object_bytes(raw, expected=(), additions=(binding(0x403000, "_new", 1),),
                                 path=PATH, pin=PIN, address_space=SPACE)
    assert written.endswith(raw)


def test_v4_direct_target_round_trip_preserves_old_identity_and_bounded_evidence():
    origin = Origin(PIN, SPACE, 0x401000)
    old = ObjectBinding(origin, "?Old@@3HA", PATH, 1, Span(0x401000, 0x401004),
                        Origin(PIN, SPACE, 0x402000), target_symbol="?Current@@3HA")
    written = write_object_bytes(b"int value;\n", expected=(), additions=(old,),
                                 path=PATH, pin=PIN, address_space=SPACE)
    parsed, = current(written)
    assert parsed.symbol == old.symbol
    assert parsed.target_symbol == old.target_symbol
    assert parsed.emitted_symbol == old.target_symbol
    assert parsed.interpretation_id == old.interpretation_id
    assert b"RECOVERY_OBJECT: v4 0x00401000 0x00401004 ?Old@@3HA 0x00402000 target ?Current@@3HA" in written


def test_replace_direct_target_is_guarded_and_does_not_rewrite_history():
    raw = b"// RECOVERY_OBJECT: v2 0x00401000 0x00401004 _old 0x00402000\nint old;\n"
    old, = current(raw)
    replacement = replace(old, line=2, target_symbol="_current")
    output = replace_object_records({PATH: raw}, expected=(old,), replacements=(replacement,),
                                    destination=PATH, pin=PIN, address_space=SPACE)
    updated, = current(output[PATH])
    assert updated.symbol == old.symbol and updated.origin == old.origin
    assert updated.span == old.span and updated.evidence == old.evidence
    assert updated.interpretation_id == old.interpretation_id
    assert updated.target_symbol == "_current"
    assert b"RECOVERY_OBJECT: v4" in output[PATH]


@pytest.mark.parametrize("splice", (b"\\\n", b"\\\r\n", b"??/\n"))
def test_replace_before_untouched_spliced_suffix(splice):
    prefix=b"// RECOVERY_OBJECT: v1 0x00401000 _old\nint old;\n"
    peer=b"// RECOVERY_OBJECT: v1 0x00402000 _peer\nint peer;\n"
    suffix=b"#define MACRO "+splice+b"    1\n"+peer
    raw=prefix+suffix
    old, untouched=current(raw)
    replacement=replace(old,line=2,target_symbol="_current")
    output=replace_object_records({PATH:raw},expected=(old,untouched),replacements=(replacement,),
                                  destination=PATH,pin=PIN,address_space=SPACE)[PATH]
    assert output.endswith(suffix)
    updated,retained=current(output)
    assert updated.origin==old.origin and updated.symbol==old.symbol
    assert updated.target_symbol=="_current"
    assert retained.origin==untouched.origin and retained.symbol==untouched.symbol
    assert retained.target_symbol==untouched.target_symbol


@pytest.mark.parametrize("splice", (b"\\\n", b"\\\r\n", b"??/\n"))
def test_replace_refuses_anchor_at_or_after_first_splice(splice):
    raw=b"// RECOVERY_OBJECT: v1 0x00401000 _old\nint old;\n#define M "+splice+b"1\n"
    old,=current(raw)
    for line in (3,4,5):
        with pytest.raises(ValueError,match="line-spliced"):
            replace_object_records({PATH:raw},expected=(old,),
                                   replacements=(replace(old,line=line,target_symbol="_current"),),
                                   destination=PATH,pin=PIN,address_space=SPACE)


def test_replace_refuses_object_records_after_splice():
    raw=b"#define M \\\n1\n// RECOVERY_OBJECT: v1 0x00401000 _old\nint old;\n"
    old,=current(raw)
    with pytest.raises(ValueError,match="line-spliced"):
        replace_object_records({PATH:raw},expected=(old,),
                               replacements=(replace(old,line=1,target_symbol="_current"),),
                               destination=PATH,pin=PIN,address_space=SPACE)


def test_replace_direct_target_rejects_target_collisions_and_stale_snapshot():
    raw = (b"// RECOVERY_OBJECT: v1 0x00401000 _old\nint old;\n"
           b"// RECOVERY_OBJECT: v1 0x00402000 _occupied\nint occupied;\n")
    first, second = current(raw)
    collision = replace(first, line=2, target_symbol="_occupied")
    with pytest.raises(ValueError, match="current object target"):
        replace_object_records({PATH: raw}, expected=(first, second), replacements=(collision,),
                               destination=PATH, pin=PIN, address_space=SPACE)
    with pytest.raises(ValueError, match="complete current source snapshot"):
        replace_object_records({PATH: raw}, expected=(first,), replacements=(replace(first, line=2,
                               target_symbol="_new"),), destination=PATH, pin=PIN,
                               address_space=SPACE)


def test_replace_alias_root_can_share_a_current_emitted_member_owner():
    raw = (b"// RECOVERY_OBJECT: v1 0x00401000 _old_root\n"
           b"int root;\n"
           b"// RECOVERY_OBJECT: v3 0x00401004 _old_child member _typed_root field\n"
           b"int child;\n")
    root, child = current(raw)
    replacement = replace(root, line=1, target_symbol="_typed_root")
    output = replace_object_records({PATH: raw}, expected=(root, child),
                                    replacements=(replacement,), destination=PATH,
                                    pin=PIN, address_space=SPACE)
    result = current(output[PATH])
    assert result[0].symbol == "_old_root" and result[0].emitted_symbol == "_typed_root"
    assert result[1].parent_symbol == result[0].emitted_symbol


@pytest.mark.parametrize("bounded", [False, True])
def test_explicit_member_to_direct_target_promotion_preserves_identity_and_peer(bounded):
    if bounded:
        member_marker = (b"// RECOVERY_OBJECT: v3 0x00401000 0x00401008 "
                        b"_old_sprite 0x00402000 member _storage sprites\n")
    else:
        member_marker = (b"// RECOVERY_OBJECT: v3 0x00401000 _old_sprite "
                        b"member _storage sprites\n")
    peer_marker = b"// RECOVERY_OBJECT: v1 0x00403000 _untouched\n"
    raw = member_marker + b"int sprite_storage;\n" + peer_marker + b"int peer;\n"
    old, peer = current(raw)
    replacement = replace(old, line=2, parent_symbol=None, member_path=(),
                          target_symbol="_sprite_class_storage")
    output = replace_object_records(
        {PATH: raw}, expected=(old, peer), replacements=(replacement,),
        destination=PATH, pin=PIN, address_space=SPACE,
        direct_owner_promotions=frozenset({old.origin.entity_id}))[PATH]

    updated, retained = current(output)
    assert updated.symbol == old.symbol
    assert updated.origin == old.origin
    assert updated.span == old.span and updated.evidence == old.evidence
    if bounded:
        assert updated.interpretation_id == old.interpretation_id
    assert updated.parent_symbol is None and updated.member_path == ()
    assert updated.target_symbol == "_sprite_class_storage"
    assert retained == peer
    assert output.endswith(b"int sprite_storage;\n" + peer_marker + b"int peer;\n")


def test_member_to_direct_target_promotion_remains_denied_without_grant():
    raw = b"// RECOVERY_OBJECT: v3 0x00401000 _old member _storage sprites\nint storage;\n"
    old, = current(raw)
    replacement = replace(old, line=2, parent_symbol=None, member_path=(),
                          target_symbol="_direct")
    with pytest.raises(ValueError, match="cannot discard its current owner"):
        replace_object_records({PATH: raw}, expected=(old,), replacements=(replacement,),
                               destination=PATH, pin=PIN, address_space=SPACE)


@pytest.mark.parametrize("grant, replacement_kind, message", [
    ([], "promotion", "frozenset"),
    (["not-an-entity-id"], "promotion", "frozenset"),
    ("wrong-shape", "promotion", "frozenset"),
    (frozenset({"b" * 64}), "promotion", "cannot discard|exactly"),
    ("use_identity", "retained_member", "does not select"),
    ("use_identity", "nonmember", "does not select"),
    ("use_identity", "null_target", "nonempty direct target"),
])
def test_direct_owner_promotion_grants_are_exact_and_one_use(grant, replacement_kind, message):
    raw = b"// RECOVERY_OBJECT: v3 0x00401000 _old member _storage sprites\nint storage;\n"
    old, = current(raw)
    if replacement_kind == "promotion":
        replacement = replace(old, line=2, parent_symbol=None, member_path=(),
                              target_symbol="_direct")
    elif replacement_kind == "retained_member":
        replacement = replace(old, line=2)
    elif replacement_kind == "nonmember":
        direct_raw = b"// RECOVERY_OBJECT: v1 0x00401000 _old\nint storage;\n"
        old, = current(direct_raw)
        raw = direct_raw
        replacement = replace(old, line=2, target_symbol="_direct")
    else:
        replacement = replace(old, line=2, parent_symbol=None, member_path=(),
                              target_symbol=None)
    if grant == "use_identity":
        grant = frozenset({old.origin.entity_id})
    with pytest.raises(ValueError, match=message):
        replace_object_records({PATH: raw}, expected=(old,), replacements=(replacement,),
                               destination=PATH, pin=PIN, address_space=SPACE,
                               direct_owner_promotions=grant)


@pytest.mark.parametrize("change", ["symbol", "span", "evidence"])
def test_direct_owner_promotion_cannot_change_historical_identity_or_bounds(change):
    raw = (b"// RECOVERY_OBJECT: v3 0x00401000 0x00401008 _old "
           b"0x00402000 member _storage sprites\nint storage;\n")
    old, = current(raw)
    replacement = replace(old, line=2, parent_symbol=None, member_path=(),
                          target_symbol="_direct")
    if change == "symbol":
        replacement = replace(replacement, symbol="_different")
    elif change == "span":
        replacement = replace(replacement, span=Span(0x00401000, 0x0040100C))
    else:
        replacement = replace(replacement, evidence=Origin(PIN, SPACE, 0x00403000))
    with pytest.raises(ValueError, match="historical identity or initializer obligation"):
        replace_object_records({PATH: raw}, expected=(old,), replacements=(replacement,),
                               destination=PATH, pin=PIN, address_space=SPACE,
                               direct_owner_promotions=frozenset({old.origin.entity_id}))


def test_direct_owner_promotion_keeps_complete_snapshot_and_target_collision_guards():
    raw = (b"// RECOVERY_OBJECT: v3 0x00401000 _old member _storage sprites\nint storage;\n"
           b"// RECOVERY_OBJECT: v1 0x00403000 _occupied\nint occupied;\n")
    old, occupied = current(raw)
    replacement = replace(old, line=2, parent_symbol=None, member_path=(),
                          target_symbol="_direct")
    with pytest.raises(ValueError, match="complete current source snapshot"):
        replace_object_records({PATH: raw}, expected=(old,), replacements=(replacement,),
                               destination=PATH, pin=PIN, address_space=SPACE,
                               direct_owner_promotions=frozenset({old.origin.entity_id}))
    collision = replace(replacement, target_symbol=occupied.symbol)
    with pytest.raises(ValueError, match="target conflicts|multiple original objects|same current object|one current object target"):
        replace_object_records({PATH: raw}, expected=(old, occupied), replacements=(collision,),
                               destination=PATH, pin=PIN, address_space=SPACE,
                               direct_owner_promotions=frozenset({old.origin.entity_id}))
