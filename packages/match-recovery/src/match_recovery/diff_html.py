"""Render a self-contained, source-linked assembly comparison page.

Alignment is heuristic navigation only. The page always retains every input
instruction and does not claim that aligned rows prove matching.
"""

from __future__ import annotations

import json
import re
from difflib import SequenceMatcher
from html import escape
from typing import Any


def _instruction_key(instruction: dict[str, Any]) -> tuple[str, str]:
    return (str(instruction.get("mnemonic", "")), str(instruction.get("operands", "")))


def _alignment(original: list[dict[str, Any]], compiled: list[dict[str, Any]]) -> list[dict[str, Any]]:
    """Pair instructions for navigation; opcode alignment is not match evidence."""
    matcher = SequenceMatcher(
        a=[str(row.get("mnemonic", "")) for row in original],
        b=[str(row.get("mnemonic", "")) for row in compiled],
        autojunk=False,
    )
    rows: list[dict[str, Any]] = []
    for _tag, i1, i2, j1, j2 in matcher.get_opcodes():
        for k in range(max(i2 - i1, j2 - j1)):
            left = original[i1 + k] if i1 + k < i2 else None
            right = compiled[j1 + k] if j1 + k < j2 else None
            if left is None:
                kind = "added"
            elif right is None:
                kind = "removed"
            elif _instruction_key(left) == _instruction_key(right):
                kind = "same"
            else:
                kind = "operand"
            rows.append({"kind": kind, "original": left, "compiled": right})
    return rows


def _safe_json(value: Any) -> str:
    # Prevent any string value from terminating a script element or starting
    # markup, while keeping valid JSON for JavaScript's JSON.parse.
    return (json.dumps(value, ensure_ascii=False, separators=(",", ":"))
            .replace("&", "\\u0026").replace("<", "\\u003c")
            .replace(">", "\\u003e").replace("\u2028", "\\u2028")
            .replace("\u2029", "\\u2029"))


def render_diff(payload: dict[str, Any]) -> str:
    """Return offline HTML for source-linked original/compiled instruction regions.

    ``payload`` has ``title``, ``source_path``, ``source_lines`` (rows with
    ``number`` and ``text``), and ``regions``. Each region has ``role``,
    ``original_start``, ``original`` and ``compiled`` instruction arrays.
    Instruction rows carry ``offset``, optional ``address``, ``bytes``,
    ``mnemonic``, ``operands``, ``relocations`` and optional ``source_line``.
    Source-line anchors are supplied by the caller from compiler evidence.
    """
    title = str(payload.get("title", "Instruction diff"))
    source_path = str(payload.get("source_path", ""))
    source_lines = payload.get("source_lines", [])
    regions = payload.get("regions", [])
    if not isinstance(source_lines, list) or not isinstance(regions, list):
        raise ValueError("source_lines and regions must be lists")

    view_data = {
        "title": title,
        "source_path": source_path,
        "source_lines": source_lines,
        "regions": [
            {
                "role": str(region.get("role", "region")),
                "original_start": region.get("original_start"),
                "rows": _alignment(region.get("original", []), region.get("compiled", [])),
            }
            for region in regions
        ],
    }
    data = _safe_json(view_data)
    # Keep title visible even before JavaScript runs; escape as HTML text.
    safe_title = escape(title, quote=True)
    template = r'''<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>__TITLE__</title><style>
:root{color-scheme:dark;--bg:#10151d;--panel:#171f2a;--line:#293545;--fg:#e5edf7;--muted:#9aabc0;--accent:#77c7ff}*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--fg);font:14px/1.45 system-ui,sans-serif}header{position:sticky;top:0;z-index:5;background:#111923f2;border-bottom:1px solid var(--line);padding:12px 18px}h1{font-size:18px;margin:0 0 6px}.meta{color:var(--muted);font-size:12px;overflow-wrap:anywhere}.toolbar{display:flex;gap:8px;align-items:center;margin-top:9px;flex-wrap:wrap}button{background:#243346;color:var(--fg);border:1px solid #42546a;border-radius:6px;padding:6px 10px;cursor:pointer}button:hover,button.on{border-color:var(--accent);color:var(--accent)}#status{color:var(--muted);margin-left:auto}main{padding:14px;display:grid;grid-template-columns:minmax(300px,35%) minmax(0,65%);gap:14px;height:calc(100vh - 130px)}section{background:var(--panel);border:1px solid var(--line);border-radius:9px;min-width:0;overflow:hidden;display:flex;flex-direction:column}.pane-title{padding:9px 12px;border-bottom:1px solid var(--line);font-weight:650}.scroll{overflow:auto;min-height:0;flex:1}.notice{color:#ffd27b;font-size:12px;padding:8px 12px;border-bottom:1px solid var(--line)}.source{margin:0;padding:8px 0;font:12px/1.6 ui-monospace,SFMono-Regular,Consolas,monospace;white-space:pre}.source-line{display:block;padding:0 10px;cursor:pointer}.source-line:hover,.source-line.sel{background:#253a4e}.ln{display:inline-block;width:50px;color:#71859c;user-select:none}.region-label{padding:8px 10px;background:#202b39;color:var(--accent);font-size:12px;font-weight:700;border-block:1px solid var(--line)}.asm-head,.diff-row{display:grid;grid-template-columns:1fr 1fr}.asm-head{background:#131a23;position:sticky;top:0;z-index:2;border-bottom:1px solid var(--line)}.asm-head div{padding:8px 10px;font-weight:650}.asm-head div+div,.cell+.cell{border-left:1px solid #263342}.diff-row{border-bottom:1px solid #202c39;min-height:31px}.cell{min-width:0;padding:4px 8px;font:11px/1.55 ui-monospace,SFMono-Regular,Consolas,monospace;white-space:pre-wrap;overflow-wrap:anywhere;cursor:pointer}.cell.empty{background:#121820;cursor:default}.addr{color:#a5b5c9}.bytes{color:#72859b}.ins{color:#e0e8f1}.reloc{color:#9dcfff;display:block}.diff-row.same{opacity:.65}.diff-row.operand{background:#332b1b}.diff-row.added .right{background:#163222}.diff-row.removed .left{background:#351d24}.diff-row.sel{outline:1px solid var(--accent);outline-offset:-1px}.diff-row.filtered{display:none}@media(max-width:900px){main{grid-template-columns:1fr;height:auto}.scroll{max-height:55vh}}
</style></head><body><header><h1 id="heading">__TITLE__</h1><div id="source-path" class="meta"></div><div class="toolbar"><button id="prev">← Previous difference</button><button id="next">Next difference →</button><button id="changes">Changes only</button><button id="cold">Show cold regions</button><span id="status"></span></div></header>
<main><section><div class="pane-title">Source</div><div class="notice">Source highlights follow supplied compiler listing anchors. Alignment is heuristic navigation only; all instructions remain available.</div><div id="source-pane" class="scroll"><pre id="source" class="source"></pre></div></section><section><div class="pane-title">Instruction comparison <span class="meta">• original address / compiled address or section offset</span></div><div class="asm-head"><div>Original</div><div>Compiled</div></div><div id="assembly" class="scroll"></div></section></main>
<script>
const D=__DATA__;
const $=id=>document.getElementById(id);
const hex=n=>{if(n===null||n===undefined)return "—";let v=Number(n);return Number.isFinite(v)?"0x"+v.toString(16).toUpperCase():String(n)};
const text=(tag,value,cls)=>{let n=document.createElement(tag);if(cls)n.className=cls;n.textContent=value===null||value===undefined?"":String(value);return n};
$("heading").textContent=D.title;$("source-path").textContent=D.source_path;
for(const line of D.source_lines){let node=document.createElement("span");node.className="source-line";node.dataset.line=String(line.number);node.append(text("span",line.number,"ln"));node.append(document.createTextNode(String(line.text)));node.addEventListener("click",()=>selectLine(line.number));$("source").append(node)}
function cell(instr,side){let box=document.createElement("div");box.className="cell "+side;if(!instr){box.classList.add("empty");return box}box.dataset.line=instr.source_line===null||instr.source_line===undefined?"":String(instr.source_line);let coord=side==="left"?(instr.address??instr.offset):instr.offset;box.append(text("span",hex(coord),"addr"),document.createTextNode(" "),text("span",instr.bytes,"bytes"),document.createTextNode(" "),text("span",[instr.mnemonic,instr.operands].filter(Boolean).join(" "),"ins"));for(const relocation of instr.relocations||[])box.append(text("span","↳ relocation → "+relocation,"reloc"));return box}
const regionNodes=[];let sequence=0;
for(const region of D.regions){let cold=/cold|eh|handler|exception/i.test(region.role);let group=document.createElement("div");group.className="region";group.dataset.cold=cold?"1":"0";if(cold)group.hidden=true;group.append(text("div",region.role+(region.original_start===null||region.original_start===undefined?"":" • original span starts "+hex(region.original_start)),"region-label"));for(const item of region.rows){let row=document.createElement("div");row.className="diff-row "+item.kind;row.dataset.kind=item.kind;row.dataset.sequence=String(sequence++);row.append(cell(item.original,"left"),cell(item.compiled,"right"));row.addEventListener("click",()=>selectRow(row));group.append(row)}$("assembly").append(group);regionNodes.push(group)}
function clearSelection(){document.querySelectorAll(".diff-row.sel,.source-line.sel").forEach(n=>n.classList.remove("sel"))}
function selectLine(number){clearSelection();let s=document.querySelector('.source-line[data-line="'+CSS.escape(String(number))+'"]');if(s){s.classList.add("sel");s.scrollIntoView({block:"nearest"})}let found=false;for(const row of document.querySelectorAll(".diff-row")){if(row.closest(".region")?.hidden)continue;let c=row.querySelector(".right");if(c&&c.dataset.line===String(number)){row.classList.add("sel");row.scrollIntoView({block:"nearest"});found=true}}$("status").textContent=found?"Compiled instructions linked to source line "+number:"No instruction anchor for source line "+number}
function selectRow(row){clearSelection();row.classList.add("sel");let line=row.querySelector(".right")?.dataset.line;if(line){let s=document.querySelector('.source-line[data-line="'+CSS.escape(line)+'"]');if(s){s.classList.add("sel");s.scrollIntoView({block:"nearest"})}}row.scrollIntoView({block:"nearest"});$("status").textContent="Instruction difference selected"}
function differences(){return [...document.querySelectorAll(".diff-row")].filter(r=>r.dataset.kind!=="same"&&!r.classList.contains("filtered")&&!r.closest(".region").hidden)}
function move(dir){let rows=differences();if(!rows.length)return;let at=rows.findIndex(r=>r.classList.contains("sel"));selectRow(rows[(at+dir+rows.length)%rows.length])}
$("prev").onclick=()=>move(-1);$("next").onclick=()=>move(1);$("changes").onclick=e=>{let on=e.currentTarget.classList.toggle("on");document.querySelectorAll(".diff-row").forEach(r=>r.classList.toggle("filtered",on&&r.dataset.kind==="same"));e.currentTarget.textContent=on?"Show all instructions":"Changes only"};$("cold").onclick=e=>{let shown=false;for(const group of regionNodes)if(group.dataset.cold==="1"){group.hidden=!group.hidden;shown=shown||!group.hidden}e.currentTarget.classList.toggle("on",shown);e.currentTarget.textContent=shown?"Hide cold regions":"Show cold regions"};document.addEventListener("keydown",e=>{if(e.key.toLowerCase()==="n")move(1);if(e.key.toLowerCase()==="p")move(-1)});
let total=0,diff=0;for(const r of D.regions)for(const row of r.rows){total++;if(row.kind!=="same")diff++}$("status").textContent=diff+" aligned difference rows / "+total+" instruction rows • navigation aid, not matching proof";
</script></body></html>'''
    replacements = {"__TITLE__": safe_title, "__DATA__": data}
    return re.sub(r"__TITLE__|__DATA__", lambda match: replacements[match.group(0)], template)

