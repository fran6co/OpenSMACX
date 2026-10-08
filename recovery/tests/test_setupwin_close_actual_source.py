"""Actual-source controls for SetupWin cleanup and its shared list traversal."""
from __future__ import annotations

from pathlib import Path
import os
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]



def extract_body(text: str, signature: str) -> str:
    start = text.index(signature + " {")
    opening = text.index("{", start)
    depth = 0
    for pos in range(opening, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError("unterminated source body: " + signature)


def current_candidate_parts():
    close = extract_body((ROOT / "src/setupwin.cpp").read_text(), "void SetupWin::close()")

    source = (ROOT / "src/stringstruct.cpp").read_text()
    clear = extract_body(source, "inline void clear_list_entries(StringStruct *self)")
    remove = extract_body(source, "void StringStruct::remove_all()")
    # The checked-in helper uses the standard nullptr keyword; replace only
    # that spelling so the otherwise C++98 host fixture matches product logic.
    clear = clear.replace("nullptr", "0")
    return close, clear, remove


HARNESS = r'''
#include <stdio.h>
#include <stdlib.h>

static int events[128];
static int event_count;
static int payload_deleted;
static int entry_deleted;
static void event(int value) { events[event_count++]=value; }
static void require(bool ok, const char *name) {
    if (!ok) { fprintf(stderr, "FAIL: %s\n", name); abort(); }
}

struct ListPayloadBase { virtual ~ListPayloadBase() {} };
struct StringStructEntry {
    ListPayloadBase *payload;
    StringStructEntry *next;
    virtual ~StringStructEntry() { ++entry_deleted; event(5); }
};
struct StringStruct {
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
    StringStruct() : head_(0), current_(0), entry_count_(0), current_position_(0) {}
    virtual void on_remove(void *) {}
    void remove_all();
};
struct StringList : StringStruct {
    int removals;
    int mutate_count_on_first_remove;
    StringList() : removals(0), mutate_count_on_first_remove(0) {}
    virtual void on_remove(void *payload) {
        ++removals; event(3);
        require(payload == head_->payload, "on_remove receives current entry payload");
        if (mutate_count_on_first_remove && removals == 1) entry_count_=1;
    }
};
struct Payload : ListPayloadBase {
    int id;
    explicit Payload(int value) : id(value) {}
    virtual ~Payload() { ++payload_deleted; event(4); }
};
struct Buffer {
    struct SetupWin *owner;
    int closed;
    Buffer() : owner(0), closed(0) {}
    void close();
};
struct Spot {
    struct SetupWin *owner;
    int shut_down;
    Spot() : owner(0), shut_down(0) {}
    void shutdown();
};
struct SetupWin {
    unsigned int selected_row_, row_count_, field_A14_, field_A18_;
    Buffer buffer_;
    Spot spot_;
    StringList field_FE0_;
    SetupWin() : selected_row_(19), row_count_(21), field_A14_(23), field_A18_(25) {
        buffer_.owner=this; spot_.owner=this;
    }
    void close();
};
void Buffer::close() {
    require(owner->selected_row_==0 && owner->row_count_==0 &&
            owner->field_A14_==0 && owner->field_A18_==0,
            "state clears precede Buffer close");
    closed=1; event(1);
}
void Spot::shutdown() {
    require(owner->buffer_.closed, "Buffer closes before Spot shutdown");
    require(owner->field_A14_==0 && owner->field_A18_==0,
            "state clears precede Spot shutdown");
    shut_down=1; event(2);
}
'''


def actual_source_translation_unit() -> str:
    close, clear, remove = current_candidate_parts()
    return HARNESS + "\n" + clear + "\n" + remove + "\n" + close + r'''
static StringStructEntry *entry(ListPayloadBase *payload, StringStructEntry *next) {
    StringStructEntry *result = new StringStructEntry;
    result->payload=payload; result->next=next; return result;
}
static void reset_events() {
    event_count=payload_deleted=entry_deleted=0;
}
int main() {
    SetupWin window;
    Payload *p2=new Payload(2);
    Payload *p1=new Payload(1);
    StringStructEntry *e2=entry(p2,0);
    StringStructEntry *e1=entry(p1,e2);
    window.field_FE0_.head_=e1;
    window.field_FE0_.current_=e2;
    window.field_FE0_.entry_count_=2;
    window.field_FE0_.current_position_=9;
    reset_events();
    window.close();
    require(window.selected_row_==0 && window.row_count_==0 &&
            window.field_A14_==0 && window.field_A18_==0,
            "all four SetupWin state words clear");
    require(window.buffer_.closed && window.spot_.shut_down,
            "owned Buffer and Spot cleaned up");
    require(event_count==8, "cleanup event count");
    int expected[8]={1,2,3,4,5,3,4,5};
    for (int i=0;i<8;++i) require(events[i]==expected[i], "cleanup call/destructor order");
    require(payload_deleted==2 && entry_deleted==2 && window.field_FE0_.removals==2,
            "both list payloads and entries released once");
    require(window.field_FE0_.head_==0 && window.field_FE0_.entry_count_==0 &&
            window.field_FE0_.current_position_==0,
            "embedded StringList reset through its existing owner");

    // Preserve the original head-null branch: the terminal cursor store runs,
    // but a stale count is not cleared and no remove callback is invoked.
    SetupWin empty;
    empty.field_FE0_.entry_count_=7;
    empty.field_FE0_.current_position_=3;
    empty.close();
    require(empty.field_FE0_.removals==0 && empty.field_FE0_.entry_count_==7 &&
            empty.field_FE0_.current_position_==0,
            "null-head path retains stale count and resets cursor only");

    // A non-null head with zero count takes the original list-reset branch
    // without deleting the unreachable nodes; deliberately do not normalize it.
    SetupWin zero_count;
    StringStructEntry *leaked=entry(new Payload(8),0);
    zero_count.field_FE0_.head_=leaked;
    zero_count.field_FE0_.entry_count_=0;
    zero_count.close();
    require(zero_count.field_FE0_.head_==0 && zero_count.field_FE0_.entry_count_==0 &&
            zero_count.field_FE0_.removals==0 && payload_deleted==2 && entry_deleted==2,
            "non-null head/zero count preserves original skipped-destruction path");

    // The shared product helper reloads entry_count_ after on_remove: shrinking
    // it in a callback stops after the first entry, preserving the original
    // callback-visible partial teardown rather than snapshotting the count.
    SetupWin changing;
    Payload *second_payload=new Payload(22);
    Payload *first_payload=new Payload(21);
    StringStructEntry *second=entry(second_payload,0);
    StringStructEntry *first=entry(first_payload,second);
    changing.field_FE0_.head_=first;
    changing.field_FE0_.current_=second;
    changing.field_FE0_.entry_count_=2;
    changing.field_FE0_.current_position_=5;
    changing.field_FE0_.mutate_count_on_first_remove=1;
    const int before_payload=payload_deleted, before_entry=entry_deleted;
    changing.close();
    require(changing.field_FE0_.removals==1 && payload_deleted==before_payload+1 &&
            entry_deleted==before_entry+1 && changing.field_FE0_.head_==0 &&
            changing.field_FE0_.entry_count_==0,
            "list count is live across the virtual removal callback");
    return 0;
}
'''


def compile_run(compiler: str, source: str, tmp_path: Path) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / "setupwin.cpp"
    exe = tmp_path / "setupwin"
    cpp.write_text(source)
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                            str(cpp), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    env = dict(os.environ)
    # This runner's ptraced test subprocess cannot use LeakSanitizer; keep
    # AddressSanitizer and UBSan enabled, with leak detection disabled only.
    env["ASAN_OPTIONS"] = "detect_leaks=0"
    return subprocess.run([str(exe)], capture_output=True, text=True, env=env)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_setupwin_close_actual_method_and_actual_list_cleanup(compiler, tmp_path):
    if shutil.which(compiler) is None:
        pytest.skip(f"{compiler} unavailable")
    source = actual_source_translation_unit()
    # The product method comes from the moved candidate; the list teardown
    # helper and remove_all body are extracted verbatim from current product.
    assert "field_FE0_.head_ != 0" in source
    assert "field_FE0_.remove_all()" in source
    assert "reinterpret_cast" not in source
    result = compile_run(compiler, source, tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_removed_setupwin_abi_shim_is_not_reintroduced(compiler, tmp_path):
    if shutil.which(compiler) is None:
        pytest.skip(f"{compiler} unavailable")
    # The emitter's source writer transfer and direct-call owner declaration
    # are independent of host ABI emulation; assert these repository paths as
    # actual source facts before running the behavior control above.
    candidate = {p: (ROOT / p).read_text() for p in ("src/stringstruct.cpp", "src/setupwin.h", "src/setupwin.cpp")}
    assert "extern \"C\" void __fastcall SetupWin_close" not in candidate["src/stringstruct.cpp"]
    assert "void close();" in candidate["src/setupwin.h"]
    callers = (ROOT / "src/newgame.cpp").read_text().count("setup.close();")
    assert callers == 0
    assert "void SetupWin::close()" in candidate["src/setupwin.cpp"]
