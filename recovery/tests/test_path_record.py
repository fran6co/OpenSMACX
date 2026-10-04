"""Compile real path-record operations and reject demonstrated branch damage."""
from pathlib import Path
import shutil
import subprocess
import pytest
from recovery.tests.test_pact_ascent import ROOT, definition


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_path_record_and_damage(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    header = (ROOT / "src/stringstruct.h").read_text()
    declarations = header[header.index("struct PathRecord {"):header.index("static_assert(sizeof(PathRecord)")]
    source = (ROOT / "src/stringstruct.cpp").read_text()
    patterns = [r"PathRecord \*PathRecord::initialize\(\)",r"void PathRecord::close\(\)",
        r"void PathRecord::set_directory\([^)]*\)",r"void PathRecord::set_path\([^)]*\)",
        r"void PathRecord::set_name\([^)]*\)"]
    pieces = [definition(source,pattern) for pattern in patterns]
    fixture = Path(__file__).with_name("path_record_control.cpp").read_text()
    mutations = {
        "wrong_initial_value": (0,"unknown_18_ = 0xfa;","unknown_18_ = 0;"),
        "wrong_initial_word": (0,"unknown_00_ = 0;","unknown_00_ = 1;"),
        "close_wrong_storage": (1,"::operator delete(name_);","::operator delete(directory_);"),
        "close_clears_path": (1,"::operator delete(path_);","::operator delete(path_); path_ = 0;"),
        "close_omits_name": (1,"::operator delete(name_);","(void)name_;"),
        "wrong_directory_slash": (2,"buffer[length] = '\\\\';","buffer[length] = '/';"),
        "directory_omits_rebase": (2,"set_path(buffer);","(void)buffer;"),
        "rebase_drops_name": (2,"strcat(buffer, scan);",'strcat(buffer, "");'),
        "wrong_basename": (3,"strcat(buffer, slash + 1);","strcat(buffer, slash);"),
        "lost_directory": (3,"strcpy(buffer, directory_);","buffer[0] = 0;"),
        "missing_path_null_reset": (3,"path_ = 0;","path_ = 0; directory_ = 0;"),
        "missing_name_null_reset": (4,"name_ = 0;","(void)name_;"),
    }
    for name in ("actual", *mutations):
        altered = list(pieces)
        if name != "actual":
            index,before,after = mutations[name]
            assert altered[index].count(before)==1,name
            altered[index]=altered[index].replace(before,after,1)
        text = fixture.replace("// @@DECLARATIONS@@",declarations).replace("// @@BODIES@@","\n".join(altered))
        cpp,binary=tmp_path/(name+".cpp"),tmp_path/name
        cpp.write_text(text)
        build=subprocess.run([compiler,"-std=c++98","-fsanitize=undefined",
            "-fno-sanitize-recover=all",str(cpp),"-o",str(binary)],capture_output=True,text=True,timeout=45)
        assert build.returncode==0,name+build.stderr
        run=subprocess.run([str(binary)],capture_output=True,text=True,timeout=10)
        assert (run.returncode==0)==(name=="actual"),name+run.stderr
