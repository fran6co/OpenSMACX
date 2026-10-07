# The source map: src/ is the single record of the decompilation

The decompilation is tracked in exactly one place: the source tree. Every
piece of the binary the project maps carries an annotation in a `.cpp` file
naming the bytes it claims, and one command asks the original compiler what
state each piece is in.

New annotations carry original entry identity, a name interpretation and explicit
image spans. The older catalogue also carries prototypes, callers, kind/flags and
call-site facts imported from IDA. Those are seed evidence, not fresh measurements
or a requirement to persist every reproducible observation. C++ declarations own
the current source structure; explicit binding rationale preserves non-derivable
evidence beside the original mapping.

Reader/writer: `match_recovery.source`, installed with `match-recovery`.
Portable tests: `uv run pytest packages/match-recovery/tests`; whole-source and
build controls: `uv run pytest recovery/tests`. `recover check` owns the configured
whole-project gate; `recover measure` owns diagnostics and
`recover record` owns production claim stamping. The active task and
structural-migration workflow is `recovery/WORKFLOW.md`, not this format reference.

## Source-role rationale

An optional source-owned role rationale is written on its own comment line
immediately before an `ORIGINAL` marker:

```cpp
// RECOVERY_ROLE: SCAFFOLD compiler adjustor only; delegates to the real owner
// ORIGINAL: 0x00401234 ?adjust@@YAXXZ 0x00401234-0x00401238
// RECOVERY_BINDING: v1 {"reason":"...","evidence":["..."]}
```

`RECOVERY_ROLE` is a durable classification decision, not a measurement or a
recovery waiver. The initial accepted role is `SCAFFOLD`; its nonblank reason
must be one real text line, with no control/line-separator characters,
embedded `ORIGINAL:` / `RECOVERY_ROLE:` tokens, or C block-comment delimiters.
That restriction prevents the rationale from manufacturing or terminating
neighboring annotation syntax. The optional binding rationale remains attached
immediately after `ORIGINAL`, so both annotations retain independent,
unambiguous attachment rules.

No role annotation means `unclassified`, not `application implementation`.
Role does not replace original identity, mode/state, claims, spans, source body
region, or historical obligations. Inventory may report whether a source body
region was extracted and its existing derived state by role, but that is not a
recovered/implemented count: region text can include sentinels or comments, and
source state is not a behavioral proof. Role does not remove a mapping from
identity checks, measurements, or preservation.

The source reader ignores annotation-looking lines inside multiline C++ raw
string literals. Ordinary quoted strings and prose remain non-directives under
the existing source grammar. This section adds no CLI or project-specific
filename rules.

## Original-function binding rationale

The native `recover bind` command adds a new, unverified function identity to an
existing source TU using explicit original spans, source freshness and evidence.
It uses the package marker writer; it does not stamp matching, infer boundaries,
write a body, rehome existing product bindings or authorize recovery scope.
See the package README for its arguments and publication limits.

An immediately adjacent `// RECOVERY_BINDING: v1 {"reason":"...","evidence":["..."]}`
comment records the non-derivable basis for that mapping. The adjoining `ORIGINAL`
marker supplies its identity and spans, so those are not copied into the rationale.
Both fields are required, with nonempty single-line text and evidence references.
Malformed/orphaned records fail source integrity. Native inventory and task context
carry this rationale to agents. Run results, timings, spelling-search attempts and
current match status do not belong here. Existing historical LEVER/TRIED comments
remain legacy seed evidence, not the destination for new run logs.

Delegated seed binding separates these inputs explicitly: `binding.reason` and
`binding.evidence` supply the source rationale, while top-level `evidence`
explains the coordinator decision. Request hashes, impact counts, permission
checks and compiler-run diagnostics belong to the latter, not the source comment.
Neither evidence channel substitutes for the other.

## Compact object bindings

`RECOVERY_OBJECT: v1 <start> <emitted-symbol>` binds an original object start
beside the owning source definition. `RECOVERY_OBJECT: v2 <start> <end>
<emitted-symbol> <evidence-address>` additionally requires the independently
bounded initial storage; end is exclusive. All addresses use `0x` notation.
The configured image pin/address space supplies the remaining original identity.
Source bounds and the evidence anchor are load-bearing inputs, not a current
measurement. The installed `recover check-data` computes compiler extents, initial
bytes and relocation-target agreement; missing bounds stay unknown. The active
workflow explains history/review preservation and lifetime limitations.

Version 3 binds an old object identity to a named embedded member of a current
parent object. The unbounded and bounded forms are:

```cpp
// RECOVERY_OBJECT: v3 <start> <old-emitted-symbol> member <parent-symbol> <member.path>
// RECOVERY_OBJECT: v3 <start> <end> <old-emitted-symbol> <evidence-address> member <parent-symbol> <member.path>
```

The old symbol, original start, optional exclusive end and evidence anchor retain
their historical identity and obligations. `parent-symbol` names the current
compiled COFF data definition; the dotted path names direct embedded CodeView
members. It does not follow pointers or inherited bases. Each fresh owner object
must provide one unambiguous standalone CodeView definition and every path step's
extent must fit its containing UDT. Several old members may share one parent only
in the same source unit and only when fresh layout plus original addresses imply
one consistent parent base. A historical object symbol cannot also name a current
member owner; that collision would redirect old relocations. Unsupported, missing
or ambiguous layout remains unknown.

For a bounded v3 record, initial bytes are compared with exactly the resolved
member slice. A relocation wholly inside that slice is rebased against the parent
base and keeps its COFF addend; a relocation crossing the slice boundary or using
an unsupported form does not borrow adjacent-member bytes and remains unknown.
Object-reference scheduling recognizes both historical child IDs and current
parent symbols, but compilation remains read-only and does not expand edit scope.
The member path is structural source evidence, not a byte claim, lifetime proof,
final-link selection proof or game acceptance.

`match_recovery.object_writer.write_object_bytes` is the pure, add-only writer
for these object bindings. Supply the complete current `parse_objects` snapshot
and reviewed `ObjectBinding` additions; their one-based lines anchor insertion
before current source lines (or one past the last line). It preserves original
source bytes, checks owner/image identity and conflicts, and reparses the result.
It refuses unsupported lexical forms and insertion inside block comments.
It performs no file IO or publication and does not infer symbols, allocation
bounds, initializers, acceptance or recovery permission. Compiler evidence and
coordinator-authorized publication remain separate requirements.

`match_recovery.object_writer.replace_object_records` is the pure rehome
transformation for a reviewed v3 migration. Supply the complete object-annotation
snapshot and bytes for every affected source unit; one-for-one replacements retain
old symbols, original addresses, spans and evidence anchors while changing only
the parent/path and insertion unit/line. It reparses the result and refuses
stale/incomplete snapshots, duplicate identities, parent-owner aliases and
insertion inside a block comment. The caller still owns fresh on-disk validation,
review and guarded publication; the transformation itself writes no files.

## TU-private literal bindings

`// RECOVERY_LITERAL: v1 <start> <exclusive-end> <emitted-symbol> <evidence-address>`
beside a literal use declares its original bounded storage. The owning translation
unit is the source file containing the annotation; identical compiler symbols in
other TUs do not share that identity. Explain the independent correspondence in
adjacent source prose. The evidence address anchors review/restoration obligations
to an original function; it is not a current measurement or the address of a pointer
variable. Keep ordinary inline C++ literals, not measurement-only global variables.

Native verification compares fresh, separately bounded emitted storage and then
uses the address only for references in its owning TU. Current i386 support requires
an independent literal COMDAT with its own complete extent; ordinary/static or
ambiguous storage remains unknown. Embedded-NUL and wide data use the full section,
not a first-terminator guess. Bindings do not prove linker selection or lifetime.
Removing, moving or rebinding a declaration retains its historical obligations;
variant decks cannot change it. No instruction operand under test or repeated-content
search may establish this source correspondence.

## Shared original function bodies

`// RECOVERY_SHARED: v1 <start> <end> <emitted-symbol> <evidence-address>`
beside an additional method definition states that it shares an already mapped
original function's complete contiguous span. End is exclusive; addresses use
`0x` notation. Keep the canonical `ORIGINAL` identity and its historical claims.
Do not duplicate that marker for each method or infer a binding from equal bytes
alone. Explain the original call/table evidence in adjacent ordinary comments;
the evidence address anchors the interpretation to its recovery ownership task.
An old catalogue class name does not establish the shared code's class owner.

Native task checks and variant decks compile declared shared owners when referenced
or required by a scoped evidence anchor. Each emitted method gets an independent
strict extent/bytes/reference result, in every configured trial. Only freshly
passing methods supply reference targets to callers, tables and bounded data.
Unfinished placeholders, missing definitions and unproved reference cycles stay
unknown. Shared bodies do not increment the canonical claim population, and
passing bytes do not establish semantic names, prototypes or complete hierarchy.
Removal, rehoming or reinterpretation retains task-history review requirements.
Variant decks may change bodies but cannot change these binding decisions.

## The grammar

One marker, valid in any comment form (`//` or inside `/* */`); the whole
map is `grep -rn "ORIGINAL: 0x" src/`. The marker NAMES the piece before
saying how to read it: the mangled name and the image spans ride the
marker line, ahead of the keywords — an annotation that cannot say what it
names and where the piece ends in the image cannot be added. (Moved to
this spelling 2026-08-18; the `// name` and `// spans` fact lines came
out of the tree with it.)

**`name` is what the IMAGE calls the piece, and it is not always what this
tree emits for it.** A body that re-expresses a compiler-generated construct
— an adjustor thunk, an atexit destructor, a scalar deleting destructor —
carries a symbol of its own by design, and a catalogued name can simply be
wrong about a return type (`?map_init@@YAXXZ` says void over a body
`src/map.cpp` defines as `BOOL __cdecl map_init()`). Both are recorded, as
two facts rather than one corrected into the other:

```
// ORIGINAL: 0x0041B4C0 ?on_scrolling@BaseWin@@QAEXHH@Z 0x0041B4C0-0x0041B4D7
// symbol    @base_win_on_scrolling_redirect@16
```

A `// symbol` line names the symbol THIS TREE'S compiler emits, measured
from the object rather than guessed, and its value is a single token — the
key alone cannot be trusted, because `src/` carries 2,037 prose lines that
begin `// symbol ` and every one of them is a sentence. 1,543 were written
on 2026-08-19 for the records the build compiles. Every reader goes through
`match_recovery.source`, which reads this fact, and an annotation without one is simply a
piece whose emitted name is its `name`.

For a compiler-observed symbol change, `match_recovery.source.write_symbol`
prepares an explicit fact update from original bytes and a fresh reader record.
It preserves the historical marker, claims, lessons and unrelated bytes; ordinary
`write`/`write_file` still deliberately leave fact blocks untouched. Publish the
result through `source.publication.publish` with current-input validation and
task authorization. This operation neither measures nor grants a claim. Duplicate
or shadowed facts and stale snapshots refuse; facts never belong to a neighboring
`ORIGINAL` marker. The bounded writer also refuses raw-string or line-spliced
source rather than guessing its lexical context. Recompile after publication:
adding a physical annotation line can affect uses of `__LINE__`.

`match_recovery.source.write_kind` provides the corresponding guarded update
for an existing catalogue `// kind` fact. It takes original bytes, a current
reader record and one annotation-safe token; it preserves identity, spans,
claims, lessons, bindings and neighboring facts. Duplicate or malformed facts
and stale snapshots refuse. This preserves historical application/dependency
classification during migration; it does not infer ownership, verify behavior,
authorize a migration or grant matching. Ordinary `write` still leaves existing
fact blocks untouched.

An adjacent `// body` fact names the source file containing a definition hosted
elsewhere, for example `// body src/stringstruct.h` for a header-defined method.
Keep the canonical original marker in its owning TU, not a duplicate per emitted
copy. The fact is a load-bearing location/ownership interpretation, not a claim
that the body compiles or matches. Native preservation additionally requires
compiler-observed body-damage witnesses before attributing copies in other TUs;
matching symbols alone are insufficient. Export and other independent metadata
remain separate verification requirements.

```
ORIGINAL: 0x00401640 ?init@Game@@QAEHH@Z 0x00401640-0x00401B00
                                  the function definition that follows is
                                  the decompiled body; it is extracted and
                                  measured in generated VC6 scaffolding
ORIGINAL: 0x00403B70 ?f@Win@@QAEXXZ 0x00403B70-0x00403F00 FILE
                                  the WHOLE file is the translation unit
                                  for this address; compiled verbatim
ORIGINAL: 0x0064A123 ?crt_init@@YAXXZ 0x0064A123-0x0064A130 EXCLUDED S1
                                  deliberately not decompiled; the token
                                  cites the ground in docs/EXCLUSIONS.md
ORIGINAL: 0x0046FB10 ?g@@YAXXZ 0x0046FB10-0x0046FB40 BYTE_EXACT
                                  RATCHET CLAIM: this was proved to
                                  recompile to the shipped bytes, and
                                  `--check` fails if it stops
```

`BYTE_EXACT` composes with `FILE` in either order, and it is the ONLY verdict
ever written into the tree. It is a claim, not a status field: every other
tier — MISMATCH, NO_COMPILE, MNEMONIC_ONLY — is measured on demand and
recorded nowhere, because a status field in a comment goes stale the moment
someone edits the body and is believed anyway. The installed `recover record`
command adds a token only for a body it measures; `--record-matches` belongs to
a retired command. Existing tokens preserve restoration obligations even when
structural work temporarily stops reproducing them.

The lookbehind matters: `PROVEN-AGAINST-ORIGINAL: 0x...` provenance
comments end in this spelling and are not map entries.

## Original layout interpretations

Place evidence-backed layout expectations beside their owning C++ declarations:

```cpp
// RECOVERY_MEMBER: v1 Window flags_ 0x4 0x00401000
// RECOVERY_MEMBER: v2 Window timer_ Timer 0x8 0x00401000
// RECOVERY_MEMBER: v3 Window canvas_ Canvas* 0x10 0x00401000
// RECOVERY_MEMBER: v4 Window panes_ Rect[2] 0x18 0x20 0x00401000
// RECOVERY_BASE: v1 Window BaseWindow 0x0 0x00401000
```

These are illustrative identities. The final address anchors original evidence;
do not create expectations from current compiler observations alone. Member v1
requires only an offset, v2 also requires an embedded class value, and v3 requires
an ordinary pointer to the named class. In the CodeView 5 backend, v3 checks a
32-bit near pointer and its unqualified class target, not qualifiers, ownership,
lifetime or complete ABI. References, integers, arrays and other class targets
cannot satisfy it merely by occupying the same bytes. Unsupported type encodings
remain unknown. `recover tasks check-layout` reports offset and type results
separately; partial layout success does not establish full structure.

Member v4 requires a one-dimensional array of the named class. Its additional
fields are the positive decimal element count in brackets and the total byte
extent after the offset. The example declares two elements occupying 0x20 bytes
at offset 0x18. Element type, count, byte extent and offset are checked separately;
the original indexed accesses/bounds must establish these inputs independently
of the candidate compiler. Missing complete element storage remains unknown.
This does not prove qualifiers, initialization, lifetime or bounds-safe behavior.

Changing/removing an interpretation preserves its historical review obligation.
Old versions remain readable for that history, not as alternative ways to waive
an established type requirement. No layout annotation banks a byte claim.

## System import bindings

`// RECOVERY_IMPORT: v1 <emitted-pointer-symbol> <DLL> name:<export>` binds a
source SDK import to an explicit dependency identity. Use `ordinal:<decimal>`
instead of `name:<export>` for an ordinal import. The pinned image supplies the
slot address; do not duplicate it in annotations or hardcode it into C++.
This is separate from an `ORIGINAL` function marker: an import slot is not a
recovered application body. Place the annotation near the relevant declaration
or use, outside another function's marker facts. The package README describes
the strict resolver and its remaining link/ABI limitations.

## Historical lesson syntax (read-only seed context)

The reader preserves existing LEVER/TRIED notes losslessly. The convention below
describes those historical comments, not instructions to add new run logs to
source. Current discoveries and decisions use task history; reproducible scores,
listings and attempts are recomputed or cached. Essential accepted structural
facts belong in source. See `recovery/WORKFLOW.md` for that boundary.

Two tokens, read only from the comment run immediately after a marker, so the
same words in prose further down a file are not a claim:

```
ORIGINAL: 0x005E3650 BYTE_EXACT
LEVER: jl/jge  flipped the `if` condition and swapped the arms
```
```
ORIGINAL: 0x0055B760
TRIED: plain immediate; named local; const; x|0xFF;
           register storage; a 1-bit bitfield
```

**They carry different durability, and that is the design.** A `LEVER` sits on a
body that MATCHED, so its divergence is gone and the fingerprint is historical —
it must be written down or the lesson cannot be filed against anything. A
`TRIED` sits on a body that has NOT matched, so its divergence is still live
and the key is **measured rather than written**: a key that is never written can
never be stale. An indented comment line with no token of its own continues the
line above.

**Both are self-refuting, which is what earns them a place under "state is
measured, not claimed".** A `LEVER` is already covered by the ratchet: the body
must keep reproducing. A `TRIED` fails the moment its body *does* match, so
a wall cannot quietly outlive its reason — it must be promoted to a `LEVER` with
its fingerprint, or deleted. There is deliberately **no `WALL:` token**: a wall
is `EXCLUDED S<n>` with its ground in docs/EXCLUSIONS.md,
or it is a long `TRIED` list, and nothing in between. One marker carrying
both "I stopped" and "nobody should try" is the trap that got a `static_assert`
migration reverted.

No automated check enforces historical LEVER/TRIED placement. A stale historical
note is not a refusal to recover the body or a current measurement.

`// EXTERN-SYMBOL LEVER:` is a different, older convention that appears inside
recovered bodies; the anchored grammar above does not read it.

**Body state is derived from source text, not recovery acceptance.** The only
declared state is EXCLUDED, because exclusion is a decision. `recover inventory`
reports source and seed body states separately: `placeholder` identifies absent
or explicitly unfinished bodies; `implemented` means a body is present, not
that its behavior or C++ structure is correct. Historical byte obligations are
separate and must be remeasured. Neither body presence nor byte matching alone
establishes completed recovery.

Prose stays as ordinary comments around the marker; the parser keys only on
the marker line. Layout is free: one function per file or many per file,
anywhere under `src/` — the scanner is decoupled from organisation by
construction.

## Working copies

`src/unrecovered/` holds working-copy units materialised by the retired
scaffold route; they are verification artifacts, not product source, and are
not build inputs. They are seeds for ownership-scoped recovery, not a mandatory
source form. Reconcile mappings and callers when establishing a product home;
do not copy a scaffold merely because the old workflow did so.
`uv run recover inspect <addr> --in <seed-file>` reads an old copy when it is
the best starting point. Ambiguous selections list the exact source/seed hosts.

## Native diagnostics and recording

```
uv run recover inventory              # source and seed populations
uv run recover inspect <addr|name> --image <image> --callers
uv run recover measure <addrs...> --image <image> # read-only current spellings
uv run recover tasks variants <task> --image <image> --deck <deck.json>
uv run recover record <addrs...> --image <image> # measure AND stamp annotations
uv run recover prepare                # generate fresh compiler recipes
```

Historical byte measurement goes through `match_recovery.source.asm`: the pinned image on one side, the
record's unit compiled by VC6 on the other, several flag sets compared, and
the best answer kept. Task variant decks score authorized private source overlays
against a baseline and damage control. Without VC6, Wine, or the pinned image the commands
print the reason instead of guessing. This comparer masks relocations and does
not prove target identity or C++ structure; the strict framework checks remain
distinct from historical masked measurement. Claims retain restoration obligations during
structural work. Never hand-edit them or automatically revert a justified
structural change merely to avoid a temporary regression.

**THE RATCHET IS `recover check`.** It reads the claims in `src/`, recompiles
every one against the pinned image, links the tree, and fails by address and
file if a claim stops reproducing. Source-history identity coverage protects
removed obligations; no persisted claim count remains. Only `recover record` writes the token, it stamps
only what it measured, and nothing ever removes one but a deliberate source
edit visible in the diff. The recorder no longer offers `--demote`, weaker
claim grants or withdrawal. Existing SEMANTIC annotations remain historical
instruction-shape obligations and are still checked; they do not establish
completed recovery.

Recording refuses missing or unverifiable historical obligations before compiling.
`recover check-claims` compares original identities and requirement strength with
the configured immutable source-history baseline. A rename or rehome does not
lose identity; replacing one claim with another at the same count fails. The
baseline revision is a history-scope decision, not another claim ledger. New
source tokens still require fresh measurement by the binary gate. Recording
preserves historical notes and source bytes;
its publication guards and interruption limits are described in `recovery/README.md`.

## Drift

There is no second catalogue left to drift against: the annotations ARE the
map, so drift is checked by re-measurement rather than by comparison with an
export. `recover inventory` separates product-source candidates from historical
seeds without claiming either is buildable. `recover check-mappings`
checks full relative source identities, build hosts and claims contradicted by
configured forwarder macros. Alternative seeds are reported separately; they are
not production owners or fresh proofs. An explicit product binding can retain
same-span seeds when it carries all their historical claim obligations too.
The mapping checker reports that owner without deleting or promoting seeds;
matching checks still remeasure claims. Conflicting spans, multiple product
owners or claims carried only by a seed require reconciliation.

## Migration — done 2026-08-09

The whole tree was migrated in one guarded pass (a flag of the retired
status tool): 1,607 files rewritten, every rewrite proven comment-only (code
content identical) and address-set-preserving before anything was written;
the full VC6 re-measurement preserved every tier; the build rebuilt clean.
The legacy spellings (`Original Offset:` blocks, the inline trailing and
opening-brace forms, the two `src/recovered/` header styles) are still
RECOGNISED read-only and flagged `deprecated`, and the package's tests pin
the convergence invariant: no deprecated spelling may remain in `src/`, and
the writer emits the marker on every new file.
