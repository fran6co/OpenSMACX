#include "stdafx.h"
#include "span_math.h"

#include <string.h>

/*
Purpose: Half the sum of an argument and a span read out of a structure.

             mov ecx,[ebp+8] / mov eax,[ecx+0x14] / mov edx,[ecx+0x10]
             sub eax,edx / mov edx,[ebp+0xc] / add eax,edx
             cdq / sub eax,edx / sar eax,1 / ret

         Arithmetic wraps in 32 bits before signed division by two; the
         original cdq/sub/sar sequence truncates toward zero.

// RECOVERY_ROLE: SCAFFOLD Bounded standalone span arithmetic; original argument type, caller domain and containing object remain unresolved.
// ORIGINAL: 0x00408470 sub_408470 0x00408470-0x0040848A BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Own the complete cdecl span calculation in the arithmetic component, with a typed access prefix rather than a void-pointer redirect; do not infer an original class or complete allocation.","evidence":["pinned:0x00408470-0x0040848A reads the pointer at stack+8 and int addend at stack+C, loads pointer+14 before pointer+10, wraps subtraction/addition and divides the signed result by two toward zero","source:EndpointPairView models only the accessed +10/+14 words; caller and original containing type remain unknown"]}
// symbol    ?half_span_plus_addend@span_math@@YAHPBUEndpointPairView@1@H@Z
// size      26 bytes
// prototype unresolved: (const prefix view *, int addend)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: (a->[0x14] - a->[0x10] + b) / 2
Status: Complete
*/
int span_math::half_span_plus_addend(const EndpointPairView *source,
                                     int addend) {
    const volatile EndpointPairView *const view = source;
    const uint32_t high_bits = view->high_at_14_;
    const uint32_t low_bits = view->low_at_10_;
    const uint32_t total_bits =
        high_bits - low_bits + static_cast<uint32_t>(addend);

    int32_t signed_total;
    memcpy(&signed_total, &total_bits, sizeof(signed_total));
    return signed_total / 2;
}
