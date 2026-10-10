"""Compile actual mapped focus and file-search bodies against observable APIs."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT = Path(__file__).resolve().parents[2]


def actual(unit, address, signature):
    record, = (r for r in read_file(ROOT / unit) if r.address == address)
    return _definition(record.region, signature)


def run(tmp_path, compiler_name, source, expected=True):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    cpp = tmp_path / "control.cpp"
    cpp.write_text(source)
    exe = tmp_path / "control"
    subprocess.run([compiler, "-std=c++11", str(cpp), "-o", str(exe)],
                   check=True, capture_output=True, text=True)
    result = subprocess.run([str(exe)], capture_output=True)
    assert (result.returncode == 0) == expected


FOCUS_PREFIX = r'''
#include <cassert>
struct Win {
    Win *win_parent_;
    Win *replacement;
    int focus, calls;
    bool mutate;
    Win():win_parent_(nullptr),replacement(nullptr),focus(1),calls(0),mutate(false) {}
    int is_dialog_focus() {
        ++calls;
        if (mutate) win_parent_ = replacement;
        return focus;
    }
};
struct GraphicWin : Win {};
struct EditBox : GraphicWin { int is_dialog_focus_chain(); };
'''
FOCUS_MAIN = r'''
int main() {
    EditBox editor;
    Win old_parent, new_parent;
    editor.focus = 0; editor.win_parent_ = &old_parent;
    assert(editor.is_dialog_focus_chain() == 0);
    assert(editor.calls == 1 && old_parent.calls == 0);
    editor.focus = -5; editor.win_parent_ = nullptr;
    assert(editor.is_dialog_focus_chain() == 1);
    editor.win_parent_ = &old_parent; old_parent.focus = 0;
    assert(editor.is_dialog_focus_chain() == 0 && old_parent.calls == 1);
    old_parent.focus = -8;
    assert(editor.is_dialog_focus_chain() == 1 && old_parent.calls == 2);
    editor.mutate = true; editor.replacement = &new_parent;
    old_parent.focus = 0; new_parent.focus = 1;
    assert(editor.is_dialog_focus_chain() == 1);
    assert(new_parent.calls == 1 && old_parent.calls == 2);
    editor.replacement = nullptr;
    assert(editor.is_dialog_focus_chain() == 1 && new_parent.calls == 1);
}
'''


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "first_focus", "parent_focus",
                                    "null_parent", "early_parent"))
def test_focus_member(tmp_path, compiler, damage):
    body = actual("src/editbox.cpp", 0x6161A0,
                  "int EditBox::is_dialog_focus_chain()")
    assert "reinterpret_cast" not in body and "0xC4" not in body
    if damage == "first_focus":
        body = body.replace("if (is_dialog_focus() == 0)", "if (false)")
    elif damage == "parent_focus":
        body = body.replace("parent->is_dialog_focus()", "is_dialog_focus()")
    elif damage == "null_parent":
        body = body.replace("    return 1;", "    return parent != nullptr;")
    elif damage == "early_parent":
        body = body.replace("    Win *const parent = win_parent_;", "")
        body = body.replace("{", "{\n    Win *const parent = win_parent_;", 1)
    run(tmp_path, compiler, FOCUS_PREFIX + body + FOCUS_MAIN, damage is None)


FILE_PREFIX = r'''
#include <cassert>
#include <cstring>
#include <cstdint>
#define __cdecl
typedef const char *LPCSTR;
typedef char *LPSTR;
typedef int BOOL;
typedef intptr_t HANDLE;
const int DRIVE_CDROM = 5;
const HANDLE INVALID_HANDLE_VALUE = -1;
struct WIN32_FIND_DATAA { int unused; };
struct Paths { char alt_path[256],relative_path[256],cd_path[256],exe_dir[256],last_path[256]; } FilefindPath;
int allocations, deletes, prompts, reads, scans, restores, closes;
int state, response, found_after;
bool fail_allocation;
struct BasePop {
    virtual ~BasePop() { ++deletes; }
    int start(char *caption,const char *label,int length,int a,int b,int c) {
        assert(std::strcmp(caption,"jackal")==0);
        assert(std::strcmp(label,"FILEFIND_NOCD")==0 && length==-1);
        assert(a==0 && b==0 && c==0); ++prompts; return response;
    }
    uint32_t input_poll_state() const { ++reads; return state; }
};
BasePop *allocate() { ++allocations; return fail_allocation ? nullptr : new BasePop; }
BasePop *(*PopupAllocHook)() = allocate;
int GetCurrentDirectoryA(int size,char *out) {
    assert(size==256); std::strcpy(out,"CURRENT"); return 7;
}
int GetDriveTypeA(char *root) {
    if (root[0]=='A') ++scans;
    return root[0]=='D' ? DRIVE_CDROM : 0;
}
HANDLE FindFirstFileA(char *path,WIN32_FIND_DATAA *) {
    assert(std::strcmp(path,"D:\\FILE")==0);
    return scans>=found_after ? 123 : INVALID_HANDLE_VALUE;
}
void FindClose(HANDLE h) { assert(h==123); ++closes; }
void SetCurrentDirectoryA(char *path) { assert(std::strcmp(path,"CURRENT")==0); ++restores; }
void reset() {
    allocations=deletes=prompts=reads=scans=restores=closes=0;
    state=response=0; found_after=100; fail_allocation=false;
    std::memset(&FilefindPath,0,sizeof(FilefindPath));
}
'''
FILE_MAIN = r'''
int main() {
    reset(); assert(filefind_init(nullptr,0)==16 && allocations==0);
    reset(); fail_allocation=true;
    assert(filefind_init("FILE",0)==4 && allocations==1 && deletes==0);
    reset(); assert(filefind_init("FILE",1)==0);
    assert(scans==1 && prompts==0 && reads==0 && deletes==1 && restores==0);
    reset(); found_after=1; assert(filefind_init("FILE",0)==0);
    assert(scans==1 && prompts==0 && closes==1 && deletes==1 && restores==1);
    assert(std::strcmp(FilefindPath.cd_path,"D:\\")==0);
    reset(); response=7; state=1; assert(filefind_init("FILE",0)==7);
    assert(prompts==1 && reads==0 && deletes==1 && restores==0);
    reset(); state=0; assert(filefind_init("FILE",0)==0);
    assert(prompts==1 && reads==1 && deletes==1 && restores==1);
    reset(); state=2; assert(filefind_init("FILE",0)==0);
    assert(prompts==1 && reads==1 && deletes==1 && restores==1);
    reset(); state=1; found_after=2; assert(filefind_init("FILE",0)==0);
    assert(scans==2 && prompts==1 && reads==1 && deletes==1 && closes==1);
    assert(restores==1 && std::strcmp(FilefindPath.relative_path,"FILE")==0);
}
'''


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "response_as_state", "nonzero_retry"))
def test_file_search_typed_state(tmp_path, compiler, damage):
    body = actual("src/general.cpp", 0x600400, "int __cdecl filefind_init(")
    assert "reinterpret_cast" not in body and "0x3100" not in body
    if damage == "response_as_state":
        body = body.replace("popup->input_poll_state() == 1", "response == 1")
    elif damage == "nonzero_retry":
        # Also bound the mutated retry so this diagnostic cannot loop forever.
        body = body.replace("popup->input_poll_state() == 1",
                            "popup->input_poll_state() != 0 && scans < 2")
    run(tmp_path, compiler, FILE_PREFIX + body + FILE_MAIN, damage is None)
