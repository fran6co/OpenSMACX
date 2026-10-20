"""Actual-source controls for Caviar's disjoint four-callback prefix."""

from pathlib import Path
import shutil
import subprocess
import tempfile


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/caviar.cpp").is_file())
HEADER = ROOT / "src/caviar.h"
SOURCE = ROOT / "src/caviar.cpp"


def extract(source, signature):
    begin = source.find(signature)
    assert begin >= 0, "missing actual source: " + signature
    opening = source.find("{", begin)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[begin:index + 1]
    raise AssertionError("unbalanced source: " + signature)


MAIN = r'''static int first_read(int, char *, long) { return 19; }
static int replacement_read(int, char *, long) { return 73; }
static int replacement_calls;
static int full_replacement_read(int, char *buffer, long count) {
    ++replacement_calls;
    for (long index = 0; index < count; ++index) buffer[index] = 'b';
    return (int)count;
}
static int replace_during_read(int, char *buffer, long count) {
    CaviarOutputMessageWindow.io_read_ = &full_replacement_read;
    if (count > 0) buffer[0] = 'a';
    return count > 1 ? 1 : (int)count;
}
static void *prefix_alloc(unsigned long n) { return reinterpret_cast<void *>(n); }
static void prefix_free(void *) {}
static int prefix_write(int, void *, long n) { return (int)n + 2; }
int main() {
    typedef char offsets_are_real[
        offsetof(CaviarOutputMessageWindowCandidate, io_allocate_) == 0 &&
        offsetof(CaviarOutputMessageWindowCandidate, io_release_) == sizeof(void *) &&
        offsetof(CaviarOutputMessageWindowCandidate, io_read_) == 2 * sizeof(void *) &&
        offsetof(CaviarOutputMessageWindowCandidate, io_write_) == 3 * sizeof(void *) &&
        offsetof(CaviarOutputMessageWindowCandidate, projection_words_) ==
            offsetof(VoxIoCallbacks, seek) ? 1 : -1];
    (void)sizeof(offsets_are_real);
    CaviarOutputMessageWindow.io_allocate_ = &prefix_alloc;
    CaviarOutputMessageWindow.io_release_ = &prefix_free;
    CaviarOutputMessageWindow.io_read_ = &first_read;
    CaviarOutputMessageWindow.io_write_ = &prefix_write;
    uint32_t seek_word = 0x12345678u;
    uint32_t tell_word = 0x87654321u;
    memcpy(reinterpret_cast<unsigned char *>(&CaviarOutputMessageWindow) +
               offsetof(VoxIoCallbacks, seek), &seek_word, sizeof(seek_word));
    memcpy(reinterpret_cast<unsigned char *>(&CaviarOutputMessageWindow) +
               offsetof(VoxIoCallbacks, tell), &tell_word, sizeof(tell_word));
    if (vox_alloc_slot(9) != reinterpret_cast<void *>(9) ||
        vox_read_slot(1, 0, 2) != 19 || vox_write_slot(1, 0, 3) != 5) return 1;
    CaviarOutputMessageWindow.io_read_ = &replacement_read;
    if (vox_read_slot(1, 0, 2) != 73) return 2;
    CaviarOutputMessageWindow.projection_words_[0] = 0xaabbccddu;
    if (vox_read_slot(1, 0, 2) != 73 || vox_write_slot(1, 0, 3) != 5) return 3;
    uint32_t seek_after = 0, tell_after = 0;
    memcpy(&seek_after, reinterpret_cast<unsigned char *>(
               &CaviarOutputMessageWindow) + offsetof(VoxIoCallbacks, seek),
           sizeof(seek_after));
    memcpy(&tell_after, reinterpret_cast<unsigned char *>(
               &CaviarOutputMessageWindow) + offsetof(VoxIoCallbacks, tell),
           sizeof(tell_after));
    if (seek_after != 0xaabbccddu || tell_after != tell_word) return 4;
    char buffer[3] = {};
    replacement_calls = 0;
    CaviarOutputMessageWindow.io_read_ = &replace_during_read;
    if (vox_read_exact(1, buffer, 3) != 0 || replacement_calls != 1 ||
        buffer[0] != 'a' || buffer[1] != 'b' || buffer[2] != 'b') return 5;
    return 0;
}
'''


def compile_and_run(mutant=None):
    header = HEADER.read_text()
    source = SOURCE.read_text()
    callbacks = extract(header, "struct VoxIoCallbacks {") + ";"
    storage = extract(header, "struct CaviarOutputMessageWindowCandidate {") + ";"
    helper = "template <typename Callback>\n" + extract(
        source, "static Callback vox_load_callback(")
    slots = "\n".join(
        line for line in source.splitlines()
        if line.startswith(("typedef long (__cdecl *VoxSeekCallback)",
                            "typedef long (__cdecl *VoxTellCallback)",
                            "#define vox_alloc_slot ", "#define vox_free_slot ",
                            "#define vox_read_slot ", "#define vox_write_slot ",
                            "#define vox_seek_slot ", "#define vox_tell_slot ")))
    read_exact = extract(source,
        "extern \"C\" int __cdecl vox_read_exact(int fd, void *buffer, int count) {")
    if mutant == "wrong_read_member":
        slots = slots.replace("CaviarOutputMessageWindow.io_read_",
                              "CaviarOutputMessageWindow.io_write_")
    elif mutant == "cached_read_callback":
        anchor = "    if (count > 0) {\n        do {"
        assert anchor in read_exact
        read_exact = read_exact.replace(
            anchor,
            "    if (count > 0) {\n"
            "        int (__cdecl *cached_read)(int, char *, long) = vox_read_slot;\n"
            "        do {", 1)
        read_exact = read_exact.replace("int read = vox_read_slot(",
                                        "int read = cached_read(", 1)
    fixture = r'''#include <stddef.h>
#include <stdint.h>
#include <string.h>
#ifndef __cdecl
#define __cdecl
#endif
'''
    stage_stub = ('static const char vox_msg_reading_file[] = "read";\n'
                  'static void vox_stage_error(const char *) {}\n')
    cpp_text = (fixture + callbacks + "\n" + storage +
                "\nCaviarOutputMessageWindowCandidate CaviarOutputMessageWindow = {};\n" +
                slots + "\n" + helper + "\n" + stage_stub + read_exact +
                "\n" + MAIN)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    available = [compiler for compiler in compilers if compiler]
    assert available, "GCC or Clang required for actual-source prefix controls"
    with tempfile.TemporaryDirectory(prefix="caviar-typed-prefix-") as directory:
        cpp = Path(directory) / "typed_prefix.cpp"
        cpp.write_text(cpp_text)
        for compiler in available:
            executable = Path(directory) / Path(compiler).name.replace("+", "p")
            built = subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
                 str(cpp), "-o", str(executable)],
                text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            assert built.returncode == 0, built.stdout
            run = subprocess.run([str(executable)], check=False)
            if mutant is None:
                assert run.returncode == 0
            else:
                assert run.returncode != 0, "mutation survived: " + mutant


def test_actual_typed_prefix_live_replacement_and_projection_overlap():
    compile_and_run()


def test_wrong_typed_member_mutant_is_rejected():
    compile_and_run("wrong_read_member")


def test_cached_callback_mutant_is_rejected():
    compile_and_run("cached_read_callback")
