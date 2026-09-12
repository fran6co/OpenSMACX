"""Optional map tables preserve placements, never inferred ownership coverage."""

import hashlib
import os
from pathlib import Path

import pytest

from match_recovery.msvc_link_map import LinkMap, SectionContribution, parse_link_map
from match_recovery.pe_coff import PeImage
from test_link_map import MAP, executable, parsed_image


CONTRIBUTIONS = (" Start         Length     Name                   Class\n"
                 " 0001:00000000 00000004H .CRT$XCA DATA\n"
                 " 0001:00000004 00000008H .CRT$XCU DATA\n"
                 " 0001:0000000c 00000004H .CRT$XCZ DATA\n")
STATICS = (" 0001:00000030 _$E19 00401030 f first.cpp.obj\n"
           " 0001:00000040 _$E19 00401040 f second.cpp.obj\n")
LINES = (" Line numbers for first.obj(q:\\sources\\sample.cpp) segment .text\n"
         " 12 0001:00000000 13 0001:00000002\n"
         " 12 0001:00000004\n")


def fixture():
    return MAP.replace(" Address", CONTRIBUTIONS + " Address") + STATICS


def parse(text):
    return parse_link_map(text.encode("ascii"))


def test_optional_tables_preserve_exact_rows_and_distinguish_absence_from_empty():
    absent = parse(MAP.replace(" Static symbols\n", ""))
    assert absent.statics is None and absent.contributions is None
    empty = parse(MAP.replace(" Address", CONTRIBUTIONS.splitlines()[0] + "\n Address"))
    assert empty.statics == empty.contributions == ()
    observed = parse(fixture().replace("\n", "\r\n"))
    assert observed.publics == absent.publics
    assert observed.contributions == (
        SectionContribution(1, 0, 4, ".CRT$XCA", "DATA"),
        SectionContribution(1, 4, 8, ".CRT$XCU", "DATA"),
        SectionContribution(1, 12, 4, ".CRT$XCZ", "DATA"))
    assert [(item.symbol, item.owner, item.address) for item in observed.statics] == [
        ("_$E19", "first.cpp.obj", 0x401030), ("_$E19", "second.cpp.obj", 0x401040)]
    observed.validate(parsed_image(executable()))
    assert LinkMap(absent.base, absent.entry, absent.publics).statics is None


def test_static_names_are_case_sensitive_but_owner_identity_is_case_insensitive():
    observed = parse(fixture() + " 0001:00000050 _$e19 00401050 f FIRST.CPP.OBJ\n")
    assert len(observed.statics) == 3
    with pytest.raises(ValueError, match="duplicate"):
        parse(fixture() + " 0001:00000050 _$E19 00401050 f FIRST.CPP.OBJ\n")


def test_alias_addresses_and_distinct_static_owners_are_not_collapsed():
    text = fixture().replace("0001:00000040 _$E19 00401040", "0001:00000030 _$E19 00401030")
    observed = parse(text)
    observed.validate(parsed_image(executable()))
    assert len(observed.statics) == 2 and observed.statics[0].address == observed.statics[1].address


@pytest.mark.parametrize("damage", ["bad_offset", "bad_address", "missing_owner", "bad_flag", "duplicate_flag",
                                   "duplicate_row", "duplicate_table", "bad_header", "trailing_garbage",
                                   "row_after_icf", "duplicate_icf", "truncated_row"])
def test_observed_static_tables_never_silently_drop_malformed_rows(damage):
    text = fixture()
    if damage == "bad_offset": text = text.replace("0001:00000030", "0001:bad")
    elif damage == "bad_address": text = text.replace("00401030", "bad-address")
    elif damage == "missing_owner": text = text.replace(" f first.cpp.obj", " f")
    elif damage == "bad_flag": text = text.replace(" f first.cpp.obj", " x first.cpp.obj")
    elif damage == "duplicate_flag": text = text.replace(" f first.cpp.obj", " f f first.cpp.obj")
    elif damage == "duplicate_row": text += STATICS.splitlines()[0] + "\n"
    elif damage == "duplicate_table": text += " Static symbols\n"
    elif damage == "bad_header": text = text.replace("Static symbols", "Static symbols BAD")
    elif damage == "trailing_garbage": text += " not-a-static-row\n"
    elif damage == "row_after_icf": text += " 10 bytes saved through ICF\n" + STATICS.splitlines()[0] + "\n"
    elif damage == "duplicate_icf": text += " 10 bytes saved through ICF\n 10 bytes saved through ICF\n"
    else: text += " 0001:00000060 _$E20\n"
    with pytest.raises(ValueError): parse(text)


@pytest.mark.parametrize("damage", ["bad_offset", "bad_size", "missing_kind", "extra_field", "duplicate_row",
                                   "duplicate_identity", "duplicate_table", "bad_header", "late_table", "row_without_table"])
def test_observed_contributions_refuse_malformed_or_duplicate_evidence(damage):
    text = fixture()
    if damage == "bad_offset": text = text.replace("0001:00000004 00000008H", "0001:ZZZZZZZZ 00000008H")
    elif damage == "bad_size": text = text.replace("00000008H", "00000008")
    elif damage == "missing_kind": text = text.replace(".CRT$XCU DATA", ".CRT$XCU")
    elif damage == "extra_field": text = text.replace(".CRT$XCU DATA", ".CRT$XCU DATA EXTRA")
    elif damage == "duplicate_row": text = text.replace(" Address", CONTRIBUTIONS.splitlines()[2] + "\n Address")
    elif damage == "duplicate_identity": text = text.replace(" Address", " 0001:00000004 00000004H .CRT$XCU CODE\n Address")
    elif damage == "duplicate_table": text = text.replace(" Address", CONTRIBUTIONS + " Address")
    elif damage == "bad_header": text = text.replace("Start         Length", "Stort         Length")
    elif damage == "late_table": text += CONTRIBUTIONS
    else: text = text.replace(CONTRIBUTIONS.splitlines()[0] + "\n", "")
    with pytest.raises(ValueError): parse(text)


@pytest.mark.parametrize("old,new", [("0001:00000030", "0002:00000030"),
                                    ("00401030", "00401031"), ("0001:00000030", "0001:00000200")])
def test_static_placements_require_actual_pe_coordinates(old, new):
    with pytest.raises(ValueError): parse(fixture().replace(old, new)).validate(parsed_image(executable()))


@pytest.mark.parametrize("damage", ["segment_zero", "segment_missing", "extent", "offset", "overlap"])
def test_contribution_ranges_require_nonoverlapping_actual_pe_sections(damage):
    text = fixture()
    if damage == "segment_zero": text = text.replace("0001:00000004 00000008H", "0000:00000004 00000008H")
    elif damage == "segment_missing": text = text.replace("0001:00000004 00000008H", "0002:00000004 00000008H")
    elif damage == "extent": text = text.replace("00000008H", "00000200H")
    elif damage == "offset": text = text.replace("0001:00000004 00000008H", "0001:00000200 00000008H")
    else: text = text.replace("0001:00000004 00000008H", "0001:00000002 00000008H")
    with pytest.raises(ValueError): parse(text).validate(parsed_image(executable()))


def test_empty_contribution_rows_and_adjacent_extents_are_valid_without_coverage_inference():
    text = fixture().replace(" Address", " 0001:00000004 00000000H .empty DATA\n Address")
    mapping = parse(text)
    mapping.validate(parsed_image(executable()))
    assert mapping.contributions[-1].size == 0


@pytest.mark.parametrize("before_static", [False, True])
def test_explicit_line_number_transitions_and_icf_preserve_static_table(before_static):
    text = fixture()
    text = text.replace(" Static symbols", LINES + " Static symbols") if before_static else text + LINES
    text += "\n 3895 bytes saved through ICF\n"
    observed = parse(text)
    assert len(observed.statics) == 2 and len(observed.contributions) == 3
    observed.validate(parsed_image(executable()))


@pytest.mark.parametrize("extra", [LINES, " 12 bad-coordinate\n", " 12 0001:00000000\n",
                                   " Line numbers for malformed\n", " 0001:00000080 _lost 00401080 f object.obj\n"])
def test_line_tables_cannot_hide_malformed_duplicate_or_unheaded_symbol_rows(extra):
    with pytest.raises(ValueError): parse(fixture() + LINES + extra)


def test_absolute_static_symbols_remain_unplaced():
    observed = parse(fixture() + " 0000:00000030 local_absolute 00000030 first.obj\n")
    observed.validate(parsed_image(executable()))
    assert observed.statics[-1].segment == 0


@pytest.mark.skipif(not os.environ.get("MATCH_RECOVERY_LINK_MAP_BUILD"), reason="optional existing map/image evidence")
def test_existing_real_map_retains_optional_tables_and_validates_coordinates():
    root = Path(os.environ["MATCH_RECOVERY_LINK_MAP_BUILD"])
    mapping = parse_link_map((root / "OpenSMACX.map").read_bytes())
    raw = (root / "OpenSMACX.exe").read_bytes()
    mapping.validate(PeImage(raw, hashlib.sha256(raw).hexdigest()))
    assert mapping.contributions and mapping.statics
    assert any(item.name == ".CRT$XCU" for item in mapping.contributions)
    owners = {item.owner for item in mapping.statics if item.symbol == "_$E19"}
    assert len(owners) > 1
