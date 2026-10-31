import json
import re

from match_recovery.diff_html import render_diff


def payload():
    return {
        "title": "Function comparison",
        "source_path": "src/example.cpp",
        "source_lines": [
            {"number": 10, "text": "void f() {"},
            {"number": 11, "text": "  return;"},
        ],
        "regions": [
            {
                "role": "primary",
                "original_start": 0x401000,
                "original": [
                    {"address": 0x401000, "offset": 0, "bytes": "90", "mnemonic": "nop",
                     "operands": "", "relocations": [], "source_line": None},
                    {"address": 0x401001, "offset": 1, "bytes": "c3", "mnemonic": "ret",
                     "operands": "", "relocations": [], "source_line": None},
                ],
                "compiled": [
                    {"address": None, "offset": 0, "bytes": "90", "mnemonic": "nop",
                     "operands": "", "relocations": [], "source_line": 10},
                    {"address": None, "offset": 2, "bytes": "c2 00 00", "mnemonic": "ret",
                     "operands": "0", "relocations": ["external_target"], "source_line": 11},
                    {"address": None, "offset": 5, "bytes": "cc", "mnemonic": "int3",
                     "operands": "", "relocations": [], "source_line": None},
                ],
            },
            {
                "role": "cold EH cleanup",
                "original_start": 0x402000,
                "original": [],
                "compiled": [
                    {"address": None, "offset": 0, "bytes": "e9 00 00 00 00", "mnemonic": "jmp",
                     "operands": "0", "relocations": ["handler"], "source_line": None},
                ],
            },
        ],
    }


def embedded_data(page):
    match = re.search(r"const D=(\{.*?\});\nconst \$=", page, re.S)
    assert match
    return json.loads(match.group(1))


def test_render_diff_retains_all_rows_and_aligns_operand_and_extra_instructions():
    page = render_diff(payload())
    data = embedded_data(page)
    primary = data["regions"][0]
    assert [row["kind"] for row in primary["rows"]] == ["same", "operand", "added"]
    assert [row["compiled"]["offset"] for row in primary["rows"] if row["compiled"]] == [0, 2, 5]
    assert data["regions"][1]["rows"][0]["kind"] == "added"
    assert "navigation aid, not matching proof" in page
    assert "Previous difference" in page and "Changes only" in page
    assert "cold regions" in page
    assert '"C++:"+instr.source_line' in page
    assert "let activeRow=null" in page and "activeRow=null;let s=" in page
    assert "no compiler source-line mapping" in page


def test_render_diff_escapes_untrusted_markup_and_needs_no_network():
    item = payload()
    item["title"] = '<img src=x onerror="alert(1)">'
    item["source_path"] = 'x</script><script>alert("path")</script>'
    item["source_lines"][0]["text"] = '<script>alert("source")</script>'
    item["regions"][0]["compiled"][1]["relocations"] = ['</script><img src=x onerror="alert(2)">']

    page = render_diff(item)
    data = embedded_data(page)
    assert "<img" not in page and "</script><script>" not in page
    assert "\\u003cscript\\u003e" in page
    assert data["title"] == item["title"]
    assert data["source_lines"][0]["text"] == item["source_lines"][0]["text"]
    assert data["regions"][0]["rows"][1]["compiled"]["relocations"] == item["regions"][0]["compiled"][1]["relocations"]
    assert not re.search(r'(?:src|href)=["\']https?://', page, re.I)

