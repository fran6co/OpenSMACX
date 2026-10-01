"""Actual-source controls for the native project-index movie wrapper."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def test_project_movie_selects_original_text_row_and_preserves_failure(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/alphamovie.cpp").read_text()
    start = source.index("void __cdecl amovie_project(int project_id)")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            body = source[start:end + 1]
            break
    else:
        raise AssertionError("unterminated wrapper")
    harness = r'''
#include <cassert>
#include <cstring>
#define __cdecl
int open_result, reads, items, movies;
char selected[]="project.wve";
char *item_result=selected;
int text_open(const char *file,const char *section) {
    assert(!std::strcmp(file,"MOVLIST") && !std::strcmp(section,"PROJECTS"));
    return open_result;
}
char *text_get() { ++reads; return nullptr; }
char *text_item() { ++items; return item_result; }
void amovie_project(char *movie) { assert(movie==item_result); ++movies; }
'''
    harness += body
    harness += r'''
void reset() { open_result=reads=items=movies=0; item_result=selected; }
int main() {
    reset(); amovie_project(3); assert(reads==4 && items==1 && movies==1);
    reset(); amovie_project(0); assert(reads==1 && items==1 && movies==1);
    reset(); amovie_project(-2); assert(reads==1 && items==1 && movies==1);
    reset(); open_result=1; amovie_project(3);
    assert(reads==0 && items==0 && movies==0);
    reset(); item_result=nullptr; amovie_project(2);
    assert(reads==3 && items==1 && movies==1);
}
'''
    cpp = tmp_path / "project_movie.cpp"
    executable = tmp_path / "project_movie"
    cpp.write_text(harness)
    result = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra",
                             "-Werror", str(cpp), "-o", str(executable)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr
