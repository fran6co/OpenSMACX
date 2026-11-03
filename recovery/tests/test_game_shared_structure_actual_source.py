"""Actual save bodies keep four independent writable function-owned strings.

These host controls check C++ storage identity and the existing map error path.
They do not establish target ABI, original allocation bounds or game acceptance.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_save_daemon_actual_source import (
    ROOT, FIXTURE, actual_storage_declarations, extract_definition,
)


PAIR_SUPPORT = r'''
static bool map_lane;
static bool observe_pair;
static const char *observed_dot;
static const char *observed_mode;
static int map_version_result;
static int map_result;
static int map_calls;
static void pair_check(bool value, const char *message) {
    if (!value) { std::fprintf(stderr, "PAIR: %s\n", message); std::exit(19); }
}
static char *pair_strcat(char *destination, const char *source) {
    if (observe_pair && source[0] == '.' && source[1] == 0) observed_dot = source;
    return std::strcat(destination, source);
}
static FILE *pair_open(const char *path, const char *mode) {
    if (observe_pair) observed_mode = mode;
    return env_open(path, mode);
}
static void pair_header(const char *header, FILE *file) {
    if (!map_lane) { header_write(header, file); return; }
    pair_check(std::strcmp(header, MapSaveHeaderText) == 0, "map header text");
    calls += "H";
}
static size_t pair_fwrite(const void *buffer, size_t size, size_t count, FILE *file) {
    if (!map_lane) return fixture_fwrite(buffer, size, count, file);
    pair_check(size == 4 && count == 1 && file == fake_file, "map version write arguments");
    int version; std::memcpy(&version, buffer, 4);
    pair_check(version == 5, "map version value"); calls += "V";
    return map_version_result;
}
static int map_data(FILE *file, int writing, int mode) {
    pair_check(file == fake_file && writing == 1 && mode == 0, "map serializer arguments");
    ++map_calls; calls += "D"; return map_result;
}
static int pair_fclose(FILE *file) {
    if (map_lane && file == 0) { ++close_calls; calls += "C"; return -1; }
    return fixture_fclose(file);
}
'''

PAIR_TEST = r'''
static void pair_reset(bool map) {
    reset_fixture(); map_lane = map; observe_pair = true;
    observed_dot = observed_mode = 0; map_version_result = 1; map_result = map_calls = 0;
}
static void test_function_owned_strings_and_map_errors() {
    pair_reset(false);
    pair_check(save_daemon(const_cast<char *>("save")) == 0, "save success");
    const char *save_dot = observed_dot, *save_mode = observed_mode;
    pair_check(save_dot && save_mode && opened_path == "save.SAV", "save string arguments");
    pair_reset(true);
    pair_check(save_map_daemon(const_cast<char *>("map")) == 0, "map success");
    const char *map_dot = observed_dot, *map_mode = observed_mode;
    pair_check(map_dot && map_mode && opened_path == "map.MP", "map string arguments");
    pair_check(save_dot != map_dot, "distinct save/map dot storage");
    pair_check(save_mode != map_mode, "distinct save/map mode storage");
    pair_check(calls == "OHVDC" && map_calls == 1 && close_calls == 1 && remove_calls == 0,
               "map success phases");
    // Both pointers refer to still-live actual mutable static product arrays.
    // Mutate one owner's bytes without ever extending its observed payload.
    const_cast<char *>(save_dot)[0] = ':'; const_cast<char *>(save_mode)[0] = 'a';
    pair_check(map_dot[0] == '.' && map_mode[0] == 'w', "save storage isolated from map");
    const_cast<char *>(save_dot)[0] = '.'; const_cast<char *>(save_mode)[0] = 'w';
    const_cast<char *>(map_dot)[0] = ':'; const_cast<char *>(map_mode)[0] = 'a';
    pair_check(save_dot[0] == '.' && save_mode[0] == 'w', "map storage isolated from save");
    const_cast<char *>(map_dot)[0] = '.'; const_cast<char *>(map_mode)[0] = 'w';
    pair_reset(false); pair_check(save_daemon(const_cast<char *>("again")) == 0, "repeat save");
    pair_check(observed_dot == save_dot && observed_mode == save_mode, "save static lifetime");
    pair_reset(true); pair_check(save_map_daemon(const_cast<char *>("again")) == 0, "repeat map");
    pair_check(observed_dot == map_dot && observed_mode == map_mode, "map static lifetime");
    pair_reset(true); open_succeeds = false;
    pair_check(save_map_daemon(const_cast<char *>("missing")) == 1, "map open failure status");
    pair_check(close_calls == 1 && calls == "OCR" && removed_path == "missing.MP",
               "map null-close preserved");
    pair_reset(true); map_version_result = 0;
    pair_check(save_map_daemon(const_cast<char *>("bad.version")) == 2, "map version failure status");
    pair_check(calls == "OHVCR" && opened_path == "bad.version" && map_calls == 0,
               "map version failure phases");
    pair_reset(true); map_result = 1;
    pair_check(save_map_daemon(const_cast<char *>("bad")) == 3, "map serializer failure status");
    pair_check(calls == "OHVDCR" && map_calls == 1 && removed_path == "bad.MP",
               "map serializer failure phases");
    map_lane = observe_pair = false;
}
'''


def actual_pair(damage=None):
    product = (ROOT / 'src/game.cpp').read_text()
    bodies = [extract_definition(product, signature, expect_semicolon=False)
              for signature in ('int __cdecl save_daemon(char *save_path)',
                                'int __cdecl save_map_daemon(char *path)')]
    global_damage = ''
    if damage in ('shared_dot', 'shared_mode'):
        name, text = ('PathDot', '.') if damage == 'shared_dot' else ('SaveFileMode', 'wb')
        anchor = f'    static char {name}[] = "{text}";\n'
        assert all(body.count(anchor) == 1 for body in bodies)
        bodies = [body.replace(anchor, '') for body in bodies]
        global_damage = f'static char {name}[] = "{text}";\n'
    elif damage == 'skip_null_close':
        assert bodies[1].count('    fclose(file);') == 1
        bodies[1] = bodies[1].replace('    fclose(file);', '    if (file != 0) fclose(file);')
    source = FIXTURE.read_text().replace('// ACTUAL_STORAGE_DECLARATIONS', actual_storage_declarations())
    source = source.replace('/* ACTUAL_GAME_DATA_STATE_HEADER */',
                            f'#include "{ROOT / "src/game_data_state_window.h"}"')
    declarations = []
    for prefix in ('char MapFileExtensionText[3] =', 'LPCSTR MapFileExtension =',
                   'static const char MapSaveHeaderText[] ='):
        rows = [line for line in product.splitlines() if line.startswith(prefix)]
        assert len(rows) == 1
        declarations += rows
    support = '\n'.join(declarations) + '\n' + PAIR_SUPPORT + global_damage
    source = source.replace('#define fwrite fixture_fwrite', support + r'''
#define strcat pair_strcat
#define env_open pair_open
#define header_write pair_header
#define fwrite pair_fwrite''')
    source = source.replace('#define fclose fixture_fclose', '#define fclose pair_fclose')
    source = source.replace('// ACTUAL_SAVE_DAEMON_BODY', '\n'.join(bodies))
    source = source.replace('#undef fwrite', '#undef strcat\n#undef env_open\n#undef header_write\n#undef fwrite')
    source = source.replace('int main() {', PAIR_TEST + '\nint main() {\n    test_function_owned_strings_and_map_errors();')
    return '#include <cstdlib>\n' + source


@pytest.mark.parametrize('damage,witness', [
    (None, None),
    ('shared_dot', 'distinct save/map dot storage'),
    ('shared_mode', 'distinct save/map mode storage'),
    ('skip_null_close', 'map null-close preserved'),
])
def test_actual_save_pair_storage_and_damage(tmp_path, damage, witness):
    compiler = shutil.which('g++')
    assert compiler, 'host g++ required for actual save-pair control'
    cpp, binary = tmp_path / 'save_pair.cpp', tmp_path / 'save_pair'
    cpp.write_text(actual_pair(damage))
    built = subprocess.run([compiler, '-std=c++98', '-Wall', '-Wextra', '-Werror',
                            str(cpp), '-o', str(binary)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
    if damage is None:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode == 19 and f'PAIR: {witness}' in ran.stderr, ran.stdout + ran.stderr
