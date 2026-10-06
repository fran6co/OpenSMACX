"""Installed-source history and bounded FameWin I/O behavioral controls."""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "recovery/WORKFLOW.md").is_file())
SOURCE_ROOT = ROOT
HISTORICAL_REVISION = "7db715fbc"
TARGETS = {
    0x0044AF90: 'extern "C" int __cdecl sub_44af90(FILE *a1, int a2)',
    0x0044B000: "int __cdecl hof_save()",
    0x0044B070: "int __cdecl hof_load()",
}


def _mask_comments_and_literals(text: str) -> str:
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        text,
    )


def _actual_body(text: str, signature: str) -> str:
    code = _mask_comments_and_literals(text)
    searchable = _mask_comments_and_literals(signature)
    start = code.find(searchable)
    assert start >= 0 and code.find(searchable, start + len(searchable)) < 0, signature
    opening = code.index("{", start + len(searchable))
    assert ";" not in code[start + len(searchable):opening], signature
    depth = 0
    for end in range(opening, len(code)):
        if code[end] == "{":
            depth += 1
        elif code[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start : end + 1]
    raise AssertionError(f"unterminated actual owner body: {signature}")


def test_halloffame_extraction_rejects_hidden_and_literal_bodies():
    signature = 'extern "C" int __cdecl sub_44af90(FILE *a1, int a2)'
    body = signature + " { return 0; }"
    fake = "/* " + body + " */\nconst char *hint = \"" + body.replace('"', '\\"') + "\";\n"
    assert _actual_body(fake + body, signature) == body
    with pytest.raises(AssertionError):
        _actual_body(fake, signature)
    with pytest.raises(AssertionError):
        _actual_body(body + "\n" + body, signature)


def _metadata_comment(text: str, address: int) -> str:
    marker = text.index(f"// ORIGINAL: 0x{address:08X}")
    opening = text.rfind("/*", 0, marker)
    closing = text.index("*/", marker) + 2
    comment = text[opening:closing]
    return "\n".join(
        line for line in comment.splitlines()
        if "RECOVERY_ROLE: SCAFFOLD" not in line
    )


def test_famewin_io_migration_retains_original_history():
    historical = subprocess.run(
        ["git", "show", f"{HISTORICAL_REVISION}:src/leaf_recoveries.cpp"],
        cwd=ROOT, check=True, capture_output=True, text=True, timeout=20,
    ).stdout
    owner_path = SOURCE_ROOT / "src/famewin.cpp"
    owner = owner_path.read_text()
    before = {record.address: record for record in
              read_text(historical, "src/leaf_recoveries.cpp")
              if record.address in TARGETS}
    after = {record.address: record for record in read_text(owner, owner_path)
             if record.address in TARGETS}
    assert set(before) == set(TARGETS)
    assert set(after) == set(TARGETS)
    leaf = read_text((ROOT / "src/leaf_recoveries.cpp").read_text(), "src/leaf_recoveries.cpp")
    assert not any(record.address in TARGETS for record in leaf)
    for address in TARGETS:
        old, current = before[address], after[address]
        for field in ("name", "image_spans", "mode", "byte_exact", "semantic",
                      "levers", "ruled_out", "unrecoverable", "deferred", "recipe"):
            assert getattr(current, field) == getattr(old, field), (hex(address), field)
        assert _metadata_comment(owner, address) == _metadata_comment(historical, address)
    for signature in TARGETS.values():
        assert _actual_body(owner, signature)


def test_famewin_io_actual_source_failure_controls_on_host_compilers(tmp_path):
    compilers = [compiler for name in ("g++", "clang++")
                 if (compiler := shutil.which(name))]
    if not compilers:
        pytest.skip("host GCC and Clang unavailable")
    owner = (SOURCE_ROOT / "src/famewin.cpp").read_text()
    harness = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
#include "halloffame.h"
HallOfFameRecord HallOfFameRecords[HallOfFameRecordCount];
int HallOfFameIOMode;
std::vector<std::string> events;
bool open_read=true, open_write=true;
size_t read_result=1, write_result=1;
int closes=0;
FILE *const fake_file=reinterpret_cast<FILE *>(static_cast<uintptr_t>(1));
FILE *test_fopen(const char *name,const char *mode) {
  assert(std::strcmp(name,"hallfame.jhh")==0);
  events.push_back(std::string("open:")+mode);
  if(std::strcmp(mode,"rb")==0) return open_read ? fake_file : nullptr;
  assert(std::strcmp(mode,"wb")==0);
  return open_write ? fake_file : nullptr;
}
size_t test_fread(void *data,size_t size,size_t count,FILE *file) {
  assert(data==HallOfFameRecords && size==0x1324 && count==1);
  assert(file==fake_file && HallOfFameIOMode==0); events.push_back("read");
  if(read_result==0) { HallOfFameRecords[0].present=9; HallOfFameRecords[0].award_flags=0xA7; }
  return read_result;
}
size_t test_fwrite(const void *data,size_t size,size_t count,FILE *file) {
  assert(data==HallOfFameRecords && size==0x1324 && count==1);
  assert(file==fake_file && HallOfFameIOMode!=0); events.push_back("write");
  return write_result;
}
int test_fclose(FILE *file) { assert(file==fake_file); ++closes; events.push_back("close"); return 0; }
#define fopen test_fopen
#define fread test_fread
#define fwrite test_fwrite
#define fclose test_fclose
'''
    harness += "\n".join(_actual_body(owner, signature)
                          for signature in TARGETS.values())
    harness += r'''
void reset() {
  events.clear(); open_read=open_write=true; read_result=write_result=1;
  closes=0; HallOfFameIOMode=77;
  for(int i=0;i<HallOfFameRecordCount;++i) {
    std::memset(&HallOfFameRecords[i],0x5A,sizeof(HallOfFameRecords[i]));
    HallOfFameRecords[i].present=1;
  }
}
void cleared(unsigned char first=0x5A) {
  for(int i=0;i<HallOfFameRecordCount;++i)
    assert(HallOfFameRecords[i].present==0 &&
           HallOfFameRecords[i].award_flags==(i==0 ? first : 0x5A));
}
int main() {
  reset(); assert(sub_44af90(fake_file,7)==0 && HallOfFameIOMode==7);
  assert(events==std::vector<std::string>{"write"} && closes==0);
  reset(); write_result=0; assert(sub_44af90(fake_file,7)==0); cleared();
  reset(); read_result=0; assert(sub_44af90(fake_file,0)==0); cleared(0xA7);
  reset(); open_write=false; assert(hof_save()==1 && HallOfFameIOMode==77);
  assert(events==std::vector<std::string>{"open:wb"} && closes==0);
  reset(); assert(hof_save()==0 && HallOfFameIOMode==1 && closes==1);
  reset(); write_result=0; assert(hof_save()==0 && closes==1); cleared();
  reset(); open_read=false; assert(hof_load()==1 && HallOfFameIOMode==77);
  cleared(); assert(events==std::vector<std::string>{"open:rb"});
  reset(); assert(hof_load()==0 && HallOfFameIOMode==0 && closes==1);
  reset(); read_result=0; assert(hof_load()==0 && closes==1); cleared(0xA7);
}
'''
    source = tmp_path / "famewin_hof_actual_source.cpp"
    source.write_text(harness)
    for index, compiler in enumerate(compilers):
        executable = tmp_path / f"famewin_hof_{index}"
        result = subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
             "-iquote", str(SOURCE_ROOT / "src"), str(source), "-o", str(executable)],
            capture_output=True, text=True, timeout=30,
        )
        assert result.returncode == 0, result.stderr
        result = subprocess.run([str(executable)], capture_output=True,
                                text=True, timeout=10)
        assert result.returncode == 0, result.stderr

        # A concrete error-path mutant must be rejected by the open-failure
        # assertion, demonstrating that the control distinguishes this quirk.
        mutant_source = tmp_path / f"famewin_hof_mutant_{index}.cpp"
        mutant_source.write_text(harness.replace("return 1;", "return 0;", 1))
        mutant = tmp_path / f"famewin_hof_mutant_{index}"
        result = subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
             "-iquote", str(SOURCE_ROOT / "src"), str(mutant_source), "-o", str(mutant)],
            capture_output=True, text=True, timeout=30,
        )
        assert result.returncode == 0, result.stderr
        result = subprocess.run([str(mutant)], capture_output=True,
                                text=True, timeout=10)
        assert result.returncode != 0
