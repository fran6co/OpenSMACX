"""Actual-source control for the pinned Probe technology-ranking comparison."""
from __future__ import annotations

import re
import shutil
import subprocess
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/probe.cpp").read_text()


def test_installed_rank_predicate_and_old_storage_mutant(tmp_path: Path) -> None:
    assert "probe_native_trade_source_word" not in SOURCE
    assert "int tech_ranking;" in (ROOT / "src/faction.h").read_text()
    assert "char tech_trade_source[92];" in (ROOT / "src/faction.h").read_text()

    # Extract the actual installed expression from its enclosing predicate so
    # tests compile source-derived code, not a separately retyped inequality.
    start = SOURCE.index("target_data.tech_ranking / 2 >")
    end = SOURCE.index(")) {\n                action_result = 5;", start)
    expression = SOURCE[start:end].strip()
    expression = re.sub(r"\s+", " ", expression)
    harness = f'''#include <cassert>
#include <cstring>
struct PlayerData {{ int tech_ranking; char tech_trade_source[92]; }};
static int stale_helper(const PlayerData &data) {{
    int value;
    std::memcpy(&value, data.tech_trade_source + 8, sizeof value);
    return value;
}}
static bool candidate(const PlayerData &target_data,
                      const PlayerData &actor_data) {{
    return {expression.replace("PlayersData[actor_faction_id].", "actor_data.")};
}}
static bool expected(int target, int actor) {{
    return target / 2 > actor / 2 + 4;
}}
int main() {{
    const int cases[][2] = {{
        {{10, 0}}, {{9, 0}}, {{11, 1}}, {{8, 0}}, {{-1, -11}},
        {{-9, -1}}, {{-10, -2}}, {{-11, -3}}, {{1, -9}}, {{-1, 9}}
    }};
    for (const auto &row : cases) {{
        PlayerData target{{}}, actor{{}};
        target.tech_ranking = row[0]; actor.tech_ranking = row[1];
        assert(candidate(target, actor) == expected(row[0], row[1]));
    }}
    PlayerData target{{}}, actor{{}};
    target.tech_ranking = 10; actor.tech_ranking = 0;
    assert(expected(target.tech_ranking, actor.tech_ranking));
    assert(! (stale_helper(target) / 2 > stale_helper(actor) / 2 + 4));
}}
'''
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), f"expected host compilers, got {compilers}"
    for index, compiler in enumerate(compilers):
        source = tmp_path / f"rank-{index}.cpp"
        binary = tmp_path / f"rank-{index}"
        source.write_text(harness)
        subprocess.run(
            [compiler, "-std=c++11", "-O1", "-Wall", "-Wextra", "-Werror",
             "-fsanitize=address,undefined", str(source), "-o", str(binary)],
            check=True,
        )
        result = subprocess.run(
            [str(binary)], capture_output=True, text=True,
            env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"},
        )
        assert result.returncode == 0, result.stderr

