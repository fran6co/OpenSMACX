#!/bin/sh
# Builds the differential oracle: the lifted image plus lifted_oracle.cpp and
# its driver, linked into one Windows executable that runs under Wine.
#
# Two things here are load-bearing and neither is obvious:
#
#   --image-base 0x10000000. The harness must NOT sit at 0x00400000, because
#   that is where the guest goes. mingw's default base for an exe is exactly
#   0x00400000, so leaving this off puts the harness on top of the thing it is
#   testing.
#
#   opensmacx_trap_hook. lifted_dispatch.cpp.o defines opensmacx_trap as
#   abort(); the oracle needs a trap to unwind into a SKIP verdict instead of
#   killing the process, so it installs a hook that the generated trap calls
#   before it prints anything.
#
#   This USED to be `objcopy --weaken-symbol` on a copy of that object, so that
#   the oracle's own strong definition of opensmacx_trap would win. It won for
#   the SHARDS, whose calls carry a relocation, and it silently did NOT win for
#   opensmacx_dispatch, which defines and calls the trap in one translation
#   unit: -O2 inlined the fprintf and the abort straight into the dispatcher,
#   leaving no symbol to redirect. The consequence was invisible until someone
#   asked for it - every lifted body reaching a callee the lift has no body for
#   ABORTED the harness rather than being recorded, so the "run it and see
#   whether the blocked path is actually taken" measurement could not be made
#   at all. Measured before the fix: --run-anyway over the extcall cohort
#   printed one `opensmacx: trap at 0x00644f3a` and wrote zero report rows.
#
# -static is inherited from build.sh's reasoning: without it the image dies at
# load with c0000135, naming no symbol.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CXX="${CXX:-/opt/homebrew/bin/i686-w64-mingw32-g++}"
LIFTED="${LIFTED:-$ROOT/build/lifted}"
OUT="${OUT:-$ROOT/build/oracle}"
mkdir -p "$OUT"

if [ ! -f "$LIFTED/lifted_dispatch.cpp.o" ]; then
    echo "no lifted objects in $LIFTED - run $LIFTED/build.sh first" >&2
    exit 1
fi

# The dispatch object is COMPILED HERE rather than taken from $LIFTED, because
# it is the one object whose source has to match this harness: the hook it
# calls is declared in lifted_runtime.h, and a stale $LIFTED/lifted_dispatch.
# cpp.o built before the hook existed links fine and traps by aborting. Cheap -
# one file - and it removes the class of failure entirely.
"$CXX" -std=c++17 -O2 -c -I"$LIFTED" -I"$ROOT/tools" \
    "$LIFTED/lifted_dispatch.cpp" -o "$OUT/lifted_dispatch.o"

# EXTRA_CXXFLAGS exists for exactly one job: the host-layout control.
#
#   OUT=$ROOT/build/oracle-shim EXTRA_CXXFLAGS=-DORACLE_LAYOUT_SHIM=0x51000 \
#       tools/lifted_oracle_build.sh
#
# builds a second oracle whose opensmacx_image sits at a different HOST
# address. Any FAIL whose detail differs between the two was reading the
# harness's own memory through an out-of-span guest address, not the program.
# Relinking at a different --image-base does NOT do this - it moves the wrapped
# read target and opensmacx_image together - which is why it is a separate knob
# and not a different value of the one that already exists.
for source in lifted_oracle.cpp lifted_oracle_main.cpp; do
    # shellcheck disable=SC2086
    "$CXX" -std=c++17 -O2 $EXTRA_CXXFLAGS -c -I"$LIFTED" -I"$ROOT/tools" \
        "$ROOT/tools/$source" -o "$OUT/${source%.cpp}.o"
done

# Everything the lift produced except its own main() and its dispatch object,
# which the freshly compiled copy above replaces.
SHARDS=""
for object in "$LIFTED"/lifted_*.cpp.o; do
    case "$object" in
        *lifted_main.cpp.o|*lifted_dispatch.cpp.o) continue ;;
    esac
    SHARDS="$SHARDS $object"
done

# shellcheck disable=SC2086
"$CXX" -std=c++17 -O2 -static -Wl,--image-base,0x10000000 \
    -o "$OUT/lifted_oracle.exe" \
    "$OUT/lifted_oracle.o" "$OUT/lifted_oracle_main.o" \
    "$OUT/lifted_dispatch.o" $SHARDS \
    -lgdi32 -luser32 -lkernel32 -ladvapi32 -lshell32 -lole32 -loleaut32 \
    -lcomdlg32 -lwinmm -lversion -lwsock32 -limm32 -luuid
echo "built $OUT/lifted_oracle.exe"
