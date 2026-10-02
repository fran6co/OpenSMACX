"""Actual Console-state consumers with small host dependency doubles.

This is bounded behavioral coverage, not Windows-class construction, VC6
layout, original-executable execution, or whole-game acceptance. The selection
control executes only veh_kill's actual state-update block, not vehicle removal.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
MEMBERS = {
    "faction_id_": "int32_t",
    "cursor_active_": "uint32_t",
    "selected_vehicle_": "int32_t",
    "focus_vehicle_": "int32_t",
    "field_23BE4_": "uint32_t",
    "field_23BE8_": "uint32_t",
}


def braced(source, start):
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated source body")


def actual_bodies():
    definitions = []
    for unit in ("game.h", "game.cpp"):
        source = (ROOT / "src" / unit).read_text()
        for match in re.finditer(r"BOOL\s+__cdecl\s+not_my_turn\s*\(\)\s*\{", source):
            definitions.append(braced(source, match.start()))
    assert len(definitions) == 1, "require one actual not_my_turn definition"
    source = (ROOT / "src/spying.cpp").read_text()
    spying = braced(source, source.index("int __cdecl spying(int subject)"))
    source = (ROOT / "src/veh.cpp").read_text()
    removal = braced(source, source.index("void __cdecl veh_kill(int veh_id)"))
    start = removal.index("if (ConsoleGlobal.selected_vehicle_ == veh_id)")
    end = removal.index("TutWinBaseWindow.check_loc(x, y, 0)", start)
    return definitions[0], spying, removal[start:end]


def member_declarations():
    source = (ROOT / "src/console.h").read_text()
    declarations = []
    for name, expected_type in MEMBERS.items():
        matches = re.findall(r"\b(int32_t|uint32_t)\s+" + name + r"\s*;", source)
        assert matches == [expected_type], (name, matches)
        declarations.append(expected_type + " " + name + ";")
    return "\n".join(declarations)


def harness(bodies):
    turn, spying, selection = bodies
    return r'''
        #include <cstdint>
        #include <cstddef>
        #include <cstdlib>
        #include <cstring>
        #define __cdecl
        typedef int BOOL;
        struct Console { MEMBERS } ConsoleGlobal, other;
        static_assert(sizeof(int32_t) == 4 && sizeof(uint32_t) == 4, "host widths");
        void require(bool condition) { if (!condition) std::exit(42); }
        int IsMultiplayerNet, NetTurnFlags, NetTurnFaction;
        struct PlayerData { uint32_t diplo_treaties[8]; } PlayersData[8];
        struct Base { unsigned char faction_id_current; } Bases[3];
        struct ProjectLocations { int empath_guild; } SecretProject;
        int GovernorFaction;
        enum { RFLAG_ALIEN = 16 };
        struct Player { uint32_t rule_flags; } Players[8];
        int hover, hover_alt, followed_value;
        int *StatusHoverVehId = &hover, *StatusHoverVehIdAlt = &hover_alt;
        int VehSelectionFollowLimit;
        int *followed;
        int **VehSelectionFollow = &followed;

        ACTUAL_TURN
        ACTUAL_SPYING
        void update_selection(int veh_id) {
            ACTUAL_SELECTION
        }

        int adjusted(int initial, int removed) {
            return initial == removed ? -1 : initial > removed ? initial - 1 : initial;
        }
        int main() {
            // Static zero initialization is not an invented selected=-1 constructor.
            require(ConsoleGlobal.selected_vehicle_ == 0);
            other.faction_id_ = 7;
            other.selected_vehicle_ = 91;
            other.focus_vehicle_ = 92;
            for (int net = 0; net != 2; ++net)
              for (int flag = 0; flag != 2; ++flag)
                for (int faction = 0; faction != 7; ++faction)
                  for (int turn = 0; turn != 8; ++turn) {
                    ConsoleGlobal.faction_id_ = faction;
                    IsMultiplayerNet = net;
                    NetTurnFlags = flag ? 0x90 : 0x80;
                    NetTurnFaction = turn;
                    require(!!not_my_turn() == (net && flag && turn != faction));
                  }
            for (int faction = 0; faction != 8; ++faction)
              for (int subject = 0; subject != 8; ++subject) {
                ConsoleGlobal.faction_id_ = faction;
                std::memset(PlayersData, 0, sizeof(PlayersData));
                std::memset(Bases, 0xff, sizeof(Bases));
                std::memset(Players, 0, sizeof(Players));
                SecretProject.empath_guild = -1;
                GovernorFaction = -1;
                require(spying(subject) == 0);
                PlayersData[faction].diplo_treaties[subject] = 0x1000;
                require(spying(subject) == 1);
                PlayersData[faction].diplo_treaties[subject] = 0x800;
                require(spying(subject) == 0);
                SecretProject.empath_guild = 2;
                Bases[2].faction_id_current = static_cast<unsigned char>(faction);
                require(spying(subject) == 1);
                Bases[2].faction_id_current = static_cast<unsigned char>((faction + 1) % 8);
                require(spying(subject) == 0);
                SecretProject.empath_guild = -1;
                GovernorFaction = faction;
                require(spying(subject) == 1);
                Players[subject].rule_flags = RFLAG_ALIEN;
                require(spying(subject) == 0);
              }
            // Exercise address identity: the selected, focus and follow-limit
            // slots were already processed and must not be decremented twice.
            for (int selected = 1; selected <= 3; ++selected)
              for (int focus = 1; focus <= 3; ++focus)
                for (int mode = 0; mode != 5; ++mode)
                  for (int net = 0; net != 2; ++net) {
                    ConsoleGlobal.selected_vehicle_ = selected;
                    ConsoleGlobal.focus_vehicle_ = focus;
                    ConsoleGlobal.cursor_active_ = 0x12345678;
                    ConsoleGlobal.field_23BE4_ = 0x87654321;
                    ConsoleGlobal.field_23BE8_ = 0x76543210;
                    hover = 2; hover_alt = 3; VehSelectionFollowLimit = 4; followed_value = 5;
                    followed = mode == 0 ? nullptr : mode == 1 ? &ConsoleGlobal.selected_vehicle_
                        : mode == 2 ? &ConsoleGlobal.focus_vehicle_
                        : mode == 3 ? &VehSelectionFollowLimit : &followed_value;
                    IsMultiplayerNet = net;
                    update_selection(2);
                    require(ConsoleGlobal.selected_vehicle_ == adjusted(selected, 2));
                    require(ConsoleGlobal.focus_vehicle_ == adjusted(focus, 2));
                    require(hover == -1 && hover_alt == 2 && VehSelectionFollowLimit == 3);
                    require(followed_value == (net && mode == 4 ? 4 : 5));
                    require(ConsoleGlobal.cursor_active_ == 0x12345678);
                    require(ConsoleGlobal.field_23BE4_ == 0x87654321);
                    require(ConsoleGlobal.field_23BE8_ == 0x76543210);
                    require(other.selected_vehicle_ == 91 && other.focus_vehicle_ == 92);
                  }
        }
    '''.replace("MEMBERS", member_declarations()).replace(
        "ACTUAL_TURN", turn).replace("ACTUAL_SPYING", spying).replace(
        "ACTUAL_SELECTION", selection)


def run_source(tmp_path, text, *, host_flags=()):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    source = tmp_path / "console_state.cpp"
    executable = tmp_path / "console_state"
    source.write_text(text)
    result = subprocess.run([compiler, "-std=c++11", "-O1", *host_flags, "-fsanitize=undefined",
                             "-fno-sanitize-recover=undefined", str(source), "-o", str(executable)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


def run_host(tmp_path, bodies):
    return run_source(tmp_path, harness(bodies))


def test_status_menu_fixture_uses_actual_handler_and_console_members(tmp_path):
    source = (ROOT / "src/statuswin.cpp").read_text()
    method = braced(source, source.index("void StatusWin::on_right_click("))
    fixture = (ROOT / "recovery/tests/status_menu_harness.cpp").read_text()
    marker = "// RECOVERED_HANDLER is replaced with read_file(...).region, never a copy."
    assert fixture.count(marker) == 1
    # This older body represents a label index as a pointer. The fixture uses
    # only the small token 17, never a host address; allow its existing 32-bit
    # cast on a 64-bit host without changing the actual handler or VC6 policy.
    result = run_source(tmp_path, "#include <cstdint>\n#define __cdecl\n" + fixture.replace(marker, method),
                        host_flags=("-fpermissive",))
    assert result.returncode == 0, result.stderr


def test_actual_consumers_share_console_members_and_preserve_pointer_identity(tmp_path):
    result = run_host(tmp_path, actual_bodies())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("part,before,after,count", [
    (0, "ConsoleGlobal.faction_id_", "other.faction_id_", 1),
    (0, "NetTurnFaction !=", "NetTurnFaction ==", 1),
    (1, "ConsoleGlobal.faction_id_", "other.faction_id_", 1),
    (1, "& 0x1000U", "& 0x0800U", 1),
    (2, "slot != &ConsoleGlobal.selected_vehicle_", "true", 1),
    (2, "slot != &ConsoleGlobal.focus_vehicle_", "true", 1),
    (2, "slot != &VehSelectionFollowLimit", "true", 1),
    (2, "ConsoleGlobal.selected_vehicle_ = -1;", "ConsoleGlobal.selected_vehicle_ = 0;", 1),
    (2, "ConsoleGlobal.focus_vehicle_ = -1;", "ConsoleGlobal.focus_vehicle_ = 0;", 1),
])
def test_console_state_damage_is_observed(tmp_path, part, before, after, count):
    bodies = list(actual_bodies())
    assert bodies[part].count(before) == count
    bodies[part] = bodies[part].replace(before, after)
    result = run_host(tmp_path, bodies)
    assert result.returncode == 42, result.stderr
