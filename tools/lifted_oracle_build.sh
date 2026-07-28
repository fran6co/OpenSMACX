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
#   The weakened opensmacx_trap. lifted_dispatch.cpp.o defines it as abort();
#   the oracle needs a trap to unwind into "SKIP, this body was never lowered"
#   instead of killing the process. objcopy --weaken-symbol on a COPY of that
#   object (under build/, never in the tree) lets the oracle's strong
#   definition win, for both the shards' calls and the dispatcher's own.
#
# -static is inherited from build.sh's reasoning: without it the image dies at
# load with c0000135, naming no symbol.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CXX="${CXX:-/opt/homebrew/bin/i686-w64-mingw32-g++}"
OBJCOPY="${OBJCOPY:-/opt/homebrew/bin/i686-w64-mingw32-objcopy}"
LIFTED="${LIFTED:-$ROOT/build/lifted}"
OUT="${OUT:-$ROOT/build/oracle}"
mkdir -p "$OUT"

if [ ! -f "$LIFTED/lifted_dispatch.cpp.o" ]; then
    echo "no lifted objects in $LIFTED - run $LIFTED/build.sh first" >&2
    exit 1
fi

"$OBJCOPY" --weaken-symbol=__Z14opensmacx_trapjPKc \
    "$LIFTED/lifted_dispatch.cpp.o" "$OUT/dispatch_weak.o"

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
# which the weakened copy replaces.
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
    "$OUT/dispatch_weak.o" $SHARDS \
    -lgdi32 -luser32 -lkernel32 -ladvapi32 -lshell32 -lole32 -loleaut32 \
    -lcomdlg32 -lwinmm -lversion -lwsock32 -limm32 -luuid
echo "built $OUT/lifted_oracle.exe"
