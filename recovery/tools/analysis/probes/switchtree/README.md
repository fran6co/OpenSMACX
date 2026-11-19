# Sparse switch layout (MATCHING_GUIDE 3.7, Console::on_key_click 0x005178C0)

Target: the final switch of on_key_click (0x51B081..0x51B66F): a three-node tree
(N1 `cmp 0x61` + table 0x21..0x28, N2 `cmp 0x10025` + table 0x62..0x69, N3 byte-indexed
table 0x10026..0x10068). Image layout, landmarks as in `okc_layout.py`:

    img  MD5 N2 ST N3 P26 J P28 P25 P27 MD1 MDE MDR RET
    ours ST N2 ST MD5 MD1 MDE MDR N3 P26 P27 P25 P27 J RET

The image runs the 0x61 block straight into the head of the direction body (a2 store,
mode/0x10000 tests, a2 == 5, arrow_left, return) and leaves the rest of that body
(arrow_right, the cursor/order paths, move_cursor) after the pan cases; pan 0x10026
hosts the shared set_center tail and the post-switch join head follows it.

Files
- `run.py [vN.py] [--show KEY]`: synthetic copy of the switch (marker calls, 0.3 s);
  `v1.py`..`v6.py` are the variant families below. Its base reproduces our layout.
- `okc_layout.py [SRC]`: landmark order of the real function, ours against the image.
- `okc_deck.py`, `okc_groups.py`, `okc_struct.py`: in-unit decks (case order of the pans,
  textual order of the 0x61+body / stubs / pans groups, control-structure variants).

Measured (all at game flags)
- Case bodies are emitted behind the last tree node that references them, in source
  order within that node: the 0x61 body after N1, the seven direction stubs (in both
  tables) after N2, the pans (0x10025 is N2's `je` but also in N3's table) after N3.
  Moving whole case groups in the text changes nothing (okc_groups: all six orders give
  one layout; v1 HPS/SPH/PSH).
- A block reached only by `goto`/fall-through (the direction body) is laid out after
  its last emitted jumper: after stub 7 when the stubs `goto` it, after the 0x61 block
  when 0x61 is its only predecessor (v6 `a1dep`, `one_stub`). It stays contiguous either
  way; no variant splits it as the image does.
- Neutral for the layout: case order inside a group (except which pan hosts the shared
  tail), `default: break`, `(unsigned)key`, the body after the switch (before or after the
  join), `break` into the body after the switch with pans `goto` the join (okc_struct
  md_join, SHbP_join), the 0x61 case as a `goto` stub with the body at the end of the
  switch (SHgP_md, HgSP_md, MD_SHgP).
- A nested switch or an `if (key >= 0x10000)` split with the pans first (v3
  `nested_pans_first`, `split_if`) gives the image's pan/join tail (`20 s 30 31 21 22 23`,
  first pan hosts, join head behind it) but a different tree (`cmp 0x10000` / two tables).
- Written-out direction bodies per case (okc_dup) constant-fold a2 and never merge.

Open: the image's split body needs the 0x61 block as the head's only predecessor at
layout time with the stubs joining later (a cross-jump), and the body's remaining blocks
at the end of the switch. No faithful spelling found.
