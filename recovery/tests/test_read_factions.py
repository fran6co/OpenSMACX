"""Live read_factions sequencing with parser/RNG/art doubles, not game proof."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def live_body():
    path = ROOT / "src/alpha.cpp"
    record, = (r for r in read_file(path) if r.address == 0x586F30)
    assert record.image_spans == ((0x586F30, 0x5871C9),)
    source = re.sub(r"/\*.*?\*/|//[^\n]*", "", path.read_text(), flags=re.S)
    signature = "BOOL __cdecl read_factions()"
    assert source.count(signature) == 1
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated read_factions")


HARNESS = r"""
    #include <cstdint>
    #include <cstdio>
    #include <cstdlib>
    #include <cstring>
    #define __cdecl
    typedef int BOOL;
    typedef char *LPSTR;
    void check(bool condition) { if (!condition) std::exit(91); }
    const int MaxPlayerNum=8, MaxBonusNameNum=41;
    struct Player { char filename[24], search_key[24]; } Players[8];
    struct Bonus { char key[24]; } BonusName[41];
    const char *AlphaxFileID="ALPHAX";
    int ExpansionEnabled, randoms, fail_at, opens, line, token, prefs_calls, rng_calls;
    int line_reads[8], items[8], events[32], event_count;
    char text[64];
    const char *initial[] = {"unused","A","B","C","D","E","F","G"};

    int text_open(const char *file, const char *section) {
        check(std::strcmp(file,"ALPHAX")==0);
        ++opens; check(opens<=6);
        const char *expected[] = {"unused","BONUSNAMES",
            ExpansionEnabled ? "NEWFACTIONS" : "FACTIONS", "CUSTOMFACTIONS",
            "FACTIONS","NEWFACTIONS","CUSTOMFACTIONS"};
        check(std::strcmp(section,expected[opens])==0);
        line=token=0;
        return opens==fail_at;
    }
    void text_get() {
        check(opens>0 && opens!=fail_at);
        ++line; token=0; ++line_reads[opens];
    }
    LPSTR text_item() {
        check(line>0 && opens!=fail_at);
        ++items[opens];
        if (opens==1) {
            check(token<8);
            std::sprintf(text,"bonus%d",(line-1)*8+token);
        } else if (opens==2) {
            check(line<=7 && token<2);
            if (token) std::sprintf(text,"key%d",line);
            else std::strcpy(text,randoms && (line==2 || line==5) ? "JENN282" : initial[line]);
        } else if (opens==3) {
            check(line<=3 && token==0);
            std::strcpy(text,line<3 ? "custom-entry" : "");
        } else {
            // One token only per random selection; search_key must copy
            // the saved filename rather than consume a second token.
            check(token==0);
            check(line==(opens==5 ? 2 : 1));
            std::strcpy(text,opens==4 ? "A" : opens==5 ? "NEW2" : "CUSTOM0");
        }
        ++token;
        return text;
    }
    void prefs_fac_load() { check(ExpansionEnabled && opens==2); ++prefs_calls; }
    // Interpose the C-library RNG named by the live body, not the game's RNG.
    extern "C" int rand() noexcept {
        const int rolls[]={0,8,14};
        check(randoms && rng_calls<3);
        return rolls[rng_calls++];
    }
    void read_faction(Player *player, int mode) {
        int id=static_cast<int>(player-Players);
        check(id>=1 && id<=7 && mode==0 && event_count<31);
        check(event_count%2==0);
        events[event_count++]=100+id;
    }
    void load_faction_art(int id) {
        check(event_count>0 && event_count%2==1 && events[event_count-1]==100+id);
        events[event_count++]=200+id;
    }

    // LIVE_BODY

    void scenario(int expansion, int random_selection, int failed_open) {
        std::memset(Players,0,sizeof(Players));
        std::memset(BonusName,0,sizeof(BonusName));
        std::memset(line_reads,0,sizeof(line_reads)); std::memset(items,0,sizeof(items));
        std::strcpy(Players[0].filename,"native-sentinel");
        ExpansionEnabled=expansion; randoms=random_selection; fail_at=failed_open;
        opens=line=token=prefs_calls=rng_calls=event_count=0;
        bool failed=read_factions()!=0;
        check(failed==(fail_at!=0 && fail_at!=3));
        check(std::strcmp(Players[0].filename,"native-sentinel")==0);
        if (fail_at==1) { check(opens==1 && event_count==0 && prefs_calls==0); return; }
        check(line_reads[1]==6 && items[1]==41);
        for (int i=0;i<41;++i) {
            char expected[24]; std::sprintf(expected,"bonus%d",i);
            check(std::strcmp(BonusName[i].key,expected)==0);
        }
        if (fail_at==2) { check(opens==2 && event_count==0 && prefs_calls==0); return; }
        check(line_reads[2]==7 && items[2]==14 && prefs_calls==expansion);
        check(line_reads[3]==(fail_at==3 ? 0 : 3) && items[3]==line_reads[3]);
        if (failed) {
            check(opens==fail_at && rng_calls==fail_at-3);
            check(event_count==(fail_at==6 ? 2 : 0));
            if (event_count) check(events[0]==102 && events[1]==202);
            return;
        }
        check(opens==(randoms ? 6 : 3) && rng_calls==(randoms ? 3 : 0));
        const int expected_ids[]={2,5,1,2,3,4,5,6,7};
        check(event_count==(randoms ? 18 : 14));
        for (int n=0;n<event_count/2;++n) {
            int id=randoms ? expected_ids[n] : n+1;
            check(events[n*2]==100+id && events[n*2+1]==200+id);
        }
        for (int id=1;id<8;++id) {
            char expected_key[24]; std::sprintf(expected_key,"key%d",id);
            const char *filename=initial[id], *key=expected_key;
            if (randoms && id==2) filename=key="NEW2";
            if (randoms && id==5) filename=key="CUSTOM0";
            check(std::strcmp(Players[id].filename,filename)==0);
            check(std::strcmp(Players[id].search_key,key)==0);
        }
        if (randoms) {
            check(line_reads[4]==1 && line_reads[5]==2 && line_reads[6]==1);
            check(items[4]==1 && items[5]==1 && items[6]==1);
        }
    }
    int main() {
        scenario(0,1,0); scenario(1,1,0); scenario(0,0,0);
        scenario(1,0,3); // Missing CUSTOMFACTIONS is optional, not fatal.
        scenario(0,1,1); scenario(1,1,2);
        scenario(1,1,4); scenario(1,1,5); scenario(1,1,6);
        return 0;
    }
"""


def run_host(tmp_path, body):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    source=tmp_path / "read_factions.cpp"
    executable=tmp_path / "read_factions"
    source.write_text(HARNESS.replace("// LIVE_BODY",body))
    compiled=subprocess.run(
        [compiler,"-std=c++11","-O1","-fsanitize=undefined",
         "-fno-sanitize-recover=undefined",str(source),"-o",str(executable)],
        capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stderr
    return subprocess.run([str(executable)],capture_output=True,text=True,timeout=10)


def test_live_read_factions_parser_rerolls_and_two_pass_order(tmp_path):
    result=run_host(tmp_path,live_body())
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("damage", [
    "second_token", "no_reroll", "no_first_parse", "skip_final_randoms",
    "unguarded_preferences", "ignored_open_failure",
])
def test_read_factions_behavior_controls(tmp_path,damage):
    body=live_body()
    expected_count=1
    if damage=="second_token":
        old,new="strcpy(Players[player].search_key, Players[player].filename);", \
            "strcpy(Players[player].search_key, text_item());"
    elif damage=="no_reroll":
        old,new="while (faction_id == -1);","while (false);"
    elif damage=="no_first_parse":
        old,new="read_faction(&Players[player], 0);","(void)0;"
        expected_count=2
    elif damage=="skip_final_randoms":
        old,new="if (&Players[player] != &Players[0])", \
            "if (&Players[player] != &Players[0] && player != 2 && player != 5)"
    elif damage=="unguarded_preferences":
        old,new="if (ExpansionEnabled)","if (true)"
    else:
        old='if (text_open(AlphaxFileID, "BONUSNAMES"))'
        new='if (text_open(AlphaxFileID, "BONUSNAMES") && false)'
    assert body.count(old)==expected_count
    body=body.replace(old,new,1)
    result=run_host(tmp_path,body)
    # Mutants must compile; exit 91 means a runtime contract was violated.
    assert result.returncode==91,result.stderr
