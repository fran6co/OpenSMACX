"""Actual-source controls for the two typed Game table-reset walks.

The tested bodies and accessed record declarations are extracted from the
worker's current C++/headers. Small wrappers provide only the owning globals;
they are host fixtures, not claims about complete native object layout or ABI.
"""

from pathlib import Path
import re
import resource
import shutil
import subprocess

import pytest

from match_recovery.source.reader import _brace_delta, function_line


ROOT = Path(__file__).resolve().parents[2]


def _function_body(source: str, name: str) -> str:
    lines = source.splitlines(keepends=True)
    first = function_line(source, name) - 1
    depth = 0
    opened = False
    in_block = False
    for index in range(first, len(lines)):
        delta, in_block, saw_open, _ = _brace_delta(lines[index], in_block)
        opened = opened or saw_open
        depth += delta
        if opened and depth == 0:
            return "".join(lines[first:index + 1]).strip()
    raise AssertionError("unterminated actual definition: " + name)


def _record_definition(source: str, signature: str) -> str:
    start = source.index(signature)
    lines = source.splitlines(keepends=True)
    first = source[:start].count("\n")
    depth = 0
    opened = False
    in_block = False
    for index in range(first, len(lines)):
        delta, in_block, saw_open, _ = _brace_delta(lines[index], in_block)
        opened = opened or saw_open
        depth += delta
        if opened and depth == 0:
            return "".join(lines[first:index + 1]).strip()
    raise AssertionError("unterminated actual type: " + signature)


def _line(source: str, expression: str) -> str:
    matches = re.findall(expression, source, re.M)
    assert len(matches) == 1, (expression, len(matches))
    return matches[0].strip()


def actual_parts():
    game = (ROOT / "src/game.cpp").read_text(encoding="utf-8")
    veh_header = (ROOT / "src/veh.h").read_text(encoding="utf-8")
    net_header = (ROOT / "src/netdaemon.h").read_text(encoding="utf-8")
    state_header = (ROOT / "src/game_data_state_window.h").read_text(
        encoding="utf-8")

    prototype = _record_definition(veh_header, "struct VehPrototype {")
    save_slot = _record_definition(
        net_header, "struct NetDaemonSaveProcessSlot {")
    max_prototypes = _line(
        veh_header, r"^\s*static const int MaxVehProtoNum = 512;\s*$")
    prototype_array = _line(
        veh_header,
        r"^\s*extern VehPrototype VehPrototypes\[MaxVehProtoNum\];.*$")
    saved_slots_member = _line(
        net_header,
        r"^\s*NetDaemonSaveProcessSlot saved_process_slots_\[8\];\s*$")

    vehicle_count = _line(
        state_header, r"^\s*int vehicle_current_count_;.*$")
    base_count = _line(state_header, r"^\s*int base_current_count_;.*$")
    assert "GameDataStateOffset_vehicle_current_count_" in state_header
    assert "GameDataStateOffset_base_current_count_" in state_header
    assert "extern GameDataStateWindow GameDataStateStorage;" in state_header

    # Keep the real declared access paths and extent in the host fixture while
    # intentionally not pretending to reproduce the enclosing full classes.
    wrapper_types = f"""
{max_prototypes}
{prototype}
typedef char prototype_size_is_0x34[sizeof(VehPrototype) == 0x34 ? 1 : -1];
typedef char prototype_flags_offset_is_0x30[offsetof(VehPrototype, flags) == 0x30 ? 1 : -1];
typedef char prototype_icon_offset_is_0x2e[offsetof(VehPrototype, icon_offset) == 0x2e ? 1 : -1];
{prototype_array.replace("extern ", "")}
struct GameDataStateFieldsProbe {{
    {vehicle_count}
    {base_count}
}};
struct GameDataStateProbe {{ GameDataStateFieldsProbe fields_; }};
GameDataStateProbe GameDataStateStorage;
{save_slot}
typedef char save_identity_offset_is_0x24[offsetof(NetDaemonSaveProcessSlot, identity_) == 0x24 ? 1 : -1];
typedef char save_status_offset_is_0x28[offsetof(NetDaemonSaveProcessSlot, status_bytes_) == 0x28 ? 1 : -1];
typedef char save_slot_size_is_0x19c[sizeof(NetDaemonSaveProcessSlot) == 0x19c ? 1 : -1];
struct NetDaemonProbe {{ {saved_slots_member} }};
NetDaemonProbe NetDaemonNet;
"""
    return (wrapper_types, _function_body(game, "sub_52a980"),
            _function_body(game, "reset_saved_process_slot_state"))


SUPPORT = r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int check_resets() {
  unsigned char prototype_before[sizeof(VehPrototypes)];
  for (int i = 0; i < MaxVehProtoNum; ++i) {
    memset(&VehPrototypes[i], 0x5a + (i & 3), sizeof(VehPrototype));
    VehPrototypes[i].flags = (uint16_t)(0xa501u | ((i & 0xff) << 8));
    VehPrototypes[i].icon_offset = (int8_t)(i & 0x7f);
  }
  memcpy(prototype_before, VehPrototypes, sizeof(VehPrototypes));
  GameDataStateStorage.fields_.vehicle_current_count_ = 0x12345678;
  GameDataStateStorage.fields_.base_current_count_ = -17;
  if (sub_52a980() != 0) return 10;
  if (GameDataStateStorage.fields_.vehicle_current_count_ != 0 ||
      GameDataStateStorage.fields_.base_current_count_ != 0) return 11;
  for (int i = 0; i < MaxVehProtoNum; ++i) {
    const unsigned char *before = prototype_before + i * sizeof(VehPrototype);
    const unsigned char *after = (const unsigned char *)&VehPrototypes[i];
    for (size_t byte = 0; byte < sizeof(VehPrototype); ++byte) {
      unsigned char expected = before[byte];
      if (byte == offsetof(VehPrototype, flags)) expected &= 0xfe;
      if (byte == offsetof(VehPrototype, icon_offset)) expected = 0xff;
      if (after[byte] != expected) return 12;
    }
    if ((VehPrototypes[i].flags & 0xff00u) !=
        ((prototype_before[i * sizeof(VehPrototype) +
          offsetof(VehPrototype, flags) + 1] & 0xffu) << 8)) return 13;
  }

  unsigned char saved_before[8 * sizeof(NetDaemonSaveProcessSlot)];
  for (int i = 0; i < 8; ++i)
    memset(&NetDaemonNet.saved_process_slots_[i], 0x30 + i,
           sizeof(NetDaemonSaveProcessSlot));
  memcpy(saved_before, NetDaemonNet.saved_process_slots_, sizeof(saved_before));
  reset_saved_process_slot_state();
  for (int i = 0; i < 8; ++i) {
    const unsigned char *before = saved_before + i * sizeof(NetDaemonSaveProcessSlot);
    const unsigned char *after = (const unsigned char *)&NetDaemonNet.saved_process_slots_[i];
    for (size_t byte = 0; byte < sizeof(NetDaemonSaveProcessSlot); ++byte) {
      unsigned char expected = before[byte];
      if (byte == offsetof(NetDaemonSaveProcessSlot, identity_)) expected = 0xff;
      if (byte == offsetof(NetDaemonSaveProcessSlot, field_25_) ||
          byte == offsetof(NetDaemonSaveProcessSlot, field_26_)) expected = 0;
      if (byte == offsetof(NetDaemonSaveProcessSlot, field_27_)) expected = 0xff;
      if (byte == offsetof(NetDaemonSaveProcessSlot, status_bytes_)) expected = 2;
      if (after[byte] != expected) return 14;
    }
  }
  return 0;
}

int main() {
  const int result = check_resets();
  if (result != 0) fprintf(stderr, "reset witness failed: %d\n", result);
  return result;
}
'''


def program(parts):
    wrappers, vehicle_body, save_body = parts
    return ("#include <assert.h>\n#include <stddef.h>\n#include <stdint.h>\n"
            "#include <stdio.h>\n#include <string.h>\n"
            "#ifndef __cdecl\n#define __cdecl\n#endif\n" + wrappers +
            "\n" + vehicle_body + "\n" + save_body + "\n" + SUPPORT)


def compile_run(tmp_path, compiler, text, label):
    tmp_path.mkdir(parents=True, exist_ok=True)
    source = tmp_path / (label + ".cpp")
    binary = tmp_path / label
    source.write_text(text, encoding="utf-8")

    def disable_core_dumps():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

    compile_result = subprocess.run(
        [compiler, "-std=c++98", "-O2", "-fno-strict-aliasing",
         str(source), "-o", str(binary)],
        cwd=tmp_path, capture_output=True, text=True, timeout=45,
        preexec_fn=disable_core_dumps)
    assert compile_result.returncode == 0, compile_result.stderr
    run_result = subprocess.run(
        [str(binary)], cwd=tmp_path, capture_output=True, text=True,
        timeout=10, preexec_fn=disable_core_dumps)
    return run_result


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_typed_reset_walks_and_damage_controls(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    parts = actual_parts()
    baseline = compile_run(tmp_path / "baseline", compiler,
                           program(parts), "actual")
    assert baseline.returncode == 0, baseline.stderr

    vehicle_body, save_body = parts[1], parts[2]
    mutations = [
        ("vehicle_wrong_flag_bit", vehicle_body.replace(
            "flags_bytes[0] &= 0xFE;", "flags_bytes[0] &= 0xFD;", 1), save_body),
        ("vehicle_skip_last_record", vehicle_body.replace(
            "prototype_id < MaxVehProtoNum", "prototype_id < MaxVehProtoNum - 1", 1), save_body),
        ("save_wrong_signed_byte", vehicle_body, save_body.replace(
            "*reinterpret_cast<int8_t *>(cursor + 1) = -1;", "*reinterpret_cast<int8_t *>(cursor + 1) = 0;", 1)),
        ("save_skip_last_record", vehicle_body, save_body.replace(
            "} while (--remaining);", "} while (--remaining > 1);", 1)),
    ]
    for label, changed_vehicle, changed_save in mutations:
        assert changed_vehicle != vehicle_body or changed_save != save_body
        damaged = compile_run(tmp_path / label, compiler,
                             program((parts[0], changed_vehicle, changed_save)),
                             label)
        assert damaged.returncode != 0, label
