"""Source-extracted NetDaemon callback/storage controls, not runtime proof."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest
from owner_transfer_test_support import assert_current_owner, assert_leaf_collection_retired

_HERE = Path(__file__).resolve()
_configured_root = os.environ.get("OPENSMACX_ROOT")
if _configured_root:
    ROOT = Path(_configured_root).resolve()
else:
    _repo_root = _HERE.parents[2]
    if not (_repo_root / "src/netdaemon.cpp").is_file():
        raise RuntimeError("set OPENSMACX_ROOT to an explicit checkout root")
    ROOT = _repo_root


def _mask_comments(text):
    masked = list(text)
    mode = "code"
    index = 0
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if mode == "code":
            if char == "/" and nxt == "/":
                masked[index] = masked[index + 1] = " "
                index += 2
                mode = "line"
                continue
            if char == "/" and nxt == "*":
                masked[index] = masked[index + 1] = " "
                index += 2
                mode = "block"
                continue
            if char == '"':
                masked[index] = " "
                mode = "string"
            elif char == "'":
                masked[index] = " "
                mode = "char"
        elif mode == "line":
            if char == "\n":
                mode = "code"
            else:
                masked[index] = " "
        elif mode == "block":
            masked[index] = " "
            if char == "*" and nxt == "/":
                masked[index + 1] = " "
                index += 2
                mode = "code"
                continue
        else:
            masked[index] = " "
            if char == "\\" and index + 1 < len(text):
                masked[index + 1] = " "
                index += 2
                continue
            if (mode == "string" and char == '"') or (mode == "char" and char == "'"):
                mode = "code"
        index += 1
    return "".join(masked)


def definition(text, signature):
    code = _mask_comments(text)
    match = re.search(signature + r"\s*\{", code)
    assert match, "missing source definition: " + signature
    opening = code.index("{", match.start())
    depth = 0
    for end in range(opening, len(code)):
        depth += (code[end] == "{") - (code[end] == "}")
        if depth == 0:
            body = text[match.start():end + 1]
            assert "BODY GOES HERE" not in body, "refusing unfinished product sentinel"
            return body
    raise AssertionError("unterminated source definition: " + signature)


def one_line_definition(text, pattern, label):
    matches = re.findall(pattern, _mask_comments(text), re.MULTILINE)
    assert len(matches) == 1, "expected one source-owned " + label
    return matches[0]


def product_source():
    netdaemon = (ROOT / "src/netdaemon.cpp").read_text()
    netwin = (ROOT / "src/netwin.cpp").read_text()
    playerlock = (ROOT / "src/playerlock.cpp").read_text()
    game = (ROOT / "src/game.cpp").read_text()
    netclass = (ROOT / "src/net_class.cpp").read_text()
    netwin_header = (ROOT / "src/netwin.h").read_text()
    assert_leaf_collection_retired(ROOT)
    assert_current_owner(ROOT, 0x0052E160, "src/netdaemon.cpp")
    assert_current_owner(ROOT, 0x0052E370, "src/netdaemon.cpp")

    callback_bodies = [
        definition(netdaemon,
                   r"void NetDaemon::on_player_added\(char\s*\*\s*name,\s*unsigned long\s+id\)"),
        definition(netdaemon,
                   r"void NetDaemon::on_player_deleted\(char\s*\*\s*(?:name\s*)?,\s*unsigned long\s+id\)"),
    ]
    assert re.search(r"NetWinLobbyRow\s+NetWinLobbyRows\s*\[\s*8\s*\]", netwin_header)
    lock_bodies = [
        definition(playerlock, r"void PlayerLock::clear\(\)"),
        definition(playerlock, r"int PlayerLock::active\(\)"),
    ]
    accessors = [
        definition(netwin, r"void NetWinSessionBlock::write_row8_prefix\(uint8_t\s+value\)"),
        definition(netwin, r"char\s*\*\s*NetWinSessionBlock::row8_leader_name\(\)"),
        definition(netwin, r"char\s*\*\s*NetWinSessionBlock::row8_noun_faction\(\)"),
    ]

    globals_ = [
        one_line_definition(game, r"^int NextTurnReady;\s*$", "NextTurnReady"),
        one_line_definition(game, r"^int NetUpkeepPhase;\s*$", "NetUpkeepPhase"),
        one_line_definition(netclass, r"^uint32_t NetHostChangeSlot;\s*$", "NetHostChangeSlot"),
        one_line_definition(netwin, r"^NetWinSessionBlock NetWinSessionData;\s*$", "original session object"),
        one_line_definition(netwin, r"^NetWinSessionOptionsState NetWinSessionOptions;\s*$", "original options object"),
    ]
    return "\n\n".join(globals_ + accessors + lock_bodies + callback_bodies)


def run(tmp_path, product, scenario, flags=()):
    selected = os.environ.get("NETDAEMON_HOST_CXX")
    compiler = shutil.which(selected) if selected else (shutil.which("clang++") or shutil.which("g++"))
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    template = Path(__file__).with_name("netdaemon_owner_harness.cpp").read_text()
    cpp, exe = tmp_path / "owner.cpp", tmp_path / "owner"
    cpp.write_text(template.replace("/* PRODUCT */", product))
    command = [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
               "-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    command.extend(flags)
    command.extend([str(cpp), "-o", str(exe)])
    subprocess.run(command, check=True, capture_output=True, text=True)
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0",
               UBSAN_OPTIONS="halt_on_error=1")
    return subprocess.run([str(exe), scenario], capture_output=True, text=True,
                          timeout=15, env=env)


@pytest.mark.parametrize("scenario", [
    "added", "delete_found", "delete_inactive_lock", "delete_active_lock",
    "delete_slot8_miss",
])
def test_actual_netdaemon_and_storage_source(tmp_path, scenario):
    result = run(tmp_path, product_source(), scenario)
    assert result.returncode == 0, result.stdout + result.stderr


# The slot-8 case seeds the ledger low byte with 3. These controls do not prove
# arbitrary ledger bytes are valid indices for locks_.records_[player_id].
@pytest.mark.parametrize("old,new,scenario", [
    ("if (primary_state_.trailing_words_[0] < 2) {\n            removed_row.prefix_",
     "if (primary_state_.trailing_words_[0] <= 2) {\n            removed_row.prefix_", "delete_inactive_lock"),
    ("candidate.active() == 0", "false", "delete_inactive_lock"),
    ("strcpy(removed_row.noun_faction_, label_get(0xFE4 / 4));",
     "strcpy(removed_row.noun_faction_, removed_row.leader_name_);", "delete_found"),
])
def test_native_control_damage_is_detected(tmp_path, old, new, scenario):
    source = product_source()
    assert old in source
    marker = "void NetDaemon::on_player_deleted"
    head, tail = source.split(marker, 1)
    assert old in tail
    result = run(tmp_path, head + marker + tail.replace(old, new, 1), scenario)
    assert result.returncode != 0, "deliberate NetDaemon damage escaped fixture"
