#include "matrix3.h"
#include <cstring>

// Chosen mathematical source view for the nine-float receiver prefix used by
// this address family. This does not declare the original class or its full
// object size; application construction and call sites remain unknown.
/*
Historical explanation of the prior untyped field-accessor scaffold follows; it does not establish the original class owner or this chosen source representation.
Purpose: Set 9 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov eax, ecx / mov edx, 0x3f800000 / xor ecx, ecx / mov dword ptr [eax], edx / mov dword ptr [eax + 4], ecx / mov dword ptr [eax + 8], ecx / mov dword ptr [eax + 0xc], ecx / mov dword ptr [eax + 0x10], edx / mov dword ptr [eax + 0x14], ecx / mov dword ptr [eax + 0x18], ecx / mov dword ptr [eax + 0x1c], ecx / mov dword ptr [eax + 0x20], edx / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x006346C0 sub_6346c0 0x006346C0-0x006346E4 BYTE_EXACT
// symbol    ?identity@Matrix3@@QAEPAU1@XZ
// size      36 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the value described above
Status: Complete
*/
Matrix3 *Matrix3::identity() {
    cells[0] = 1.0f; cells[1] = 0.0f; cells[2] = 0.0f;
    cells[3] = 0.0f; cells[4] = 1.0f; cells[5] = 0.0f;
    cells[6] = 0.0f; cells[7] = 0.0f; cells[8] = 1.0f;
    return this;
}

/*
Historical explanation of the prior untyped field-accessor scaffold follows; it does not establish the original class owner or this chosen source representation.
Purpose: Set 9 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov edx, 0x3f800000 / xor eax, eax / mov dword ptr [ecx], edx / mov dword ptr [ecx + 4], eax / mov dword ptr [ecx + 8], eax / mov dword ptr [ecx + 0xc], eax / mov dword ptr [ecx + 0x10], edx / mov dword ptr [ecx + 0x14], eax / mov dword ptr [ecx + 0x18], eax / mov dword ptr [ecx + 0x1c], eax / mov dword ptr [ecx + 0x20], edx / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x006346F0 sub_6346f0 0x006346F0-0x00634712 BYTE_EXACT
// symbol    ?reset_identity@Matrix3@@QAEXXZ
// size      34 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Matrix3::reset_identity() {
    cells[0] = 1.0f; cells[1] = 0.0f; cells[2] = 0.0f;
    cells[3] = 0.0f; cells[4] = 1.0f; cells[5] = 0.0f;
    cells[6] = 0.0f; cells[7] = 0.0f; cells[8] = 1.0f;
}

/*
Purpose: Add another matrix into this one through an identity scratch
         buffer, then publish the completed sum as a Matrix3 value.

         The identity fill of the scratch is fully overwritten by the sum
         - every one of its nine slots is stored before any read - but the
         original pays for it anyway, so it is kept rather than tidied
         away.

// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x00634720 sub_634720 0x00634720-0x006347BE FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x006347A3/0x006347B2/0x006347BB: load first entry stack argument into EAX, copy nine dwords to it, retain EAX through ret 8", "source:the chosen nine-float Matrix3 view does not assert an original complete class, allocation size, constructor or lifetime", "source:src/vector.h and mapped Vector operator+6343E0/scale634670 establish independently observed class-result conventions; their receiver, result-copy/EAX and ret8 protocol matches this routine."], "reason": "Model the observed staged nine-float result as a C++ Matrix3 value in the chosen mathematical view. The first stack slot is supplied by the current compiler as a class-result pointer. Pinned protocol and established Vector value-result ABI support this interpretation, but do not distinguish an original explicit output-pointer API or establish original class identity, complete extent or lifetime."}
// Historical explicit-pointer binding: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x006347A3/0x006347B2/0x006347BB: load first entry stack argument into EAX, copy nine dwords to it, retain EAX through ret 8", "source:the chosen nine-float Matrix3 view does not assert an original complete class, allocation size, constructor or lifetime"], "reason": "Represent the observed output buffer as a typed pointer to the chosen Matrix3 view; hidden-class-result semantics and receiver ownership remain unresolved."}
// TRIED: nothing yet; the nested row/column countdown pair is the original's own shape.
// symbol   ?add@Matrix3@@QAE?AU1@PBU1@@Z
// size      158 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the completed nine-float result value
Status: Complete
*/
Matrix3 Matrix3::add(const Matrix3 *other) {
    float scratch[9] = {1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 1.0f};
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            scratch[row * 3 + column] = other->cells[row * 3 + column] +
                                        cells[row * 3 + column];
        }
    }
    Matrix3 result;
    std::memcpy(result.cells, scratch, sizeof(scratch));
    return result;
}

/*
Purpose: Add another 3x3 float matrix into this one, element by element.

             mov edx,eax / mov ebx,3 / mov esi,ecx / mov edi,3 / sub esi,eax
             fld [esi+edx] / fadd [edx] / add edx,4 / dec edi
             fstp [edx-4] / jne inner / dec ebx / jne outer / ret 4

         ESI is the BYTE DELTA between the two matrices, computed once, and
         EDX walks forward without being reset between outer iterations - so
         the two nested threes are nine consecutive floats, not three rows of
         three restarted each time.

// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x006347C0 sub_6347c0 0x006347C0-0x006347F0
// symbol    ?add_assign@Matrix3@@QAEXPBU1@@Z
// size      48 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Matrix3::add_assign(const Matrix3 *other) {
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            cells[row * 3 + column] = other->cells[row * 3 + column] +
                                     cells[row * 3 + column];
        }
    }
}

/*
Purpose: Replace this 3x3 matrix with its product with another: a
         triple-nested row/column walk into a scratch buffer, copied back
         only when every cell is settled.

// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x006347F0 sub_6347f0 0x006347F0-0x006348BC FILE
// symbol   ?multiply_assign@Matrix3@@QAEXPBU1@@Z
// size      204 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Matrix3::multiply_assign(const Matrix3 *other) {
    float scratch[9] = {1.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 1.0f};
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            float sum = 0.0f;
            for (int depth = 0; depth < 3; ++depth) {
                sum = other->cells[depth * 3 + column] *
                      cells[row * 3 + depth] + sum;
            }
            scratch[row * 3 + column] = sum;
        }
    }
    std::memcpy(cells, scratch, sizeof(scratch));
}

/*
Purpose: Scale a 3x3 float matrix by a scalar.

             mov edi,3 / mov esi,3 / fld [esp+0xc] / fmul [edx] / add edx,4
             dec esi / fstp [edx-4] / jne inner / dec edi / jne outer / ret 4

         The scalar sits at [esp+0xc] because ESI and EDI were pushed first;
         before those pushes it is the single argument at [esp+4]. It is
         re-loaded inside the inner loop, which changes nothing - it is the
         same float32 every time.

// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x006348C0 sub_6348c0 0x006348C0-0x006348E7
// symbol    ?scale_assign@Matrix3@@QAEXM@Z
// size      39 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Matrix3::scale_assign(float scale) {
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            cells[row * 3 + column] = scale * cells[row * 3 + column];
        }
    }
}

/*
Purpose: Subtract another 3x3 float matrix from this one.

             fld [edx] / fsub [esi+edx] / ... / ret 4

         The same nine-element walk as 006347C0, but loading THIS first: the
         result is this - other, and subtraction does not commute.

// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x006348F0 sub_6348f0 0x006348F0-0x00634920
// symbol    ?subtract_assign@Matrix3@@QAEXPBU1@@Z
// size      48 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Matrix3::subtract_assign(const Matrix3 *other) {
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            cells[row * 3 + column] = cells[row * 3 + column] -
                                      other->cells[row * 3 + column];
        }
    }
}

/*
Historical explanation of the prior untyped field-accessor scaffold follows; it does not establish the original class owner or this chosen source representation.
Purpose: Set 9 field(s) to constants.
         Emitted by tools/generate_field_accessors.py from

             mov eax, ecx / mov edx, 0x3f800000 / xor ecx, ecx / mov dword ptr [eax], edx / mov dword ptr [eax + 4], ecx / mov dword ptr [eax + 8], ecx / mov dword ptr [eax + 0xc], ecx / mov dword ptr [eax + 0x10], edx / mov dword ptr [eax + 0x14], ecx / mov dword ptr [eax + 0x18], ecx / mov dword ptr [eax + 0x1c], ecx / mov dword ptr [eax + 0x20], edx / ret

         The name is the ADDRESS, not an invention, and `self` is a void
         pointer because this body needs `this` to be nothing more than a
         pointer and an offset - the class it belongs to is not established.
         The adapter declares 0 stack dword(s) so it
         cleans 0 bytes, taken from the body's own `ret` -
         the only statement of this function's arity that exists, since it has
         no mangled name. Declaring fewer would leave them on the caller's
         stack.
// Structural debt: observed cells[0..8] only; original class, complete extent,
// containing owner, lifecycle, callers, TU, and x87 precision remain unknown.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x00634B70 sub_634b70 0x00634B70-0x00634B94 BYTE_EXACT
// symbol    ?write_identity@Matrix3@@QAEPAU1@XZ
// size      36 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the value described above
Status: Complete
*/
Matrix3 *Matrix3::write_identity() {
    cells[0] = 1.0f; cells[1] = 0.0f; cells[2] = 0.0f;
    cells[3] = 0.0f; cells[4] = 1.0f; cells[5] = 0.0f;
    cells[6] = 0.0f; cells[7] = 0.0f; cells[8] = 1.0f;
    return this;
}

/*
// Structural debt: the chosen Matrix3 source view covers the observed
// nine-float prefix only. Original class/extent/lifetime/callers/TU and x87
// precision are not established. The pointer-to-int scratch delta below the prior body was
// a source-form experiment; it has been replaced by equivalent bounded index
// traversal, not promoted as an ABI or matching claim.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x00634980 sub_634980 0x00634980-0x00634A50 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x00634A35/0x00634A44/0x00634A4D: load first entry stack argument into EAX, copy nine dwords to it, retain EAX through ret 8", "source:the chosen nine-float Matrix3 view does not assert an original complete class, allocation size, constructor or lifetime", "source:src/vector.h and mapped Vector operator+6343E0/scale634670 establish independently observed class-result conventions; their receiver, result-copy/EAX and ret8 protocol matches this routine."], "reason": "Model the observed staged nine-float result as a C++ Matrix3 value in the chosen mathematical view. The first stack slot is supplied by the current compiler as a class-result pointer. Pinned protocol and established Vector value-result ABI support this interpretation, but do not distinguish an original explicit output-pointer API or establish original class identity, complete extent or lifetime."}
// Historical explicit-pointer binding: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x00634A35/0x00634A44/0x00634A4D: load first entry stack argument into EAX, copy nine dwords to it, retain EAX through ret 8", "source:the chosen nine-float Matrix3 view does not assert an original complete class, allocation size, constructor or lifetime"], "reason": "Represent the observed output buffer as a typed pointer to the chosen Matrix3 view; hidden-class-result semantics and receiver ownership remain unresolved."}
// Historical source symbol hypothesis: ?multiply@Mat3@@QAEXPAM0@Z
// symbol    ?multiply@Matrix3@@QAE?AU1@PBU1@@Z
// TRIED: `extern "C" int __stdcall sub_634980(int a1, int a2)` per the given def head - disasm reads *ecx directly as a float* with no matching stack slot (a 3x3 matrix "this"), so the receiver is a __thiscall member, not a 2-arg __stdcall free function; changed to a `Mat3::multiply` member. `dst`/`a` tracked as independent incrementing pointers gives a MISMATCH at the prologue (#4, 87% similarity); an `int off = dst - a` delta recomputed each outer iteration (matching the original's spill of that delta to `[esp+0x14]` and its `add esi, ebx` re-derivation) gets to 89.5% similarity, 7 edits, all register-allocation choices (which callee-saved reg holds which pointer) rather than control-flow shape. Pointer-to-int delta form is retired here; the observation remains historical, not an implementation requirement.
// size      208 bytes
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the completed nine-float result value
Status: Complete
*/
Matrix3 Matrix3::multiply(const Matrix3 *other) {
    float local[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    int row = 0;
    do {
        int column = 0;
        do {
            local[row * 3 + column] = 0.0f;
            float sum = local[row * 3 + column];
            int depth = 0;
            do {
                sum = cells[row * 3 + depth] *
                      other->cells[depth * 3 + column] + sum;
                ++depth;
            } while (depth < 3);
            local[row * 3 + column] = sum;
            ++column;
        } while (column < 3);
        ++row;
    } while (row < 3);
    Matrix3 result;
    std::memcpy(result.cells, local, sizeof(local));
    return result;
}

/*
// Structural debt: exact nine-float receiver window only; no original class,
// full extent, lifetime, or x87 equivalence asserted.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x00634920 sub_634920 0x00634920-0x00634975 BYTE_EXACT
// Historical source symbol hypothesis: ?equals@Mat3@@QAE_NPBM@Z
// symbol    ?equals@Matrix3@@QAE_NPBM@Z
// LEVER: two indexed reads, not a hoisted difference. The image keeps `this`
// and `other` apart and re-derives their difference inside the outer
// loop (`sub edi, ebp` at 0x634939, one per row); spelling that
// difference out as a local lets /O2 prove it loop-invariant and hoist
// it past the loop it belongs to, which is 69 bytes against 85.
Return Value: true when all cell pairs compare equal under the native predicate.
The pinned VC6/x87 lowering accepts unordered NaN pairs; ordinary host C++ !=
rejects them, so host and native NaN behavior is characterized separately.
Status: Complete
*/
bool Matrix3::equals(const float *other) {
    const float *mine = cells;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            // The pinned VC6 FCOMP lowering accepts unordered NaN pairs with
            // masked exceptions. Standard host C++ != rejects them instead.
            // Signaling-NaN, unmasked exceptions and x87 status remain unknown.
            if (mine[row * 3 + col] != other[row * 3 + col]) {
                return false;
            }
        }
    }
    return true;
}

/*
// Structural debt: exact nine-float receiver window only; no original class,
// full extent, lifetime, or x87 equivalence asserted.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x00634A50 sub_634a50 0x00634A50-0x00634AC9 FILE
// Historical source symbol hypothesis: ?transform@Matrix3@@QAEXPAMPBM@Z
// symbol    ?transform@Matrix3@@QAEXPAMPBM@Z
// size      121 bytes
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Matrix3::transform(float *out, const float *in) {
    const float *m = cells;
    float result[3];
    result[0] = m[1] * in[1] + m[2] * in[2] + m[0] * in[0];
    result[1] = m[4] * in[1] + m[3] * in[0] + m[5] * in[2];
    result[2] = m[7] * in[1] + m[6] * in[0] + m[8] * in[2];
    out[0] = result[0];
    out[1] = result[1];
    out[2] = result[2];
}

/*
// Structural debt: exact nine-float receiver window only; no original class,
// full extent, lifetime, or x87 equivalence asserted. Replaced the old
// pointer-to-int scratch delta with flat indexing for host-width safety.
// RECOVERY_ROLE: SCAFFOLD Bounded numerical receiver view only; shared original object ownership, containing class, full extent, callers and lifetime remain unproved. Source consolidation and complete arithmetic bodies do not resolve this structural obligation.
// ORIGINAL: 0x00634AD0 sub_634ad0 0x00634AD0-0x00634B67 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x00634B4E/0x00634B5D/0x00634B64: load first entry stack argument into EAX, copy nine dwords to it, retain EAX through ret 8", "source:the chosen nine-float Matrix3 view does not assert an original complete class, allocation size, constructor or lifetime", "source:src/vector.h and mapped Vector operator+6343E0/scale634670 establish independently observed class-result conventions; their receiver, result-copy/EAX and ret8 protocol matches this routine."], "reason": "Model the observed staged nine-float result as a C++ Matrix3 value in the chosen mathematical view. The first stack slot is supplied by the current compiler as a class-result pointer. Pinned protocol and established Vector value-result ABI support this interpretation, but do not distinguish an original explicit output-pointer API or establish original class identity, complete extent or lifetime."}
// Historical explicit-pointer binding: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x00634B4E/0x00634B5D/0x00634B64: load first entry stack argument into EAX, copy nine dwords to it, retain EAX through ret 8", "source:the chosen nine-float Matrix3 view does not assert an original complete class, allocation size, constructor or lifetime"], "reason": "Represent the observed output buffer as a typed pointer to the chosen Matrix3 view; hidden-class-result semantics and receiver ownership remain unresolved."}
// Historical source symbol hypothesis: ?scale_into@Mat3x3@@QAEXPAMM@Z
// symbol    ?scaled@Matrix3@@QAE?AU1@M@Z
// TRIED: extern "C" __stdcall(int,int) contract head - disasm reads ecx as an implicit receiver (thiscall) into a 3x3 float matrix, not a stack int; rewrote as Mat3x3::scale_into(float*,float). MISMATCH #1 push/lea, prologue layout only, structure otherwise matches Ghidra's nested-loop scale+copy. The old pointer-to-int delta is retired; preserve this as historical source-form evidence.
// size      151 bytes
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the completed nine-float result value
Status: Complete
*/
Matrix3 Matrix3::scaled(float scale) {
    float tmp[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    for (int i = 0; i < 9; ++i) {
        tmp[i] = scale * cells[i];
    }
    Matrix3 result;
    std::memcpy(result.cells, tmp, sizeof(tmp));
    return result;
}
