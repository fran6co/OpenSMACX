"""Actual CouncWin mask stages and callback-visible byte rereads.

These host controls compile extracted current owner bodies against the
production-shaped lock-mask union. They are behavioral controls, not original
ABI, complete owner-TU, or game-runtime proof.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from councwin_owner_control_fixture import (
    INIT_HARNESS,
    LEFT_CLICK_HARNESS,
    adapt_body,
    adapt_harness,
    extract_function,
)

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/councwin.cpp"


def build_and_run(tmp_path, compiler_name, harness, *, expect_success=True):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    unit = tmp_path / f"councwin-mask-{compiler_name}.cpp"
    executable = tmp_path / f"councwin-mask-{compiler_name}"
    unit.write_text(harness)
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-O2",
         str(unit), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True, timeout=15)
    if expect_success:
        assert run.returncode == 0, run.stdout + run.stderr
    else:
        assert run.returncode != 0, "behavioral mutation unexpectedly passed"


def left_click_harness(body):
    source = SOURCE.read_text()
    prefix = LEFT_CLICK_HARNESS.split("int main() {")[0]
    test_main = r'''int main() {
  CouncWin win;
  ConsoleGlobal.faction_id_=1;
  IsMultiplayerNet=0;
  win.spot_.result=0; win.spot_.faction=2;
  LockEnableMask=(1u<<1)|(1u<<10); // target 2 is alive at the early gate
  PlayersData[2].unk_29[0]=0;
  prompt_result=1; mutate_alive_after_prompt=1; alive_mask_after_prompt=0;
  win.on_left_click(0,0);
  bool eliminated=false, bought=false;
  for(const std::string &event: events) {
    eliminated |= event=="pop:ELIMINATED";
    bought |= event.find("buy:")==0;
  }
  assert(eliminated && !bought);
}
'''
    fixture = prefix.replace("NetMsg *NetMsgPopup = &net_msg;",
                             "NetMsg *NetMsgPopup = &net_msg; NetMsg NetMsgPrimary;")
    return adapt_harness(fixture.replace("BODY", body) + test_main)


def government_loop_harness(body, *, mutation_case):
    prefix = LEFT_CLICK_HARNESS.split("int main() {")[0]
    scenario = {
        "alive_nonhuman": r'''int main() {
  CouncWin win;
  ConsoleGlobal.faction_id_=1;
  win.spot_.result=0; win.spot_.faction=0;
  LockEnableMask=(1u<<1)|(1u<<3)|(1u<<8)|(1u<<10); // faction 3 human but dead; faction 2 alive and nonhuman
  win.on_left_click(0,0);
  assert((government_option_ids == std::vector<int>{2}));
}
''',
        "refresh_next": r'''int main() {
  CouncWin win;
  ConsoleGlobal.faction_id_=1;
  win.spot_.result=0; win.spot_.faction=0;
  LockEnableMask=(1u<<1)|(1u<<8)|(1u<<9)|(1u<<10);
  mutate_alive_after_eligible_faction=1;
  alive_mask_after_eligible=static_cast<uint8_t>((1u<<0)|(1u<<1));
  win.on_left_click(0,0);
  assert((government_option_ids == std::vector<int>{1}));
}
''',
    }[mutation_case]
    fixture = prefix.replace("NetMsg *NetMsgPopup = &net_msg;",
                             "NetMsg *NetMsgPopup = &net_msg; NetMsg NetMsgPrimary;")
    return adapt_harness(fixture.replace("BODY", body) + scenario)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("stale_cache", [False, True])
def test_on_left_click_observes_post_prompt_alive_byte(tmp_path, compiler_name, stale_cache):
    source = SOURCE.read_text()
    body = adapt_body(extract_function(source, "void CouncWin::on_left_click(int x, int y)"))
    fresh_read = "LockMasks.bytes_.alive_mask_ & clicked_bit"
    assert body.count(fresh_read) == 1
    if stale_cache:
        body = body.replace(fresh_read, "alive & clicked_bit", 1)
    build_and_run(tmp_path, compiler_name, left_click_harness(body),
                  expect_success=not stale_cache)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [None, "human-mask", "cached-mask"])
def test_government_candidates_use_live_alive_membership_per_iteration(
    tmp_path, compiler_name, mutation
):
    source = SOURCE.read_text()
    body = adapt_body(extract_function(source, "void CouncWin::on_left_click(int x, int y)"))
    assert "const uint8_t alive = static_cast<uint8_t>(LockMasks.enable_mask_ >> 8);" in body
    live_condition = "LockMasks.bytes_.alive_mask_ &"
    assert body.count(live_condition) == 2  # early target snapshot is distinct; post-prompt and candidate stages are live
    candidate_condition = "if ((LockMasks.bytes_.alive_mask_ &\n                 (1u << faction)) == 0 || !eligible(faction)) {"
    assert body.count(candidate_condition) == 1
    if mutation == "human-mask":
        body = body.replace(candidate_condition,
            "if ((LockMasks.bytes_.human_mask_ &\n                 (1u << faction)) == 0 || !eligible(faction)) {")
    elif mutation == "cached-mask":
        body = body.replace(candidate_condition,
            "if ((alive & (1u << faction)) == 0 || !eligible(faction)) {")
    build_and_run(tmp_path, compiler_name,
                  government_loop_harness(body, mutation_case="alive_nonhuman"),
                  expect_success=mutation != "human-mask")
    build_and_run(tmp_path, compiler_name,
                  government_loop_harness(body, mutation_case="refresh_next"),
                  expect_success=mutation != "cached-mask")


def init_harness(body):
    source = SOURCE.read_text()
    helper = extract_function(source, "static void set_council_rect(")
    return adapt_harness(INIT_HARNESS.replace("BODY", helper + "\n" + body))


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("cached_alive", [False, True])
def test_init_refreshes_alive_byte_before_later_candidate_eligibility(
    tmp_path, compiler_name, cached_alive
):
    source = SOURCE.read_text()
    body = adapt_body(extract_function(source, "void CouncWin::init(int proposal, int faction_id)"))
    live_candidate_read = (
        "LockMasks.bytes_.alive_mask_ & (1u << candidate)"
    )
    assert body.count(live_candidate_read) == 1
    if cached_alive:
        body = body.replace(live_candidate_read, "alive_factions & (1u << candidate)", 1)
    build_and_run(tmp_path, compiler_name, init_harness(body),
                  expect_success=not cached_alive)

