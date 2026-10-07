"""Host-only controls read only current installed source owner definitions."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import struct

import pytest

ROOT = next(p for p in Path(__file__).resolve().parents if (p / 'src/technology.cpp').is_file())
HISTORY_BASELINE = "5fe79d193062cacf6b9eb9c5b901f9a0501cd2d8"


def owner_path(name):
    return ROOT / "src" / name


def source_body(path, function_name):
    # Use the installed code-aware definition reader, then its lexical mask
    # for an exact brace extent. This prevents quoted/commented history from
    # becoming a false implementation fixture.
    from match_recovery.includes import COMMENTS_OR_STRINGS
    from match_recovery.source import function_line

    text = path.read_text()
    line = function_line(path, function_name)
    lines = text.splitlines(keepends=True)
    start = sum(len(x) for x in lines[:line - 1])
    masked = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), text)
    opening = masked.find("{", start)
    assert opening >= 0
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unclosed source function {function_name}")


def code_definition(text, signature):
    """Extract one exact function signature from real source, ignoring comments/strings."""
    from match_recovery.includes import COMMENTS_OR_STRINGS
    masked = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), text)
    if masked.count(signature) != 1:
        raise AssertionError(f"signature occurrence not unique: {signature}")
    start = masked.index(signature)
    opening = masked.find("{", start)
    assert opening >= 0
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unclosed exact definition: {signature}")


def compile_run(tmp_path, compiler, source):
    source_file = tmp_path / "control.cpp"
    binary = tmp_path / "control"
    source_file.write_text(source)
    compile_result = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(source_file), "-o", str(binary)],
        text=True, capture_output=True,
    )
    assert compile_result.returncode == 0, compile_result.stdout + compile_result.stderr
    run = subprocess.run([str(binary)], text=True, capture_output=True)
    assert run.returncode == 0, run.stdout + run.stderr
    return run.returncode


def compile_status(tmp_path, compiler, source):
    source_file = tmp_path / "mutant.cpp"
    binary = tmp_path / "mutant"
    source_file.write_text(source)
    result = subprocess.run([compiler, "-std=c++11", str(source_file), "-o", str(binary)],
                           text=True, capture_output=True)
    assert result.returncode == 0, result.stdout + result.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True).returncode


COMPILERS = [name for name in ("g++", "clang++") if shutil.which(name)]
if not COMPILERS:
    pytest.skip("host GCC/Clang unavailable", allow_module_level=True)


@pytest.mark.parametrize("compiler", COMPILERS)
def test_read_tamper_keeps_original_registry_gates_and_ignored_query_statuses(tmp_path, compiler):
    body = source_body(owner_path("game.cpp"), "read_tamper")
    harness = r'''
#include <cstdint>
#include <cstring>
using HKEY = void*; using LONG = int; using DWORD = unsigned long;
using BYTE = unsigned char; using LPBYTE = BYTE*;
#define __cdecl
static HKEY HKEY_LOCAL_MACHINE = reinterpret_cast<HKEY>(1);
struct TamperRecord { unsigned char bytes[24]; };
static TamperRecord TamperRegistryRecords[50]; static int TamperRegistryCursor;
static char TamperReadApplicationKey[] = "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\Sid Meier's Alpha Centauri";
static char TamperReadValueMVM[] = "MVM"; static char TamperReadValueMVB[] = "MVB";
static char TamperReadClass[1];
static int create_status = 0, create_calls = 0, query_calls = 0, close_calls = 0;
static HKEY seen_root; static const char *seen_key; static const char *seen_class;
static const char *query_names[2]; static DWORD query_sizes[2];
LONG RegCreateKeyExA(HKEY root, const char *key, DWORD, char *cls, DWORD, DWORD,
                     void*, HKEY *out, DWORD*) {
    ++create_calls; seen_root=root; seen_key=key; seen_class=cls; *out=reinterpret_cast<HKEY>(2); return create_status;
}
LONG RegQueryValueExA(HKEY, const char *name, DWORD*, DWORD*, LPBYTE, DWORD *size) {
    query_names[query_calls]=name; query_sizes[query_calls]=*size; ++query_calls; return 5;
}
LONG RegCloseKey(HKEY) { ++close_calls; return 0; }
'''
    checks = r'''
int main() {
    create_status=3; read_tamper();
    if (create_calls!=1 || query_calls || close_calls) return 1;
    create_status=0; read_tamper();
    if (create_calls!=2 || query_calls!=2 || close_calls!=1) return 2;
    if (seen_root!=HKEY_LOCAL_MACHINE || seen_key!=TamperReadApplicationKey || seen_class!=TamperReadClass) return 3;
    if (query_names[0]!=TamperReadValueMVM || query_names[1]!=TamperReadValueMVB) return 4;
    if (query_sizes[0]!=4 || query_sizes[1]!=0x4B0) return 5;
    return 0;
}
'''
    with tempfile.TemporaryDirectory(dir=tmp_path) as td:
        compile_run(Path(td), compiler, harness + body + checks)
    mutant = body.replace("if (status == 0)", "if (status != 0)", 1)
    assert mutant != body
    with tempfile.TemporaryDirectory(dir=tmp_path) as td:
        assert compile_status(Path(td), compiler, harness + mutant + checks) != 0


@pytest.mark.parametrize("compiler", COMPILERS)
def test_sub51e6f0_keeps_decimal_space_text_call_order_and_logger_tails(tmp_path, compiler):
    path = owner_path("log.cpp")
    text = path.read_text()
    body = source_body(path, "sub_51e6f0")
    typedef = "typedef char *(*StrcatFn)(char *, const char *);"
    assert text.count(typedef) == 1
    harness = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#define __cdecl
static const char BufferJoinSpace[]=" "; static char got[256]; static int got_value, got_a, got_b, calls;
static char *_itoa(int value, char *buffer, int radix) { if (radix!=10) return nullptr; std::sprintf(buffer,"%d",value); return buffer; }
static void log_say_hex(const char *s, int v, int a, int b) { std::strcpy(got,s); got_value=v; got_a=a; got_b=b; ++calls; }
'''
    checks = r'''
int main() { sub_51e6f0("tail", -27); if (calls!=1) return 1;
 if (std::strcmp(got,"-27 tail") || got_value!=-27 || got_a || got_b) return 2; return 0; }
'''
    with tempfile.TemporaryDirectory(dir=tmp_path) as td:
        compile_run(Path(td), compiler, harness + typedef + "\n" + body + checks)
    mutant = body.replace("BufferJoinSpace", '""', 1)
    assert mutant != body
    with tempfile.TemporaryDirectory(dir=tmp_path) as td:
        assert compile_status(Path(td), compiler, harness + typedef + "\n" + mutant + checks) != 0


@pytest.mark.parametrize("compiler", COMPILERS)
def test_tech_advance_preserves_early_return_batch_gate_and_callback_order(tmp_path, compiler):
    body = source_body(owner_path("technology.cpp"), "tech_advance")
    harness = r'''
#include <cstdint>
#define __cdecl
struct PlayerData { int tech_id_researching=0, earned_techs_saved=0; };
struct Player { char *name_leader; };
struct LockMasks { unsigned char enable_mask_=0; };
struct Fields { LockMasks lock_masks_; int turn_current_num_=0, base_current_count_=0; };
struct Storage { Fields fields_; };
struct Console { int faction_id_=0; };
static PlayerData PlayersData[8]; static char player_name[]="Faction"; static Player Players[8]={{player_name}};
static Storage GameDataStateStorage; static Console ConsoleGlobal;
static int IsMultiplayerNet=0, IsMultiplayerPBEM=0, TurnUpkeepPhase=0;
static int BaseIDCurrentSelected=0, TechAchievedGate=0, AiResearchBatchActive=0;
static int log_calls=0, achieve_calls=0, reset_calls=0, set_calls=0, compute_calls=0, select_calls=0;
static int trace[8], trace_count=0, selection_value=-9;
static void log_say(const char*, char*, int, int, int) { ++log_calls; trace[trace_count++]=1; }
static int tech_selection(int) { ++select_calls; trace[trace_count++]=2; return selection_value; }
static void tech_achieved(int,int,int,int) { ++achieve_calls; trace[trace_count++]=3; }
static void bases_reset(int,int,int) { ++reset_calls; trace[trace_count++]=4; }
static void set_base(int) { ++set_calls; trace[trace_count++]=5; }
static void base_compute(int) { ++compute_calls; trace[trace_count++]=6; }
'''
    checks = r'''
int main() {
  PlayersData[1].tech_id_researching=7; Players[1].name_leader=player_name; ConsoleGlobal.faction_id_=0;
  GameDataStateStorage.fields_.turn_current_num_=1; GameDataStateStorage.fields_.base_current_count_=4; BaseIDCurrentSelected=2;
  if (tech_advance(1)!=7 || PlayersData[1].tech_id_researching!=-1) return 1;
  if (log_calls!=1 || achieve_calls!=1 || reset_calls!=1 || set_calls!=1 || compute_calls!=1) return 2;
  if (trace_count!=5 || trace[0]!=1 || trace[1]!=3 || trace[2]!=4 || trace[3]!=5 || trace[4]!=6) return 3;
  // Net phase 1 logs the earned-tech text and increments the saved counter, not callbacks.
  IsMultiplayerNet=1; TurnUpkeepPhase=1; int before=trace_count;
  if (tech_advance(1)!=-1 || PlayersData[1].earned_techs_saved!=1 || trace_count!=before+2) return 4;
  // Failed reselection returns immediately before mutation/callback/counter increment.
  IsMultiplayerNet=0; TurnUpkeepPhase=0; PlayersData[2].tech_id_researching=-1; selection_value=-7;
  int earned=PlayersData[2].earned_techs_saved, callbacks=achieve_calls+reset_calls+set_calls+compute_calls;
  if (tech_advance(2)!=-7 || PlayersData[2].earned_techs_saved!=earned ||
      achieve_calls+reset_calls+set_calls+compute_calls!=callbacks) return 5;
  return 0;
}
'''
    with tempfile.TemporaryDirectory(dir=tmp_path) as td:
        compile_run(Path(td), compiler, harness + body + checks)
    mutant = body.replace("if (selection < 0)", "if (selection >= 0)", 1)
    assert mutant != body
    with tempfile.TemporaryDirectory(dir=tmp_path) as td:
        assert compile_status(Path(td), compiler, harness + mutant + checks) != 0


def test_owner_transfer_preserves_original_mappings_and_lessons_from_history():
    from match_recovery.source import Mode, read_text

    history = subprocess.run(
        ["git", "show", f"{HISTORY_BASELINE}:src/leaf_recoveries.cpp"],
        cwd=ROOT, check=True, text=True, capture_output=True,
    ).stdout
    fields = ("address", "name", "image_spans", "byte_exact", "semantic", "levers",
              "ruled_out", "unrecoverable", "deferred", "kind")
    destinations = {
        0x51E000: ("game.cpp", "read_tamper"),
        0x51E6F0: ("log.cpp", "sub_51e6f0"),
        0x5BE530: ("technology.cpp", "tech_advance"),
    }
    old_rows = {r.address: r for r in read_text(history, "src/leaf_recoveries.cpp")
                if r.address in destinations}
    assert set(old_rows) == set(destinations)
    for address, (filename, function) in destinations.items():
        current_text = owner_path(filename).read_text()
        rows = [r for r in read_text(current_text, "src/" + filename) if r.address == address]
        assert len(rows) == 1
        current = rows[0]
        before = old_rows[address]
        assert current.mode is Mode.BODY
        for field in fields:
            assert getattr(current, field) == getattr(before, field), (hex(address), field)
        old_lessons = tuple((lesson.kind, lesson.key, lesson.prose,
                             tuple(line for _, line in lesson.lines))
                            for lesson in before.origin or ())
        current_lessons = tuple((lesson.kind, lesson.key, lesson.prose,
                                 tuple(line for _, line in lesson.lines))
                                for lesson in current.origin or ())
        assert current_lessons == old_lessons, hex(address)
        assert len(re.findall(rf"\b{re.escape(function)}\s*\(",
                              __import__("match_recovery.includes", fromlist=["COMMENTS_OR_STRINGS"])
                              .COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), current_text))) >= 1
        source_body(owner_path(filename), function)
    assert not (ROOT / "src/leaf_recoveries.cpp").exists(), "retired leaf owner source reappeared"


def test_farm_get_table_bindings_match_current_source_and_pinned_data():
    import hashlib
    from match_recovery.model import Span
    from match_recovery.object_source import parse_objects
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_config import SourceConfiguration

    pin_match = re.search(r"(?m)^sha256 ([0-9a-f]{64})$", (ROOT / "src/IMAGE_PIN").read_text())
    assert pin_match
    pin = pin_match.group(1)
    image_bytes = (ROOT / ".opensmacx/game/terranx_original.exe").read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == pin
    image = PeImage(image_bytes, pin)
    expected = ((0x66EF28, 0x66EF38, "?RiverStepX@@3QBHB", (1, 1, -1, -1)),
                (0x66EF3C, 0x66EF4C, "?RiverStepY@@3QBHB", (-1, 1, 1, -1)))
    map_text = (ROOT / "src/map.cpp").read_text()
    code = __import__("match_recovery.includes", fromlist=["COMMENTS_OR_STRINGS"])
    masked = code.COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), map_text)
    object_rows = parse_objects(map_text, "src/map.cpp", pin,
                                address_space=SourceConfiguration.read(ROOT).address_space)
    for start, end, symbol, values in expected:
        rows = [r for r in object_rows if r.origin.address == start]
        assert len(rows) == 1 and rows[0].symbol == symbol
        assert rows[0].span == Span(start, end) and rows[0].evidence.address == 0x409100
        assert struct.unpack("<4i", image.read(Span(start, end))) == values
        initializer = f"const int {symbol.split('@@')[0][1:]}[4] = {{"
        assert masked.count(initializer) == 1


def farm_fixture(body, radius_move_body, table_definitions):
    return r'''#include <assert.h>
#include <stdint.h>
#define __cdecl
#define MEASURED
int RadiusOffsetX[289];
int RadiusOffsetY[289];
''' + table_definitions + r'''
struct Rect { int left, top, right, bottom; };
struct BufferMock {
    int pixel, calls, last_x, last_y;
    BufferMock() : pixel(10), calls(0), last_x(-1), last_y(-1) {}
    int get_pixel(int x, int y) { ++calls; last_x=x; last_y=y; return pixel; }
};
struct BaseWin { Rect rect1_; BufferMock buffer2_; int farm_get(int,int); };
''' + radius_move_body + "\n" + body + r'''
static void set_match(int x, int y) {
    for (int i=0; i<289; ++i) RadiusOffsetX[i]=RadiusOffsetY[i]=1000;
    RadiusOffsetX[7]=x; RadiusOffsetY[7]=y;
}
int main() {
    BaseWin win;
    win.rect1_.left=0; win.rect1_.right=384; win.rect1_.top=0; win.rect1_.bottom=192;
    const int dx[5]={0,1,1,-1,-1}, dy[5]={0,-1,1,1,-1};
    const int pixels[12]={0,9,10,25,26,41,42,57,58,73,74,89};
    const int indexes[12]={0,0,0,0,1,1,2,2,3,3,4,4};
    for (int n=0; n<12; ++n) {
        int idx=indexes[n];
        set_match(-1+dx[idx],-1+dy[idx]);
        win.buffer2_.pixel=pixels[n]; win.buffer2_.calls=0;
        assert(win.farm_get(125,63)==7);
        assert(win.buffer2_.calls==1 && win.buffer2_.last_x==29 && win.buffer2_.last_y==15);
    }
    win.buffer2_.calls=0;
    assert(win.farm_get(-1,63)==-1 && win.farm_get(384,63)==-1);
    assert(win.farm_get(125,-1)==-1 && win.farm_get(125,192)==-1);
    assert(win.buffer2_.calls==0);
    return 0;
}
'''


@pytest.mark.parametrize("compiler", COMPILERS)
def test_current_farm_get_tables_and_index_domain_controls(tmp_path, compiler):
    from match_recovery.includes import COMMENTS_OR_STRINGS

    basewin = (ROOT / "src/basewin.cpp").read_text()
    maph = (ROOT / "src/map.h").read_text()
    mapcpp = (ROOT / "src/map.cpp").read_text()
    farm = source_body(ROOT / "src/basewin.cpp", "BaseWin::farm_get")
    radius = code_definition(maph, "MEASURED inline int __cdecl radius_move(int x_radius_off, int y_radius_off, int range)")
    map_code = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), mapcpp)
    table_defs = []
    for name in ("RiverStepX", "RiverStepY"):
        matches = list(re.finditer(rf"(?m)^const int {name}\[4\] = \{{[^}}]+\}};", map_code))
        assert len(matches) == 1
        table_defs.append(mapcpp[matches[0].start():matches[0].end()])
    tables = "\n".join(table_defs)
    assert "reinterpret_cast<const int *>(0x0066EF" not in farm
    assert "RiverStepX[idx - 1]" in farm and "RiverStepY[idx - 1]" in farm
    assert "if (idx != 0)" in farm
    assert "idx >=" not in farm and "idx <" not in farm  # no new safety/range behavior

    code = farm_fixture(farm, radius, tables)
    with tempfile.TemporaryDirectory(dir=tmp_path) as td:
        compile_run(Path(td), compiler, code)
    for mutant in (
        farm.replace("RiverStepX[idx - 1]", "RiverStepY[idx - 1]", 1),
        farm.replace("RiverStepX[idx - 1]", "RiverStepX[idx]", 1),
    ):
        assert mutant != farm
        with tempfile.TemporaryDirectory(dir=tmp_path) as td:
            assert compile_status(Path(td), compiler, farm_fixture(mutant, radius, tables)) != 0
