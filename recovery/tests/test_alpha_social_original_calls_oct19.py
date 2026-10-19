"""Actual-source host controls and pinned call witnesses for read_rules SOCIAL."""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import textwrap

import pytest


ROOT = Path(__file__).resolve().parents[2]
ALPHA = ROOT / "src/alpha.cpp"
FACTION = ROOT / "src/faction.h"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def without_comments(source: str) -> str:
    import re
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.S)


def block_from(source: str, start: int) -> str:
    opening = source.index("{", start)
    depth = 0
    for at in range(opening, len(source)):
        depth += (source[at] == "{") - (source[at] == "}")
        if depth == 0:
            return source[start:at + 1]
    raise AssertionError("unterminated source block")


def read_rules_body() -> str:
    clean = without_comments(ALPHA.read_text())
    signature = "BOOL __cdecl read_rules(BOOL tgl_all_rules)"
    return block_from(clean, clean.index(signature))


def social_copy_loops() -> tuple[str, str]:
    body = read_rules_body()
    anchor = body.index('if (text_open(AlphaxFileID, "SOCIO"))')
    first_start = body.index("for (i = 0; i < MaxSocialEffectNum; i++)", anchor)
    first = block_from(body, first_start)
    second_start = body.index("for (i = 0; i < MaxSocialEffectNum; i++)", first_start + len(first))
    return first, block_from(body, second_start)


def social_struct() -> str:
    source = without_comments(FACTION.read_text())
    return block_from(source, source.index("struct RulesSocialEffect")) + ";"


def integer_constant(name: str) -> int:
    import re
    source = without_comments(FACTION.read_text())
    match = re.search(rf"static const int {name}\s*=\s*(\d+)\s*;", source)
    assert match, name
    return int(match.group(1))


def test_read_rules_social_copies_are_plain_and_ordered() -> None:
    set1, set2 = social_copy_loops()
    assert "strcpy(SocialEffects[i].set1, text_item());" in set1
    assert "strcpy_s" not in set1
    assert "strcpy(SocialEffects[i].set2, text_item());" in set2
    assert "strcpy_s" not in set2
    body = read_rules_body()
    soc = body.index('if (text_open(AlphaxFileID, "SOCIO"))')
    first_get = body.index("text_get();", soc)
    first_loop = body.index("for (i = 0; i < MaxSocialEffectNum; i++)", first_get)
    second_get = body.index("text_get();", first_loop + len(set1))
    second_loop = body.index("for (i = 0; i < MaxSocialEffectNum; i++)", second_get)
    next_get = body.index("text_get();", second_loop + len(set2))
    assert first_get < first_loop < second_get < second_loop < next_get


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_social_copy_loops_keep_fields_order_and_row_extent(
    tmp_path: Path, compiler_name: str,
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    count = integer_constant("MaxSocialEffectNum")
    source = textwrap.dedent(r"""
        #include <cassert>
        #include <cstddef>
        #include <cstring>
        #include <string>
        typedef char *LPSTR;
        static const int MaxSocialEffectNum = EFFECT_COUNT;
        STRUCT_RULES_SOCIAL_EFFECT
        static RulesSocialEffect SocialEffects[MaxSocialEffectNum];
        static const char *tokens[MaxSocialEffectNum * 2];
        static int get_calls = 0, item_calls = 0, cursor = 0, i = 0;
        static void check(bool value) { assert(value); }
        static void text_get() { ++get_calls; }
        static LPSTR text_item() {
            ++item_calls;
            check(cursor < MaxSocialEffectNum * 2);
            return const_cast<char *>(tokens[cursor++]);
        }
        static void load_names() {
          text_get();
          FIRST_LOOP
          text_get();
          SECOND_LOOP
        }
        int main() {
          static_assert(sizeof(RulesSocialEffect) == 0x68, "row stride");
          static_assert(offsetof(RulesSocialEffect, set2) == 0x18, "set2 offset");
          for (int row = 0; row < MaxSocialEffectNum; ++row) {
            static std::string first[MaxSocialEffectNum];
            static std::string second[MaxSocialEffectNum];
            first[row] = "axis-one-" + std::to_string(row);
            second[row] = "axis-two-" + std::to_string(row);
            tokens[row] = first[row].c_str();
            tokens[MaxSocialEffectNum + row] = second[row].c_str();
            std::memset(SocialEffects[row].padding, 0xA5, sizeof(SocialEffects[row].padding));
          }
          load_names();
          check(get_calls == 2 && item_calls == MaxSocialEffectNum * 2);
          check(cursor == MaxSocialEffectNum * 2);
          for (int row = 0; row < MaxSocialEffectNum; ++row) {
            const std::string first = "axis-one-" + std::to_string(row);
            const std::string second = "axis-two-" + std::to_string(row);
            check(std::strcmp(SocialEffects[row].set1, first.c_str()) == 0);
            check(std::strcmp(SocialEffects[row].set2, second.c_str()) == 0);
            for (size_t byte = 0; byte < sizeof(SocialEffects[row].padding); ++byte)
              check(static_cast<unsigned char>(SocialEffects[row].padding[byte]) == 0xA5);
          }
          return 0;
        }
    """)
    first, second = social_copy_loops()
    source = (source.replace("EFFECT_COUNT", str(count))
                    .replace("STRUCT_RULES_SOCIAL_EFFECT", social_struct())
                    .replace("FIRST_LOOP", first)
                    .replace("SECOND_LOOP", second))
    cpp = tmp_path / "social_copy.cpp"
    executable = tmp_path / "social_copy"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stderr


def test_pinned_social_copy_calls_use_original_destinations_and_stride() -> None:
    pytest.importorskip("capstone")
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    image = PeImage(image_path.read_bytes(), PIN)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x00587E6C, 0x00587EB4)), 0x00587E6C
    ))
    by_address = {instruction.address: (instruction.mnemonic, instruction.op_str)
                  for instruction in instructions}
    assert by_address[0x00587E71] == ("mov", "esi, 0x946580")
    assert by_address[0x00587E76] == ("call", "0x5fd670")
    assert by_address[0x00587E7B] == ("push", "eax")
    assert by_address[0x00587E7C] == ("push", "esi")
    assert by_address[0x00587E7D] == ("call", "0x645460")
    assert by_address[0x00587E82] == ("add", "esi, 0x68")
    assert by_address[0x00587E88] == ("cmp", "esi, 0x9469f8")
    assert by_address[0x00587E95] == ("mov", "esi, 0x946598")
    assert by_address[0x00587E9A] == ("call", "0x5fd670")
    assert by_address[0x00587E9F] == ("push", "eax")
    assert by_address[0x00587EA0] == ("push", "esi")
    assert by_address[0x00587EA1] == ("call", "0x645460")
    assert by_address[0x00587EA6] == ("add", "esi, 0x68")
    assert by_address[0x00587EAC] == ("cmp", "esi, 0x946a10")
