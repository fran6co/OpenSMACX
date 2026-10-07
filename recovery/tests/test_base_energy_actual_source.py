"""Execute the live mapped Base energy owner with bounded host controls."""
from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CPP_PREFIX = r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define __cdecl
typedef int BOOL;
static const int MaxPlayerNum = 8;
static const int PROP_REPEAL_GLOBAL_TRADE_PACT = 0;
static const int PROP_GLOBAL_TRADE_PACT = 1;
static const int DTREATY_TREATY = 2;
static const int DTREATY_PACT = 1;
static const int RFLAG_ALIEN = 0x80;
static const int FAC_BIOLOGY_LAB = 9;
static const int FAC_HOLOGRAM_THEATRE = 11;
static const int FAC_NETWORK_NODE = 8;
static const int FAC_ENERGY_BANK = 7;
static const int FAC_RESEARCH_HOSPITAL = 17;
static const int FAC_NANOHOSPITAL = 18;
static const int FAC_TREE_FARM = 13;
static const int FAC_HYBRID_FOREST = 14;
static const int FAC_FUSION_LAB = 15;
static const int FAC_QUANTUM_LAB = 16;
enum { SP_VIRTUAL_WORLD=6, SP_SUPERCOLLIDER=12, SP_THEORY_OF_EVERYTHING=18,
       SP_SPACE_ELEVATOR=27, SP_LONGEVITY_VACCINE=14,
       SP_NETWORK_BACKBONE=21, SP_PLANETARY_ENERGY_GRID=36 };
struct SocialCategory { enum { ECONOMICS=1 }; unsigned models_[4]; };
struct SocialEffect { enum { EFFICIENCY=0 }; int values_[11]; };
struct PlayerData {
  unsigned diplo_treaties[8]; int sanction_turns; int tech_commerce_bonus;
  int se_alloc_psych, se_alloc_labs; SocialCategory soc_category_pending;
  SocialEffect soc_effect_pending; unsigned current_num_bases; int unk_13;
};
struct Player { unsigned rule_flags; int rule_research; };
struct Base {
  unsigned char faction_id_current; int energy_intake_1, energy_intake_2;
  int energy_consumption, energy_inefficiency, energy_surplus;
  int mineral_surplus;
  int psych_total, economy_total, labs_total, unk_10;
  int specialist_total; unsigned spec[2]; unsigned char facilities_built[12];
  int queue_production_id[10], production_id_last;
};
struct CitizenDef { int preq_tech, obsol_tech, ops_bonus, psych_bonus, research_bonus; };
struct StateFields { int council_resolution_count_[2]; int governor_faction_;
  int base_current_count_; int secret_project_[64]; };
struct State { StateFields fields_; };
Base bases[8], *BaseCurrent = &bases[0];
Base (&Bases)[8] = bases;
Player Players[8]; PlayerData PlayersData[8]; CitizenDef Citizen[7];
State GameDataStateStorage;
int BaseIDCurrentSelected, BaseCurrentConvoyTo[3], BaseCurrentConvoyFrom[3];
int TechCommerceCount=1, BaseUpkeepStage=0, ExpansionEnabled=0;
int BaseCommerceReceipts[7], BaseCommercePartnerReceipts[7];
int black_market_calls, inject_black_market_receipt;
int black_market(int amount) {
  (void)amount;
  ++black_market_calls;
  if (inject_black_market_receipt && black_market_calls == 2) BaseCommerceReceipts[1] = 50;
  return 0;
}
int base_rank(int faction, int rank) { (void)faction; (void)rank; return 1; }
int best_specialist() { return 0; }
static unsigned long long tech_owned[8];
int has_tech(int tech, int faction) { return tech > 0 && tech < 64 && ((tech_owned[faction] >> tech) & 1); }
int has_fac(int facility, int base_id, int queue_count) {
  (void)queue_count; return (bases[base_id].facilities_built[facility >> 3] >> (facility & 7)) & 1;
}
int has_project(int project, int faction) {
  const int base_id = GameDataStateStorage.fields_.secret_project_[project];
  return base_id >= 0 && bases[base_id].faction_id_current == faction;
}
void base_psych() {}
'''

CPP_MAIN = r'''
void reset() {
  memset(bases, 0, sizeof(bases)); memset(Players, 0, sizeof(Players));
  memset(PlayersData, 0, sizeof(PlayersData)); memset(Citizen, 0, sizeof(Citizen));
  memset(&GameDataStateStorage, 0, sizeof(GameDataStateStorage));
  memset(BaseCurrentConvoyTo, 0, sizeof(BaseCurrentConvoyTo));
  memset(BaseCurrentConvoyFrom, 0, sizeof(BaseCurrentConvoyFrom));
  memset(BaseCommerceReceipts, 0, sizeof(BaseCommerceReceipts));
  memset(BaseCommercePartnerReceipts, 0, sizeof(BaseCommercePartnerReceipts));
  black_market_calls=0; inject_black_market_receipt=0;
  memset(tech_owned, 0, sizeof(tech_owned));
  for (int i=0;i<64;++i) GameDataStateStorage.fields_.secret_project_[i] = -1;
  BaseCurrent = &Bases[0]; BaseIDCurrentSelected = 0;
  GameDataStateStorage.fields_.base_current_count_ = 1;
  TechCommerceCount=1; BaseUpkeepStage=0; ExpansionEnabled=0;
  PlayersData[1].se_alloc_psych=2; PlayersData[1].se_alloc_labs=3;
  Bases[0].faction_id_current=1; Bases[0].energy_intake_1=30; Bases[0].energy_intake_2=30;
}
int main() {
  // Upkeep bank uses pre-second-market surplus; project multipliers are later.
  reset(); BaseUpkeepStage=1; base_energy();
  if (PlayersData[1].unk_13 != 30 || Bases[0].energy_surplus != 30) return 1;
  reset(); BaseUpkeepStage=1; Bases[0].energy_intake_2=150000; base_energy();
  if (PlayersData[1].unk_13 != 99999) return 9;
  reset(); BaseUpkeepStage=1; Bases[0].energy_intake_2=-20; base_energy();
  if (PlayersData[1].unk_13 != 0) return 10;

  // The project multipliers and economics-conditioned vaccine effect compose.
  reset();
  GameDataStateStorage.fields_.secret_project_[SP_SUPERCOLLIDER]=0;
  GameDataStateStorage.fields_.secret_project_[SP_THEORY_OF_EVERYTHING]=0;
  GameDataStateStorage.fields_.secret_project_[SP_SPACE_ELEVATOR]=0;
  GameDataStateStorage.fields_.secret_project_[SP_LONGEVITY_VACCINE]=0;
  PlayersData[1].soc_category_pending.models_[SocialCategory::ECONOMICS]=1;
  base_energy();
  if (Bases[0].labs_total != 30 || Bases[0].economy_total != 41) return 2;
  reset(); GameDataStateStorage.fields_.base_current_count_=2;
  GameDataStateStorage.fields_.secret_project_[SP_VIRTUAL_WORLD]=1;
  Bases[1].faction_id_current=1;
  PlayersData[1].soc_effect_pending.values_[SocialEffect::EFFICIENCY]=4;
  base_energy();
  if (Bases[0].labs_total != 9) return 11;
  reset(); GameDataStateStorage.fields_.base_current_count_=2;
  GameDataStateStorage.fields_.secret_project_[SP_VIRTUAL_WORLD]=0;
  PlayersData[1].soc_effect_pending.values_[SocialEffect::EFFICIENCY]=4;
  base_energy();
  if (Bases[0].labs_total != 14) return 13;

  // Independently derived social-allocation cases distinguish both native
  // branch ordering and the negative-delta fallback's psych/labs operand order.
  reset(); PlayersData[1].se_alloc_psych=1; PlayersData[1].se_alloc_labs=7;
  base_energy();
  if (Bases[0].labs_total != 18 || Bases[0].economy_total != 4) return 7;
  reset(); PlayersData[1].se_alloc_psych=5; PlayersData[1].se_alloc_labs=1;
  base_energy();
  if (Bases[0].labs_total != 3 || Bases[0].economy_total != 11) return 8;

  // Obsolete specialist promotes to the first eligible nonobsolete higher-yield
  // entry and publishes the new packed nibble.
  reset(); Bases[0].specialist_total=1; Bases[0].spec[0]=1;
  Citizen[1].ops_bonus=Citizen[1].psych_bonus=Citizen[1].research_bonus=1;
  Citizen[1].obsol_tech=11; tech_owned[1] |= (1ull<<11);
  Citizen[2].preq_tech=12; Citizen[2].ops_bonus=2;
  Citizen[2].psych_bonus=2; Citizen[2].research_bonus=2;
  tech_owned[1] |= (1ull<<12);
  base_energy();
  if ((Bases[0].spec[0] & 15u) != 2 || Bases[0].economy_total != 16
      || Bases[0].psych_total != 8 || Bases[0].labs_total != 9) return 3;

  // Expansion alien subtotal is defined here by an eligible trade path; the
  // historical no-trade uninitialized-local path is intentionally not invoked.
  reset(); ExpansionEnabled=1; Players[1].rule_flags=RFLAG_ALIEN;
  PlayersData[1].diplo_treaties[2]=DTREATY_TREATY;
  PlayersData[2].current_num_bases=1; GameDataStateStorage.fields_.base_current_count_=2;
  Bases[1].faction_id_current=2; Bases[0].facilities_built[0]=3;
  GameDataStateStorage.fields_.secret_project_[0]=0;
  base_energy();
  if (Bases[0].energy_intake_2 != 33) return 4;

  // Network Backbone includes built network nodes from every current base and
  // published nonalien/unsanctioned commerce receipts.
  reset(); GameDataStateStorage.fields_.base_current_count_=2;
  Bases[1].facilities_built[FAC_NETWORK_NODE >> 3] = 1u << (FAC_NETWORK_NODE & 7);
  PlayersData[2].diplo_treaties[1]=DTREATY_TREATY;
  PlayersData[1].diplo_treaties[2]=DTREATY_TREATY;
  PlayersData[2].current_num_bases=1; Bases[1].faction_id_current=2;
  base_energy();
  const int labs_without_backbone = Bases[0].labs_total;
  reset(); GameDataStateStorage.fields_.base_current_count_=2;
  Bases[1].facilities_built[FAC_NETWORK_NODE >> 3] = 1u << (FAC_NETWORK_NODE & 7);
  GameDataStateStorage.fields_.secret_project_[SP_NETWORK_BACKBONE]=0;
  PlayersData[2].diplo_treaties[1]=DTREATY_TREATY;
  PlayersData[1].diplo_treaties[2]=DTREATY_TREATY;
  PlayersData[2].current_num_bases=1; Bases[1].faction_id_current=2;
  base_energy();
  if (Bases[0].labs_total <= labs_without_backbone) return 5;

  // The current-faction sanction guard suppresses Backbone's receipt pass.
  // The stub injects a late ledger value after the market callback to make the
  // guard observable independently of the normal earlier receipt zeroing.
  reset(); GameDataStateStorage.fields_.base_current_count_=2;
  GameDataStateStorage.fields_.secret_project_[SP_NETWORK_BACKBONE]=0;
  PlayersData[1].sanction_turns=1; PlayersData[2].current_num_bases=1;
  PlayersData[2].diplo_treaties[1]=DTREATY_TREATY;
  PlayersData[1].diplo_treaties[2]=DTREATY_TREATY;
  Bases[1].faction_id_current=2; Bases[1].facilities_built[FAC_NETWORK_NODE >> 3]
    = 1u << (FAC_NETWORK_NODE & 7);
  PlayersData[1].soc_effect_pending.values_[SocialEffect::EFFICIENCY]=4;
  inject_black_market_receipt=1;
  base_energy();
  if (Bases[0].labs_total != 10) return 14;

  // The final -69 production sentinel reads Base +0xD4 (mineral_surplus),
  // or its rounded 5/4-scaled amount when this faction owns the Energy Grid.
  reset(); Bases[0].mineral_surplus=2; Bases[0].queue_production_id[0]=-0x45;
  Bases[0].production_id_last=-0x45;
  GameDataStateStorage.fields_.secret_project_[SP_PLANETARY_ENERGY_GRID]=0;
  PlayersData[1].soc_effect_pending.values_[SocialEffect::EFFICIENCY]=4;
  base_energy();
  if (Bases[0].economy_total != 25) return 6;
  reset(); Bases[0].mineral_surplus=-2; Bases[0].queue_production_id[0]=-0x45;
  Bases[0].production_id_last=-0x45;
  base_energy();
  if (Bases[0].economy_total != 14) return 12;
  return 0;
}
'''


def test_actual_base_energy_preserves_original_paths() -> None:
    source_path = Path(os.environ.get("OPENSMACX_BASE_SOURCE", ROOT / "src/base.cpp"))
    projected = source_path.read_text()
    start = projected.index("void __cdecl base_energy() {")
    end = projected.index("\n// ORIGINAL:", start)
    body = projected[start:end]
    compilers = [c for c in ("g++", "clang++") if shutil.which(c)]
    if not compilers:
        raise SystemExit("neither g++ nor clang++ is available")
    with tempfile.TemporaryDirectory(prefix="base-energy-source-") as td:
        cpp = Path(td) / "actual_base_energy.cpp"
        exe = Path(td) / "base_energy_controls"
        cpp.write_text(CPP_PREFIX + "\n" + body + "\n" + CPP_MAIN)
        for compiler in compilers:
            subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
            print(f"PASS {compiler}: extracted live base_energy source body and behavioral controls")
