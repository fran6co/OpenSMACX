"""Actual network tail layout and source phase-store controls.

Only selected statements and the tail declaration are compiled here; the full
NetWin owner controls cover lifecycle ordering, and native checks cover the ABI.
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file
from test_netdaemon_early_routes_oct12 import declaration


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / 'src/netwin.cpp').is_file())
PINNED_IMAGE_SHA256 = '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
EXEC_ADDRESS = 0x00482490
EXEC_SIGNATURE = 'int NetWin::exec() {'
SOURCE_PHASE_ONE = 'NetDaemonNet.primary_state_.session_phase_ = 1;'
SOURCE_PHASE_TWO = 'NetDaemonNet.primary_state_.session_phase_ = 2;'
WAITMASK_ANNOTATION = (
    '// RECOVERY_OBJECT: v3 0x0093E8EC ?ConsoleFactionsWaitingMask@@3HA '
    'member ?NetDaemonNet@@3VNetDaemon@@A primary_state_.trailing_words_[6]'
)


def _function_from_region(region: str, signature: str) -> str:
    start = region.find(signature)
    assert start >= 0, f'missing complete mapped function: {signature}'
    opening = region.find('{', start)
    assert opening >= 0
    depth = 0
    state = 'code'
    index = opening
    while index < len(region):
        char = region[index]
        following = region[index + 1] if index + 1 < len(region) else ''
        if state == 'code':
            if char == '/' and following == '/':
                state = 'line'; index += 2; continue
            if char == '/' and following == '*':
                state = 'block'; index += 2; continue
            if char == '"': state = 'string'
            elif char == "'": state = 'char'
            elif char == '{': depth += 1
            elif char == '}':
                depth -= 1
                if depth == 0:
                    return region[start:index + 1]
        elif state == 'line':
            if char == '\n': state = 'code'
        elif state == 'block':
            if char == '*' and following == '/':
                state = 'code'; index += 2; continue
        else:
            if char == '\\': index += 2; continue
            if state == 'string' and char == '"': state = 'code'
            elif state == 'char' and char == "'": state = 'code'
        index += 1
    raise AssertionError('unbalanced NetWin::exec source definition')


def _actual_inputs() -> tuple[str, str, str]:
    header = (ROOT / 'src/netdaemon.h').read_text(encoding='utf-8')
    tail = declaration(header, 'PrimaryLockStateTail')
    rows = [row for row in read_file(ROOT / 'src/netwin.cpp')
            if row.address == EXEC_ADDRESS]
    assert len(rows) == 1, 'native source reader must identify the complete exec owner'
    exec_body = _function_from_region(rows[0].region, EXEC_SIGNATURE)
    assert exec_body.count(SOURCE_PHASE_ONE) == 1
    assert exec_body.count(SOURCE_PHASE_TWO) == 1
    assert exec_body.count('NetDaemonNet.primary_state_.session_phase_ =') == 2
    assert exec_body.index(SOURCE_PHASE_ONE) < exec_body.index(SOURCE_PHASE_TWO)
    return tail, SOURCE_PHASE_ONE, SOURCE_PHASE_TWO


CPP = r'''#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
using uint32_t = std::uint32_t;

TAIL_DECLARATION

static_assert(sizeof(PrimaryLockStateTail) == 0x38, "tail size");
static_assert(sizeof(int) == 4, "session phase word size");
static_assert(offsetof(PrimaryLockStateTail, flags_) == 0x0C, "lock flag offset");
static_assert(offsetof(PrimaryLockStateTail, session_phase_) == 0x10, "phase offset");
static_assert(offsetof(PrimaryLockStateTail, trailing_words_) == 0x14, "tail words offset");
static_assert(std::is_same<decltype(PrimaryLockStateTail::session_phase_), int>::value,
              "phase is signed int storage");

struct NetDaemonSnippetFixture { PrimaryLockStateTail primary_state_; };
static NetDaemonSnippetFixture NetDaemonNet;

int main() {
    auto &state = NetDaemonNet.primary_state_;
    state.leading_words_[0] = 0xA1000000u;
    state.leading_words_[1] = 0xA1000001u;
    state.leading_words_[2] = 0xA1000002u;
    state.flags_ = 0xB10CF1A6u;
    state.session_phase_ = -0x12345;
    for (int i = 0; i < 9; ++i)
        state.trailing_words_[i] = 0xC0140000u + static_cast<uint32_t>(i);

    unsigned char expected[sizeof state];
    std::memcpy(expected, &state, sizeof state);

    ACTUAL_PHASE_ONE
    assert(state.session_phase_ == 1);
    assert(state.flags_ == 0xB10CF1A6u);
    const int phase_one = 1;
    std::memcpy(expected + offsetof(PrimaryLockStateTail, session_phase_),
                &phase_one, sizeof phase_one);
    assert(std::memcmp(expected, &state, sizeof state) == 0);

    ACTUAL_PHASE_TWO
    assert(state.session_phase_ == 2);
    assert(state.flags_ == 0xB10CF1A6u);
    const int phase_two = 2;
    std::memcpy(expected + offsetof(PrimaryLockStateTail, session_phase_),
                &phase_two, sizeof phase_two);
    assert(std::memcmp(expected, &state, sizeof state) == 0);
    return 0;
}
'''


def _assert_source_layout_bindings() -> None:
    net_h = (ROOT / 'src/netdaemon.h').read_text(encoding='utf-8')
    net_cpp = (ROOT / 'src/netdaemon.cpp').read_text(encoding='utf-8')
    assert '// RECOVERY_MEMBER: v1 NetDaemon primary_state_ 0x1B30 0x005389F0' in net_h
    assert 'extern NetDaemon NetDaemonNet;  // 0x0093CD90' in net_h
    assert WAITMASK_ANNOTATION in net_cpp
    assert 0x0093CD90 + 0x1B30 + 0x10 == 0x0093E8D0
    assert 0x0093CD90 + 0x1B30 + 0x0C == 0x0093E8CC
    assert 0x0093CD90 + 0x1B30 + 0x14 + 6 * 4 == 0x0093E8EC


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_net_session_phase_actual_owner_tail_and_exec_statements(tmp_path: Path,
                                                                  compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f'{compiler_name} unavailable')
    _assert_source_layout_bindings()
    tail, phase_one, phase_two = _actual_inputs()
    generated = (CPP.replace('TAIL_DECLARATION', tail)
                 .replace('ACTUAL_PHASE_ONE', phase_one)
                 .replace('ACTUAL_PHASE_TWO', phase_two))
    cpp = tmp_path / 'net_session_phase_actual_statements.cpp'
    exe = tmp_path / 'net_session_phase_actual_statements'
    cpp.write_text(generated, encoding='utf-8')
    built = subprocess.run(
        [compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror',
         str(cpp), '-o', str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert ran.returncode == 0, ran.stdout + ran.stderr

