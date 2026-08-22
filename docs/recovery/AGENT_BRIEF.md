Working rules and the levers this tree has already measured. Read before starting.

IF YOU ARE IN A GIT WORKTREE (your own copy of the repo, not
/home/fran6co/code/OpenSMACX), run this FIRST, before anything else:

    eval "$(bash /home/fran6co/code/OpenSMACX/tools/agent_setup.sh /home/fran6co/code/OpenSMACX)"
    uv run tools/osmx.py configure          # builds this worktree's own build/

`agent_setup.sh` exists because two agents in a row lost time to what this
paragraph used to merely ASSERT. It fast-forwards your worktree to the root's
HEAD - one worktree was branched 911 commits back and had none of the tools
named below - and it exports `OPENSMACX_IMAGE`, because `.opensmacx/` is
gitignored and the pinned image is therefore NOT in your worktree. `configure`
then writes this worktree's OWN `build/compile_commands.json`, which every
measurement reads; do not point that at the root's copy, because it names the
root's paths and nothing in your worktree will match them.

RUNNING THIS SCRIPT IS EXPLICITLY PERMITTED even if your instructions say git
commands are the coordinator's. It uses git internally, and only ever
`merge --ff-only` - it cannot rewrite history, cannot discard work, and refuses
outright rather than fast-forwarding over anything uncommitted. An agent that
declined to run it lost a whole dispatch to a worktree 937 commits behind
`master`, reporting the blocker instead of clearing it. Clear it.

If `agent_setup.sh` refuses, read what it says: it will not fast-forward over
uncommitted work, and the fix is to hand that diff back first.

Working in a worktree is why you can edit freely: nothing you touch collides
with another agent.

THE `int` IN `??0Class@@QAE@H@Z` IS NOT AN ARGUMENT
- It is MSVC's compiler-injected MOST-DERIVED FLAG, and its presence means the
  class has VIRTUAL BASES.
- BUT YOU CANNOT REACH IT WITH A REAL CONSTRUCTOR, and an earlier version of
  this section said you could. Measured with `nm` and `objdump` on VC6
  12.00.8168, 2026-08-21:
    * a constructor with NO explicit parameter, on a class with a genuine
      virtual base, compiles to `??0Class@@QAE@XZ` - no `H` at all. The hidden
      flag is a separate stack parameter and is NEVER part of the mangled name.
    * a constructor written to take `int a1` does mangle to `@H@Z`, but reads
      a1 from [ebp+8] and the hidden flag from [ebp+0xc], and a call site
      written `->Class::Class(1)` pushes `1` TWICE. The image's own `??__E`
      initialisers push it exactly ONCE.
  So the `construct(int a1)` METHOD spelling is the right one for these: a
  plain method never receives the hidden flag, and its call site pushes once.
  Where the tree already has `construct(int)`, KEEP IT.
- The virtual inheritance itself is still real and still worth declaring - it
  is what gives the LAYOUT, the vbtable-relative vtable stores and the
  vtordisp. It is only the constructor spelling that cannot follow.
- DESTRUCTORS are the opposite case and ARE safe as real C++ destructors: they
  never carry the flag. For this family `this` is the address of the embedded
  GraphicWin virtual base rather than the class's own front, which
  guarded_teardowns.cpp's already-byte-exact calls prove.
- The body tells you the same thing three times over. `ListBox::ListBox`
  (0x00609DB0) reads the parameter, compares it to zero, and SKIPS its base
  construction when it is zero - that is the flag's whole meaning, "construct
  the virtual bases only if I am the most-derived object". Then every vtable
  store goes through the vbtable rather than to a fixed offset:
  `mov eax, [esi]` / `mov ecx, [eax + 4]` / `mov [ecx + esi], <vtable>`. And
  `lea ecx, [eax - 0x48]; mov [eax + esi - 4], ecx` writes a vtordisp. None of
  those three shapes can occur without virtual inheritance.
- Destructors carry it too: `??1RadioButton@@QAE@H@Z` at 0x00406F60 takes the
  same flag, for the same reason in reverse.
- `src/mapwin.h` already declares `class MapWin : protected virtual GraphicWin`
  and is the worked example. But `ListBox`, `CheckBox`, `Dialog` and
  `SpriteBox` are all declared with NO BASE CLASS AT ALL, which is why their
  constructors cannot currently reproduce anything. Fixing the declaration is
  the recovery; the body is usually short once the inheritance is right.
- `uv run tools/most_derived_flag.py` finds them, and does NOT trust the name
  to decide: a most-derived flag and a real `int` parameter mangle identically,
  so it confirms each one against the BODY - the vbtable-relative store
  `mov [reg + <this>], <vtable>` that only virtual inheritance produces. That
  check demoted 7 of 17 candidates on its first run; `StringStruct` and `Text`
  really do take an int, and adding virtual inheritance to them would have
  broken working code. Nine are confirmed: CheckBox, Console, Dialogs,
  EditGroup, ListBox, PlanWin, RadioButton (both ctor and dtor) and SpriteBox.

DO NOT BUILD A MEMBER THE COMPILER ALREADY BUILT
- `uv run tools/double_construction.py`. In a REAL CONSTRUCTOR the compiler
  has already built every declared member before the body runs, so a
  `new (&member_) T()` or `new (self + 0xNNN) T()` in the body builds it a
  second time on top of itself. The program survives it - the second
  construction overwrites a freshly built object with an identical one - which
  is exactly why nobody notices.
- It is not free. `SocialWin::SocialWin` was doing it to eleven members and
  compiled to 232 instructions against an image of 121; removing the
  redundant construction dropped it to 133.
- In a `construct()` METHOD the same code is correct and necessary - the
  object exists but its members were never implicitly built. The check knows
  the difference; do not "fix" a construct() method on its say-so.

PLACEMENT NEW COSTS A NULL GUARD, EVERY TIME
- VC6 guards every placement new-expression with a null test on the pointer -
  `cmp ecx, ebx; je` - because `operator new` may return null. The guard needs
  a spill slot, which is why a body doing `new (&member_) T()` emits
  `sub esp, 8` where the image emits `push ecx`.
- That single defect wore a disguise for a long time, refused across four
  bodies as "a stack-frame-size difference from spill-slot count, not control
  flow". It is control flow, and the frame size is downstream of it.
- The fix is an ordinary member of a REAL constructor: built implicitly, in
  declaration order, with no guard. `MultiDebug` went 13/28 to BYTE_EXACT 28/28
  on exactly this, and `Menu` went 10/38 to 15/38.
- WEIGH IT AT THE CALL SITES BEFORE CONVERTING. If the object is built at a
  fixed address by a dynamic initialiser, that initialiser can then only reach
  a real constructor through placement new ITSELF, which moves the guard into
  it. `NetMsg` gains a tier that way (11/31 -> SHAPE_EXACT 27/31) and costs two
  BYTE_EXACT initialiser claims (0x0047A7A0 went to 1 of 9). Measured, and
  reverted; check `init_thunks.cpp` for your class before converting.
- And it interacts with the construct()-method rule below: use a method where
  the image does NOT construct its bases, and a real constructor where it does.
  Menu's image calls GraphicWin's real constructor at 0x005D4CF0 and then
  Spot's at 0x005FA860 - base then member, in declaration order - which is a
  real constructor and nothing else.

A `construct()` METHOD IS HOW THE SEH FRAME IS AVOIDED
- The SEH-frame family (flatbutton.cpp documents it at length) is a real
  ceiling for REAL CONSTRUCTORS: a GraphicWin-derived class whose constructor
  calls a base `construct()` picks up an unwind frame under `/GX` that the
  image does not pay for. Swapping members for raw storage does not drop it.
- But many of these classes did not HAVE a real constructor in the original.
  `CheckButton::construct()` at 0x00633750 is BYTE_EXACT precisely because it
  is a `construct()` method rather than a constructor: there is no base
  construction to protect, so no frame is emitted. If the image's body is
  reached with an already-built object and does not construct its bases, model
  it as a method and the frame problem disappears.
- The catalogued name is not the authority here. `??0Class@@QAE@XZ` says
  "constructor", but what matters is whether the BODY constructs bases. Where
  it does not, give the method a `// symbol` fact recording what this tree
  emits instead of forcing a constructor spelling to match a catalogue label.
- Do not reach for `/GX-`. It was added as a per-function flag axis and scored
  against every other set on 2026-08-21, and it LOSES: FlatButton stays best
  at `/O1 /GX` (0.565) against 0.440-0.513 for the `/GX-` variants, and Scroll
  scores 0.848 against 0.959 even though `/GX-` brings its instruction count
  from 99 down to 79 against an image of 83. Fewer instructions is not a
  better match - dropping the frame shifts everything after it.

A `MEASURED inline` THAT THE IMAGE CALLS AT SOME SITES IS A REAL CEILING
- Several bodies are capped by a helper this tree declares `MEASURED inline`
  which the image INLINES at most call sites and CALLS at one or two. VC6 6.0
  has no `__declspec(noinline)`, so there is no way to split that per site.
  Known instances: `port_to_port` (map.h) caps `get_there`; `has_tech`
  (technology.h) caps `good_sensor`; `do_all_non_input` (temp.h) caps
  `Path::continent`.
- THE OUT-OF-LINE FORWARDER IDIOM DOES NOT FIX IT, and that is measured, not
  assumed. Building a `do_all_non_input_call` for `Path::continent` - the same
  shape as `sleep_call` and `base_cost_call` - made it worse in both
  directions: agreement 16/206 -> 1/206, and call_diff went from MORE (5
  against 4) to FEWER (2 against 4) rather than agreeing, because it resolves
  callees BY ADDRESS and a forwarder is a different address than the one the
  image calls.
- The idiom works only where the forwarder IS the image's callee, which is why
  `sleep_call` and `base_cost_call` succeed. Do not build a third one for this
  family; record the ceiling and move on.

DO NOT CHAIN CONDITIONS THE IMAGE TESTS SEPARATELY
- `if (A || B) return 0;` compiles to ONE shared `return 0` epilogue that both
  tests jump to. The image very often has a separate inline epilogue after
  each test - fall through to `xor eax, eax; pop ebp; ret`, then test the
  next - which is what TWO separate `if` statements produce.
- Three bodies went BYTE_EXACT on this in one day: `prototype_factor`
  (18/48 -> 48/48), `map_write` and `map_read`, where the image chains neither
  its `fwrite`s nor its `freads` - the first two are early-return `if`s and
  the last is a direct `return !fwrite(...)`. `wants_prototype` moved
  0.617 -> 0.865 on the same shape.
- THE ORDER IS ALSO THE IMAGE'S, and it is cheap to check. Splitting
  `prototype_factor`'s guards the other way round scores 5 of 48 instead of
  48 of 48. Measure both orders before settling.
- The mirror of this is `Buffer::write_cent_l`, where a two-guard split scored
  WORSE (34 against 39 of 76). The image sometimes does merge tails. Measure;
  do not apply this on sight.

A VIRTUAL BASE'S VTORDISP IS EARNED, NOT REQUESTED
- VC6 emits a vtordisp for a virtual base ONLY when the derived class overrides
  a NON-DESTRUCTOR virtual of it. Measured 2026-08-22 on a minimal case: two
  virtual bases with virtual DESTRUCTORS only produce no displacement at all,
  under every setting including `/vd2`; add one plain virtual override and it
  emits one per base. `#pragma vtordisp(on)` is the default and changes
  nothing.
- So a layout that needs a 4-byte displacement before a virtual base needs a
  real override to produce it. RadioButton got its Dialog displacement by
  `Dialog::close()` becoming `virtual void close()`, which
  `RadioButton::close()` already matched.
- READ THE IMAGE TO SEE WHICH DISPLACEMENT IS WHOSE. 0x0060D0E0 writes exactly
  one by hand - `*(self + off1 - 4) = off1 - 0x18` - so THAT one is faithfully
  a declared member and only the other has to come from the compiler.

A QUALIFIED CALL SUPPRESSES VIRTUAL DISPATCH
- Making a method virtual to earn a vtordisp turns every `p->close()` into
  `call dword ptr [vtable]` where the image has `call rel32`. Two BYTE_EXACT
  claims regressed on that in one change.
- `p->Dialog::close()` names the class and emits the direct call. Both bodies
  came straight back to BYTE_EXACT. Use it at every call site that reaches a
  virtual through a base pointer.

SIZEOF FROM A FILE THAT DOES NOT COMPILE IS A LIE
- VC6 reports `sizeof` in the error message of a TU that failed for an
  unrelated reason, and the number can be WRONG. A layout change measured
  0xB20 for four separate probes purely because the constructor still named
  members the header had dropped; the true value was 0xB24 all along. Fix every
  error before believing a size.

RUNNING THE EXECUTABLE: THE INVOCATION IS PART OF THE MEASUREMENT
- From `.opensmacx/game` (653 files are opened by relative path), under the
  PROJECT'S wine prefix, inside a wine VIRTUAL DESKTOP:

      cd .opensmacx/game
      WINEPREFIX=$HOME/opt/vc6/.wineprefix DISPLAY=:1 \
          wine explorer /desktop=smac,1024x768 <exe>

- Get any of those three wrong and the program faults in a way that looks like
  a recovery defect and is not. Under the default prefix with no virtual
  desktop the SHIPPED IMAGE faults too, at 0x005F7E90 - its own
  `Win::is_visible`, reading 0x9C off a null popup slot. An entire runtime
  investigation on 2026-08-21 was built on that crash, concluded "DirectDraw
  fails under wine, so this path is faithful", and wrote it into popup.cpp.
  All of it was the invocation.
- ALWAYS RUN THE SHIPPED IMAGE THE SAME WAY FIRST. It is the baseline, it
  takes ten seconds, and if it faults where your build faults you have learned
  nothing about your build.
- `WINEDEBUG=-all` hides the fault line. A run that "completes" with no output
  may be a crash whose debugger sat at a prompt until `timeout` killed it -
  which reads as exit 124, exactly like a healthy long run. Leave the debug
  output on when you care about the answer, and count `page fault` lines
  rather than trusting an exit code.
- Kill leftovers between runs (`pkill -f OpenSMACX.exe`); wine leaves the
  process alive after `timeout` fires and a stale one perturbs the next run.
- WHERE BOTH BINARIES ACTUALLY GET TO, measured 2026-08-22 with winedbg
  driven from a pipe (`printf 'break *0xADDR\ncont\nquit\n' | winedbg ...`;
  note that `run` fails because winedbg starts already attached - use `cont`):

      Win::init_class   ->   DDInit::init   ->   [stops before
      BasePop::init_class]

  THE SHIPPED IMAGE AND THIS BUILD FOLLOW THAT SEQUENCE IDENTICALLY, and
  neither reaches `BasePop::init_class` even after fourteen continues. So the
  early stop is not a recovery defect - `jackal_init_real` runs a chain of
  `init_class` calls, each returning early on failure, and both binaries leave
  it at the same point. Matching the image's behaviour, including where it
  gives up, is the goal.
- winedbg resolves THIS build's symbols with file and line
  (`DDInit::init [win.cpp:2449]`) straight from the PDB, so a breakpoint
  address from `build/OpenSMACX.map` gives readable frames. Its ARGUMENT
  display is not trustworthy under /O2 - a frame with the pointer omitted
  shows the caller's stack slots as this function's parameters, which reads
  like an argument-shift bug and is not one.

A REFUSAL ONLY COUNTS AS A `// TRIED:` LINE
- Write what you measured as a LESSON LINE - `// LEVER:` or `// TRIED:` -
  in the marker's comment block. Written as ordinary prose ("cannot reach
  byte-exact", "not chased further") it is invisible to `decomp.reader`, so
  `frontier.py --untouched` keeps offering the body and the next pass
  re-derives your wall from scratch.
- `uv run tools/prose_refusals.py` lists the 20 bodies currently hiding that
  way. `Buffer::fill` was one: its comment explained at length that the
  image's body is hand-written assembly, and it still read as untouched.
- Anywhere in the block is fine now. The reader used to stop at the scaffold's
  unprefixed `Return Value:` and `Status:` lines, which hid anything below
  them; that is fixed. You no longer need to reposition notes to the top.
- A refusal is a claim and needs the same evidence as a success. Say what you
  measured and what it scored, not that something "seems" impossible.

VTABLE STORES GO FIRST IN A CONSTRUCTOR BODY
- Nothing in the GraphicWin/Win chain is declared `virtual`, deliberately, so
  a constructor's vtable stores are EXPLICIT assignments rather than something
  VC6 generates. The idiom is `GraphicWin::construct` (src/graphicwin.cpp:87):
  `uint32_t *const object = reinterpret_cast<uint32_t *>(this);` then
  `object[0x000 / 4]` and `object[0x444 / 4]`, primary and Buffer-subobject.
  Read the two constants off the image's own `mov dword ptr [esi], 0x...` and
  `mov dword ptr [esi + 0x444], 0x...`. A compiled body missing exactly those
  two stores is missing them for this reason.
- Write them BEFORE any member assignment in the body. This reads backwards
  until measured: the image appears to store the low half of a `double`, then
  both vtables, then the high half - the vtables landing INSIDE a member,
  which no source can express. It is the reverse. VC6 hoists the double's ZERO
  half ahead of whatever precedes it, so putting the vtable stores first is
  what produces the image's own interleaving. Measured on `Gamma::Gamma`:
  stores after the assignment gives SHAPE_EXACT 39/42, before it gives 42/42.
- If you are stuck at "everything agrees except the order of the trailing
  stores", try the reorder before concluding anything about the body.

FOR THE COORDINATOR: COLLECTING A FINISHED AGENT
- `uv run tools/collect_agent.py <agent-id>` does the whole sequence, and
  exists because doing it by hand went wrong twice in one day. It never pipes
  `git apply` (git prints per-file success BEFORE writing, so a `| head`
  closing the pipe kills it in between - it says "Applied cleanly" and writes
  nothing), and it never trusts `git apply --check --3way`, which EXITS 0 FOR
  PATCHES THAT CONFLICT: four in a row reported "applies" and then produced
  `UU`. It reads the index afterwards instead.
- It refuses outright if the main checkout is dirty, because collecting on
  top of uncommitted work makes the two indistinguishable when the gate fails.
- `--resolve-ours` keeps this checkout's side and PRINTS every discarded line.
  Agents branch before later corrections land, so their side is usually the
  superseded one - but read them. Three of four discards checked that way were
  correct, and reading them is the only thing that established it.

A CONSTRUCTOR MAY BE AN EMPTY INLINE OVER A REAL BODY
- `uv run tools/hollow_bodies.py --stubbed` lists 43 classes whose method is
  `{ ; }` in a header while the image has a real body and an artifact holds a
  transcription of it. These are NOT the same as the promotable list above:
  their artifacts do not reproduce yet, so promoting one means writing the
  body, not moving it.
- Do it anyway when one is handed to you. `{ ; }` is not a stub the way a
  `pending_bodies.cpp` forwarder is - the forwarder is honest and faults
  loudly, whereas an empty inline SILENTLY constructs nothing. Measured: the
  built executable faulted in `_initterm` before ever reaching `WinMain`
  because `MainInterface::MainInterface` was `{ ; }` and so never built its
  `StringBox`, which never built its `Scroll`.
- Expect the promotion to make the runtime WORSE before better, and let it.
  Giving a class a real constructor is what makes `_initterm` actually run it,
  so faults appear that an empty inline was hiding. That is the defect
  surfacing, not the promotion causing it.

A BODY MAY ALREADY BE WRITTEN, SOMEWHERE THAT IS NOT COMPILED
- `uv run tools/promotable.py` lists byte-exact bodies that exist only in
  `src/recovered/`, `src/recovered/units/` or `src/unrecovered/`. There are 878,
  and 36 are called by code the build compiles. A claim in one of those files
  proves the ARTIFACT, not this tree: nothing links them, and the product source
  often does not declare the function at all.
- Promoting one is not copying it. The artifact reaches its fields through
  `reinterpret_cast<char *>(this) + 0x...`; you have to model the layout,
  declare the member, delete the artifact, and delete any `pending_bodies.cpp`
  forwarder it had. The linker catches the last one for you - two definitions
  of a symbol is LNK2005.

DEFECTS THAT ARE NOT MATCHING DEFECTS
- The recovered program can be WRONG, not merely spelled differently, and no
  similarity score points at that. Three found on 2026-08-21: `Wave::load`
  called ITSELF without bound where the image calls `Sound::load`;
  `world_geothermal` carried a FABRICATED duplicate call block a prior pass
  invented, with `LM_GEOTHERMAL` where the argument is `BIT2_GEOTHERMAL`; and
  0x005C55C0 had `if (i >= 32)` inside a loop bounded `i < 20`, because the
  image's `cmp esi, 0x20` is the counter strength-reduced by four and the real
  threshold is 8.
- `uv run tools/call_diff.py --all` finds the first two: MORE means you call
  something the image never calls, WRONG CALLEE means same count and different
  target. `uv run tools/dead_branches.py` finds the third.
- Fixing these moves no claim. Do it anyway and say so in the report - the
  recovery is supposed to produce a program that RUNS.

BEFORE YOU GRIND A BODY
- `uv run tools/handwritten_asm.py` lists bodies whose shipped bytes use
  opcodes VC6 never emits - `loop`, bare `lodsb`, `xlatb`. Those were inline
  assembly in the original, byte-exactness is NOT reachable from C++, and the
  ceiling is semantic equivalence. Seven are already marked in the tree.
  DO NOT answer one of these with `__asm`: a semantic C++ body is worth more
  than a byte-exact assembly one.
- `uv run tools/listing_diff.py <addr>` prints EVERY divergence, aligned and
  relocation-masked. `measure` reports only the first, which on `Buffer::hline`
  was a jump whose target had moved because of something 70 instructions later.

COLLECTING A WORKTREE'S WORK (for whoever hands one out)
- From the WORKTREE: `git diff -- src/ > patch`. From the ROOT:
  `git apply --3way patch`. Check `git branch --show-current` before any git
  write - a `cd` into a worktree persists across commands.
- NEVER USE `git stash`. It is per-REPOSITORY, not per-worktree: every worktree
  pushes onto ONE shared stack, so parallel agents pop each other's work. Eight
  stray entries survived one Workflow run, three of them titled "RECOVERED by
  ... diff popped by mistake (shared stash stack)". Park work in a patch file
  instead - `git diff -- src/ > /tmp/mine.patch` is worktree-local.
- NEVER PIPE `git apply` INTO `head`. It prints "Applied patch to 'x' cleanly"
  for every file BEFORE it writes them; `head` closes the pipe, SIGPIPE kills
  git between the message and the write, and you get a success report over a
  completely unchanged tree. This silently discarded a merge twice on
  2026-08-21. Verify the side effect independently - `git status --short`, or
  grep the file for the change - rather than trusting the report.
- `uv run tools/reap_worktrees.py` after collecting. `--experiments` also
  clears `refute-*` scratch worktrees, which a Workflow leaves by the dozen.

PICKING A BATCH (for whoever hands one out)
- DO NOT SELECT BY SIZE ALONE. Asking for the ten smallest fresh map.cpp bodies
  returned ten instances of ONE problem: `alt_at`, `alt_detail_at`,
  `region_at`, `is_ocean` and their neighbours are a single accessor family
  sharing a single register-allocation plateau, and the whole pass produced
  only TRIED notes. Spread a batch across FILES, or select by defect shape
  from `call_diff --all` / `hollow_bodies.py`, which name a cause rather than a
  size.
- `uv run tools/frontier.py --fresh` lists WinMain-reachable bodies with no
  `TRIED:` notes on them. Rows without `--fresh` carry `[N tried]`.
  A batch picked off the raw depth order sent one agent eight bodies that were
  already exhausted, and it correctly reported eight no-changes.
- `uv run tools/call_diff.py --all` ranks bodies whose CALL COUNT disagrees
  with the image. Those are structural defects, not register noise, and they
  are the highest-yield rows on the board: the FEWER direction means this tree
  skips work the image does.

MEASURING
- `uv run tools/osmx.py measure <addr>` scores one body. `--all-flags` scores every
  flag set; the winner is picked on similarity across sets that answer different
  questions, so a real improvement can look like a no-op without it.
- `uv run tools/osmx.py show <addr>` prints the image's bytes.
- `uv run tools/osmx.py calls <addr>` says what the image REALLY calls. "0 call(s)"
  means every callee is written out inline there.
- `uv run tools/store_order.py <addr>` prints the field-write order in the image
  beside this tree's, normalised to each side's own first store.
- `uv run tools/call_diff.py --all` lists bodies whose CALL COUNT disagrees with the
  image and names the helper being over-called. A call the image does not make is a
  SEMANTIC difference, not an encoding one - fix those first.
- `uv run tools/try_spellings.py <file> <addr> <candidates.json>` scores candidate
  bodies and restores the file. If its control does not score 0 agreeing
  instructions, STOP: the span is not being replaced and every result is meaningless.

LEVERS THAT HAVE PAID, most productive first

- A CALLEE BOUND AS A POINTER costs its caller the `E8`. `T *const F = (T *)0x...`
  compiles `call dword ptr [F]` where the image emits `call rel32`. Declare it as
  an ordinary function and add a forwarder in `src/pending_bodies.cpp`; the target
  of an `E8` is a relocation on both sides and is discounted, so any symbol
  matches. Same defect for a CALLBACK bound as `extern const void *const C;`
  defined in a .cpp - it compiles `push dword ptr [C]` against the image's
  `push 0xADDR`, and the fix is to DEFINE it in the header, where a namespace
  scope `const` has internal linkage and folds to the immediate. Worth 202 claims
  in one change on 2026-08-21, plus the whole message-pump family.

- A CONSTRUCTOR CALL IS NOT THE WAY TO REACH A CONSTRUCTOR. When a `??__E`
  initialiser dispatches through a `func_opaque_ctor_*` seam, the fix is a
  `construct` METHOD on the class, not a real constructor: VC6 adds its hidden
  most-derived flag to an explicit `->Class::Class(n)` call, so the site pushes
  the argument TWICE. `Win::construct` is the existing idiom, and its marker
  carries a `symbol` fact naming it. The call target is a relocation on both
  sides, so the method's own name costs nothing. Five initialisers went
  byte-exact this way on 2026-08-21; the constructor spelling made them worse.

- `volatile` CUTS BOTH WAYS, and you must MEASURE which way before touching it.

  As an ALIAS it is a wall: `volatile Font **const fonts` read through, instead
  of three independent absolute lvalues, cost five basebutton.cpp bodies their
  match - `set_text_color` x3, `set_def_font` and `init` all went byte-exact the
  moment it came out, and `Sprite::close` went 4/38 to 35/38. Reading an object
  through a volatile handle blocks VC6's memory-operand folding, so where the
  image emits `cmp dword ptr [esi+0x28], edi` you get a `mov` into a register
  first, at every site.

  As a STORE QUALIFIER it is the fix: `Scroll::set_border_color` and
  `expand_rect` KEEP their `volatile` deliberately, because without it VC6
  proves the unconditional RECT writes dead and eliminates stores the image
  performs. Removing it there took `set_bar_thickness` from 7/22 to 8/22 while
  deleting four instructions the image has, and `set_thumb_rect` from 12/20
  down to 7/20.

  The distinction is what the volatile is DOING: hiding a field behind a raw
  offset (remove it, and name the field) versus keeping a store alive (leave
  it). `uv run tools/volatile_aliases.py` lists them; it flags bodies that
  already reproduce, and those are never to be touched on this evidence.

- A CLAMP MAY BE A MACRO, and a macro re-evaluates. `Font::width`'s image calls
  `strlen` TWICE - `call strlen; cmp; jl; call strlen` - because its `min` is a
  macro, and caching the length in a local collapsed six instructions into
  three. If `call_diff` says FEWER and the missing callee is something cheap
  you hoisted into a local, put it back at every use. Watch the signedness at
  the same time: `size_t` gives `jbe` where the image has `jl`.

- SIGNED `/ 2` IS NOT `>> 1`. VC6 emits a round-toward-zero fixup
  (`cdq; sub eax, edx`) before each `sar`. Where the value cannot be negative -
  a sum of `abs()` results, a count, a length - they mean the same and the
  fixup is pure loss: `vector_dist` went byte-exact on it alone, `base_find`
  31/79 -> 53/79, `del_site` 36/95 -> 61/95. `uv run tools/signed_divide.py`
  finds bodies emitting a fixup the image does not. A fixup BOTH sides emit is
  correct - the value really can go negative there - so never "fix" one of
  those.

- PLACEMENT-NEW PULLS IN AN SEH FRAME. `new (obj) Class(...)` makes VC6 emit
  unwind-protection scaffolding when the class has a non-trivial destructor and
  the inlined constructor calls a non-intrinsic function. The image has no such
  frame, so the divergence starts at INSTRUCTION 0 and reads as a total
  mismatch. An ordinary `construct(...)` method - no `new`-expression - drops
  it; that is the `Win::construct` idiom. `uv run tools/placement_new.py` lists
  the 28 sites and flags the 5 in bodies that already reproduce, which are
  correct as they stand and must be left alone.

- `nullptr` TO A `__fastcall` REDIRECT COSTS AN INSTRUCTION. A
  `..._redirect(Thing *, void *, ...)` called with `nullptr` for the unused
  second argument materialises `xor edx, edx`, which the image does not emit.
  Call the METHOD instead. SIX bodies so far - `ImageButton::close`,
  `ImageButton::init` and `PickTech::close` byte-exact on it alone. Search for
  it as `_redirect\([^;]*, *nullptr[,)]`: a pattern matching only TWO-argument
  redirects missed three of the six. Related: a `void construct()` never emits the image's closing
  `mov eax, esi` - if the image returns `this`, say so in the signature, which
  closed the last instruction on four Ambience constructors.

- A LOCAL MUST MATCH THE WIDTH IT READS FROM. This is the most reliable lever
  in the tree and it has now taken FOUR bodies byte-exact on its own.
  `guard_check` needed `plan_region` as `uint8_t`, the element type of the
  `region_base_plan[128]` it is assigned from: `uint32_t` forces an
  `xor edx, edx` zero-extend and word-form compares where the image has
  `cmp dl, N`. A stack-climb induction variable needs `int16_t` where the field
  it walks is `int16_t`: `int` forces an early `movsx` the image DEFERS, keeping
  the 16-bit value in a register and widening only at the indexing.
  Check EVERY local against the field or array element it is loaded from.

- DO NOT CACHE WHAT THE IMAGE RE-READS - but cache what it reads ONCE. Both
  directions are real and both were measured on 2026-08-21. `veh_lift` went
  4/60 to 49/60 because the image re-reads `Vehs[veh_id].x` at every use;
  0x005BF130 went 51/70 to 65/70 because caching `map_loc(x, y)->bit2` once
  lets VC6 fold the `Map` stride into the load's SIB addressing, where two
  accessor calls could not. Caching a `Map *tile` POINTER instead is worse
  still, 65 back to 21. Read the listing; do not assume either way.

- THE IMAGE PEELS ITS LOOPS. `do { B } while (C);` compiles one copy of the body;
  the image runs `B; while (C) { B }` - same program, and the difference was four
  instructions on each of three message pumps. If the image's loop has TWO calls
  to the condition where you have one, this is why.

- BRANCH POLARITY IS SOURCE-SHAPED. `if (x) { work; return true; } return false;`
  and `if (!x) { return false; } work; return true;` are not the same codegen.
  The image often jumps TO the work and falls through to the early return, which
  means it never emits the `xor eax, eax` the first form needs. If your listing
  has an extra `xor eax, eax` and an inverted jump, rewrite it as a guard clause.

1. A helper the image INLINES that this tree defines in a .cpp. `osmx calls` shows no
   call to it. Move the body to the header - `inline` if it has no ORIGINAL marker,
   `MEASURED inline` if it does (that keeps its own claim measurable) - and LEAVE THE
   MARKER IN THE .cpp, because decomp's reader globs only *.cpp and *.c. Put the
   definition at the END of the header so it can see what it uses.
2. `memcpy`-based accessors and other helpers VC6 refuses to inline: `__forceinline`.
3. Assignments in declaration order where the image uses another. `store_order` shows
   the permutation directly.
4. `uint32_t` where the image's branch is signed: `jl` not `jb`, `sar` not `shr`.
   `uv run tools/signedness.py` finds these.
5. A value read AFTER a store where the image reads it before - hoist it into a local.
6. `&record->field` taken once, rather than the field read and written twice: the
   image folds the member offset into the `lea`.
7. `T *p = &Array[index];` then `p->a`, `p->b` rather than `Array[index].a`,
   `Array[index].b`. This one is PER FUNCTION - it has also made bodies worse - so
   measure it, do not sweep it.
8. `volatile` and hand-rolled bit-casts written to pin store order or wrapping: both
   defeat VC6 and are usually unnecessary. Try the plain spelling.
9. A vtable dispatch spelled `(ORIGINAL(x)->*original_slot<T>(vtable + N))()` emits
   `mov reg,[vtable+N]; call reg` where the image has one `call dword ptr [vtable+N]`.
   `vtable_slot<Fn>(x, N)(x)` from original_seam.h emits the single call; declare `Fn`
   as `__fastcall` with the RECEIVER AS ITS ONLY PARAMETER, or a second one costs a
   `xor edx, edx`.

RULES
- After ANY source change `cmake --build build` must exit 0.
- Bank a body with `uv run tools/osmx.py record <addr>`.
- The gate is `uv run tools/osmx.py check`. NEVER pipe it to `tail` - the exit code is
  the verdict. It must stay at 0 REGRESSED, and you must run it before finishing.
- Record what you learn under each marker: a `// LEVER:` line for what worked, a
  `// TRIED:` line for a spelling you measured and rejected. Never touch the
  `// ORIGINAL:` marker line itself.
- Never write a comment asserting something the measurement does not show. If a body
  plateaus after a handful of measured attempts, leave it and say so.
- Do not commit. Do not change any file outside src/.
