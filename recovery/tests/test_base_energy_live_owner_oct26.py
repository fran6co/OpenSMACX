"""Execute Base energy's pin-observed live-owner reloads with controlled callbacks."""
from pathlib import Path
import shutil
import subprocess
import tempfile

from recovery.tests.test_base_energy_actual_source import CPP_PREFIX

ROOT = Path(__file__).resolve().parents[2]


def test_base_energy_reloads_live_base_after_both_black_market_callbacks() -> None:
    """The pin rereads BaseCurrent after each market callback; keep faction captured."""
    source_path = ROOT / "src/base.cpp"
    projected = source_path.read_text()
    start = projected.index("void __cdecl base_energy() {")
    end = projected.index("\n// ORIGINAL:", start)
    body = projected[start:end]
    helper_start = projected.index("int base_yield_signed_bits(")
    helper_end = projected.index("int base_yield_denominator(", helper_start)
    helpers = projected[helper_start:helper_end]

    old_stub = r'''int black_market_calls, inject_black_market_receipt;
int black_market(int amount) {
  (void)amount;
  ++black_market_calls;
  if (inject_black_market_receipt && black_market_calls == 2) BaseCommerceReceipts[1] = 50;
  return 0;
}
int base_rank(int faction, int rank) { (void)faction; (void)rank; return 1; }'''
    live_stub = r'''int black_market_calls, inject_black_market_receipt;
int live_owner_switch, black_market_inputs[2], base_rank_calls, last_rank_faction;
int black_market(int amount) {
  const int call = black_market_calls++;
  if (call < 2) black_market_inputs[call] = amount;
  if (live_owner_switch && call == 0) BaseCurrent = &Bases[1];
  if (live_owner_switch && call == 1) BaseCurrent = &Bases[2];
  return call == 0 ? 7 : 9;
}
int base_rank(int faction, int rank) {
  (void)rank; ++base_rank_calls; last_rank_faction = faction; return 1;
}'''
    assert old_stub in CPP_PREFIX, "expected callback stub anchor changed"
    live_prefix = CPP_PREFIX.replace(old_stub, live_stub, 1)

    live_main = r'''
int main() {
  memset(bases, 0, sizeof(bases)); memset(Players, 0, sizeof(Players));
  memset(PlayersData, 0, sizeof(PlayersData)); memset(Citizen, 0, sizeof(Citizen));
  memset(&GameDataStateStorage, 0, sizeof(GameDataStateStorage));
  memset(BaseCurrentConvoyTo, 0, sizeof(BaseCurrentConvoyTo));
  memset(BaseCurrentConvoyFrom, 0, sizeof(BaseCurrentConvoyFrom));
  memset(BaseCommerceReceipts, 0, sizeof(BaseCommerceReceipts));
  memset(BaseCommercePartnerReceipts, 0, sizeof(BaseCommercePartnerReceipts));
  memset(tech_owned, 0, sizeof(tech_owned));
  black_market_calls=0; inject_black_market_receipt=0; live_owner_switch=1;
  base_rank_calls=0; last_rank_faction=-1;
  black_market_inputs[0]=black_market_inputs[1]=-1;
  for (int i=0;i<64;++i) GameDataStateStorage.fields_.secret_project_[i] = -1;
  BaseCurrent=&Bases[0]; BaseIDCurrentSelected=0;
  GameDataStateStorage.fields_.base_current_count_=3;
  TechCommerceCount=1; BaseUpkeepStage=0; ExpansionEnabled=0;
  BaseCurrentConvoyFrom[2]=3;
  Bases[0].faction_id_current=1; Bases[0].intake_2[2]=20;
  Bases[1].faction_id_current=2; Bases[1].intake_1[2]=0;
  Bases[1].intake_2[2]=40; Bases[1].energy_consumption=5;
  Bases[2].faction_id_current=3; Bases[2].intake_2[2]=60;
  Bases[2].energy_consumption=10;
  PlayersData[1].diplo_treaties[2]=DTREATY_TREATY;
  PlayersData[2].diplo_treaties[1]=DTREATY_TREATY;
  PlayersData[2].current_num_bases=1;
  base_energy();
  if (black_market_calls != 2) return 1;
  if (black_market_inputs[0] != 17 || black_market_inputs[1] != 35) return 2;
  if (Bases[1].energy_inefficiency != 7 || Bases[1].energy_surplus != 28) return 3;
  if (Bases[2].energy_inefficiency != 9 || Bases[2].energy_surplus != 41) return 4;
  if (Bases[0].energy_inefficiency != 0) return 5;
  if (BaseCurrent != &Bases[2]) return 6;
  // The partner-2 treaty was selected with entry faction 1 even after the
  // first callback changed BaseCurrent to a base owned by faction 2.
  if (base_rank_calls != 1 || last_rank_faction != 2) return 7;
  return 0;
}
'''

    # Negative control recreates the original entry-bound Base reference while
    # keeping the same callback and oracle. This is a source-derived mutation,
    # not a hand-written second implementation.
    cached_body = body.replace(
        "    const int faction = BaseCurrent->faction_id_current;",
        "    Base &base = *BaseCurrent;\n    const int faction = base.faction_id_current;",
        1,
    ).replace("BaseCurrent->", "base.")
    assert cached_body != body and "Base &base = *BaseCurrent;" in cached_body

    compilers = [c for c in ("g++", "clang++") if shutil.which(c)]
    assert compilers, "neither g++ nor clang++ is available"
    with tempfile.TemporaryDirectory(prefix="base-energy-live-owner-") as td:
        cpp = Path(td) / "live_base_energy.cpp"
        exe = Path(td) / "base_energy_live_owner"
        for compiler in compilers:
            warning_flags = ["-Wno-error=maybe-uninitialized"] if Path(compiler).name.startswith("g++") else []
            for optimization in ("-O0", "-O2"):
                for candidate, expected in ((body, 0), (cached_body, -1)):
                    cpp.write_text(live_prefix + "\n" + helpers + "\n" + candidate + "\n" + live_main)
                    subprocess.run([compiler, "-std=c++11", optimization, "-Wall", "-Wextra", "-Werror", *warning_flags, str(cpp), "-o", str(exe)], check=True)
                    result = subprocess.run([str(exe)], check=False)
                    if expected == 0:
                        assert result.returncode == 0, f"{compiler} {optimization}: live-owner source failed: {result.returncode}"
                    else:
                        assert result.returncode != 0, f"{compiler} {optimization}: cached-owner mutant escaped the reload oracle"
