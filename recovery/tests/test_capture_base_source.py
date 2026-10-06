"""Actual-candidate structural controls; not a native/runtime acceptance test."""

from pathlib import Path
import re
import subprocess


ROOT = Path(__file__).resolve().parents[2]
from match_recovery.source import read_file
RECORD, = [row for row in read_file(ROOT / "src/base.cpp") if row.address == 0x50C510]
SIGNATURE = "void __cdecl capture_base(int base_id, int faction_id, int mode)"
BODY = RECORD.region[RECORD.region.index(SIGNATURE):]


def ordered(text: str, *needles: str) -> bool:
    cursor = -1
    for needle in needles:
        cursor = text.find(needle, cursor + 1)
        if cursor < 0:
            return False
    return True


def test_candidate_keeps_canonical_source_identity_and_typed_entry():
    assert "void __cdecl capture_base(int base_id, int faction_id, int mode)" in BODY
    assert "ORIGINAL:" not in BODY
    assert "BODY GOES HERE" not in BODY
    assert "reinterpret_cast<OriginalObject" not in BODY


def test_candidate_has_no_executable_address_views_or_seed_pointer_aliases():
    assert not re.search(r"0x00[0-9A-Fa-f]{6,8}", BODY)
    assert not re.search(r"\bg_[A-Za-z0-9_]+\b", BODY)
    assert "PENDING_BODY" not in BODY


def test_original_capture_pipeline_remains_present_in_order():
    assert ordered(
        BODY,
        "treaty_off(faction_id",
        "PlayersData[uVar14].diplo_treaties[uVar13]",
        "local_40 = (Bases[base_id].population_size",
        "PlayersData[faction_id].energy_reserves += local_40",
        "if (wipeOut)",
        "Bases[base_id].faction_id_current = bVar11",
        "set_fac(k, base_id, 0)",
        "RadiusOffsetX[dirOff / 4]",
        "spot_base(base_id, (int)local_8)",
        "reset_territory()",
        "kill(i)",
        "base_first(base_id)",
        "owner_set(local_28, local_1c, faction_id)",
        "UnkBitfield1 |= 2",
        "WorldClimateWorldWin.redraw()",
        "popp(ScriptPopupFile, local_174",
        "steal_tech(faction_id, (int)local_8, 1)",
        "mon_conquer_base(faction_id, local_74)",
        "eliminate_player((int)local_8, faction_id)",
        "set_base(base_id)",
        "base_compute(1)",
    )


def test_project_messages_use_existing_project_facility_and_label_owners():
    assert "base_project(iVar5 - FAC_HUMAN_GENOME_PROJ)" in BODY
    assert "base_project(project_record_id) == base_id" in BODY
    assert "Facility[70 + project_record_id].name" in BODY
    assert "label_get(403)" in BODY
    assert "label_get(405)" in BODY
    assert "label_get(404)" in BODY
    assert "label_get(406)" in BODY
    assert "StringTable.get(" not in BODY


def test_raw_project_owner_offset_maps_to_universal_translator_not_network_backbone():
    assert "SecretProject.universal_translator() == base_id" in BODY
    assert "SecretProject.network_backbone() == base_id" not in BODY


def test_known_semantic_mutants_are_caught_against_actual_candidate():
    critical = (
        "PlayersData[faction_id].energy_reserves += local_40",
        "if (wipeOut)",
        "reset_territory()",
        "eliminate_player((int)local_8, faction_id)",
    )
    for absent in critical:
        mutant = BODY.replace(absent, "", 1)
        assert not all(token in mutant for token in critical)

    reorder = BODY.replace(
        "reset_territory();\n        for (i = VehCurrentCount - 1;",
        "for (i = VehCurrentCount - 1;",
        1,
    ).replace("        reset_territory();\n", "", 1)
    assert not ordered(reorder, "spot_base(base_id, (int)local_8)",
                       "reset_territory()", "kill(i)")


def test_current_airdrop_call_uses_canonical_argument_order():
    product = (ROOT / "src/veh.cpp").read_text()
    assert "capture_base(base_here, faction, 0);" in product
    # The ignored source patch removes the misleading redundant declaration;
    # the product caller now relies on base.h's canonical typed declaration.
    assert "capture_base(int faction_id, int base_owner, int mode)" not in product
    assert "capture_base(int base_id, int faction_id, int mode);" in (
        ROOT / "src/base.h"
    ).read_text()


def test_actual_candidate_body_in_host_projection_and_capture_mutants(tmp_path):
    start = BODY.index("void __cdecl capture_base(")
    opening = BODY.index("{", start)
    depth = 0
    end = None
    for pos in range(opening, len(BODY)):
        depth += (BODY[pos] == "{") - (BODY[pos] == "}")
        if depth == 0:
            end = pos + 1
            break
    assert end is not None
    actual_body = BODY[start:end]
    # The product ABI is 32-bit. Widen only the pointer-to-int parse slot in
    # this x64 host projection; the actual implementation body stays unchanged.
    host_body = actual_body.replace(
        "reinterpret_cast<int>(Facility[70 + project_record_id].name)",
        "static_cast<int>(reinterpret_cast<intptr_t>(Facility[70 + project_record_id].name))",
    )
    host_body = host_body.replace(
        "(int)Facility[(int)local_18].name",
        "static_cast<int>(reinterpret_cast<intptr_t>(Facility[(int)local_18].name))",
    ).replace(
        "(int)Facility[iVar5].name",
        "static_cast<int>(reinterpret_cast<intptr_t>(Facility[iVar5].name))",
    )
    prefix = (ROOT / "recovery/tests/capture_base_host.hpp").read_text()
    runtime = (ROOT / "recovery/tests/capture_base_host.cpp").read_text()
    source = tmp_path / "candidate_host.cpp"
    binary = tmp_path / "candidate_host"
    source.write_text(prefix + "\n" + host_body + "\n" + runtime)
    compiled = subprocess.run(
        ["g++", "-std=c++98", str(source), "-o", str(binary)],
        text=True, capture_output=True, check=False,
    )
    assert compiled.returncode == 0, compiled.stderr
    executed = subprocess.run([str(binary)], check=False)
    assert executed.returncode == 0

    # Mutate the real candidate body in-memory: omitting the capture owner
    # store must fail the same runtime control, so the fixture isn't a second
    # hand-written implementation of that store.
    mutated = host_body.replace(
        "Bases[base_id].faction_id_current = bVar11;", "", 1,
    )
    source.write_text(prefix + "\n" + mutated + "\n" + runtime)
    mutant_compile = subprocess.run(
        ["g++", "-std=c++98", str(source), "-o", str(binary)],
        text=True, capture_output=True, check=False,
    )
    assert mutant_compile.returncode == 0, mutant_compile.stderr
    mutant_run = subprocess.run([str(binary)], check=False)
    assert mutant_run.returncode != 0

    # The seed's direct +0x50 field is universal_translator; make the adjacent
    # +0x54 network_backbone differ. A candidate that tests the adjacent field
    # misses the second steal_tech call in the actual extracted body fixture.
    field_mutant = host_body.replace(
        "SecretProject.universal_translator() == base_id",
        "SecretProject.network_backbone() == base_id",
        1,
    )
    source.write_text(prefix + "\n" + field_mutant + "\n" + runtime)
    field_compile = subprocess.run(
        ["g++", "-std=c++98", str(source), "-o", str(binary)],
        text=True, capture_output=True, check=False,
    )
    assert field_compile.returncode == 0, field_compile.stderr
    field_run = subprocess.run([str(binary)], check=False)
    assert field_run.returncode != 0
