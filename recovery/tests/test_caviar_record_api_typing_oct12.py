"""Actual-source API and factory controls, not native ABI/runtime acceptance.

Host records have host-width pointers. Allocator/installer boundaries below are
instrumented doubles; record creation, both installers and clip APIs are read
from their current C++ definitions. The existing ramp suite executes that body.
"""
from collections import Counter
from dataclasses import replace
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.binding_source import parse as parse_binding_evidence
from match_recovery.source import read, read_text
from test_caviar_descriptor_ramps_overlay_oct9 import extract
from test_function_span_tails_oct11 import expected_extended_record
from test_verified_tail_claims_oct11 import expected_recorded_tail


ROOT = Path(__file__).resolve().parents[2]
BASE = "de37a7cda0820dc5827e58919e350e22fa0c6c6f"
API_CHECKPOINT = "b1bc317be9f0cc44a62e9bd169f856b4133927cb"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SUBJECTS = {0x618520, 0x618560, 0x6393C0, 0x63AD60, 0x63AE20, 0x63F9B0}
SYMBOL_TRANSITIONS = {
    0x618520: ("?clip_rect@Caviar@@QAEPAXXZ", "?clip_rect@Caviar@@QAEPAIXZ"),
    0x618560: ("?fill_clip_rect@Caviar@@QAEXH@Z", "?fill_clip_rect@Caviar@@QAEXPAI@Z"),
    0x63AE20: ("?sub_63ae20@@YAXPAE@Z", "?sub_63ae20@@YAXPAUVoxRenderRecord@@@Z"),
    0x6171A0: ("?render_multi_table_dest@Caviar@@QAEXPAVBuffer@@HHHPAEH@Z",
               "?render_multi_table_dest@Caviar@@QAEXPAVBuffer@@HHHPAY0BAA@EH@Z"),
    0x63F000: ("?vox_draw_shadow@@YAXPAUVoxRenderRecord@@PBUCvrShape@@PBUVOX_Vect@@PAUVOX_Matrix@@PBX@Z",
               "?vox_draw_shadow@@YAXPAUVoxRenderRecord@@PBUCvrShape@@PBUVOX_Vect@@PAUVOX_Matrix@@H@Z"),
}


def assert_api_symbol_transition(address, before, after, current=None):
    """Retain stable ABI hints; permit the narrowly observed generated family."""
    if before == after:
        return
    if (re.fullmatch(r"_\$E[0-9]+", before or "")
            and re.fullmatch(r"_\$E[0-9]+", after or "")):
        assert current is not None
        assert current.mode.value == "file"
        assert current.path.relative_to(ROOT).as_posix() in {
            "src/caviar.cpp", "src/caviar_cpu.cpp", "src/caviar_data.cpp",
            "src/caviar_objects.cpp", "src/caviar_renderer_objects.cpp",
        }
        return
    assert address in SYMBOL_TRANSITIONS
    assert (before, after) == SYMBOL_TRANSITIONS[address]


def code_tokens(text):
    result, cursor = [], 0
    for match in COMMENTS_OR_STRINGS.finditer(text):
        result.extend(text[cursor:match.start()].split())
        if not match[0].startswith(("//", "/*")):
            result.append(match[0])
        cursor = match.end()
    return result + text[cursor:].split()


def replace_exact(text, new, old, count=1):
    assert text.count(new) == count, (new, text.count(new))
    return text.replace(new, old)


def restore_api_migration(text, relative="src/caviar.cpp"):
    """Reverse only the six authorized changes for earlier baseline checks.

    Each replacement has an exact spelling and occurrence count. Neither a
    whole function nor an address is blanked out, so unrelated code damage
    still differs from the historical baseline.
    """
    if relative == "src/caviar.h":
        text = replace_exact(text, "  uint32_t *clip_rect();", "  void *clip_rect();")
        return replace_exact(text, "  void fill_clip_rect(uint32_t *destination);",
                             "  void fill_clip_rect(int destination);")
    assert relative == "src/caviar.cpp"
    for name in ("sub_63ad60", "sub_63ae20", "sub_63f9b0"):
        text = replace_exact(text, name + "(VoxRenderRecord *record)",
                             name + "(unsigned char *record)", 2)
    text = replace_exact(text, "VoxRenderRecord *const typed = record;",
                         "VoxRenderRecord *const typed = reinterpret_cast<VoxRenderRecord *>(record);", 3)
    for field in ("colour_width_", "height_"):
        text = replace_exact(text,
            "static_cast<int>(static_cast<int16_t>(static_cast<uint16_t>(\n"
            "            setup_block->" + field + "))) - 1",
            "static_cast<int>(static_cast<uint16_t>(setup_block->" + field + ")) - 1")
    text = replace_exact(text, "sub_63ad60(record);", "sub_63ad60((unsigned char *)record);")
    text = replace_exact(text, "sub_63ae20(record);", "sub_63ae20((unsigned char *)record);")
    text = replace_exact(text, "sub_63f9b0(record) != 0", "sub_63f9b0((unsigned char *)record) != 0")
    text = replace_exact(text, "uint32_t *Caviar::clip_rect()", "void *Caviar::clip_rect()")
    text = replace_exact(text, "void Caviar::fill_clip_rect(uint32_t *destination)",
                         "void Caviar::fill_clip_rect(int destination)")
    return replace_exact(text, "uint32_t *out = destination;", "int *out = (int *)destination;")


def test_historical_api_migration_and_current_owner_continuity():
    # The immutable integrated checkpoint is the exact historical before/after
    # proof. Later source-owner recovery is checked by bounded identities below.
    for relative in ("src/caviar.cpp", "src/caviar.h"):
        baseline = subprocess.check_output(["git", "show", f"{BASE}:{relative}"], cwd=ROOT, text=True)
        integrated = subprocess.check_output(
            ["git", "show", f"{API_CHECKPOINT}:{relative}"], cwd=ROOT, text=True)
        assert code_tokens(restore_api_migration(integrated, relative)) == code_tokens(baseline)

    before = read_text(old_cpp(), ROOT / "src/caviar.cpp")
    current_rows = read(ROOT / "src")
    addresses = {record.address for record in before}
    current_by_address = unique_original_records(current_rows, addresses)
    assert set(current_by_address) == addresses
    current_by_path = {}
    for path in {record.path for record in current_by_address.values()}:
        owner_rows = [record for record in current_rows if record.path == path]
        current_by_path[path] = parse_binding_evidence(path.read_text(), owner_rows)
    for original in before:
        expected = expected_recorded_tail(expected_extended_record(original))
        current = current_by_address[original.address]
        assert_caviar_record_continuity(expected, current)
        assert_api_symbol_transition(original.address, original.symbol, current.symbol, current)
        if original.address in (0x63AEE0, 0x63AF00, 0x63AF20, 0x63AF40):
            evidence = current_by_path[current.path].get(current.line)
            assert evidence is not None
            assert "zero-argument setup callback" in evidence["reason"]


def _lesson_signatures(record):
    return Counter((item.kind, item.key, item.prose,
                    tuple(text for _, text in item.lines))
                   for item in (record.origin or ()))


def _history_signatures(record, field):
    return Counter(getattr(record, field))


def assert_caviar_record_continuity(before, current):
    assert current is not None
    assert current.address == before.address
    assert current.name == before.name
    assert current.image_spans == before.image_spans
    assert current.mode == before.mode
    assert current.kind == before.kind
    assert current.exclusion == before.exclusion
    assert current.source_role == before.source_role
    assert current.source_role_reason == before.source_role_reason
    assert before.function_fragment == current.function_fragment
    assert not before.byte_exact or current.byte_exact
    assert not before.semantic or current.semantic or current.byte_exact
    for field in ("levers", "ruled_out", "unrecoverable", "deferred"):
        before_history = _history_signatures(before, field)
        current_history = _history_signatures(current, field)
        if before.address == 0x616DA0 and field == "levers":
            old_rows = [row for row in before_history if row[0] == "0"]
            new_rows = [row for row in current_history if row[0] == "0"]
            assert len(old_rows) == len(new_rows) == 1
            old_correction, new_correction = old_rows[0], new_rows[0]
            assert "FOLDED into the implicit array construction" in old_correction[1]
            assert "VoxInstance" in new_correction[1]
            before_history[old_correction] -= 1
            current_history[new_correction] -= 1
            before_history += Counter()
            current_history += Counter()
        assert before_history <= current_history

    before_lessons = _lesson_signatures(before)
    current_lessons = _lesson_signatures(current)
    if before.address == 0x616DA0:
        old_rows = [row for row in before_lessons if row[0:2] == ("lever", "0")]
        new_rows = [row for row in current_lessons if row[0:2] == ("lever", "0")]
        assert len(old_rows) == len(new_rows) == 1
        old_correction, new_correction = old_rows[0], new_rows[0]
        assert "FOLDED into the implicit array construction" in old_correction[2]
        assert "VoxInstance" in new_correction[2]
        before_lessons[old_correction] -= 1
        current_lessons[new_correction] -= 1
        before_lessons += Counter()
        current_lessons += Counter()
    assert before_lessons <= current_lessons


def unique_original_records(records, addresses):
    grouped = {}
    for record in records:
        if record.address in addresses:
            grouped.setdefault(record.address, []).append(record)
    assert set(grouped) == set(addresses)
    assert all(len(matches) == 1 for matches in grouped.values())
    return {address: matches[0] for address, matches in grouped.items()}


def test_caviar_owner_continuity_rejects_missing_identity_span_claim_or_history():
    before = read_text(old_cpp(), ROOT / "src/caviar.cpp")
    addresses = {record.address for record in before}
    current_rows = read(ROOT / "src")
    rows = unique_original_records(current_rows, addresses)
    assert len(rows) == len(addresses)

    with pytest.raises(AssertionError):
        unique_original_records([row for row in current_rows if row.address != before[0].address], addresses)

    original = next(record for record in before if record.origin)
    expected = expected_recorded_tail(expected_extended_record(original))
    current = rows[original.address]
    assert expected.image_spans
    span = expected.image_spans[0]
    with pytest.raises(AssertionError):
        assert_caviar_record_continuity(expected, replace(current,
            image_spans=((span[0], span[1] - 1),) + current.image_spans[1:]))
    with pytest.raises(AssertionError):
        assert_caviar_record_continuity(replace(expected, byte_exact=True),
                                        replace(current, byte_exact=False))
    with pytest.raises(AssertionError):
        assert_caviar_record_continuity(replace(expected, semantic=True),
                                        replace(current, semantic=False, byte_exact=False))
    # Native claim integrity permits a historical instruction-shape claim to
    # advance to the stronger byte-exact claim, but never to neither claim.
    assert_caviar_record_continuity(replace(expected, semantic=True),
                                    replace(current, semantic=False, byte_exact=True))
    with pytest.raises(AssertionError):
        assert_caviar_record_continuity(expected, replace(current, origin=()))

    assert_caviar_record_continuity(
        expected, replace(current, origin=current.origin + current.origin[:1]))


def old_cpp():
    return subprocess.check_output(["git", "show", BASE + ":src/caviar.cpp"], cwd=ROOT, text=True)


@pytest.mark.parametrize("damage", ("wrong_fill_store", "changed_factory_error", "changed_other_method"))
def test_continuity_bridge_does_not_hide_behavior_damage(damage):
    text = (ROOT / "src/caviar.cpp").read_text()
    mutations = {
        "wrong_fill_store": ("out[2] = CaviarScene.created_->primary_clip_.right_ + 1;",
                             "out[2] = CaviarScene.created_->primary_clip_.right_;"),
        "changed_factory_error": ("vox_stage_error(vox_msg_no_colortab);", "vox_stage_error(vox_msg_no_handler);"),
        "changed_other_method": ("void Caviar::UNK8(int menu_id)", "void Caviar::UNK8(unsigned int menu_id)"),
    }
    old, new = mutations[damage]
    assert text.count(old) == 1
    assert code_tokens(restore_api_migration(text.replace(old, new))) != code_tokens(old_cpp())


def declarations():
    header = (ROOT / "src/caviar.h").read_text()
    return r'''#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#define __cdecl
struct VOX_Vect;
struct VOX_Matrix;
''' + "\n".join(extract(header, signature) + ";" for signature in (
        "struct CaviarSceneSetupDescriptorCandidate {", "struct VoxRenderClipBounds {",
        "struct VoxRenderRecord {")) + "\n"


def run_actual(tmp_path, text, succeeds=True):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert any(compilers), "actual-source host compiler required"
    cpp = tmp_path / "record-api.cpp"
    cpp.write_text(text)
    for compiler in filter(None, compilers):
        executable = tmp_path / Path(compiler).name.replace("+", "p")
        compilation = subprocess.run([compiler, "-std=c++98", "-O2", "-fsanitize=undefined",
                                      "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
                                     capture_output=True, text=True, timeout=60)
        assert compilation.returncode == 0, compilation.stdout + compilation.stderr
        result = subprocess.run([str(executable)], capture_output=True, timeout=30)
        assert (result.returncode == 0) == succeeds, result.stderr.decode()


FACTORY_BOUNDARIES = r'''
// Native requests remain 0x80; the host double returns a host-sized record.
static VoxRenderRecord storage;
static unsigned char colours[0x3000], shadows[0x20000];
static unsigned alloc_count, free_count, ramp_count, error_count;
static char events[16];
static unsigned event_count;
static void event(char code) { assert(event_count<15); events[event_count++]=code; events[event_count]=0; }
static unsigned fail_allocation;
static unsigned long requests[4];
static int ramp_failure;
static int32_t ramp_width, ramp_height;
static uint32_t vox_record_count;
static int CaviarVoxelAlgoFlag;
static const char vox_msg_no_colortab[]="colour input";
static const char vox_msg_no_handler[]="record allocation";
static const char vox_msg_no_colour[]="colour allocation";
static const char vox_msg_no_shadow[]="shadow allocation";
static const char vox_msg_unsupported_pixel[]="pixel size";
static const char vox_msg_cpu_unsupported[]="cpu rank";
static const char *last_error;
static void vox_stage_error(const char *message) { event('E'); ++error_count; last_error=message; }
static void *vox_alloc_slot(unsigned long size) {
    event('A');
    requests[alloc_count]=size;
    ++alloc_count;
    if (alloc_count==fail_allocation) return 0;
    if (alloc_count==1) { assert(size==0x80); return &storage; }
    if (alloc_count==2) return colours;
    assert(alloc_count==3); return shadows;
}
static void vox_free_slot(void *pointer) {
    // None of these error paths releases the allocated record itself.
    assert(pointer==colours);
    event('F');
    ++free_count;
}
void __cdecl caviar_original_666fd0(int32_t,int32_t,uint32_t,const uint8_t*,int32_t) {}
void __cdecl caviar_original_666a80(int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t) {}
void __cdecl caviar_original_6647f0(int32_t,int32_t,int32_t,uint32_t,const uint8_t*,int32_t) {}
void __cdecl caviar_original_664000(int32_t,int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t) {}
void __cdecl caviar_original_665640(int32_t,int32_t,int32_t,uint32_t,const uint8_t*,int32_t) {}
void __cdecl caviar_original_664e50(int32_t,int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t) {}
void __cdecl caviar_original_666440(int32_t,int32_t,int32_t,uint32_t,const uint8_t*,int32_t) {}
void __cdecl caviar_original_665ca0(int32_t,int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t) {}
void __cdecl caviar_original_667b20(int32_t,int32_t,int32_t,uint32_t,const uint8_t*,int32_t) {}
void __cdecl caviar_original_667380(int32_t,int32_t,int32_t,const int32_t*,uint32_t,const uint8_t*,int32_t) {}
void __cdecl caviar_original_63aee0() {}
void __cdecl caviar_original_63af00() {}
void __cdecl caviar_original_63af20() {}
void __cdecl caviar_original_63af40() {}
extern "C" void __cdecl sub_63ad60(VoxRenderRecord *record);
void __cdecl sub_63ae20(VoxRenderRecord *record);
extern "C" char __cdecl sub_63f9b0(VoxRenderRecord *record) {
    assert(record==&storage);
    assert(record->shadow_contained_==&caviar_original_666fd0);
    assert(record->shadow_clipped_==&caviar_original_666a80);
    if (record->setup_block_->pixel_size_==1 && (record->setup_id_&1U)) {
        assert(record->part_contained_==&caviar_original_665640);
        assert(record->part_clipped_==&caviar_original_664e50);
        assert(record->setup_callback_==&caviar_original_63af40);
    } else if (record->setup_block_->pixel_size_==1) {
        assert(record->part_contained_==&caviar_original_6647f0);
        assert(record->part_clipped_==&caviar_original_664000);
        assert(record->setup_callback_==&caviar_original_63af00);
    } else if (record->setup_block_->pixel_size_==2 && (record->setup_id_&1U)) {
        assert(record->part_contained_==&caviar_original_667b20);
        assert(record->part_clipped_==&caviar_original_667380);
        assert(record->setup_callback_==&caviar_original_63af20);
    } else {
        assert(record->setup_block_->pixel_size_==2);
        assert(record->part_contained_==&caviar_original_666440);
        assert(record->part_clipped_==&caviar_original_665ca0);
        assert(record->setup_callback_==&caviar_original_63aee0);
    }
    event('R');
    ++ramp_count;
    ramp_width=record->setup_block_->colour_width_;
    ramp_height=record->setup_block_->height_;
    return (char)ramp_failure;
}
static void reset() {
    alloc_count=free_count=ramp_count=error_count=0;
    fail_allocation=0; ramp_failure=0; last_error=0;
    event_count=0; events[0]=0;
    vox_record_count=17; CaviarVoxelAlgoFlag=1;
    memset(&storage,0xA5,sizeof(storage));
    storage.part_contained_=&caviar_original_665640;
    storage.part_clipped_=&caviar_original_664e50;
    storage.shadow_contained_=&caviar_original_666fd0;
    storage.shadow_clipped_=&caviar_original_666a80;
    storage.setup_callback_=&caviar_original_63af40;
    memset(colours,0,sizeof(colours)); memset(shadows,0,sizeof(shadows));
}
'''


FACTORY_MAIN = r'''
int main() {
    unsigned char input[0x3000], shadow_input[0x20000];
    memset(input,0x42,sizeof(input)); memset(shadow_input,0x56,sizeof(shadow_input));
    CaviarSceneSetupDescriptorCandidate setup={};
    setup.pixel_size_=1;
    struct Bounds { int32_t width,height,right,bottom,extent_x,extent_y; unsigned ramps; };
    const Bounds cases[]={
        {2,3,1,2,2,3,1}, {0x7FFF,2,32766,1,32767,2,1},
        {0x8000,2,-32769,1,0,0,1}, {0xFFFF,2,-2,1,0,0,1},
        {2,0x8000,1,-32769,0,0,1}, {2,0xFFFF,1,-2,0,0,1},
        {0x10002,0x20003,1,2,2,3,1}, {0x10000,2,-1,1,0,0,1},
        {2,0x10000,1,-1,0,0,1}, {-1,2,-2,1,0,0,1},
        {0,2,-1,1,0,0,0}, {2,0,1,-1,0,0,0}
    };
    for (unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);++i) {
        reset(); setup.colour_width_=cases[i].width; setup.height_=cases[i].height;
        VoxRenderRecord *record=vox_create_record(3,input,0,&setup,0x101);
        assert(record==&storage && record->setup_block_==&setup);
        assert(record->setup_id_==3 && record->setup_size_code_==1);
        assert(record->draw_right_==cases[i].right && record->draw_bottom_==cases[i].bottom);
        assert(record->draw_width_==cases[i].extent_x && record->draw_height_==cases[i].extent_y);
        assert(ramp_count==cases[i].ramps);
        assert(record->shadow_contained_==&caviar_original_666fd0);
        assert(record->shadow_clipped_==&caviar_original_666a80);
        assert(record->part_contained_==&caviar_original_665640);
        assert(record->part_clipped_==&caviar_original_664e50);
        assert(record->setup_callback_==&caviar_original_63af40);
        if (ramp_count) assert(ramp_width==cases[i].width && ramp_height==cases[i].height);
        assert(alloc_count==2 && requests[1]==0x1800 && free_count==0 && error_count==0);
        assert(vox_record_count==18 && memcmp(colours,input,0x1800)==0);
        assert(strcmp(events,cases[i].ramps ? "AAR" : "AA")==0);
    }
    setup.colour_width_=setup.height_=2;
    for (int rank=1;rank<=5;++rank) {
        reset(); CaviarVoxelAlgoFlag=rank;
        assert(vox_create_record(2,input,shadow_input,&setup,7)==&storage);
        assert(ramp_count==1 && alloc_count==3 && requests[2]==0x100);
        assert(storage.part_contained_==&caviar_original_6647f0);
        assert(storage.part_clipped_==&caviar_original_664000);
        assert(storage.setup_callback_==&caviar_original_63af00);
        assert(storage.shadow_contained_==&caviar_original_666fd0);
        assert(storage.shadow_clipped_==&caviar_original_666a80);
        assert(memcmp(shadows,shadow_input,0x100)==0);
        assert(strcmp(events,"AAAR")==0);
    }
    reset(); setup.pixel_size_=2;
    assert(vox_create_record(0,input,shadow_input,&setup,2)==&storage);
    assert(requests[1]==0x3000 && requests[2]==0x20000);
    assert(memcmp(colours,input,0x3000)==0 && memcmp(shadows,shadow_input,0x20000)==0);
    setup.pixel_size_=1;
    reset(); assert(vox_create_record(0,0,0,&setup,0)==0);
    assert(alloc_count==0 && last_error==vox_msg_no_colortab);
    assert(strcmp(events,"E")==0);
    reset(); fail_allocation=1; assert(vox_create_record(0,input,0,&setup,0)==0);
    assert(alloc_count==1 && free_count==0 && last_error==vox_msg_no_handler);
    assert(strcmp(events,"AE")==0);
    reset(); fail_allocation=2; assert(vox_create_record(0,input,0,&setup,0)==0);
    assert(alloc_count==2 && free_count==0 && last_error==vox_msg_no_colour);
    assert(strcmp(events,"AAE")==0);
    reset(); fail_allocation=3; assert(vox_create_record(0,input,shadow_input,&setup,0)==0);
    assert(alloc_count==3 && free_count==1 && last_error==vox_msg_no_shadow);
    assert(strcmp(events,"AAAEF")==0);
    reset(); setup.pixel_size_=3; assert(vox_create_record(0,input,0,&setup,0)==0);
    assert(alloc_count==1 && free_count==0 && last_error==vox_msg_unsupported_pixel);
    assert(strcmp(events,"AE")==0);
    reset(); setup.pixel_size_=1; CaviarVoxelAlgoFlag=6;
    assert(vox_create_record(0,input,0,&setup,0)==0);
    assert(alloc_count==2 && free_count==0 && last_error==vox_msg_cpu_unsupported);
    // Factory zero-initialization occurs, but unsupported rank exits before
    // either real dispatcher can install any slot.
    assert(storage.part_contained_==0 && storage.part_clipped_==0);
    assert(storage.shadow_contained_==0 && storage.shadow_clipped_==0);
    assert(storage.setup_callback_==0);
    assert(strcmp(events,"AAE")==0);
    reset(); ramp_failure=1; assert(vox_create_record(0,input,0,&setup,0)==0);
    assert(alloc_count==2 && free_count==0 && ramp_count==1 && error_count==0 && vox_record_count==17);
    assert(strcmp(events,"AAR")==0);
    // The two installer functions retain distinct identities and linkage.
    void (__cdecl *a)(VoxRenderRecord *)=&sub_63ad60;
    void (__cdecl *b)(VoxRenderRecord *)=&sub_63ae20;
    char (__cdecl *rows)(VoxRenderRecord *)=&sub_63f9b0;
    assert(a && b && rows);
}
'''


def factory_program():
    source = (ROOT / "src/caviar.cpp").read_text()
    installers = "\n".join(extract(source, name) for name in (
        "void __cdecl sub_63ad60(VoxRenderRecord *record) {",
        "void __cdecl sub_63ae20(VoxRenderRecord *record) {"))
    factory = extract(source, "VoxRenderRecord *__cdecl vox_create_record(")
    return declarations() + FACTORY_BOUNDARIES + installers + factory + FACTORY_MAIN


def test_actual_factory_installer_pointer_types_signed_low_words_and_error_lifetimes(tmp_path):
    run_actual(tmp_path, factory_program())


@pytest.mark.parametrize("damage", ("unsigned_width", "unsigned_height", "narrow_full_width_guard",
                                   "free_record_on_colour_failure", "reordered_shadow_failure"))
def test_factory_controls_reject_unsigned_bounds_narrowing_and_leak_fixes(tmp_path, damage):
    text = factory_program()
    if damage.startswith("unsigned_"):
        field = "colour_width_" if damage == "unsigned_width" else "height_"
        old = "static_cast<int>(static_cast<int16_t>(static_cast<uint16_t>(\n            setup_block->" + field + ")))"
        new = "static_cast<int>(static_cast<uint16_t>(setup_block->" + field + "))"
    elif damage == "narrow_full_width_guard":
        old = "if (setup_block->colour_width_ != 0 && setup_block->height_ != 0 &&"
        new = "if (static_cast<int16_t>(setup_block->colour_width_) != 0 && setup_block->height_ != 0 &&"
    elif damage == "free_record_on_colour_failure":
        old = "vox_stage_error(vox_msg_no_colour);"
        new = "vox_free_slot(record); vox_stage_error(vox_msg_no_colour);"
    else:
        old = "vox_stage_error(vox_msg_no_shadow);\n                vox_free_slot(record->colour_table_);"
        new = "vox_free_slot(record->colour_table_);\n                vox_stage_error(vox_msg_no_shadow);"
    assert text.count(old) >= 1
    run_actual(tmp_path, text.replace(old, new, 1), False)


def clip_program():
    source = (ROOT / "src/caviar.cpp").read_text()
    clip = extract(source, "uint32_t *Caviar::clip_rect()")
    fill = extract(source, "void Caviar::fill_clip_rect(uint32_t *destination)")
    return declarations() + r'''
// Observe each handle evaluation without relying on host pointer-punning UB.
// This is a source reevaluation witness, not native alias/ABI equivalence.
struct CreatedProbe {
    VoxRenderRecord *first, *later;
    unsigned reads;
    operator VoxRenderRecord *() { ++reads; return first; }
    VoxRenderRecord *operator->() { ++reads; return reads<=2 ? first : later; }
};
static struct { CreatedProbe created_; } CaviarScene;
static uint32_t vox_clip_rect[4]={7,8,9,10};
struct Caviar {
    uint32_t *clip_rect();
    void fill_clip_rect(uint32_t *destination);
};
''' + clip + fill + r'''
int main() {
    Caviar owner;
    VoxRenderRecord first={}, later={};
    first.primary_clip_.left_=11; first.primary_clip_.top_=12;
    first.primary_clip_.right_=13; first.primary_clip_.bottom_=14;
    later.primary_clip_.left_=21; later.primary_clip_.top_=22;
    later.primary_clip_.right_=0xFFFFFFFFU; later.primary_clip_.bottom_=0x7FFFFFFFU;
    CaviarScene.created_.first=&first; CaviarScene.created_.later=&later; CaviarScene.created_.reads=0;
    uint32_t out[4]={1,2,3,4};
    owner.fill_clip_rect(out);
    assert(CaviarScene.created_.reads==5);
    assert(out[0]==11 && out[1]==22 && out[2]==0 && out[3]==0x80000000U);
    CaviarScene.created_.reads=0; owner.fill_clip_rect(0);
    assert(CaviarScene.created_.reads==0);
    CaviarScene.created_.first=0; CaviarScene.created_.reads=0;
    owner.fill_clip_rect(out); assert(CaviarScene.created_.reads==1);
    assert(out[0]==11 && out[1]==22 && out[2]==0 && out[3]==0x80000000U);
    assert(owner.clip_rect()==0 && vox_clip_rect[0]==7 && vox_clip_rect[3]==10);
    CaviarScene.created_.first=&first; CaviarScene.created_.reads=0;
    uint32_t *cache=owner.clip_rect(); assert(cache==vox_clip_rect);
    assert(CaviarScene.created_.reads==1 && cache[0]==11 && cache[1]==12 && cache[2]==14 && cache[3]==15);
    cache[0]=99; assert(vox_clip_rect[0]==99);
    CaviarScene.created_.first=0; assert(owner.clip_rect()==0 && cache[0]==99);
    uint32_t *(Caviar::*get)()=&Caviar::clip_rect;
    void (Caviar::*copy)(uint32_t *)=&Caviar::fill_clip_rect;
    assert(get && copy);
}
'''


def test_actual_clip_bodies_preserve_live_reads_null_order_mutable_cache_and_wrap(tmp_path):
    run_actual(tmp_path, clip_program())


def test_clip_control_rejects_caching_the_created_record_between_stores(tmp_path):
    text = clip_program()
    old = "uint32_t *out = destination;"
    assert text.count(old) == 1
    changed = text.replace(old, old + "\n    VoxRenderRecord *const cached = CaviarScene.created_;")
    changed = changed.replace("CaviarScene.created_->", "cached->")
    run_actual(tmp_path, changed, False)


def test_pinned_signed_words_record_calls_and_clip_stack_protocol():
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    capstone = pytest.importorskip("capstone")
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.exists() and "OPENSMACX_IMAGE" not in os.environ:
        pytest.skip("pinned image unavailable; set OPENSMACX_IMAGE for original evidence")
    image = PeImage(path.read_bytes(), PIN)
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    def instructions(low, high):
        rows = list(decoder.disasm(image.read(Span(low, high)), low))
        assert sum(i.size for i in rows) == high-low
        return {i.address: (i.mnemonic, i.op_str) for i in rows}
    bounds = instructions(0x63957B, 0x6395A8)
    assert bounds[0x63957B] == ("movsx", "eax, word ptr [esi + 0xc]")
    assert bounds[0x639586] == ("movsx", "eax, word ptr [esi + 0x10]")
    assert bounds[0x639590] == ("jl", "0x6395a2")
    assert bounds[0x639594] == ("jl", "0x6395a2")
    calls = instructions(0x6395BC, 0x6395E8)
    assert calls[0x6395BD] == ("call", "0x63ad60")
    assert calls[0x6395C5] == ("call", "0x63ae20")
    assert calls[0x6395E0] == ("call", "0x63f9b0")
    assert calls[0x6395D0] == ("mov", "eax, dword ptr [esi + 0xc]")
    assert calls[0x6395DA] == ("cmp", "dword ptr [esi + 0x10], ebp")
    fill = instructions(0x618560, 0x6185A0)
    assert fill[0x618560] == ("mov", "eax, dword ptr [esp + 4]")
    assert fill[0x61859D] == ("ret", "4")
    assert [a for a, (_, operand) in fill.items() if "[0x9bb478]" in operand] == [
        0x618568, 0x618577, 0x618584, 0x618590]
