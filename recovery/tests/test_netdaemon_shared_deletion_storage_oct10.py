"""Shared NetDaemon deletion storage recovery; not complete game acceptance."""
from dataclasses import fields
from functools import lru_cache
from pathlib import Path
import difflib
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_text, write

ROOT = Path(__file__).resolve().parents[2]
BASE = "1b080a05a2c866f3bd4bdf7d388f79ce6ee828a5"
UNITS = ["src/basewin.cpp","src/console.cpp","src/councwin.cpp","src/faction.cpp","src/famewin.cpp","src/game.cpp","src/general.cpp","src/maininterface.cpp","src/mapwin.cpp","src/monuwin.cpp","src/multiwin.cpp","src/netdaemon.cpp","src/netwin.cpp","src/newgame.cpp","src/probe.cpp","src/quaylewin.cpp","src/reportwin.cpp","src/statuswin.cpp","src/stringstruct.cpp","src/veh.cpp"]

protected=re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',re.S)

UNITS.append('src/deletionlist.cpp')
EVIDENCE_EDITS = {"src/console.cpp":[["source:src/netdaemon.h:NetDaemon::lock_state_","source:src/netdaemon.h:NetDaemon::primary_state_"],["The existing faction waiting-mask symbol remains a historical duplicate view of NetDaemon lock-state storage; its central ownership correction is still outstanding.","The historical faction waiting-mask identity now binds to NetDaemonNet.primary_state_.trailing_words_[7]; the singleton owns that shared storage."]],"src/veh.cpp":[["#include \"deletionlist.h\"  // VehDeletionList - the multiplayer deletion queue","#include \"deletionlist.h\"  // NetDaemonNet owns the embedded deletion queue"]],"src/deletionlist.cpp":[["src/veh.cpp defines the DeletionList global at 0x0093E31C.","Historical src/veh.cpp defined a separate DeletionList global at 0x0093E31C; pinned veh_kill5C08C0 and NetDaemon::fixup_message534330 establish the current NetDaemonNet.deletion_list_ receiver at that address."]]}

def migration_recipe(original, relative):
 """Fixed reviewed spelling/storage changes, independent of current source."""
 p = ROOT / relative
 s=original
 if p.name=='netdaemon.cpp':
  s,n=re.subn(r'class DeletionList \{ public:\n.*?\n\};\n','',s,flags=re.S);assert n==1
  s=s.replace('NetDaemon::NetDaemon() : AlphaNet() {','NetDaemon::NetDaemon() : AlphaNet(), deletion_list_(DeletionList::Uninitialized()) {')
  before='''            uint32_t *seq = &lock_state_[0].leading_words_[0];
            *seq = *seq + 1;
            *reinterpret_cast<int *>(message + 0xc) = *seq;'''
  assert s.count(before)==1
  s=s.replace(before,'''            const uint32_t seq = deletion_list_.advance_sequence();
            memcpy(message + 0x0C, &seq, sizeof seq);''')
  s=s.replace('LockTableEntry &entry = daemon.lock_table_[table_index];','DeletionRecord &entry = daemon.deletion_list_.records_[table_index];')
  s=s.replace('entry.flag_','entry.acknowledgements_').replace('entry.field_4_ !=','static_cast<uint32_t>(entry.owner_) !=')
 if p.name=='veh.cpp':
  before='''// 0x0093E31C. The queue the multiplayer deletion path parks a dying vehicle on
// until the next turn boundary, guarded by IsMultiplayerNet.
DeletionList VehDeletionList;  // 0x0093E31C
'''
  assert s.count(before)==1
  s=s.replace(before,'''// Original0x0093E31C is NetDaemonNet.deletion_list_, not a separate global.
// veh_kill passes this exact embedded receiver to add_deletion52DE70.
''')
 if p.name=='game.cpp':
  s=s.replace('extern DeletionList VehDeletionList; // veh.cpp, original93E31C\n','')
 has_alias='state = NetDaemonNet.lock_state_;' in s
 def code_transform(c):
  c=re.sub(r'(?:const )?LockStateEntry \*(?:const )?state = NetDaemonNet\.lock_state_;',
           'NetDaemon &network = NetDaemonNet;',c)
  if has_alias:
   c=c.replace('state[','network.lock_state_[')
  c=c.replace('lock_state_[0].leading_words_[0]','deletion_list_.sequence_bits()')
  for i in (1,2,3):
   c=c.replace(f'lock_state_[0].leading_words_[{i}]',f'primary_state_.leading_words_[{i-1}]')
  c=c.replace('lock_state_[0].payload_.flags','primary_state_.flags_')
  c=c.replace('lock_state_[0].trailing_words_','primary_state_.trailing_words_')
  c=c.replace('lock_state_[1]','channels_[0]').replace('lock_state_[2]','channels_[1]')
  c=c.replace('save_storage_.process_slots_','saved_process_slots_').replace('save_storage_.file_index_records_','file_index_records_')
  c=c.replace('VehDeletionList','NetDaemonNet.deletion_list_')
  c=c.replace('lock_table_[m].flag_','deletion_list_.records_[m].acknowledgements_').replace('lock_table_[m].word_8_','deletion_list_.records_[m].count_')
  c=c.replace('reinterpret_cast<DeletionList *>(lock_table_)','&deletion_list_')
  # Replacing a pointer expression with an actual member call.
  c=re.sub(r'&deletion_list_\s*->convert','deletion_list_.convert',c)
  c=re.sub(r'\+\+((?:\b\w+\.)?deletion_list_)\.sequence_bits\(\);',r'\1.advance_sequence();',c)
  c=re.sub(r'((?:\b\w+\.)?deletion_list_)\.sequence_bits\(\) \+= 1;',r'\1.advance_sequence();',c)
  c=re.sub(r'((?:\b\w+\.)?deletion_list_)\.sequence_bits\(\)\s*=\s*([^;]+);',lambda m:m[1]+'.set_sequence_bits('+m[2].rstrip()+');',c)
  return c
 out=[];last=0
 for m in protected.finditer(s):
  out.append(code_transform(s[last:m.start()]));out.append(m[0]);last=m.end()
 out.append(code_transform(s[last:]));new=''.join(out)
 if p.name=='netdaemon.cpp':
  new=new.replace('    deletion_list_.set_sequence_bits(0);\n    for (int32_t m = 0;', '    deletion_list_.current_owner_ = 0;\n    for (int32_t m = 0;',1)

 if p.name=='netdaemon.cpp':
  new=new.replace("member ?NetDaemonNet@@3VNetDaemon@@A lock_state_[0].trailing_words_[7]",
                  "member ?NetDaemonNet@@3VNetDaemon@@A primary_state_.trailing_words_[7]")
  new=new.replace("member ?NetDaemonNet@@3VNetDaemon@@A lock_state_[2].trailing_words_[7]",
                  "member ?NetDaemonNet@@3VNetDaemon@@A channels_[1].trailing_words_[7]")
 for before, after in EVIDENCE_EDITS.get(relative, ()):
  assert new.count(before)==1
  new=new.replace(before,after,1)
 if relative == "src/netdaemon.cpp":
  from test_netdaemon_wire_owner_oct11 import apply_wire_migration
  new = apply_wire_migration(new)
 return new

@lru_cache(None)
def baseline(relative):
 return subprocess.check_output(["git","show",BASE+":"+relative],cwd=ROOT).decode()

@lru_cache(None)
def migration_chunks(relative):
 before=baseline(relative).splitlines(keepends=True)
 after=migration_recipe("".join(before),relative).splitlines(keepends=True)
 groups=difflib.SequenceMatcher(a=before,b=after).get_grouped_opcodes(3)
 return tuple(("".join(before[group[0][1]:group[-1][2]]),
               "".join(after[group[0][3]:group[-1][4]])) for group in groups)

def restore_shared_storage_migration(source, relative="src/netdaemon.cpp"):
 """Undo only recipe-derived substitutions, never an arbitrary source diff."""
 for before,after in reversed(migration_chunks(relative)):
  assert source.count(after)==1, relative
  source=source.replace(after,before,1)
 return source

@pytest.mark.parametrize("relative",UNITS)
def test_only_reviewed_storage_migration_and_all_function_history_survives(relative):
 path=ROOT/relative
 original=baseline(relative)
 current=path.read_text()
 assert current==migration_recipe(original,relative)
 assert restore_shared_storage_migration(current,relative)==original
 rows=read_text(current,path)
 previous={row.address:row for row in read_text(original,path)}
 assert {row.address for row in rows}==set(previous)
 assert write(current,rows)==current
 for row in rows:
  for field in fields(row):
   if field.name not in {"line","region","origin"}:
    assert getattr(row,field.name)==getattr(previous[row.address],field.name)
  def lessons(record):
   return [(x.kind,x.key,x.prose,tuple(text for _,text in x.lines)) for x in record.origin]
  assert lessons(row)==lessons(previous[row.address])

# Fixed reviewed header deltas; not recomputed from the working tree.
HEADER_EDITS = {
  "src/netdaemon.h": [
    [
      "\n#include \"original_seam.h\"\n#include \"alphanet.h\"\n#include \"lock.h\"        // locks_ - the whole network-lock block\n#include \"playerlock.h\"  // its records are PlayerLocks\n#include <stddef.h>\n",
      "\n#include \"original_seam.h\"\n#include \"alphanet.h\"\n#include \"deletionlist.h\"\n#include \"lock.h\"        // locks_ - the whole network-lock block\n#include \"playerlock.h\"  // its records are PlayerLocks\n#include <stddef.h>\n"
    ],
    [
      "  // `call rel32`. Spelled `class X : Base` - private, since that is\n  // what `class` means - those calls do not compile at all, and the\n  // seam that stood in for them cost the caller `call [ptr]`.\n// The lock-table entry (0x158C in NetDaemon) and its parallel state entry.\nstruct LockTableEntry {\n  uint8_t flag_;        // the constructor sets this byte to 0xFF\n  uint8_t pad_1_[3];\n  uint32_t field_4_;  // compared with message sequence at +0x0C\n  uint16_t word_8_;\n  uint8_t pad_A_[0x32];\n};\nstatic_assert(offsetof(LockTableEntry, field_4_) == 4, \"lock-table sequence field offset\");\nstatic_assert(sizeof(LockTableEntry) == 0x3C, \"lock table stride\");\n\n// ONE LOCK-STATE ENTRY, 0x3C bytes. Which entry and which dword a NetDaemon\n// offset names: lock_state_ starts at 0x1B2C, so\n// `offset - 0x1B2C` / 0x3C picks the entry and the remainder / 4 picks the\n// dword. THE MAP, as far as the recovered bodies pin it -\n//\n//   0x1B2C  [0].dword_[0]   the shared checksum sequence, incremented per\n//                           forwarded message and stamped into message+0xC\n",
      "  // `call rel32`. Spelled `class X : Base` - private, since that is\n  // what `class` means - those calls do not compile at all, and the\n  // seam that stood in for them cost the caller `call [ptr]`.\n// Native channel offsets below use the former conceptual channel0..2\n// numbering. Channel0's first dword is DeletionList.current_owner_, not a\n// second object. Its remaining bytes are PrimaryLockStateTail; channels1/2\n// are the two LockStateEntry objects. The map is supported by callers:\n//\n//   0x1B2C  [0].dword_[0]   the shared checksum sequence, incremented per\n//                           forwarded message and stamped into message+0xC\n"
    ],
    [
      "//   0x1BD0  [2].dword_[11]  the ctor sets this one to 0xFF, the only byte it\n//                           does not zero\n//\n// Entry3 aliases the file-index pointer and persistent save view below.\n// The constructor's 0x538A53 owner clear is outside the record loop:\n// 0x538A64 branches to 0x538A59. It does not initialize entries3..23.\n// Only channels0..2 have evidenced runtime state accesses; the broader array\n// and overlapping save view below remain structural recovery obligations.\n// The console's bodies read\n// the same run through `NetDaemonNet.lock_state_` - it is the 0x0093E8BC run\n// in the image - which is why the struct and the member are public.\nstruct LockStateEntry {\n  uint32_t leading_words_[4];\n  // Channel0/2 use flags; channel1 borrows a vehicle-ID slot. These are\n",
      "//   0x1BD0  [2].dword_[11]  the ctor sets this one to 0xFF, the only byte it\n//                           does not zero\n//\n// Runtime accesses establish only channels0..2. The file-index pointer and\n// saved process slots afterward are disjoint fields, not another21 channels.\n// 0x538A64 branches to0x538A59, after the one-time owner clear at0x538A53.\nstruct PrimaryLockStateTail {\n  uint32_t leading_words_[3];\n  uint32_t flags_;\n  uint32_t trailing_words_[10];\n};\nstatic_assert(sizeof(PrimaryLockStateTail) == 0x38,\n              \"primary channel excludes the deletion owner's dword\");\nstatic_assert(offsetof(PrimaryLockStateTail, flags_) == 0xC,\n              \"primary channel flag offset from its tail\");\nstatic_assert(offsetof(PrimaryLockStateTail, trailing_words_) == 0x10,\n              \"primary channel trailing words offset\");\nstruct LockStateEntry {\n  uint32_t leading_words_[4];\n  // Channel0/2 use flags; channel1 borrows a vehicle-ID slot. These are\n"
    ],
    [
      "              \"lock-state trailing words\");\nstatic_assert(sizeof(LockStateEntry) == 0x3C, \"lock state stride\");\n\n// Persistent save/restore storage view beginning eight bytes into state entry 3.\n// The native secondary initializer writes the same five leading bytes as an\n// AlphaNet process record at 0x93E99C + n*0x19C; save_daemon copies the eight\n// complete live records over this view, and game_data transfers the same\n",
      "              \"lock-state trailing words\");\nstatic_assert(sizeof(LockStateEntry) == 0x3C, \"lock state stride\");\n\n// Persistent save/restore storage begins at NetDaemon+0x1BE8.\n// The native secondary initializer writes the same five leading bytes as an\n// AlphaNet process record at 0x93E99C + n*0x19C; save_daemon copies the eight\n// complete live records over this view, and game_data transfers the same\n"
    ],
    [
      "static_assert(sizeof(NetDaemonSaveProcessSlot) * 8 == 0xCE0,\n              \"saved process-slot table extent\");\n\n// At NetDaemon+0x1B2C: three complete LockStateEntries, the file-index pointer\n// word at +0x1BE0, the following opaque dword, then the 8-record save view at\n// +0x1BE8. This remains an overlapping storage view, not a disjoint object.\nstruct NetDaemonSaveStorageView {\n  LockStateEntry lock_prefix_[3];\n  void *file_index_records_;\n  uint32_t field_1BE4_;\n  NetDaemonSaveProcessSlot process_slots_[8];\n};\nstatic_assert(offsetof(NetDaemonSaveStorageView, file_index_records_) == 0xB4,\n              \"snapshot overlay file-index pointer offset\");\nstatic_assert(offsetof(NetDaemonSaveStorageView, process_slots_) == 0xBC,\n              \"snapshot overlay process slots offset\");\nstatic_assert(sizeof(NetDaemonSaveStorageView) == 0xD9C,\n              \"snapshot overlay extent from lock-state start\");\nclass NetDaemonTransferModalCloseGuard;\nclass Filemap;\n\n",
      "static_assert(sizeof(NetDaemonSaveProcessSlot) * 8 == 0xCE0,\n              \"saved process-slot table extent\");\n\nclass NetDaemonTransferModalCloseGuard;\nclass Filemap;\n\n"
    ],
    [
      "  // Borrowed selection storage already owned by channel1 at this+0x1B78.\n  // Callers retain the original callback/clear ordering and lifetime obligations.\n  void set_vehicle_selection_follow(int *slot) {\n    lock_state_[1].payload_.vehicle_selection_follow = slot;\n  }\n  int *vehicle_selection_follow() const {\n    return lock_state_[1].payload_.vehicle_selection_follow;\n  }\n\n  // Shared faction wait mask, at NetDaemon+0x1B5C (entry 0 trailing dword 7).\n  // The original address is also read by callers outside this class; keeping\n  // the accessor on the genuine owner avoids a second scalar storage object.\n  uint32_t &factions_waiting_mask() {\n    return lock_state_[0].trailing_words_[7];\n  }\n  const uint32_t &factions_waiting_mask() const {\n    return lock_state_[0].trailing_words_[7];\n  }\n  // MessageWin's network message count shares entry 2, trailing dword 7.\n  uint32_t &network_message_count() {\n    return lock_state_[2].trailing_words_[7];\n  }\n  const uint32_t &network_message_count() const {\n    return lock_state_[2].trailing_words_[7];\n  }\n\n public:\n",
      "  // Borrowed selection storage already owned by channel1 at this+0x1B78.\n  // Callers retain the original callback/clear ordering and lifetime obligations.\n  void set_vehicle_selection_follow(int *slot) {\n    channels_[0].payload_.vehicle_selection_follow = slot;\n  }\n  int *vehicle_selection_follow() const {\n    return channels_[0].payload_.vehicle_selection_follow;\n  }\n\n  // Shared faction wait mask, at NetDaemon+0x1B5C (entry 0 trailing dword 7).\n  // The original address is also read by callers outside this class; keeping\n  // the accessor on the genuine owner avoids a second scalar storage object.\n  uint32_t &factions_waiting_mask() {\n    return primary_state_.trailing_words_[7];\n  }\n  const uint32_t &factions_waiting_mask() const {\n    return primary_state_.trailing_words_[7];\n  }\n  // MessageWin's network message count shares entry 2, trailing dword 7.\n  uint32_t &network_message_count() {\n    return channels_[1].trailing_words_[7];\n  }\n  const uint32_t &network_message_count() const {\n    return channels_[1].trailing_words_[7];\n  }\n\n public:\n"
    ],
    [
      "  // 0x1580/0x1584/0x1588.\n  Lock locks_;\n\n  // THE NET LOCK TABLE, 0x158C..0x1B2C: 24 entries of 0x3C. The constructor\n  // sets each entry's flag byte to 0xFF and its word at +8 to 0.\n  LockTableEntry lock_table_[24];  // 0x158C\n\n  // THE LOCK-TABLE STATE - the array AFTER lock_table_ (24 * 0x3C = 0x5A0\n  // bytes later). The ctor clears its first word ONCE before the record\n  // header loop; it does not sweep state/pointer/save words at this stride.\n  // Entries e0..e2 carry the named dwords; the broader array aliases the\n  // file-index pointer and save view but is not a proven original array.\n  // Lock-state view spans0x1B2C..0x20CC / image93E8BC..93EE5C.\n  // The overlapping save view extends through+0x28C8 / image93F658.\n  union {\n    LockStateEntry lock_state_[24];  // 0x1B2C; native lock-state view\n    NetDaemonSaveStorageView save_storage_;  // overlaps from lock_state_[3]+8\n  };\n\n  // Observed storage reaches+0x28C8 through the persistent save table.\n  // Extent only: this is not proof of complete original sizeof or union intent.\n  // Separate NetPlayerDropped93F65C and network flags93F660 remain globals.\n};\n\n",
      "  // 0x1580/0x1584/0x1588.\n  Lock locks_;\n\n  // fixup_message534330 passes this+158C to DeletionList::convert52DDC0.\n  // veh_kill5C08C0 passes93E31C to add_deletion52DE70; that is exactly\n  // NetDaemonNet93CD90+158C. There is one embedded object/lifetime, not a\n  // separate VehDeletionList global. current_owner_ also owns sequence1B2C.\n  // RECOVERY_MEMBER: v1 NetDaemon deletion_list_ 0x158C 0x00534330\n  DeletionList deletion_list_;\n  // RECOVERY_MEMBER: v1 NetDaemon primary_state_ 0x1B30 0x005389F0\n  PrimaryLockStateTail primary_state_;\n  // RECOVERY_MEMBER: v1 NetDaemon channels_ 0x1B68 0x005389F0\n  LockStateEntry channels_[2];\n  // The constructor leaves the file-index pointer and save rows untouched.\n  // RECOVERY_MEMBER: v1 NetDaemon file_index_records_ 0x1BE0 0x0052ED40\n  void *file_index_records_;\n  uint32_t field_1BE4_;\n  // Separate from AlphaNet's live process_slots_ at +0x76C.\n  // RECOVERY_MEMBER: v1 NetDaemon saved_process_slots_ 0x1BE8 0x0052DB40\n  NetDaemonSaveProcessSlot saved_process_slots_[8];\n\n  // Observed storage reaches+0x28C8 through the persistent save table.\n  // Extent only: this is not proof of complete original sizeof.\n  // Separate NetPlayerDropped93F65C and network flags93F660 remain globals.\n};\n\n"
    ]
  ],
  "src/deletionlist.h": [
    [
      "#pragma once\n\n#include <stddef.h>\n\nstruct DeletionRecord {\n  uint8_t acknowledgements_;  // 0x00: completed faction bits; 0xFF marks an empty slot.\n",
      "#pragma once\n\n#include <stddef.h>\n#include <string.h>\n\nstruct DeletionRecord {\n  uint8_t acknowledgements_;  // 0x00: completed faction bits; 0xFF marks an empty slot.\n"
    ],
    [
      "  int add_deletion(int veh_id);\n  int convert(int original_id, int new_owner);\n\n  // RECOVERY_MEMBER: v1 DeletionList records_ 0x0 0x0052DCF0\n  DeletionRecord records_[24];\n  // RECOVERY_MEMBER: v1 DeletionList current_owner_ 0x5A0 0x0052DDC0\n  int current_owner_;\n};\n\nstatic_assert(offsetof(DeletionList, current_owner_) == 0x5A0,\n",
      "  int add_deletion(int veh_id);\n  int convert(int original_id, int new_owner);\n\n  // The network sequence and deletion owner are the same native dword.\n  // Preserve its bits for unsigned wrapping network arithmetic without\n  // converting an out-of-range unsigned value to int or adding alias storage.\n  uint32_t sequence_bits() const {\n    // Signed-to-unsigned conversion is defined modulo2^32; native int is\n    // four-byte two's-complement. The reverse store uses the representation.\n    return static_cast<uint32_t>(current_owner_);\n  }\n  void set_sequence_bits(uint32_t bits) {\n    memcpy(&current_owner_, &bits, sizeof bits);\n  }\n  uint32_t advance_sequence() {\n    const uint32_t next = sequence_bits() + 1;\n    set_sequence_bits(next);\n    return next;\n  }\n\n  // RECOVERY_MEMBER: v1 DeletionList records_ 0x0 0x0052DCF0\n  DeletionRecord records_[24];\n  // RECOVERY_MEMBER: v1 DeletionList current_owner_ 0x5A0 0x0052DDC0\n  int current_owner_;\n\n private:\n  friend class NetDaemon;\n  // NetDaemon's native constructor initializes this member after clearing\n  // its locks. This path starts its lifetime without duplicating those writes.\n  struct Uninitialized {};\n  explicit DeletionList(Uninitialized) {}\n};\n\nstatic_assert(offsetof(DeletionList, current_owner_) == 0x5A0,\n"
    ]
  ]
}

def restore_shared_header_migration(source, relative="src/netdaemon.h"):
 for before, after in reversed(HEADER_EDITS[relative]):
  assert source.count(after)==1, relative
  source=source.replace(after,before,1)
 return source

@pytest.mark.parametrize("relative", list(HEADER_EDITS))
def test_only_reviewed_header_ownership_and_lifetime_changes(relative):
 assert restore_shared_header_migration((ROOT/relative).read_text(),relative)==baseline(relative)

def test_actual_deletion_member_lifetime_initialization_and_sequence_bits(tmp_path):
 """Exercise the real header and initialization statements, not an alias mock.

 This host control establishes touched bytes and sequence sharing, not the
 native application ABI, full constructor or network/runtime acceptance.
 """
 source=(ROOT/'src/netdaemon.cpp').read_text()
 start=source.index('    deletion_list_.current_owner_ = 0;',source.index('NetDaemon::NetDaemon()'))
 end=source.index('\n    }',start)+len('\n    }')
 fragment=source[start:end]
 constructor=re.search(r'DeletionList::DeletionList\(\) \{.*?\n\}',
                       (ROOT/'src/deletionlist.cpp').read_text(),re.S)[0]
 harness=r'''#include <stdint.h>
#include <new>
#include <string.h>
#include "deletionlist.h"
@@CTOR@@
class NetDaemon {
public:
 DeletionList deletion_list_;
 NetDaemon():deletion_list_(DeletionList::Uninitialized()){}
 void initialize(){@@BODY@@}
};
int main(){
 union Storage { unsigned char bytes[sizeof(NetDaemon)]; long double alignment; } storage;
 unsigned char expected[sizeof(NetDaemon)];
 memset(storage.bytes,0xA5,sizeof storage.bytes);
 NetDaemon *daemon=new(storage.bytes) NetDaemon;
 for(unsigned i=0;i<sizeof storage.bytes;++i) if(storage.bytes[i]!=0xA5)return 1;
 memcpy(expected,storage.bytes,sizeof expected);
 memset(expected+0x5A0,0,4);
 for(int i=0;i<24;++i){expected[i*0x3C]=0xFF;expected[i*0x3C+8]=0;expected[i*0x3C+9]=0;}
 daemon->initialize();
 if(memcmp(storage.bytes,expected,sizeof expected))return 2;
 const uint32_t patterns[]={0,1,0x7FFFFFFF,0x80000000,0xFFFFFFFF};
 for(unsigned i=0;i<sizeof patterns/sizeof patterns[0];++i){
  daemon->deletion_list_.set_sequence_bits(patterns[i]);
  if(daemon->deletion_list_.sequence_bits()!=patterns[i])return 3;
  uint32_t stored;memcpy(&stored,&daemon->deletion_list_.current_owner_,4);
  if(stored!=patterns[i])return 4;
  if(daemon->deletion_list_.advance_sequence()!=patterns[i]+1U)return 5;
  memcpy(&stored,&daemon->deletion_list_.current_owner_,4);
  if(stored!=patterns[i]+1U)return 6;
 }
 daemon->~NetDaemon();
 memset(storage.bytes,0xA5,sizeof storage.bytes);
 DeletionList *list=new(storage.bytes) DeletionList;
 if(memcmp(storage.bytes,expected,sizeof expected))return 7;
 list->~DeletionList();
 return 0;
}
'''.replace('@@CTOR@@',constructor).replace('@@BODY@@',fragment)
 compilers=[compiler for name in ('g++','clang++') if (compiler:=shutil.which(name))]
 if not compilers:pytest.skip('no host C++ compiler available')
 for compiler in compilers:
  for label,body,expected_exit in (
   ('actual',harness,0),
   ('missing-owner-clear',harness.replace(fragment,fragment.replace('deletion_list_.current_owner_ = 0;','')),2),
   ('duplicate-initialization',harness.replace('deletion_list_(DeletionList::Uninitialized())','deletion_list_()'),1),
  ):
   cpp=tmp_path/(Path(compiler).name+'-'+label+'.cpp');binary=cpp.with_suffix('')
   cpp.write_text(body)
   subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror',
                   '-iquote',str(ROOT/'src'),str(cpp),'-o',str(binary)],check=True,timeout=30)
   assert subprocess.run([str(binary)],timeout=30).returncode==expected_exit

def test_pinned_receiver_identity_and_lifetime_entry():
 from match_recovery.pe_coff import PeImage
 from match_recovery.model import Span
 from match_recovery.source_inventory import image_pin
 path=ROOT/'.opensmacx/game/terranx_original.exe'
 if not path.exists():pytest.skip('pinned original unavailable')
 image=PeImage(path.read_bytes(),image_pin(ROOT))
 assert image.read(Span(0x534372,0x534378))==bytes.fromhex('8db98c150000')
 assert image.read(Span(0x5C08DC,0x5C08E1))==bytes.fromhex('b91ce39300')
 assert image.read(Span(0x5C08E1,0x5C08E6))==bytes.fromhex('e88ad5f6ff')
 assert image.read(Span(0x52DB10,0x52DB1A))==bytes.fromhex('b990cd9300e8d6ae0000')
 assert 0x93CD90+0x158C==0x93E31C


