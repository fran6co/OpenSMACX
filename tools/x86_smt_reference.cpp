// Ground truth for the SMT encoding: lifted_x86.h itself, applied to concrete
// states.
//
// The equivalence spike introduces a SECOND semantics for x86, and a second
// semantics is a second chance to be wrong - in the worst way, because an
// encoding that is wrong in a COLLAPSING direction proves wrong recoveries
// equal and looks exactly like success. The defence is differential: drive this
// program and the Z3 encoding with the same random states and require the two
// to agree bit for bit.
//
// It is deliberately dumb. One operation, one width, two operands and an
// incoming EFLAGS on stdin; the result and the outgoing EFLAGS on stdout. No
// decoding, no memory, no control flow - those belong to the encoder under
// test, and putting any of them here would let a shared misunderstanding pass
// unnoticed on both sides.
//
//   echo "add32 <a> <b> <eflags>" | x86_smt_reference
//   -> "<result> <eflags>"
//
// Values are hexadecimal without a prefix.

#include "static_recompile_runtime.h"
#include "lifted_x86.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

namespace {

bool apply(const std::string &op, uint32_t a, uint32_t b,
           OpensmacxStaticRecompileState &state, uint32_t &result) {
    // Every entry names the width explicitly. A default width is exactly the
    // kind of shared assumption this program exists not to have.
    if (op == "add32") { result = opensmacx_add_w<32>(state, a, b); return true; }
    if (op == "add16") { result = opensmacx_add_w<16>(state, a, b); return true; }
    if (op == "add8")  { result = opensmacx_add_w<8>(state, a, b);  return true; }
    if (op == "adc32") { result = opensmacx_adc_w<32>(state, a, b); return true; }
    if (op == "adc8")  { result = opensmacx_adc_w<8>(state, a, b);  return true; }
    if (op == "sub32") { result = opensmacx_sub_w<32>(state, a, b); return true; }
    if (op == "sub16") { result = opensmacx_sub_w<16>(state, a, b); return true; }
    if (op == "sub8")  { result = opensmacx_sub_w<8>(state, a, b);  return true; }
    if (op == "sbb32") { result = opensmacx_sbb_w<32>(state, a, b); return true; }
    if (op == "sbb8")  { result = opensmacx_sbb_w<8>(state, a, b);  return true; }
    if (op == "and32") { result = opensmacx_and_w<32>(state, a, b); return true; }
    if (op == "and16") { result = opensmacx_and_w<16>(state, a, b); return true; }
    if (op == "and8")  { result = opensmacx_and_w<8>(state, a, b);  return true; }
    if (op == "or32")  { result = opensmacx_or_w<32>(state, a, b);  return true; }
    if (op == "or8")   { result = opensmacx_or_w<8>(state, a, b);   return true; }
    if (op == "xor32") { result = opensmacx_xor_w<32>(state, a, b); return true; }
    if (op == "xor8")  { result = opensmacx_xor_w<8>(state, a, b);  return true; }
    if (op == "inc32") { result = opensmacx_inc_w<32>(state, a);    return true; }
    if (op == "inc8")  { result = opensmacx_inc_w<8>(state, a);     return true; }
    if (op == "dec32") { result = opensmacx_dec_w<32>(state, a);    return true; }
    if (op == "dec8")  { result = opensmacx_dec_w<8>(state, a);     return true; }
    if (op == "adc16") { result = opensmacx_adc_w<16>(state, a, b); return true; }
    if (op == "sbb16") { result = opensmacx_sbb_w<16>(state, a, b); return true; }
    if (op == "or16")  { result = opensmacx_or_w<16>(state, a, b);  return true; }
    if (op == "xor16") { result = opensmacx_xor_w<16>(state, a, b); return true; }
    if (op == "inc16") { result = opensmacx_inc_w<16>(state, a);    return true; }
    if (op == "dec16") { result = opensmacx_dec_w<16>(state, a);    return true; }
    return false;
}

}  // namespace

int main() {
    char line[256];
    while (std::fgets(line, sizeof line, stdin)) {
        char op[32] = {};
        unsigned a = 0, b = 0, flags = 0;
        if (std::sscanf(line, "%31s %x %x %x", op, &a, &b, &flags) != 4) {
            std::printf("bad\n");
            continue;
        }
        OpensmacxStaticRecompileState state{};
        state.eflags = flags;
        uint32_t result = 0;
        if (!apply(op, a, b, state, result)) {
            std::printf("unsupported\n");
            continue;
        }
        std::printf("%08x %08x\n", result, state.eflags);
        std::fflush(stdout);
    }
    return 0;
}
