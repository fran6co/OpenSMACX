"""Strict actual-source controls for the installed NetWin/hot-seat owners.

This permanent-style pytest deliberately has no proposal/emitter fallback and
no source-hash catalogue. It fails if any installed definition or historical
identity is absent, then compiles bodies extracted from the current product
translation units with narrow host doubles.
"""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest
from match_recovery.source import read_text
from owner_transfer_test_support import assert_current_owner, assert_leaf_collection_retired
from match_recovery.includes import COMMENTS_OR_STRINGS


HISTORY_COMMIT = "7db715fbc674f53db325ce3fd8b788be363d30d3"
OWNERS = {
    0x0047B020: ("src/netwin.cpp", "void __cdecl send_faction_data(int slot_id)"),
    0x0047B0F0: ("src/netwin.cpp", "void __cdecl send_all_player_packets(int slot_id)"),
    0x0047B1D0: ("src/netwin.cpp", "void __cdecl sub_47b1d0(int faction_id, void *payload)"),
    0x0047B260: ("src/netwin.cpp", "void __cdecl send_session_packet(int slot_id, int target)"),
    0x0047B2E0: ("src/netwin.cpp", "void __cdecl send_session2_packet(int slot_id, int target)"),
    0x00440F80: ("src/diplowin.cpp", "void __cdecl clear_hotseat_chat()"),
    0x00441040: ("src/diplowin.cpp", 'extern "C" void __cdecl sub_441040(int a1, int a2)'),
    0x00441100: ("src/diplowin.cpp", 'extern "C" void __cdecl sub_441100(int faction_id, int faction_id_with)'),
}


def repository_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "CMakeLists.txt").is_file() and (parent / "src/netwin.cpp").is_file():
            return parent
    raise RuntimeError("cannot locate OpenSMACX repository root")


def mask_comments_and_strings(text: str) -> str:
    """Blank comments/literals while preserving all offsets and newlines."""
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        text,
    )


def extract_definition(source: str, signature: str) -> str:
    masked = mask_comments_and_strings(source)
    masked_signature = mask_comments_and_strings(signature)
    start = masked.find(masked_signature)
    if start < 0 or masked.find(masked_signature, start + len(signature)) >= 0:
        raise AssertionError(f"expected one active installed definition: {signature}")
    signature_opening = signature.find("{")
    opening = (start + signature_opening if signature_opening >= 0
               else masked.find("{", start + len(signature)))
    if opening < 0 or ";" in masked[start:opening]:
        raise AssertionError(f"signature is not an active definition: {signature}")
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{": depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated active definition: {signature}")


def installed_sources() -> tuple[Path, dict[int, str], dict[int, object]]:
    root = repository_root()
    assert_leaf_collection_retired(root)
    texts = {rel: (root / rel).read_text(encoding="utf-8")
             for rel in ("src/netwin.cpp", "src/diplowin.cpp")}
    bodies = {}
    current_rows = {}
    for address, (owner, signature) in OWNERS.items():
        definition = extract_definition(texts[owner], signature)
        marker = f"// ORIGINAL: 0x{address:08X}"
        if texts[owner].count(marker) != 1:
            raise AssertionError(f"installed owner mapping missing/ambiguous: {address:08X}")
        assert_current_owner(root, address, owner)
        bodies[address] = definition
        rows = [row for row in read_text(texts[owner], owner) if row.address == address]
        if len(rows) != 1:
            raise AssertionError(f"installed source reader does not find one identity: {address:08X}")
        current_rows[address] = rows[0]
    return root, bodies, current_rows


def installed_packet_declarations(names: tuple[str, ...] | None = None) -> str:
    """Extract the canonical row, mutable view and packet shapes from netwin.h."""
    root = repository_root()
    header = (root / "src/netwin.h").read_text(encoding="utf-8")
    source = (root / "src/netwin.cpp").read_text(encoding="utf-8")
    declarations = (
        ("struct", "NetWinLobbyRow"),
        ("class", "NetWinLobbyRowView"),
        ("struct", "NetWinSessionBlock"),
        ("struct", "NetWinSessionOptionsState"),
        ("struct", "NetWinFactionRecord"),
        ("struct", "NetWinFactionDataPrefix"),
        ("class", "NetWinFactionDataPacketView"),
        ("struct", "NetWinLobbyPacketHeader"),
        ("struct", "NetWinLobbySinglePacket"),
        ("struct", "NetWinLobbyFullPacket"),
        ("struct", "NetWinLobbyLeavePacket"),
        ("class", "NetWinPacketView"),
        ("class", "NetWinLobbyPacketView"),
        ("class", "NetWinSessionStatusView"),
        ("struct", "NetWinSessionOptionsPacket"),
        ("struct", "NetWinSessionBlockPacket"),
        ("struct", "NetWinSessionLobbyPacket"),
    )
    if names is not None:
        requested = set(names)
        available = {name for _, name in declarations}
        if requested - available:
            raise AssertionError(f"unknown canonical NetWin declarations: {sorted(requested - available)}")
        declarations = tuple(item for item in declarations if item[1] in requested)
    return "\n".join(
        extract_definition(
            source if name in {"NetWinFactionDataPrefix", "NetWinFactionDataPacketView"}
            else header, f"{kind} {name} {{") + ";"
        for kind, name in declarations
    )


def test_history_identity_and_fields_survive_owner_transfer() -> None:
    root, _bodies, current = installed_sources()
    old_leaf = subprocess.run(
        ["git", "show", f"{HISTORY_COMMIT}:src/leaf_recoveries.cpp"],
        cwd=root, capture_output=True, text=True, check=True, timeout=20).stdout
    historical = {row.address: row for row in read_text(old_leaf, "src/leaf_recoveries.cpp")
                  if row.address in OWNERS}
    assert set(historical) == set(OWNERS)
    fields = ("mode", "name", "image_spans", "symbol", "kind", "exclusion",
             "recipe", "byte_exact", "semantic", "levers", "ruled_out",
             "unrecoverable", "deferred")
    for address in OWNERS:
        assert current[address].path.as_posix().endswith(OWNERS[address][0])
        for field in fields:
            if address == 0x0047B1D0 and field == "symbol":
                # The old seed had no emitted-symbol fact. The genuine
                # owner definition is now bound to its observed VC6 COFF
                # symbol; this does not rename its original identity.
                assert historical[address].symbol == ""
                assert current[address].symbol == "?sub_47b1d0@@YAXHPAX@Z"
                continue
            assert getattr(current[address], field) == getattr(historical[address], field), (
                f"{address:08X}: historical {field} changed")


def test_extractor_ignores_comment_and_string_fakes() -> None:
    signature = "void __cdecl send_session_packet(int slot_id, int target)"
    genuine = f"{signature} {{ return; }}"
    assert extract_definition(genuine, signature) == genuine
    fake = f'/* {signature} {{ return; }} */\nconst char *s = "{signature} {{}}";'
    with pytest.raises(AssertionError, match="active installed definition"):
        extract_definition(fake, signature)
    # This is the full-TU shape of a removed body plus a comment/string decoy;
    # neither lexically masked occurrence is eligible as installed source.
    damaged = f'/* {signature} {{ return; }} */\nconst char *s = "{signature} {{}}";'
    with pytest.raises(AssertionError, match="active installed definition"):
        extract_definition(damaged, signature)
    # Exercise the same failure against a real complete owner TU: appending
    # decoys cannot create a second active definition, and removing the real
    # body while retaining those decoys must refuse extraction.
    root = repository_root()
    actual_tu = (root / "src/netwin.cpp").read_text(encoding="utf-8")
    actual_body = extract_definition(actual_tu, signature)
    decoys = (actual_tu + f'\n/* {signature} {{ return; }} */\n'
              f'const char *recovery_decoy = "{signature} {{}}";\n')
    assert extract_definition(decoys, signature) == actual_body
    no_body = decoys.replace(actual_body, "", 1)
    with pytest.raises(AssertionError, match="active installed definition"):
        extract_definition(no_body, signature)


HOST_PRELUDE = r'''#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#define __cdecl
static std::vector<unsigned char> sent;
static std::uint32_t sent_to;
static unsigned send_calls;
struct Net { void send(void *data, unsigned long size, unsigned long target, int) {
  const unsigned char *p=static_cast<const unsigned char *>(data); sent.assign(p,p+size);
  sent_to=static_cast<std::uint32_t>(target); ++send_calls; } };
struct AlphaNetProcessSlot {
  char name_[0x20]; std::uint32_t pid_; unsigned char rest[0x19C-0x24];
};
struct NetDaemonNetType : Net {
  uint32_t local_player_id_ = 0, host_player_id_ = 0;
  uint32_t local_player_id() const { return local_player_id_; }
  uint32_t host_player_id() const { return host_player_id_; }
  struct PrimaryState { unsigned leading_words_[3]; } primary_state_ = {{1,0,0}};
  AlphaNetProcessSlot process_slots_[8];
} NetDaemonNet;
// INSERT_INSTALLED_PACKET_DECLARATIONS
static NetWinLobbyRow NetWinLobbyRows[8];
static NetWinSessionBlock NetWinSessionData;
static NetWinSessionOptionsState NetWinSessionOptions;
static std::vector<void *> allocations;
static void *mem_get(unsigned n) { void *p=std::malloc(n); assert(p); allocations.push_back(p); return p; }
struct NetWinDialogType { NetWinFactionRecord *faction_records_; void setup_faction_data(int) { assert(faction_records_); } } NetWinDialog;
struct StringStructPayload { char *text; };
struct StringStructEntry { int id; StringStructPayload *payload; StringStructEntry *next,*previous;
  StringStructPayload *string_payload() { return payload; } };
static int remove_calls;
struct StringList {
  StringStructEntry *head_=0,*current_=0; int entry_count_=0,current_position_=0;
  void *allocator_=0; char *pending_text_=0; unsigned field_20_=0,field_24_=0;
  void remove_all() { if(!head_) return; int index=0; if(entry_count_>0) do {
    StringStructEntry *entry=head_; current_=entry->next; StringStructPayload *payload=entry->payload;
    ++remove_calls; delete payload; head_->payload=0; delete head_; ++index; head_=current_;
  } while(index<entry_count_); head_=0; current_position_=0; entry_count_=0; }
  int add(int id); int seek_pos(int index) { current_position_=0; current_=head_;
    for(int i=0;i<index && i<entry_count_;++i) { current_=current_->next; ++current_position_; }
    return current_!=0; }
  StringStructPayload *current_entry() { return current_?current_->payload:0; }
  int current_id() { return current_?current_->id:0; }
};
static_assert(sizeof(void *)==8,"host runner requires a 64-bit host");
static_assert(sizeof(StringList)==0x30,"strided source table model");
static std::map<StringList *,std::vector<int> > added_ids;
static std::map<StringList *,std::vector<std::string> > added_text;
int StringList::add(int id) { added_ids[this].push_back(id); added_text[this].push_back(pending_text_?pending_text_:""); ++entry_count_; return 1; }
struct StringBox { StringList list; std::vector<std::string> drawn; int clears=0;
  StringList &strings() { return list; } void clear() { ++clears; drawn.clear(); }
  void add(char *text,int id,int) { drawn.push_back(text?text:""); drawn.back()+="#"+std::to_string(id); } };
struct EditBox { int calls=0; char *value=reinterpret_cast<char *>(1); void set_text(char *v) { ++calls; value=v; } };
struct DiploWin { StringBox stringBox_; EditBox editBox_;
  StringBox &shared_string_box() { return stringBox_; } EditBox &shared_edit_box() { return editBox_; } } DiploWindow;
static StringList DiplomacyTranscriptLists[8][8];
static void swap(int *a,int *b) { int t=*a; *a=*b; *b=t; }
static StringStructEntry *append_entry(StringList &list,int id,const char *text) {
  StringStructEntry *e=new StringStructEntry(); StringStructPayload *p=new StringStructPayload();
  p->text=new char[std::strlen(text)+1]; std::strcpy(p->text,text); e->id=id; e->payload=p;
  if(!list.head_) { e->next=e; e->previous=e; list.head_=e; }
  else { StringStructEntry *tail=list.head_->previous; tail->next=e; e->previous=tail; e->next=list.head_; list.head_->previous=e; }
  ++list.entry_count_; return e;
}
'''


HOST_MAIN = r'''static void reset_send(){sent.clear();sent_to=0;send_calls=0;}
int main(){
  for(unsigned i=0;i<8;++i)NetDaemonNet.process_slots_[i].pid_=0x1000+i;
  NetDaemonNet.local_player_id_=9;NetDaemonNet.host_player_id_=99;
  for(unsigned i=0;i<sizeof(NetWinSessionOptions);++i)reinterpret_cast<unsigned char *>(&NetWinSessionOptions)[i]=i*3+1;
  for(unsigned i=0;i<sizeof(NetWinSessionData);++i)reinterpret_cast<unsigned char *>(&NetWinSessionData)[i]=i*5+7;
  reset_send();send_session_packet(77,-1);assert(send_calls==1&&sent.size()==0x28&&sent_to==0);
  assert(*reinterpret_cast<unsigned short*>(sent.data())==0x4F02);
  assert(!std::memcmp(sent.data()+0x10,&NetWinSessionOptions,0x18));
  reset_send();send_session_packet(77,0);assert(send_calls==1&&sent_to==(unsigned)NetDaemonNet.host_player_id_);
  assert(*reinterpret_cast<unsigned short*>(sent.data())==0x2F02);
  reset_send();send_session_packet(77,3);assert(send_calls==1&&sent_to==NetDaemonNet.process_slots_[3].pid_);
  assert(*reinterpret_cast<unsigned short*>(sent.data())==0x1F02);
  reset_send();send_session2_packet(55,-1);assert(send_calls==1&&sent.size()==0x178&&sent_to==0);
  assert(*reinterpret_cast<unsigned short*>(sent.data())==0x4F03);
  assert(!std::memcmp(sent.data()+0x10,&NetWinSessionData,0x168));
  reset_send();send_session2_packet(55,0);assert(send_calls==1&&sent_to==(unsigned)NetDaemonNet.host_player_id_);
  assert(*reinterpret_cast<unsigned short*>(sent.data())==0x2F03);
  reset_send();send_session2_packet(55,3);assert(send_calls==1&&sent_to==NetDaemonNet.process_slots_[3].pid_);
  assert(*reinterpret_cast<unsigned short*>(sent.data())==0x1F03);
  for(unsigned i=0;i<sizeof(NetWinLobbyRows);++i)reinterpret_cast<unsigned char*>(NetWinLobbyRows)[i]=i*7+9;
  NetDaemonNet.primary_state_.leading_words_[0]=0;reset_send();send_all_player_packets(-1);assert(send_calls==0);
  reset_send();send_all_player_packets(2);assert(send_calls==1&&sent.size()==0xA74&&sent_to==NetDaemonNet.process_slots_[2].pid_);
  assert(*reinterpret_cast<unsigned short*>(sent.data())==0x1F04);
  assert(!std::memcmp(sent.data()+0x10,&NetWinLobbyRows[1],0xA64));
  reset_send();NetDaemonNet.primary_state_.leading_words_[0]=1;send_all_player_packets(-1);assert(send_calls==1&&sent_to==0);
  assert(*reinterpret_cast<unsigned short*>(sent.data())==0x4F04);
  alignas(NetWinFactionRecord) unsigned char faction[0x400];for(unsigned i=0;i<sizeof(faction);++i)faction[i]=0xA0+i;faction[1]=1;
  NetWinDialog.faction_records_=reinterpret_cast<NetWinFactionRecord *>(faction);NetDaemonNet.local_player_id_=9;NetDaemonNet.host_player_id_=10;
  reset_send();send_faction_data(-1);assert(send_calls==0);NetDaemonNet.local_player_id_=NetDaemonNet.host_player_id_;
  reset_send();send_faction_data(-1);assert(send_calls==1&&sent_to==0);
  assert(*reinterpret_cast<unsigned short*>(sent.data())==0x0F0D);
  reset_send();send_faction_data(2);assert(send_calls==1&&sent_to==NetDaemonNet.process_slots_[2].pid_);
  assert(sent.size()==133*3+0x1A4&&*reinterpret_cast<unsigned short*>(sent.data())==0x0F0D);
  assert(!std::memcmp(sent.data()+0x10,faction,0x194+133*3));
  unsigned char payload[0x17C];for(unsigned i=0;i<sizeof(payload);++i)payload[i]=i^0x5A;
  reset_send();sub_47b1d0(0,payload);assert(send_calls==0);reset_send();sub_47b1d0(1,0);assert(send_calls==0);
  reset_send();sub_47b1d0(2,payload);assert(send_calls==1&&sent.size()==0x18C&&sent_to==(unsigned)NetDaemonNet.host_player_id_);
  assert(*reinterpret_cast<unsigned short*>(sent.data())==0x2F04&&*reinterpret_cast<int*>(sent.data()+4)==2);
  assert(!std::memcmp(sent.data()+0x10,payload,0x17C));
  StringList &empty=DiplomacyTranscriptLists[0][0];empty.entry_count_=7;empty.current_position_=3;empty.allocator_=(void*)1;
  StringList &live=DiplomacyTranscriptLists[0][1];append_entry(live,1,"a");append_entry(live,2,"b");
  live.current_position_=8;live.allocator_=(void*)2;int before=remove_calls;clear_hotseat_chat();
  assert(remove_calls==before+2&&live.head_==0&&live.entry_count_==0&&live.current_position_==0&&live.allocator_==0);
  assert(empty.head_==0&&empty.entry_count_==7&&empty.current_position_==0&&empty.allocator_==0);
  StringList &recent=DiploWindow.stringBox_.strings();append_entry(recent,10,"zero");append_entry(recent,11,"one");
  append_entry(recent,12,"two");append_entry(recent,13,"three");append_entry(recent,14,"four");
  StringList &dest=DiplomacyTranscriptLists[1][2];dest.field_20_=0x55;dest.field_24_=0x66;sub_441040(2,1);
  assert(added_ids[&dest].size()==3&&added_ids[&dest][0]==12&&added_ids[&dest][2]==14);
  assert(added_text[&dest][0]=="two"&&added_text[&dest][2]=="four"&&dest.field_20_==0&&dest.field_24_==0);
  assert(recent.current_position_==4&&recent.current_->id==14);
  StringList &saved=DiplomacyTranscriptLists[1][3];append_entry(saved,20,"old");append_entry(saved,21,"older");
  append_entry(saved,22,"recent-a");append_entry(saved,23,"recent-b");append_entry(saved,24,"recent-c");
  sub_441100(3,1);assert(DiploWindow.shared_string_box().clears==1);
  assert(DiploWindow.shared_edit_box().calls==1&&DiploWindow.shared_edit_box().value==0);
  const std::vector<std::string>&drawn=DiploWindow.shared_string_box().drawn;
  assert(drawn.size()==3&&drawn[0]=="recent-a#22"&&drawn[1]=="recent-b#23"&&drawn[2]=="recent-c#24");
  for(void *p:allocations)std::free(p);
}
'''


def compile_and_run(compiler: str, source: str, directory: Path, name: str) -> subprocess.CompletedProcess:
    cpp = directory / f"{name}.cpp"
    binary = directory / name
    cpp.write_text(source, encoding="utf-8")
    subprocess.run([compiler, "-std=c++17", "-O0", str(cpp), "-o", str(binary)],
                   check=True, capture_output=True, text=True, timeout=60)
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


def test_installed_owner_bodies_and_behavioral_mutants(tmp_path: Path) -> None:
    _root, bodies, _rows = installed_sources()
    compilers = [path for name in ("g++", "clang++") if (path := shutil.which(name))]
    assert compilers, "GCC/Clang host compiler unavailable"
    mutations = (
        (0x0047B260, "if (target != 0)", "if (target == 0)"),
        (0x00440F80, "if (list.head_ != 0)", "if (list.head_ == 0)"),
        (0x00441040, "entry->pending_text_ = f2;", "entry->pending_text_ = 0;"),
        (0x00441100, "if (faction_id > faction_id_with)", "if (faction_id < faction_id_with)"),
    )
    for compiler_index, compiler in enumerate(compilers):
        prelude = HOST_PRELUDE.replace("// INSERT_INSTALLED_PACKET_DECLARATIONS",
                                       installed_packet_declarations())
        source = prelude + "\n".join(bodies[address] for address in OWNERS) + HOST_MAIN
        baseline = compile_and_run(compiler, source, tmp_path, f"baseline-{compiler_index}")
        assert baseline.returncode == 0, baseline.stderr
        for index, (address, old, new) in enumerate(mutations):
            assert bodies[address].count(old) == 1, f"mutation anchor changed for {address:08X}"
            mutant_bodies = dict(bodies)
            mutant_bodies[address] = bodies[address].replace(old, new, 1)
            mutant = prelude + "\n".join(mutant_bodies[a] for a in OWNERS) + HOST_MAIN
            damaged = compile_and_run(compiler, mutant, tmp_path,
                                      f"mutant-{compiler_index}-{index}")
            assert damaged.returncode != 0, f"behavioral mutant survived for {address:08X}"
