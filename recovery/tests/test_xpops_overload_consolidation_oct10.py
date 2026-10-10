"""Exercise the actual mapped popup overloads, not replacement fixture bodies."""
from pathlib import Path
import shutil
import subprocess

from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/xpops.cpp"


def test_primary_overloads_forward_original_arguments(tmp_path):
    records = {record.address: record for record in read_file(SOURCE)}
    specs = (
        (0x005BF7D0, "int __cdecl X_pops(",
         "?X_pops@@YAHPBDHPAVSprite@@P6AHXZ@Z"),
        (0x005BF330, "int __cdecl X_pop(",
         "?X_pop@@YAHPADPBDP6AHXZ@Z"),
    )
    bodies = []
    for address, signature, symbol in specs:
        record = records[address]
        assert record.byte_exact  # historical obligation, not a fresh result
        assert record.symbol == symbol
        bodies.append(_definition(record.region, signature))
    text = SOURCE.read_text()
    assert "RECOVERY_SHARED: v1 0x005BF7D0" not in text
    assert "RECOVERY_SHARED: v1 0x005BF330" not in text
    assert text.count("int __cdecl X_pops(const char *label, int value,") == 1
    assert text.count("int __cdecl X_pop(char *caption, const char *label,\n") == 1
    fixture = r'''
#include <cassert>
#define __cdecl
struct Sprite {};
char PopupStartCaption[32];
char other_caption[32];
const char *expected_label;
char *expected_caption;
Sprite *expected_sprite;
int expected_value;
int (*expected_callback)();
int calls;
int callback_calls;
int callback() { ++callback_calls; return 111; }
int X_pops(char *caption, const char *label, int length, char *text,
           int value, Sprite *sprite, int flag1, int flag2, int (*cb)()) {
    assert(caption == PopupStartCaption && label == expected_label);
    assert(length == -1 && text == nullptr && value == expected_value);
    assert(sprite == expected_sprite && flag1 == 1 && flag2 == 1);
    assert(cb == expected_callback);
    ++calls;
    return expected_value;
}
int X_pop(char *caption, const char *label, int length, char *text,
          int value, int (*cb)()) {
    assert(caption == expected_caption && label == expected_label);
    assert(length == -1 && text == nullptr && value == 0);
    assert(cb == expected_callback);
    ++calls;
    return -37;
}
'''
    main = r'''
int main() {
    Sprite sprite;
    const int values[] = {-1, 0, 0x100000};
    for (int i = 0; i != 3; ++i) {
        expected_value = values[i];
        expected_sprite = i == 0 ? nullptr : &sprite;
        expected_callback = i == 0 ? nullptr : callback;
        expected_label = i == 0 ? nullptr : "LABEL";
        assert(X_pops(expected_label, expected_value, expected_sprite,
                      expected_callback) == expected_value);
        expected_caption = i == 0 ? nullptr : other_caption;
        assert(X_pop(expected_caption, expected_label, expected_callback) == -37);
    }
    assert(calls == 6 && callback_calls == 0);
}
'''
    source = tmp_path / "control.cpp"
    # Disposable host control; no tracked-source or executable-derived writes.
    source.write_text(fixture + "\n".join(bodies) + main)
    executable = tmp_path / "control"
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "host C++ compiler required"
    subprocess.run([compiler, "-std=c++11", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True)
    subprocess.run([str(executable)], check=True)
