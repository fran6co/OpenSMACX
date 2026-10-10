"""Actual Win::on_key routing controls, not an original Win32 ABI oracle."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_listbox_rendering_actual_source import extract_function


ROOT = Path(__file__).resolve().parents[2]

HARNESS = r'''
#include <cassert>
#include <vector>
struct MsgT {};
struct Win;
typedef int (*WinKeyHookFn)(int);
static std::vector<int> events, sampled;
static unsigned int modifiers, expected_key;
static int window_valid, pending_chars, peeks, WinKeyModifiers;
static Win *WinCallbackWindow;
static void *HandleMain = reinterpret_cast<void *>(1);
struct Sound {
    int playing, checks, plays;
    int is_playing() { events.push_back(7); ++checks; return playing; }
    void play() { events.push_back(8); ++plays; }
};
struct FocusNode { Win *focus_owner_; };
struct FocusList { int count_; void *head_; FocusNode *current_; };
struct Win {
    unsigned int iFlags_, iSomeFlag_;
    FocusList list_;
    WinKeyHookFn field_434_;
    Sound *val_16_;
    int focus_handled;
    void on_key(unsigned int, long, int, unsigned int);
    int key_up_event(unsigned int key) {
        assert(key == expected_key); events.push_back(4); return focus_handled;
    }
    int on_key_up(unsigned int key) {
        assert(key == expected_key); events.push_back(6); return 1;
    }
    int key_down_event(unsigned int key) {
        assert(key == expected_key);
        assert(static_cast<unsigned int>(WinKeyModifiers) == expected_key);
        events.push_back(2); return 1;
    }
};
struct Net {
    unsigned int voice_hotkey_48_, voice_hotkey_4c_;
    unsigned long voice_target_50_;
    unsigned int field_54_;
    int starts_unsigned, starts_signed, stops;
    int start_voice(unsigned long target) {
        assert(target == voice_target_50_); ++starts_unsigned; events.push_back(10); return 0;
    }
    int start_voice(long target) {
        assert(target == static_cast<long>(field_54_)); ++starts_signed; events.push_back(11); return 0;
    }
    void stop_voice() { ++stops; events.push_back(12); }
};
static Net *ActiveNet;
static short GetKeyState(int key) {
    sampled.push_back(key);
    unsigned bit = key == 0x10 ? 1 : key == 0x11 ? 2 : 4;
    // A low toggle bit must not be mistaken for the pressed high bit.
    return static_cast<short>((modifiers & bit ? 0x8000 : 0) | 1);
}
static int IsWindow(void *handle) {
    assert(handle == HandleMain);
    assert(static_cast<unsigned int>(WinKeyModifiers) == expected_key);
    events.push_back(1); return window_valid;
}
static int PeekMessageA(MsgT *, void *handle, unsigned lo, unsigned hi, unsigned remove) {
    assert(handle == HandleMain && lo == 0x102 && hi == 0x102 && remove == 1);
    ++peeks; events.push_back(9);
    if (pending_chars) { --pending_chars; return 1; }
    return 0;
}
static void do_all_chars() {
    assert(static_cast<unsigned int>(WinKeyModifiers) == expected_key);
    events.push_back(3);
}
LIVE_BODY
static Win *hook_receiver;
static Sound *replacement_sound;
static int hook(int key) {
    assert(static_cast<unsigned int>(key) == expected_key);
    assert(WinCallbackWindow == hook_receiver);
    events.push_back(5);
    hook_receiver->val_16_ = replacement_sound;
    return 1; // Original ignores this return and still calls on_key_up.
}
static void reset(unsigned int key, unsigned int bits = 0) {
    events.clear(); sampled.clear(); modifiers = bits;
    expected_key = key | (bits << 16);
    WinKeyModifiers = 0x1234; WinCallbackWindow = 0; ActiveNet = 0;
    window_valid = 1; pending_chars = peeks = 0;
    hook_receiver = 0; replacement_sound = 0;
}
static void sampled_once() { assert((sampled == std::vector<int>{0x10, 0x11, 0x12})); }
int main() {
    Win receiver{};
    for (unsigned bits = 0; bits != 8; ++bits) {
        reset('A', bits); receiver.on_key('A', 7, 3, 11);
        sampled_once(); assert((events == std::vector<int>{1, 2, 3}));
        assert(static_cast<unsigned int>(WinKeyModifiers) == expected_key && !peeks);
    }
    reset('B'); window_valid = 0; receiver.on_key('B', -1, 0, 0);
    assert((events == std::vector<int>{1, 3}));
    for (int queued = 0; queued != 3; ++queued) {
        reset(0x60); pending_chars = queued; receiver.on_key(0x60, 1, 0, 0);
        assert(peeks == queued + 1 && events[0] == 1 && events[1] == 2);
        assert(events.back() == 9); // Numeric keypad drains and skips do_all_chars.
    }
    reset(0x60, 1); receiver.on_key(0x60, 1, 0, 0);
    assert(!peeks && (events == std::vector<int>{1, 2, 3}));

    // Both hotkey overloads precede ordinary routing and leave the global alone.
    for (int family = 0; family != 2; ++family) {
        for (int down = 0; down != 2; ++down) {
            reset('V', 2); Net net{}; ActiveNet = &net;
            net.voice_hotkey_48_ = family == 0 ? expected_key : 0xffffffffU;
            net.voice_hotkey_4c_ = family == 1 ? expected_key : 0xffffffffU;
            net.voice_target_50_ = 21; net.field_54_ = 37;
            pending_chars = 2; receiver.on_key('V', down, 0, 0);
            sampled_once(); assert(WinKeyModifiers == 0x1234 && !WinCallbackWindow);
            assert(net.stops == !down && net.starts_unsigned == (down && !family));
            assert(net.starts_signed == (down && family) && peeks == 3);
            assert(events[0] == (!down ? 12 : family ? 11 : 10));
            assert(events.size() == 4 && events.back() == 9);
        }
    }

    // Ordinary key-up leaves the global unchanged; guards bypass callbacks.
    for (int gate = 0; gate != 2; ++gate) {
        receiver = Win{}; reset('C', 3);
        if (gate) receiver.iSomeFlag_ = 8; else receiver.iFlags_ = 0x200000;
        receiver.on_key('C', 0, 0, 0); sampled_once();
        assert(events.empty() && WinKeyModifiers == 0x1234);
    }
    Win focus{}; FocusNode node{&focus}; Sound original{}, replacement{};
    for (int handled = 0; handled != 2; ++handled) {
        for (int playing = 0; playing != 2; ++playing) {
            receiver = Win{}; original = Sound{}; replacement = Sound{};
            reset('D', 4); focus.focus_handled = handled;
            receiver.list_ = FocusList{1, &node, &node};
            receiver.field_434_ = hook; receiver.val_16_ = &original;
            hook_receiver = &receiver; replacement_sound = &replacement;
            replacement.playing = playing;
            receiver.on_key('D', 0, 0, 0); sampled_once();
            assert(WinKeyModifiers == 0x1234 && !original.checks && !original.plays);
            if (handled) {
                assert((events == std::vector<int>{4}) && !WinCallbackWindow);
            } else {
                assert(WinCallbackWindow == &receiver && replacement.checks == 1);
                assert(replacement.plays == !playing);
                assert(events == (playing ? std::vector<int>{4,5,6,7} : std::vector<int>{4,5,6,7,8}));
            }
        }
    }
    receiver = Win{}; reset('E'); receiver.on_key('E', 0, 0, 0);
    assert((events == std::vector<int>{6}) && WinKeyModifiers == 0x1234);
}
'''


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("WinKeyModifiers = with_modifiers;", "(void)with_modifiers;"),
    ("with_modifiers |= 0x10000;", "with_modifiers |= 0x20000;"),
    ("if (IsWindow(HandleMain))", "if (1)")))
def test_actual_keyboard_modifier_store_and_routes(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    text = (ROOT / "src/win.cpp").read_text()
    body = extract_function(text, "\nvoid Win::on_key(unsigned int key, long flags, int repeat, unsigned int scan)")
    assert "reinterpret_cast<unsigned int *>(WinKeyModifiers)" not in body
    assert body.count("WinKeyModifiers = with_modifiers;") == 1
    if damage:
        assert body.count(damage[0]) == 1
        body = body.replace(*damage, 1)
    source = tmp_path / "keyboard.cpp"
    source.write_text(HARNESS.replace("LIVE_BODY", body))
    binary = tmp_path / "keyboard"
    result = subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(binary)], capture_output=True)
    assert (result.returncode == 0) == (damage is None)
