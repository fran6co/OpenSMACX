# Compiler evidence, not workflow policy

Relevant observations extracted from the retired agent brief. They are historical
starting points, not current measurements or universal rules. The original brief
and its complete experiments remain available with
`git show b7f45b19:docs/recovery/AGENT_BRIEF.md`. Current ownership and verification
policy is in `WORKFLOW.md`; no spelling trick overrides that policy.

## Structural questions first

- Contiguous executable addresses are not sufficient to recover an original
  object module. A control with the configured VC6 compiler gives two ordinary
  C-linkage functions returning the same string separate code COMDATs under
  `/O2 /GX`, both with explicit `/Ob1` and `/Ob2`, without an explicit `/Gy`.
  Both references share one string instance. `/Od /GX` instead gives one ordinary
  code section and two string instances. These are observations of the current
  compiler, not recovered original switches or proof of historical TU boundaries.
  Combine address neighborhoods with literal storage, callers, generated
  providers and lifecycle evidence; retain discontiguous provider obligations.
- Equal strings at distinct original addresses establish distinct linked
  storage, not named source arrays or separate original TUs by themselves.
  With the observed optimized pooling behavior, finer TU boundaries are a
  concrete alternative to named arrays or different pooling options. The
  606-identity interface/assets correspondence therefore remains inferred.
- Reuniting `Sound::set_type` with its Voice consumers makes `/Ob2` inline the
  calls in the VoiceRx and VoiceTx constructors. Moving its definition after
  those consumers did not restore the original calls. Definition visibility
  across this compiling TU matters; source order alone does not isolate this
  body from VC6's inliner. Preserve the constructor claims and investigate original TU
  visibility/compiler policy rather than forcing a local noinline workaround.
- Bounded unchanged-source Sound controls do not support an `/Ob1` fix:
  replacing only `/Ob2` with `/Ob1` leaves both Voice constructor misses and
  loses masked matches for `release_sound` and `Sound::Sound`. Removing the
  explicit `Sound::` qualification from the two Voice `set_type` calls also
  leaves their inlining and verdicts unchanged. These are negative controls,
  not changes to production compiler policy.
- A paired visibility control exposes the real dependency more narrowly.
  Making the complete `SoundFadeState` and `Sound` constructor bodies and
  `Sound::set_fade` visible inline, with the `set_type` definition removed from
  this diagnostic overlay's compiling unit, restores both Voice constructor bodies
  and their EH regions **only when the existing function-call lowering of
  `memset` is preserved**. Moving definitions ahead of that intrinsic context
  does not restore them. The strict reference-aware comparator passes both
  constructor body rows in the corrected overlay; aggregate qualification
  remains unknown, including generated-table linker selection. A production
  provider and linked target for `set_type` remain unverified. The overlay
  also stops emitting the standalone base constructors at `0x004C6080` and
  `0x004C8410`; their complete mappings and historical obligations must remain.
  No independent source use requiring those standalone emissions is currently
  established. This is a constraint on inline visibility and provider placement,
  not proof of a historical object boundary or an integrated recovery. Do not
  manufacture constructor uses, export attributes or noinline controls to hide
  the remaining emission question. The diagnostic overlays are not source claims.

- Catalogue return decorations need provenance too. The stripped executable's
  `Buffer::box_sprite` entry leaves incidental EAX on guard and no-mask paths;
  no result use was observed across 380 mapped direct call sites. A void model
  emits the same code as the former int form without a missing-return defect.
  Win's slot-86 default at 0x005F6A40 is only `ret 12`, and its catalogued int
  decoration likewise does not establish a result contract. Preserve original
  identities and claims, verify emitted symbols, slots and callers, and retain
  nominal-type/indirect-call uncertainty. Neither instruction equality nor
  an analysis database's guessed prototype recovers an original declaration.

- Imported disassembler names and `library` flags are not dependency or class
  identities. Tiny bodies can collide with library signatures: the IDA9 export
  calls `0x004042B0` an `Iostream_init` constructor, while the original table
  slots and adjustor at `0x00404420`, preserved in `src/graphicwin.cpp`, identify
  the real `GraphicWin::get_graphic_win` self-query. The same export's
  `Iostream_init` labels at `0x00616D70` and `0x00634BA0` therefore do not prove
  runtime ownership. Its Concurrency template labels at `0x00447D10` and
  `0x0050EA30` likewise have no retained signature/database provenance and
  conflict with the pinned PE's December 1999 timestamp and linker version 6.0.
  Microsoft documents ConcRT/PPL as part of the platform introduced with
  [Visual Studio 2010](https://learn.microsoft.com/en-us/archive/msdn-magazine/2010/april/visual-c-exploring-new-c-and-mfc-features-in-visual-studio-2010).
  Treat these labels as unverified hypotheses, not exclusions; actual callers,
  table cells, receiver relationships and dependency provenance must establish
  ownership. Missing static references do not establish dead code.
- Virtual-base flags are ABI inputs, not necessarily source parameters. Earlier
  catalogue names and the brief contradicted themselves about constructors and
  destructors. Determine explicit arguments from callers, base construction,
  vbtable accesses and emitted symbols. Do not globally translate constructor
  labels into `construct(int)` methods or infer virtual inheritance from a name.
- The old RadioButton experiments found compiler-emitted vtordisp thunks after
  genuine virtual bases and overrides were declared. Hand-written argument
  forwarding produced a call/frame where the image had a tail jump. Inspect
  generated COMDATs and map the emitted code; do not reproduce a compiler thunk
  with arithmetic over `this`.
- Adding virtual declarations affects callers and table slots. Qualified calls
  can intentionally suppress virtual dispatch, but only the original call and
  target evidence establish whether that is appropriate. Reconstructed Q/U
  catalogue decorations are hints, not a complete proof of virtualness.
- Placement construction of already-declared members duplicates lifecycle work.
  The SocialWin and MultiDebug experiments improved after real member
  construction replaced it. When initializers regress, investigate actual global
  ownership and callers together; do not restore raw storage automatically.
- An exception-frame mismatch can reflect missing bases, embedded objects,
  cleanup funclets, or the wrong selected symbol. The old suggestion to replace
  constructors with methods merely to remove SEH is not a recovery rule.
- Empty inline constructors hide initialization defects; making them real can
  expose additional failures. Track the dependent construction graph rather than
  treating the newly visible fault as evidence against the constructor.
- Old Palette recovery demonstrated that byte-exact bodies can retain wrong
  names, types, raw offsets and incorrect ownership. Names come from behavior and
  callers; a mangled rename must be reconciled with the actual emitted symbol.

## Constructor tables and intrinsic call boundaries

Catalogue Q/U decorations are interpretations, not a virtualness verdict.
AlphaSave's primary table at 0x00669EC8 selects its own show/hide bodies in
slots 1/2, even though the imported names use Q. Renaming those methods into
nonvirtual helpers preserved their local bytes while leaving the inherited
Win implementations in the table. Constructor-selected table words and actual
callers must settle overrides together with their emitted-symbol bindings.
CouncWin's primary slot 2 at 0x0066AAF0 selects 0x00427150, a jump to
Win::is_visible. Preserve that query-only hide override; its surprising effect
is not permission to replace it with Win::hide or move it outside the interface.

Net's ten source zeroing sites in seven methods call the verified CRT memset
at 0x006465F0 in the pin. Intrinsic stores erase those call boundaries and can
also hide required global reloads. The owner TU uses a narrow function(memset)
policy; this does not justify disabling all intrinsics across the application.
An uninitialized SDK output slot must also remain uninitialized until its
callee writes it when that is the original path. These are source/compiler
meaning observations, not matching claims or proof of complete original TUs.

## Source-form experiments after structure

OpenSMACX currently selects two release comparison flag sets in its local
compiler configuration: `/c /O2 /Ob2 /Gy /GR- /GX` and the same with `/Oy-`.
The explicit `/Ob2` aligns unpinned standalone measurements with the measured
CMake Release producer. A real VC6 control retains a normal helper call with
`/O2` alone and expands it with `/Ob2`; identical `/O2` spellings therefore do
not establish matching inlining policy across modern CMake and VC6. This is
current-producer calibration, not proof of the original command line. Owner
exceptions still require fresh actual recipes and full preservation checks.
There is no `--all-flags` option. Flag-policy changes are coordinator decisions,
not a per-function search for a permissive checker.

- VC6 is pre-modern C++. Use compiler-supported syntax and evidence-backed
  widths/signedness; `int` and unsigned integer parameters mangle differently.
  Do not change a signed interface into an unsigned one merely for instruction
  selection, or apply an all-parameters-must-be-int rule to real pointer types.
- Separate guards versus merged conditions, guard order, branch polarity and
  peeled loops can change epilogues and register allocation. The old
  `prototype_factor`/Buffer measurements differed in direction: measure locally.
- The receiver-spill pattern observed in Palette survived several guard/local
  experiments. Treat previous failed attempts as a reason to seek new context,
  not to repeat the same spellings or declare the function impossible.
- Match a local's width to its underlying field and observe when the image
  widens it. Signed division rounds toward zero; a right shift is not equivalent
  on all negative inputs. Preserve semantics before considering encoding.
- `volatile` aliases can prevent memory-operand folding; genuine volatile stores
  can preserve writes the image performs. Neither adding nor removing volatile
  is a safe tree-wide transformation.
- Observe repeated calls and reads. Hoisting an expression can remove calls
  from a macro expansion or change aliasing behavior. A call-count difference is
  a semantic question before it is a similarity question.
- A function bound through a pointer can change a direct call into an indirect
  one. Recover a real callee/global binding, not an original-address forwarder.
  The old relocation-masked score could not prove that the referenced target was
  correct; strict reference checks are required.
- Never trust a layout/sizeof observation from a failed compile. Likewise,
  a source-integrity match, a byte match and a successful link prove different
  properties. None independently proves a functioning reconstructed application.

## Compiler identity before more spelling searches

The `CL.EXE` banner is not the complete VC6 compiler identity. The release and
SP3 toolchains can share the 12.00.8168 driver while their C1/C1XX/C2 DLLs differ.
The inspected SP3 frontends report 12.0.8472.0 and its backend 12.0.8447.0;
the installed release DLLs report 12.0.8168.0. Verify the actual files, not a
banner or an assumed service-pack name. The independent
[HoMM3 reconstruction's toolchain pins](https://github.com/sushi-shi/homm3-decomp/blob/decomp-complete-4.0/scripts/homm3/vc6/_toolchain.py)
and [original-media extraction recipe](https://github.com/sushi-shi/homm3-decomp/blob/decomp-complete-4.0/scripts/create-toolchain-release.py)
provide useful provenance leads; qualify downloaded files locally and preserve
their licenses. Do not run an installer over the selected compiler.

The pinned game's Rich records include both 8168 and 8447 compiler-build tags.
Those aggregate records do not assign a class, function, current source file,
historical TU or Jackal/application partition to either compiler. They also do
not prove SP3 exclusivity. A compiler configuration change requires evidence
for its affected ownership/build domain, not a per-function search for a more
permissive result.

For an isolated comparison, retain the source, SDK, linker, flags and baselines;
stage immutable private compiler files and use the existing compiler interaction
and resource lease. A fresh `Compiler` fingerprints the selected DLLs and SDK.
Use physical unchanged-replica and target-damage controls in each flag trial,
and inspect the emitted COFF `@comp.id` to distinguish the tested backend tags.
Do not change DLLs during a live measurement epoch. Source overlays currently
refuse nonempty `VC6_INCLUDE`; a private root can instead resolve its default
INCLUDE/LIB directories to the unchanged installed SDK.

The bounded SocialWin/TutWin/Palette comparison changed the emitted compiler tag
but not the measured function output. Do not repeat the same compiler/source
cohort as another spelling attempt without new evidence. This does not establish
equivalence for other TUs, justify switching the production compiler, or waive
strict references, original dependency identity, linked retention or runtime proof.
Keep executable-derived reports and compiler artifacts ignored; the selected
production toolchain remains unchanged.

A separate control at `d3f8c3332` compared Font, Datalink and Base under their
unchanged actual application recipes: 213 mapped records, eighteen compiles
across the installed and retained SP3 frontend/backend sets. Each TU/profile
passed whole-object unchanged-overlay and target-body damage controls; the
installed profile also reproduced each retained production object. No mapped
verdict category changed. The SP3 objects did **not** pass whole-object equality
against production; the saved summaries retain those failures but not their
detailed section diagnostics. Unchanged verdicts therefore do not establish
unchanged function bytes. This cohort supplies no measured exact-count gain or
authority to switch compilers, and is not an exclusion for other source/recipe
cohorts. Both experiments retain the installed linker, headers and libraries;
neither establishes a complete historical SP3 build environment.

Later retained evidence covers additional domains. The Oct25 physical SP3
application build at `b4dcea86699cf23d6ce8afe91f81634b49f318be` contains
193 application objects, including Caviar and NetFifo, with compiler product
11/build8447 witnesses. Its classification delta lists no Caviar or NetFifo
verdict change. These are older source inputs, not current-source qualification
or an assignment of those original TUs to SP3. Do not describe either owner as
never compiled with the retained SP3 backend.

The `cpu-caviar-cohort-oct26ck` experiment at
`f4e244b6e2978129685651d4a8943a6600a3387b` tested all116 Caviar mappings with
the default processor policy and `/G5`, `/G6`, and `/GB`, including
`vox_cross_product` and `vox_normalise_vector`. Replica and target-damage
controls passed. `/G5` and `/GB` supplied no exact gains or losses; `/G6`
supplied no gains and lost11 exact matches in that source context. Cross product
remained22 instructions against25, and normalization35 against37. A later
unchanged-source `/G6` diagnostic retained those two count gaps. Extra x87
`FXCH` instructions therefore do not establish an untested CPU-target fix.
Compare the target body, dependencies, recipe and compiler identity with these
retained trials before requeueing; a changed aggregate TU hash alone is not
evidence of a new hypothesis. None of these diagnostics changes production
compiler policy or grants a byte claim.

## Shared frames and private helper boundaries

Normalize addresses against the owning caller's frame before translating a
register-only helper into a typed interface. An x86 `CALL` pushes a four-byte
return address: with no callee stack adjustment, callee `ESP+4` is caller `ESP`,
not caller scratch word one. The movie producer at `0x005D0AAB` uses this shared
frame; its six destinations are caller DWORD offsets 0, 8, 128, 136, 256 and
320. Trace both the original stores and their consumer loads before classifying
a source scratch read as original uninitialized behavior. The existing local DC
coefficient product/store is a separate operation and needs no new initialization.

A private-linkage compiler experiment proves a callable ABI only if the emitted
object retains that call boundary. In the bounded VC6 transform control, the
private helpers disappeared into their callers while the external helpers kept
ordinary calls. This is inconclusive about the original movie helpers' register
contracts. Neither register-only entry patterns nor a current analysis-generated
symbol establish original C++ parameter types, linkage or compiler provenance.

## Operational lessons retained

Global initialization/atexit dependencies matter alongside entry-point reachability.
Recovered seeds must be reconciled with product definitions and their callers;
removing a duplicate body does not ensure callers no longer use a redirect.

Git stash is shared across worktrees. Piped patch application can die before
writing, and `git apply --check --3way` is not proof of a conflict-free applied
index. Inspect actual side effects. Isolate compiler state; do not reuse a build
database naming another worktree or kill a shared Wine server.

For runtime investigations, use the same working directory, game assets and
display/Wine conditions for original and candidate controls. A crash caused by
launch conditions is not evidence of a decompilation defect. Runtime acceptance
is not yet wired into the supplied recovery driver.
