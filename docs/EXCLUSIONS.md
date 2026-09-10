# Recovery scope and limits of evidence

Application code remains in scope unless a source-owned component interpretation
or an explicitly reviewed platform boundary says otherwise. Difficulty, a failed
compiler experiment, a missing runtime oracle, or a good local byte score does
not authorize an exclusion, a weaker claim, or an invented structural model.

Historical EXCLUDED S<n> annotations refer to the numbered topics below. They
are provenance to investigate, not current acceptance receipts. Preserve original
identities and historical obligations through the native workflow; do not erase
them to make a gate green. AGENTS.md and recovery/WORKFLOW.md define current
acceptance. Build/link success, masked-byte agreement, strict reference checks,
structural evidence and runtime evidence establish different things.

The former census and superseded tool instructions are available in this file's
Git history. They are not current counts or a second ledger. Recompute inventories
with the installed framework and retain disposable reports only as caches.

## 1. Runtime and third-party libraries

Recognized dependencies should be identified and verified, not rewritten as game
code. Preserve their provenance, version uncertainty, archive identity and license
constraints. A familiar symbol name or an available compiler distribution does
not prove which library member the original linker selected.

src/vendor/vc6-runtime.recovery.h owns the currently declared bounded runtime
interpretation. It identifies selected members of a pinned external archive; it
does not establish coverage of the entire CRT. The archive is not redistributed.
Use the native component checks documented in recovery/README.md to test those
interpretations, including references and final-link evidence where supported.
Unknown dependency targets remain unknown; do not introduce original-address
function pointers or fabricate application globals to resolve them.

Floating-point helpers, formatting/conversion routines and allocator internals
can have observable behavior beyond their public signatures. Substituting a
modern library is a separate compatibility decision requiring evidence, not an
automatic consequence of classifying the original as third-party code.

## 2. Exception frames and compiler-generated code

Identify frame registrations and handlers from actual instructions and references,
not loose byte patterns. An argument sequence can resemble an exception prologue;
a table or data tail can resemble executable instructions.

Recover genuine construction, destruction, inheritance and object lifetimes so
the compiler can emit their supporting code. Absence of an observed throw site
does not by itself prove that an exception path is unreachable or dispensable.
Do not hand-install exception frames or weaken restoration obligations.

### 2a. Unwind funclets

A funclet borrowing an enclosing function's frame is not necessarily a standalone
C++ function. Recover it with the owning lifecycle/body and verify the emitted
supporting code and original spans. Failure to compile it in isolation is evidence
about the work unit, not permission to omit its behavior. Address proximity alone
does not identify a funclet or its owner.

### 2b. Vtables, receiver adjustments and casts

The former claim that manual vtables were inherently correct is withdrawn.
Constructor/destructor table transitions can be compiler-generated evidence of
real hierarchy and lifetime stages. src/wave.h already declares Wave : Sound;
its older comments are recovery history, not a rule against further structural
repair. Audit actual declarations, constructors, destructors, callers and emitted
tables together.

A negative receiver offset can indicate a base/subobject adjustment, a thunk, or
an incorrectly assigned owner. It does not establish that raw pointer arithmetic
is unavoidable, nor that the whole function should be excluded. Distinguish real
embedding/container operations from missing inheritance using evidence.

Original-address calls, opaque vtable storage and self-offset member access are
unfinished structural work unless independently justified by actual behavior.
Prefer proper bases, members, virtual methods, globals and source-owned callees.
Preserve justified migrations when matching temporarily regresses; record the
cross-function discovery, obtain scope/temporary-regression authorization, and
restore obligations through the native checks. Do not revert correct structure
automatically or demote claims.

### 2c. Multiple emitted symbols

The retired one-symbol restriction is not a recovery limitation. A genuine class
can cause the compiler to emit destructors, adjustors and other supporting symbols.
Use named subject selection and inspect those outputs. Never change an embedded
object into a pointer, flatten a class, or substitute arbitrary padding merely to
suppress a required symbol or satisfy a local score.

### 2d. Writable/executable and self-modifying code

The legacy masked-byte route conservatively refuses writable/executable sections.
Section permissions alone do not prove how a body behaves at runtime, and a copied
address interval is not a portable classifier. Keep original identities and report
the limitation honestly. Runtime transformation needs an explicit, independently
supported interpretation and verification strategy; an unavailable measurement is
not acceptance. Do not misclassify such bodies as unwind funclets by address range.

## 3. Hardware-specific operations

Direct port I/O cannot be replaced by an ordinary hosted C++ expression. Confirm
reachable instructions first; linear decoding of switch tables can produce false
I/O instructions. Any platform replacement needs reviewed scope and a stated
observable-behavior criterion before implementation. For a palette operation,
compare the resulting palette entries and channel conversion, not just a screenshot.
An intended replacement is not evidence that the criterion has passed.

## 4. Graphics, audio and other system interfaces

Application functions using DirectDraw, DirectPlay, DirectSound or MSVFW32 remain
recovery targets. System implementations are dependencies, not game bodies to
recreate. Imports and COM dispatch need correct interface/slot/type and lifetime
interpretations; copying a vtable offset into a manual dispatcher is not sufficient.
Source and binary checks remain useful without a runtime oracle, but missing
runtime evidence must not be represented as full application acceptance.

## 5. Indirect calls

Direct-call inventories do not cover unresolved virtual, callback or function-
pointer targets. Treat their reachability and caller counts as incomplete. Recover
ownership and target evidence rather than using a historical percentage allowance
or silently assuming missing edges cannot affect a change.

## 6. Multiplayer

Networking and NetDaemon code remain in scope. Lack of smoke-test coverage is a
verification gap, not an exclusion. Matching an isolated body does not independently
prove protocol behavior, synchronization, object layout or the integrated game.

## 7. External decompilations, mods and analysis databases

Thinker, PRACX, kuna, Ghidra, IDA databases, hand decompilations and notes can seed
the workflow. Treat their names, types, offsets and ownership as attributed hints,
not unquestionable facts. Shared ancestry means two agreeing sources may not be
independent corroboration. A partial declaration is not necessarily a wrong prefix,
but it cannot establish a complete object size or lifecycle.

Validate names against behavior and callers; keep uncertain names provisional.
For types and layouts, check original access widths, signedness, offsets, embedding
versus pointers, construction/destruction and emitted layout evidence as applicable.
Identical sizes or matching bytes at one use do not prove those interpretations.
A static_assert checks the current declaration against an asserted number; it
does not establish that number's original provenance.

Keep load-bearing original bindings and structural evidence in source. Keep
non-derivable scope/review decisions in the versioned task journal. Derived
inventories and executable-derived artifacts belong in ignored caches, and external
source redistribution remains subject to its actual license and provenance.
