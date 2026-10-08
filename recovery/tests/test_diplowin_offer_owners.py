"""Complete source-owned offer methods; host controls are not native ABI proof."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from test_selectpart_vehicle_owners import _definition

ROOT = Path(__file__).resolve().parents[2]
SIGNATURES = ("void DiploWin::UNK3(int a1)",
              "void DiploWin::kill_offer(int a1, int a2)",
              "void DiploWin::add_offer(int a1, int a2, int a3)")


def product():
    source = (ROOT / "src/diplowin.cpp").read_text()
    return "\n".join(_definition(source, signature) for signature in SIGNATURES)


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
static int DiploOffersDirty;
struct Message { int code, who, a, b, c, d; };
static std::vector<Message> messages;
static std::vector<int> events;
struct DiploWin {
  ACTUAL_FIELDS
  virtual void redraw() { assert(!field_A24_ && !field_A28_ && DiploOffersDirty==1); events.push_back(2); }
  void UNK3(int); void kill_offer(int,int); void add_offer(int,int,int);
};
static DiploWin *message_owner;
static int message_changes_count;
static void message_data(int code,int who,int a,int b,int c,int d) {
  messages.push_back({code,who,a,b,c,d}); events.push_back(1);
  if(message_changes_count && code==0x1507) message_owner->offer_count_[0]=2;
}
PRODUCT
static void reset(DiploWin &w) {
  w.offer_count_[0]=w.offer_count_[1]=0;
  for(int side=0;side<2;++side)for(int i=0;i<8;++i){ w.offer_type_[side][i]=30+i; w.offer_value_[side][i]=80+i; }
  w.field_A24_=9; w.field_A28_=10; w.faction_id_with_=73;
  messages.clear(); events.clear(); DiploOffersDirty=0;
  message_owner=&w; message_changes_count=0;
}
int main() {
  DiploWin w;
  for(int side=0;side<2;++side) {
    reset(w); w.offer_count_[0]=4; w.offer_count_[1]=6; w.UNK3(side);
    assert(w.offer_count_[side]==0 && w.offer_count_[1-side]==(side?4:6));
    assert(!w.field_A24_ && !w.field_A28_ && !DiploOffersDirty && events.empty());
    assert(w.offer_type_[side][0]==30 && w.offer_value_[side][0]==80);
    reset(w); w.offer_count_[side]=3; w.kill_offer(side,0);
    assert(w.offer_count_[side]==2 && w.offer_count_[1-side]==0);
    assert(w.offer_type_[side][0]==31 && w.offer_type_[side][1]==32 && w.offer_type_[side][2]==32);
    assert(w.offer_value_[side][0]==81 && w.offer_value_[side][1]==82 && w.offer_value_[side][2]==82);
    assert(events==(side?std::vector<int>{2}:std::vector<int>{1,2}));
    if(!side) assert(messages[0].code==0x1507 && messages[0].who==73 && messages[0].a==0);
    reset(w); w.offer_count_[side]=3; w.kill_offer(side,2);
    assert(w.offer_count_[side]==2 && w.offer_type_[side][1]==31 && w.offer_value_[side][2]==82);
    reset(w); w.offer_count_[side]=3; w.kill_offer(side,3);
    assert(w.offer_count_[side]==3 && w.field_A24_==9 && w.field_A28_==10 && !DiploOffersDirty);
    assert(events==(side?std::vector<int>{}:std::vector<int>{1}));
    reset(w); w.kill_offer(side,0);
    assert(!w.offer_count_[side] && w.field_A24_==9 && !DiploOffersDirty);
    // All treaty classes replace the first existing treaty without appending.
    for(int type=2;type<=4;++type) {
      reset(w); w.offer_count_[side]=3; w.offer_type_[side][0]=0; w.offer_type_[side][1]=4;
      w.add_offer(side,type,456);
      assert(w.offer_count_[side]==3 && w.offer_type_[side][0]==0 && w.offer_type_[side][1]==type);
      assert(w.offer_value_[side][1]==456 && w.offer_type_[side][2]==32);
      assert(events==(side?std::vector<int>{2}:std::vector<int>{1,2}));
      if(!side) assert(messages[0].code==0x1506 && messages[0].who==73 && messages[0].a==type && messages[0].b==456 && !messages[0].c && !messages[0].d);
    }
    reset(w); w.offer_count_[side]=2; w.offer_type_[side][0]=1; w.add_offer(side,1,45);
    assert(w.offer_count_[side]==2 && w.offer_value_[side][0]==45);
    for(int amount: {0,-1}) {
      reset(w); w.offer_count_[side]=2; w.offer_type_[side][0]=1; w.add_offer(side,1,amount);
      assert(w.offer_count_[side]==1 && w.offer_type_[side][0]==31 && w.offer_value_[side][0]==81);
      if(!side) assert(messages.size()==1 && messages[0].code==0x1507);
    }
    // Nonpositive cash is still appended if there was no existing cash offer.
    reset(w); w.add_offer(side,1,0); assert(w.offer_count_[side]==1 && w.offer_type_[side][0]==1 && !w.offer_value_[side][0]);
    for(int type: {0,5}) {
      reset(w); w.offer_count_[side]=1; w.offer_type_[side][0]=type; w.offer_value_[side][0]=14;
      w.add_offer(side,type,14);
      assert(DiploOffersDirty==1 && events.empty() && w.offer_count_[side]==1 && w.field_A24_==9 && w.field_A28_==10);
      w.add_offer(side,type,15);
      assert(w.offer_count_[side]==2 && w.offer_type_[side][1]==type && w.offer_value_[side][1]==15);
    }
    reset(w); w.offer_count_[side]=7; w.add_offer(side,7,999);
    assert(w.offer_count_[side]==8 && w.offer_type_[side][7]==7 && w.offer_value_[side][7]==999);
    reset(w); w.offer_count_[side]=8; w.add_offer(side,7,999);
    assert(DiploOffersDirty==1 && events.empty() && w.offer_count_[side]==8 && w.field_A24_==9);
    w.offer_type_[side][6]=2; w.add_offer(side,3,999);
    assert(w.offer_count_[side]==8 && w.offer_type_[side][6]==3 && w.offer_value_[side][6]==999);
    reset(w); w.offer_count_[side]=1; w.offer_type_[side][0]=7; w.offer_value_[side][0]=999;
    w.add_offer(side,7,999); assert(w.offer_count_[side]==2);
  }
  reset(w); w.offer_count_[0]=3; message_changes_count=1; w.kill_offer(0,0);
  assert(w.offer_count_[0]==1 && w.offer_type_[0][0]==31 && w.offer_type_[0][1]==31);
}
'''


def fixture():
    header = (ROOT / "src/diplowin.h").read_text()
    fields = []
    for name in ("offer_count_", "field_A24_", "field_A28_", "offer_type_", "offer_value_", "faction_id_with_"):
        declaration = re.search(r"(?m)^\s*(?:int|int32_t) " + name + r"[^;]*;", header)
        assert declaration is not None, name
        fields.append(declaration.group().strip())
    return HARNESS.replace("ACTUAL_FIELDS", "\n".join(fields))


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_offer_bodies_and_mutants(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    body = product()
    variants = {"actual": body}
    for label, old, new in (
        ("wrong_value_lane", "*value = value[1];", "*value = type[1];"),
        ("wrong_party", "if (a1 == 0)", "if (a1 == 1)"),
        ("wrong_capacity", "count >= 8", "count >= 7"),
        ("wrong_duplicate", "offer_value_[a1][i] == a3", "offer_value_[a1][i] != a3"),
    ):
        assert old in body
        variants[label] = body.replace(old, new, 1)
    for label, candidate in variants.items():
        cpp, exe = tmp_path / (label + ".cpp"), tmp_path / label
        cpp.write_text(fixture().replace("PRODUCT", candidate))
        built = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                                "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                                str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=30)
        assert built.returncode == 0, built.stderr
        run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10,
                             env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))
        if label == "actual":
            assert run.returncode == 0, run.stderr
        else:
            assert run.returncode != 0, "behavioral mutant survived: " + label
