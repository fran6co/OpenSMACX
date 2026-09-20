"""Bounded Path::find/move controls, not full pathfinding or ABI proof.

Host/UBSan tests compile expressions and blocks extracted from the product.
Map records, queues, and players are explicit doubles; the selected arithmetic,
guards, and loop bodies are live source. No complete planner is substituted.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def parenthesized(text, start):
    assert text[start] == "("
    depth = 0
    for end in range(start, len(text)):
        depth += (text[end] == "(") - (text[end] == ")")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated expression")


def expressions():
    source = (ROOT / "src/path.cpp").read_text()
    body = source.split("int Path::find(", 1)[1].split("int Path::move(", 1)[0]
    body = re.sub(r"/\*.*?\*/|//[^\n]*", "", body, flags=re.S)
    distances = [m for m in re.findall(r"int dist\s*=\s*([^;]+);", body)
                 if "dx" in m and "abs(y_next - y_dst)" in m]
    penalties = []
    for match in re.finditer(r"if\s*\(", body):
        condition = parenthesized(body, match.end() - 1)
        if "owner_base > 0" in condition:
            penalties.append(condition)
    territories = []
    territory = r"(?:map_loc\([^()]*\)|tile)->territory"
    for match in re.finditer(r"\b1[uU]?\s*<<\s*", body):
        start = match.end()
        if body[start] == "(":
            rhs = parenthesized(body, start)
        else:
            raw = re.match(territory, body[start:])
            if not raw:
                continue
            rhs = raw.group()
        if re.search(territory, rhs):
            territories.append(body[match.start():start] + rhs)
    assert len(distances) == 2
    assert len(penalties) == 2
    assert len(territories) == 9
    # The tenth read is a signed owner comparison, not a shift operand.
    remaining = body
    for expression in territories:
        remaining = remaining.replace(expression, "EXTRACTED_SHIFT", 1)
    remaining, count = re.subn(r"int territory_next\s*=\s*tile->territory;", "", remaining)
    assert count == 1
    assert not re.search(territory, remaining)
    return distances, penalties, territories


def harness(distances, penalties, territories):
    functions = []
    for i, expr in enumerate(distances):
        functions.append(f"int distance{i}(int dx, int y_next, int y_dst) {{ return {expr}; }}")
    for i, expr in enumerate(penalties):
        functions.append(f"""bool penalty{i}(unsigned mask_treaty, unsigned mask_truce,
                int owner_base, bool has_base) {{
            Record value = {{0, has_base ? BIT_BASE_IN_TILE : 0}};
            Record *tile_base = &value;
            return {expr};
        }}""")
    for i, expr in enumerate(territories):
        functions.append(f"""unsigned territory{i}(int t) {{
            Record value = {{static_cast<signed char>(t), 0}};
            Record *tile = &value;
            current = &value;
            return {expr};
        }}""")
    checks = []
    for i in range(len(distances)):
        checks.append(f"""for (int dx : {{0,1,2,3,255,256,257,258,511,512,1024}})
            for (int dy : {{-257,-2,-1,0,1,2,257}})
                if (distance{i}(dx, dy, 0) != ((dx + abs(dy)) / 2) * 2) return {10+i};""")
    for i in range(len(penalties)):
        # An unrelated nearby base must still double a nonzero penalty. The
        # original OR is not a membership test; no-base/owner bounds still gate.
        checks.append(f"""for (int owner = -1; owner <= 8; ++owner)
            for (bool present : {{false, true}})
                for (unsigned mask : {{0u, 2u, 128u}})
                    if (penalty{i}(mask, 4u, owner, present)
                        != (present && owner > 0 && owner < 8)) return {20+i};""")
    for i in range(len(territories)):
        checks.append(f"""for (int t = -1; t <= 7; ++t) {{
            unsigned expected = t < 0 ? 0x80000000u : (1u << t);
            if (territory{i}(t) != expected) return {30+i};
        }}""")
    return """
        #include <cstdlib>
        #include <initializer_list>
        using std::abs;
        const int BIT_BASE_IN_TILE = 1;
        struct Record { signed char territory; int bit; };
        Record *current;
        Record *map_loc(int, int) { return current; }
        const int x_src=0, y_src=0, x_dst=0, y_dst=0, x_threat=0, y_threat=0;
    """ + "\n".join(functions) + "\nint main() {\n" + "\n".join(checks) + "\nreturn 0; }\n"


def run_host(tmp_path, groups):
    return run_host_source(tmp_path, harness(*groups))


def run_host_source(tmp_path, source):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    cpp = tmp_path / "expressions.cpp"
    executable = tmp_path / "expressions"
    cpp.write_text(source)
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


def statement_block(text, start):
    """Extract a brace-delimited live statement, never its handwritten clone."""
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated block")


def control_blocks():
    source = (ROOT / "src/path.cpp").read_text()
    source = re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.S)
    find = statement_block(source, source.index("int Path::find("))
    move = statement_block(source, source.index("int Path::move("))
    floors = [statement_block(find, m.start())
              for m in re.finditer(r"if \(unk1 & 0x8 &&", find)]
    threats = []
    for match in re.finditer(r"for \(int i = 0; i < 9; i\+\+\)", find):
        block = statement_block(find, match.start())
        if "threat_mode =" in block:
            threats.append(block)
    gates = []
    for match in re.finditer(r"if\s*\(", find):
        condition = parenthesized(find, match.end() - 1)
        if (re.fullmatch(r"\(threat && faction_id > 0(?: && owner_next < 0)?\)", condition)
                and "BOOL treaty_penalty" in statement_block(find, match.start())):
            gates.append(condition)
    start = find.index("if (cur_cost >")
    queue = find[start:find.index("int zoc = 0;", start)]
    condition = re.search(r"if \(veh_moves\(veh_id\)", move)
    assert condition is not None
    assert len(floors) == 2 and len(threats) == 3 and len(gates) == 2
    return {
        "sea": statement_block(find, find.index("if (is_sea)")),
        "floors": floors, "threats": threats, "gates": gates, "queue": queue,
        "coordinates": statement_block(find, find.index("while (j < index1_")),
        "moves": parenthesized(move, condition.start() + 3),
        "treaties": statement_block(find, find.index("for (int f = 1; f < 8; f++)")),
    }


HOST_HEADERS = """
    #include <cstdint>
    #include <initializer_list>
    #include <cassert>
    typedef int BOOL;
    struct { uint32_t move_rate_roads; } Rules;
"""


def assert_host_control(tmp_path, source, *, damaged=False):
    result = run_host_source(tmp_path, HOST_HEADERS + source)
    # Compilation must succeed even for damage: rejection is an observed result,
    # not a missing dependency or a broken extraction anchor.
    assert result.returncode == (91 if damaged else 0), result.stderr


@pytest.mark.parametrize("damage", [False, True])
def test_live_sea_mask_preserves_upper_routing_bits(tmp_path, damage):
    block = control_blocks()["sea"]
    if damage:
        assert "~0xc0" in block
        block = block.replace("~0xc0", "0x3f")
    source = "int apply(int unk1, bool is_sea) {" + block + "return unk1;}" + """
        int main() {
            for (int flags : {0, 0xc0, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x1234567f})
                for (bool sea : {false, true}) {
                    int expected = sea ? flags - (flags & 0xc0) : flags;
                    if (apply(flags, sea) != expected) return 91;
                }
            return 0;
        }
    """
    assert_host_control(tmp_path, source, damaged=damage)


@pytest.mark.parametrize("site", range(2))
@pytest.mark.parametrize("damage", [False, True])
def test_live_hex_cost_floor_at_both_sites(tmp_path, site, damage):
    block = control_blocks()["floors"][site]
    if damage:
        assert "hex <" in block
        block = block.replace("hex <", "hex >")
    source = "int apply(int hex, int unk1) {" + block + "return hex;}" + """
        int main() {
            for (unsigned roads : {0u, 1u, 3u, 100u}) {
                Rules.move_rate_roads = roads;
                for (int hex : {-1, 0, 1, 2, 3, 99, 100, 999})
                    for (int flags : {0, 8, 0x108}) {
                        int floor = static_cast<int>(roads) - 1;
                        int expected = (flags & 8) && hex < floor ? floor : hex;
                        if (apply(hex, flags) != expected) return 91;
                    }
            }
            return 0;
        }
    """
    assert_host_control(tmp_path, source, damaged=damage)


@pytest.mark.parametrize("site", range(3))
@pytest.mark.parametrize("damage", [False, True])
def test_live_threat_mode_stops_at_first_hit(tmp_path, site, damage):
    block = control_blocks()["threats"][site]
    mode, = re.findall(r"(\w+_threat_mode)\s*=", block)
    assert block.count("break;") == 1
    if damage:
        block = block.replace("break;", "")
    source = """
        struct Map { signed char territory; } tiles[9];
        int lookups;
        int xrange(int x) { return x; }
        bool on_map(int x, int y) { return x >= 0 && x < 9 && y == 0; }
        Map *map_loc(int x, int) { ++lookups; return &tiles[x]; }
        int RadiusOffsetX[9] = {0,1,2,3,4,5,6,7,8};
        int RadiusOffsetY[9] = {};
        int apply() {
            int x_src=0, y_src=0, x_dst=0, y_dst=0, x_cur=0, y_cur=0;
            int mask_vendetta=2, dst_threat_mode=0, cur_threat_mode=0;
    """ + block + "return " + mode + ";}" + """
        int main() {
            for (int i=0; i<9; ++i) tiles[i].territory=1;
            lookups=0;
            if (apply()!=1 || lookups!=1) return 91;
            tiles[0].territory=0;
            lookups=0;
            if (apply()!=2 || lookups!=2) return 91;
            for (int i=0; i<9; ++i) tiles[i].territory=-1;
            lookups=0;
            if (apply()!=0 || lookups!=5) return 91;
            return 0;
        }
    """
    assert_host_control(tmp_path, source, damaged=damage)


@pytest.mark.parametrize("damage", [None, "cutoff_break", "fixed_cutoff", "source_expansion"])
def test_live_queue_cutoff_and_source_hit_continue_processing(tmp_path, damage):
    block = control_blocks()["queue"]
    assert block.count("continue;") == 2
    if damage == "cutoff_break":
        block = block.replace("continue;", "break;", 1)
    elif damage == "fixed_cutoff":
        block = block.replace("cur_cost > best_cost", "cur_cost > 2000000000")
    elif damage == "source_expansion":
        at = block.rindex("continue;")
        block = block[:at] + block[at:].replace("continue;", "", 1)
    source = """
        struct Entry { int cost, x, y; };
        bool apply(const Entry *queue, int count, int expected_best,
                   int expected_expanded, int expected_sum) {
            int best_cost=2000000000, x_src=1, y_src=2;
            int visits=0, expanded=0, sum=0;
            for (int n=0; n<count; ++n) {
                ++visits;
                int cur_cost=queue[n].cost, x_cur=queue[n].x, y_cur=queue[n].y;
    """ + block + """
                ++expanded; sum += cur_cost;
            }
            return visits==count && best_cost==expected_best
                && expanded==expected_expanded && sum==expected_sum;
        }
        int main() {
            Entry cheaper[]={{11,1,2},{30,0,0},{7,1,2},{8,0,0},{6,0,0}};
            if (!apply(cheaper,5,7,1,6)) return 91;
            Entry equal[]={{11,1,2},{11,0,0},{10,0,0}};
            if (!apply(equal,3,11,2,21)) return 91;
            return 0;
        }
    """
    assert_host_control(tmp_path, source, damaged=damage is not None)


@pytest.mark.parametrize("damage", [None, "x_next", "y_next"])
def test_live_queue_coordinates_compare_full_int_values(tmp_path, damage):
    block = control_blocks()["coordinates"]
    if damage:
        assert "!= " + damage in block
        block = block.replace("!= " + damage, "!= (int16_t)" + damage)
    source = """
        int apply(int16_t x, int16_t y, int x_next, int y_next) {
            int16_t x_table_[1]={x}, y_table_[1]={y};
            int j=0, index1_=1;
    """ + block + "return j;}" + """
        int main() {
            for (int stored : {-32768,-1,0,32767})
                for (int next : {-65537,-32769,-32768,-1,0,32767,32768,65535,65536}) {
                    int expected=stored==next ? 0 : 1;
                    if (apply(stored,0,next,0)!=expected) return 91;
                    if (apply(0,stored,0,next)!=expected) return 91;
                }
            return 0;
        }
    """
    assert_host_control(tmp_path, source, damaged=damage is not None)


@pytest.mark.parametrize("damage", [None, "search", "reconstruction"])
def test_live_search_and_reconstruction_occupancy_gates_differ(tmp_path, damage):
    search, reconstruction = control_blocks()["gates"]
    if damage == "search":
        search = search.replace(" && owner_next < 0", "")
    elif damage == "reconstruction":
        reconstruction = reconstruction[:-1] + " && owner_next < 0)"
    source = "bool search(bool threat,int faction_id,int owner_next){return " + search + ";}" \
        + "bool reconstruction(bool threat,int faction_id,int owner_next){return " + reconstruction + ";}" + """
        int main() {
            for (bool threat : {false,true})
                for (int faction : {-1,0,1,7})
                    for (int owner : {-1,0,3,7}) {
                        bool active=threat && faction>0;
                        if (search(threat,faction,owner)!=(active && owner<0)) return 91;
                        if (reconstruction(threat,faction,owner)!=active) return 91;
                    }
            return 0;
        }
    """
    assert_host_control(tmp_path, source, damaged=damage is not None)


@pytest.mark.parametrize("damage", [False, True])
def test_live_move_road_budget_uses_signed_comparison(tmp_path, damage):
    condition = control_blocks()["moves"]
    if damage:
        assert "static_cast<int>(Rules.move_rate_roads)" in condition
        condition = condition.replace("static_cast<int>(Rules.move_rate_roads)", "Rules.move_rate_roads")
    source = "int moves; int veh_moves(int) {return moves;} bool apply(int veh_id){return " + condition + ";}" + """
        int main() {
            for (unsigned roads : {0u,1u,3u,100u,0x7fffffffu,0x80000000u,0xffffffffu}) {
                Rules.move_rate_roads=roads;
                for (int budget : {0,1,2,3,999}) {
                    moves=budget;
                    bool expected=roads<0x80000000u && static_cast<unsigned>(budget)<=roads;
                    if (apply(0)!=expected) return 91;
                }
            }
            return 0;
        }
    """
    assert_host_control(tmp_path, source, damaged=damage)


@pytest.mark.parametrize("damage", [None, "priority", "own_faction"])
def test_live_treaty_mask_loop_priority_and_own_faction_exclusion(tmp_path, damage):
    block = control_blocks()["treaties"]
    if damage == "priority":
        assert block.count("else if") == 2
        block = block.replace("else if", "if")
    elif damage == "own_faction":
        assert block.count("continue;") == 1
        block = block.replace("continue;", "")
    source = """
        enum { DTREATY_PACT=1, DTREATY_TREATY=2, DTREATY_TRUCE=4,
               DTREATY_VENDETTA=16, DTREATY_SHALL_BETRAY=0x400, DTREATY_UNK_800=0x800 };
        struct Player { uint32_t diplo_treaties[8]; int region_total_bases[4]; } PlayersData[8];
        struct Masks { int treaty,truce,vendetta; };
        Masks apply(int faction_id) {
            int region_dst=2, mask_treaty=0, mask_truce=0, mask_vendetta=0;
    """ + block + "return Masks{mask_treaty,mask_truce,mask_vendetta};}" + """
        struct Case { unsigned flags; int bases, category; };
        int main() {
            // Hand-labelled precedence cases: a pact consumes a qualifying
            // treaty; betrayal disables treaty/truce eligibility, not vendetta.
            Case cases[]={{23,1,0},{22,1,1},{20,1,2},{16,0,0},{16,1,3},
                          {0x412,1,3},{0x814,1,3},{0x402,1,0},{2,0,1},{4,0,2}};
            for (int faction=1; faction<8; ++faction)
                for (int other=1; other<8; ++other)
                    for (const Case &c : cases) {
                        for (int i=0;i<8;++i) PlayersData[i]=Player{};
                        PlayersData[faction].diplo_treaties[other]=c.flags;
                        PlayersData[other].region_total_bases[2]=c.bases;
                        Masks actual=apply(faction);
                        int bit=other==faction ? 0 : 1<<other;
                        if (actual.treaty!=(c.category==1 ? bit : 0)
                            || actual.truce!=(c.category==2 ? bit : 0)
                            || actual.vendetta!=(c.category==3 ? bit : 0)) return 91;
                    }
            return 0;
        }
    """
    assert_host_control(tmp_path, source, damaged=damage is not None)


def test_actual_path_arithmetic(tmp_path):
    result = run_host(tmp_path, expressions())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("site", range(2))
def test_old_distance_mask_is_rejected(tmp_path, site):
    groups = expressions()
    groups[0][site], count = re.subn(r"&\s*~1\b", "& 0xFE", groups[0][site])
    assert count == 1
    result = run_host(tmp_path, groups)
    assert result.returncode == 10 + site, result.stderr


@pytest.mark.parametrize("site", range(2))
def test_old_base_membership_test_is_rejected(tmp_path, site):
    groups = expressions()
    groups[1][site], count = re.subn(
        r"mask_treaty\s*\|\s*mask_truce\s*\|\s*\(1[uU]?\s*<<\s*owner_base\)",
        "(mask_treaty | mask_truce) & (1u << owner_base)", groups[1][site],
    )
    assert count == 1
    result = run_host(tmp_path, groups)
    assert result.returncode == 20 + site, result.stderr


@pytest.mark.parametrize("site", range(9))
def test_old_signed_territory_shift_is_rejected(tmp_path, site):
    groups = expressions()
    operand = re.search(r"(?:map_loc\([^()]*\)|tile)->territory", groups[2][site]).group()
    assert "static_cast<unsigned>" in groups[2][site]
    groups[2][site] = "1 << " + operand
    result = run_host(tmp_path, groups)
    assert result.returncode != 0
    assert "shift exponent -1 is negative" in result.stderr


def test_pinned_original_arithmetic_instructions():
    image = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image.is_file():
        pytest.skip("pinned original image unavailable; set OPENSMACX_IMAGE")
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    pe = PeImage(image.read_bytes(), PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)

    def instructions(start, end):
        return [(i.mnemonic, i.op_str) for i in decoder.disasm(pe.read(Span(start, end)), start)]

    for address in (0x59AF07, 0x59B797):
        assert instructions(address, address + 5) == [("and", "al, 0xfe"), ("cmp", "eax, 2")]
    for start, end in ((0x59B1EA, 0x59B1FA), (0x59BA5F, 0x59BA6F)):
        actual = instructions(start, end)
        assert actual[:4] == [("mov", "edx, 1"), ("shl", "edx, cl"),
                              ("or", "edx, dword ptr [ebp - 0x38]"), ("or", "edx, eax")]
        assert actual[4][0] == "je"
        assert actual[5] == ("add", "edi, edi")
    # All nine territory-to-bit operations load a byte into CL and use SHL.
    for address in (0x59A647, 0x59A657, 0x59A779, 0x59A9F6, 0x59AFF9,
                    0x59B0DB, 0x59B524, 0x59B884, 0x59B95F):
        assert instructions(address, address + 2) in [
            [("shl", "eax, cl")], [("shl", "edx, cl")]]
