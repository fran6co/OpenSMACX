"""Host controls over the real Console disband body, not native/game proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def disband_body():
    source = (ROOT / "src/console.cpp").read_text()
    start = source.index("void Console::disband(int veh_id) {")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated production disband body")


HARNESS = r'''#include <cstdint>
#include <cstdio>
#include <cstring>
#define __cdecl

struct Veh {
  int16_t x, y;
  uint32_t state;
  uint16_t flags, proto_id, unk;
  uint8_t faction_id;
};
struct VehPrototype {
  char veh_name[32];
  uint32_t ability_flags;
  uint8_t chassis_id, weapon_id, armor_id, reactor_id, carry_capacity, cost;
};
struct ChassisRecord { int triad; };
struct Player { uint32_t rule_flags; };
struct WeaponRecord { int8_t offense_rating; };
struct Base {
  int minerals_accumulated;
  int production_id_last;
  int queue_production_id[10];
  int minerals_accumulated_2;
};

Veh Vehs[8];
VehPrototype VehPrototypes[128];
ChassisRecord Chassis[16];
Player Players[8];
WeaponRecord Weapon[32];
Base Bases[8];
char StringTemp[256];
const char *ScriptPopupFile = "script";
int GameState, IsMultiplayerNet, BaseAtResult, PopupAnswer, CostFactorResult;
int PopupCalls, ParseCalls, CostCalls, SyncCalls, CheckCalls;
int KillCalls, DestroyCalls, MessageCalls, AwaitCalls;
int LastVehicleMessage[4], LastCostFaction;
const char *LastPopupText, *LastPopupPicture, *LastNetPopupText;
void (*PopupMutation)();
void (*CostMutation)();

struct MessagePopup {
  void pop(const char *text, int, int, const char *) { LastNetPopupText = text; }
} MessagePopupObject;
MessagePopup *NetMsgPopup = &MessagePopupObject;
struct Network {
  int lock_result, lock_calls, unlock_calls;
  int lock_veh(int *, int, int, int, int) { ++lock_calls; return lock_result; }
  void unlock_veh() { ++unlock_calls; }
  void await_exec(int) { ++AwaitCalls; }
} NetDaemonNet;
struct BaseWindow { void check_base(int) { ++CheckCalls; } } TutWinBaseWindow;
bool check(const char *name, bool passed) {
  if (!passed) std::printf("FAIL %s\n", name);
  return passed;
}

int __cdecl base_at(int, int) { return BaseAtResult; }
int __cdecl parse_says(int, const char *, int, int) { ++ParseCalls; return 0; }
int __cdecl popp(const char *, const char *text, int, const char *picture, void *) {
  ++PopupCalls; LastPopupText = text; LastPopupPicture = picture;
  if (PopupMutation) PopupMutation();
  return PopupAnswer;
}
int __cdecl cost_factor(int faction, int, int) {
  ++CostCalls; LastCostFaction = faction;
  if (CostMutation) CostMutation();
  return CostFactorResult;
}
void __cdecl synch_base(int) { ++SyncCalls; }
void __cdecl kill(int) { ++KillCalls; }
void __cdecl action_destruct(int) { ++DestroyCalls; }
void __cdecl message_veh(int a, int b, int c, int d) {
  ++MessageCalls; LastVehicleMessage[0]=a; LastVehicleMessage[1]=b;
  LastVehicleMessage[2]=c; LastVehicleMessage[3]=d;
}

class Console {
 public:
  int faction_id_ = 1;
  int field_23BE4_ = 9;
  void disband(int veh_id);
};
'''


TESTS = r'''
static int failures;
static void expect(const char *name, bool condition) { failures += !check(name, condition); }
static Console console;
static void reset() {
  std::memset(Vehs, 0, sizeof(Vehs));
  std::memset(VehPrototypes, 0, sizeof(VehPrototypes));
  std::memset(Chassis, 0, sizeof(Chassis));
  std::memset(Players, 0, sizeof(Players));
  std::memset(Weapon, 0, sizeof(Weapon));
  std::memset(Bases, 0, sizeof(Bases));
  std::memset(StringTemp, 0, sizeof(StringTemp));
  GameState=0; IsMultiplayerNet=0; BaseAtResult=0; PopupAnswer=1;
  CostFactorResult=3; PopupCalls=ParseCalls=CostCalls=SyncCalls=CheckCalls=0;
  KillCalls=DestroyCalls=MessageCalls=AwaitCalls=0;
  LastVehicleMessage[0]=LastVehicleMessage[1]=LastVehicleMessage[2]=LastVehicleMessage[3]=0;
  LastCostFaction=-1; LastPopupText=LastPopupPicture=LastNetPopupText=nullptr;
  PopupMutation=CostMutation=nullptr;
  NetDaemonNet.lock_result=NetDaemonNet.lock_calls=NetDaemonNet.unlock_calls=0;
  console.faction_id_=1; console.field_23BE4_=9;
  Vehs[0].x=7; Vehs[0].y=8; Vehs[0].proto_id=1; Vehs[0].faction_id=1;
  VehPrototypes[1].chassis_id=1; VehPrototypes[1].weapon_id=1; VehPrototypes[1].cost=5;
  Weapon[1].offense_rating=2; Chassis[1].triad=0;
}
static void mutate_to_refund_proto() { Vehs[0].proto_id=2; Vehs[0].faction_id=3; }
static void mutate_to_cost_proto() { Vehs[0].proto_id=3; }

static void test_initial_guard_and_lock_failure() {
  reset(); Vehs[0].flags=0x20;
  console.disband(0);
  expect("special-unit prompt before lock", NetDaemonNet.lock_calls==0 &&
         std::strcmp(LastNetPopupText,"DISBANDOBJ")==0);
  reset(); NetDaemonNet.lock_result=1;
  console.disband(0);
  expect("lock refusal does not unlock or show popup", NetDaemonNet.lock_calls==1 &&
         NetDaemonNet.unlock_calls==0 && PopupCalls==0);
}

static void test_foreign_faction_gate() {
  reset(); Vehs[0].faction_id=2; IsMultiplayerNet=1;
  console.disband(0);
  expect("multiplayer foreign unit is unlocked and rejected",
         NetDaemonNet.unlock_calls==1 && PopupCalls==0 && KillCalls==0);
  reset(); Vehs[0].faction_id=2; GameState=0x80;
  console.disband(0);
  expect("offline scenario bit permits foreign unit", PopupCalls==1 && KillCalls==1);
}

static void test_image_table_and_no_base_suffix() {
  const char *normal[3]={"unit_sm.pcx","navun_sm.pcx","air_sm.pcx"};
  const char *alien[3]={"alunit_sm.pcx","navun_sm.pcx","alair_sm.pcx"};
  for (int faction=0; faction!=2; ++faction) for (int triad=0; triad!=3; ++triad) {
    reset(); Chassis[1].triad=triad; Players[1].rule_flags=faction ? 0x80 : 0;
    console.disband(0);
    expect("triad/faction image table", std::strcmp(LastPopupPicture,
           faction ? alien[triad] : normal[triad])==0);
  }
  reset(); BaseAtResult=-1;
  console.disband(0);
  expect("no-base prompt gets suffix 2", std::strcmp(LastPopupText,"DISBAND2")==0);
}

static void test_refund_fields_and_eligibility() {
  reset(); Bases[0].minerals_accumulated=10; Bases[0].minerals_accumulated_2=4;
  Bases[0].queue_production_id[0]=77; Bases[0].production_id_last=-1;
  console.disband(0);
  expect("refund adds half cost", Bases[0].minerals_accumulated==17);
  expect("secondary mineral total follows accumulator", Bases[0].minerals_accumulated_2==17);
  expect("production id follows queue head", Bases[0].production_id_last==77);
  expect("refund synchronizes base and tutorial window", SyncCalls==1 && CheckCalls==1);

  reset(); Vehs[0].proto_id=64; VehPrototypes[64].cost=8; BaseAtResult=0;
  Weapon[0].offense_rating=-1; console.disband(0);
  expect("proto >= 64 bypasses weapon/15 exclusions", Bases[0].minerals_accumulated==12 && CostCalls==1);

  reset(); Vehs[0].proto_id=1; Weapon[1].offense_rating=-1; console.disband(0);
  expect("negative weapon offense excludes refund", CostCalls==0 && SyncCalls==0);
  reset(); Vehs[0].proto_id=15; console.disband(0);
  expect("proto 15 excludes refund", CostCalls==0 && SyncCalls==0);
  reset(); PopupAnswer=2; console.disband(0);
  expect("answer 2 excludes refund", CostCalls==0 && SyncCalls==0);
  reset(); BaseAtResult=-1; console.disband(0);
  expect("no base excludes refund", CostCalls==0 && SyncCalls==0);
}

static void test_callback_rereads_and_dispatch() {
  reset(); VehPrototypes[2].cost=8; VehPrototypes[3].cost=11;
  PopupMutation=mutate_to_refund_proto; CostMutation=mutate_to_cost_proto;
  console.disband(0);
  expect("popup-mutated proto/faction reread", LastCostFaction==3 &&
         Bases[0].minerals_accumulated==16);

  reset(); PopupAnswer=2; console.disband(0);
  expect("local answer 2 invokes destruction", DestroyCalls==1 && KillCalls==0);
  reset(); IsMultiplayerNet=1; PopupAnswer=2; console.disband(0);
  expect("network path sends and awaits", MessageCalls==1 && AwaitCalls==1 &&
         LastVehicleMessage[0]==0x2407 && LastVehicleMessage[1]==0 &&
         LastVehicleMessage[2]==2 && LastVehicleMessage[3]==0 &&
         DestroyCalls==0 && KillCalls==0);
}

int main() {
  test_initial_guard_and_lock_failure();
  test_foreign_faction_gate();
  test_image_table_and_no_base_suffix();
  test_refund_fields_and_eligibility();
  test_callback_rereads_and_dispatch();
  std::printf("%s (%d failures)\n", failures ? "FAIL" : "PASS", failures);
  return failures ? 1 : 0;
}
'''


def execute_disband(tmp_path, body):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ unavailable")
    executable = tmp_path / "disband"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    "-x", "c++", "-", "-o", str(executable)],
                   input=HARNESS + "\n" + body + "\n" + TESTS,
                   capture_output=True, text=True, check=True)
    return subprocess.run([str(executable)], capture_output=True, text=True)


def test_disband_guards_refunds_callbacks_and_dispatch(tmp_path):
    result = execute_disband(tmp_path, disband_body())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("before,after", [
    ("(IsMultiplayerNet || !(GameState & 0x80))",
     "(!IsMultiplayerNet && !(GameState & 0x80))"),
    ('    if (base_id < 0) strcat(StringTemp, "2");\n', ""),
    ("refund_proto_id >= 0x40", "refund_proto_id < 0x40"),
    ("            Bases[base_id].production_id_last = Bases[base_id].queue_production_id[0];\n", ""),
    ("VehPrototypes[Vehs[veh_id].proto_id].cost", "VehPrototypes[refund_proto_id].cost"),
])
def test_disband_behavior_damage_controls(tmp_path, before, after):
    body = disband_body()
    assert before in body
    result = execute_disband(tmp_path, body.replace(before, after, 1))
    assert result.returncode != 0
