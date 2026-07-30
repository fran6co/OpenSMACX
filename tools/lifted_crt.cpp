// Implementations for the CRT the lift deliberately never translates.
// See lifted_crt.h for why this layer exists and what a shim owes its caller.

#include "lifted_crt.h"
#include "lifted_crt_table.h"
#include "lifted_loader.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

namespace {

// A guest pointer, checked. `opensmacx_at` is a subtraction with no bounds
// test, so an out-of-span pointer silently addresses the harness's own memory -
// the exact failure that has been mistaken for a lowering bug here five times.
// Trapping names the routine and stops; returning host memory would produce a
// plausible wrong answer instead.
unsigned char *checked(uint32_t address, uint32_t length, const char *who) {
    const uint64_t low = address;
    const uint64_t high = uint64_t(address) + length;
    if (address == 0 || low < OpensmacxImageBase
        || high > uint64_t(OpensmacxImageBase) + OpensmacxSpanSize) {
        opensmacx_trap(address, who);
    }
    return opensmacx_at(address);
}

// A guest C string, checked to the terminator without leaving the span.
const char *checked_string(uint32_t address, const char *who) {
    if (address < OpensmacxImageBase
        || address >= uint32_t(OpensmacxImageBase + OpensmacxSpanSize)) {
        opensmacx_trap(address, who);
    }
    const uint32_t limit = OpensmacxImageBase + OpensmacxSpanSize;
    for (uint32_t at = address; at < limit; ++at) {
        if (*opensmacx_at(at) == '\0')
            return reinterpret_cast<const char *>(opensmacx_at(address));
    }
    opensmacx_trap(address, "CRT string argument is not terminated inside the guest span");
    return nullptr;
}

uint32_t arg(OpensmacxStaticRecompileState &s, int index) {
    return opensmacx_mem32(s.esp + 4U + 4U * uint32_t(index));
}

// cdecl: the caller cleans the arguments, so a shim pops only the return
// address. This is `ret` with no immediate, and it must match x86_lower.py's
// lowering of it exactly or the caller resumes on a shifted stack.
void ret(OpensmacxStaticRecompileState &s, uint32_t eax) {
    s.eax = eax;
    s.esp += 4U;
}

// ---------------------------------------------------------------------------
// The shims
// ---------------------------------------------------------------------------

void crt_strlen(OpensmacxStaticRecompileState &s) {
    const char *text = checked_string(arg(s, 0), "_strlen");
    ret(s, uint32_t(std::strlen(text)));
}

void crt_strcpy(OpensmacxStaticRecompileState &s) {
    const uint32_t destination = arg(s, 0);
    const char *source = checked_string(arg(s, 1), "_strcpy");
    const uint32_t length = uint32_t(std::strlen(source)) + 1U;
    std::memcpy(checked(destination, length, "_strcpy"), source, length);
    ret(s, destination);
}

void crt_strcat(OpensmacxStaticRecompileState &s) {
    const uint32_t destination = arg(s, 0);
    const char *existing = checked_string(destination, "_strcat");
    const char *source = checked_string(arg(s, 1), "_strcat");
    const uint32_t at = uint32_t(std::strlen(existing));
    const uint32_t length = uint32_t(std::strlen(source)) + 1U;
    std::memcpy(checked(destination + at, length, "_strcat"), source, length);
    ret(s, destination);
}

void crt_strcmp(OpensmacxStaticRecompileState &s) {
    const char *left = checked_string(arg(s, 0), "_strcmp");
    const char *right = checked_string(arg(s, 1), "_strcmp");
    ret(s, uint32_t(int32_t(std::strcmp(left, right))));
}

void crt_strncmp(OpensmacxStaticRecompileState &s) {
    const char *left = checked_string(arg(s, 0), "_strncmp");
    const char *right = checked_string(arg(s, 1), "_strncmp");
    ret(s, uint32_t(int32_t(std::strncmp(left, right, arg(s, 2)))));
}

void crt_strchr(OpensmacxStaticRecompileState &s) {
    const uint32_t base = arg(s, 0);
    const char *text = checked_string(base, "_strchr");
    const char *found = std::strchr(text, int(int8_t(arg(s, 1) & 0xFFU)));
    ret(s, found ? base + uint32_t(found - text) : 0U);
}

void crt_strrchr(OpensmacxStaticRecompileState &s) {
    const uint32_t base = arg(s, 0);
    const char *text = checked_string(base, "_strrchr");
    const char *found = std::strrchr(text, int(int8_t(arg(s, 1) & 0xFFU)));
    ret(s, found ? base + uint32_t(found - text) : 0U);
}

void crt_strstr(OpensmacxStaticRecompileState &s) {
    const uint32_t base = arg(s, 0);
    const char *text = checked_string(base, "_strstr");
    const char *needle = checked_string(arg(s, 1), "_strstr");
    const char *found = std::strstr(text, needle);
    ret(s, found ? base + uint32_t(found - text) : 0U);
}

void crt_strncpy(OpensmacxStaticRecompileState &s) {
    const uint32_t destination = arg(s, 0);
    const uint32_t count = arg(s, 2);
    const char *source = checked_string(arg(s, 1), "_strncpy");
    std::strncpy(reinterpret_cast<char *>(
                     checked(destination, count, "_strncpy")),
                 source, count);
    ret(s, destination);
}

void crt_memcpy(OpensmacxStaticRecompileState &s) {
    const uint32_t destination = arg(s, 0);
    const uint32_t count = arg(s, 2);
    std::memcpy(checked(destination, count, "_memcpy"),
                checked(arg(s, 1), count, "_memcpy"), count);
    ret(s, destination);
}

void crt_memset(OpensmacxStaticRecompileState &s) {
    const uint32_t destination = arg(s, 0);
    const uint32_t count = arg(s, 2);
    std::memset(checked(destination, count, "_memset"),
                int(arg(s, 1) & 0xFFU), count);
    ret(s, destination);
}

void crt_memcmp(OpensmacxStaticRecompileState &s) {
    const uint32_t count = arg(s, 2);
    ret(s, uint32_t(int32_t(std::memcmp(checked(arg(s, 0), count, "_memcmp"),
                                        checked(arg(s, 1), count, "_memcmp"),
                                        count))));
}

void crt_memchr(OpensmacxStaticRecompileState &s) {
    const uint32_t base = arg(s, 0);
    const uint32_t count = arg(s, 2);
    unsigned char *area = checked(base, count, "_memchr");
    void *found = std::memchr(area, int(arg(s, 1) & 0xFFU), count);
    ret(s, found ? base + uint32_t(static_cast<unsigned char *>(found) - area)
                 : 0U);
}

void crt_atoi(OpensmacxStaticRecompileState &s) {
    ret(s, uint32_t(int32_t(std::atoi(checked_string(arg(s, 0), "_atoi")))));
}

void crt_toupper(OpensmacxStaticRecompileState &s) {
    ret(s, uint32_t(std::toupper(int(int32_t(arg(s, 0))))));
}

void crt_tolower(OpensmacxStaticRecompileState &s) {
    ret(s, uint32_t(std::tolower(int(int32_t(arg(s, 0))))));
}

void crt_strcmpi(OpensmacxStaticRecompileState &s) {
    const char *left = checked_string(arg(s, 0), "__strcmpi");
    const char *right = checked_string(arg(s, 1), "__strcmpi");
    int result = 0;
    while (!result && (*left || *right)) {
        result = std::tolower((unsigned char)*left)
               - std::tolower((unsigned char)*right);
        if (!*left || !*right) break;
        ++left; ++right;
    }
    ret(s, uint32_t(int32_t(result < 0 ? -1 : (result > 0 ? 1 : 0))));
}

// _itoa(value, buffer, radix). Written out rather than delegated, because the
// host's itoa is not standard C and its absence is a build failure on some
// toolchains rather than a link error on all of them.
void crt_itoa(OpensmacxStaticRecompileState &s) {
    const int32_t value = int32_t(arg(s, 0));
    const uint32_t buffer = arg(s, 1);
    const uint32_t radix = arg(s, 2);
    char scratch[36];
    unsigned length = 0;
    if (radix < 2U || radix > 36U) {
        opensmacx_trap(radix, "__itoa radix out of range");
    }
    uint32_t magnitude = (radix == 10U && value < 0)
                       ? uint32_t(-(int64_t)value) : uint32_t(value);
    const bool negative = (radix == 10U && value < 0);
    if (magnitude == 0U) scratch[length++] = '0';
    while (magnitude) {
        const uint32_t digit = magnitude % radix;
        scratch[length++] = char(digit < 10U ? '0' + digit : 'a' + digit - 10U);
        magnitude /= radix;
    }
    if (negative) scratch[length++] = '-';
    unsigned char *out = checked(buffer, length + 1U, "__itoa");
    for (unsigned i = 0; i < length; ++i) out[i] = (unsigned char)scratch[length - 1U - i];
    out[length] = 0;
    ret(s, buffer);
}

struct Shim {
    const char *name;
    OpensmacxLiftedFunction function;
};

const Shim Shims[] = {
    {"_strlen", &crt_strlen},     {"_strcpy", &crt_strcpy},
    {"_strcat", &crt_strcat},     {"_strcmp", &crt_strcmp},
    {"_strncmp", &crt_strncmp},   {"_strchr", &crt_strchr},
    {"_strrchr", &crt_strrchr},   {"_strstr", &crt_strstr},
    {"_strncpy", &crt_strncpy},   {"_memcpy", &crt_memcpy},
    {"_memcpy_0", &crt_memcpy},   {"_memset", &crt_memset},
    {"_memcmp", &crt_memcmp},     {"_memchr", &crt_memchr},
    {"_atoi", &crt_atoi},         {"_toupper", &crt_toupper},
    {"_toupper_0", &crt_toupper}, {"_tolower", &crt_tolower},
    {"_tolower_0", &crt_tolower}, {"__strcmpi", &crt_strcmpi},
    {"__itoa", &crt_itoa},
};

OpensmacxLiftedFunction shim_for_name(const char *name) {
    for (const Shim &shim : Shims) {
        if (!std::strcmp(shim.name, name)) return shim.function;
    }
    return nullptr;
}

const OpensmacxCrtEntry *entry_for(uint32_t address) {
    // Linear, because this runs once per distinct CRT call site and the table
    // is 327 entries. A binary search here would be a claim about the table
    // being sorted that nothing checks.
    for (unsigned i = 0; i < OpensmacxCrtTableSize; ++i) {
        if (OpensmacxCrtTable[i].address == address)
            return &OpensmacxCrtTable[i];
    }
    return nullptr;
}

}  // namespace

namespace {

// SURVEY MODE, off by default. See opensmacx_crt_survey in the header for why
// the evidence it produces is weaker and spelled differently.
bool Surveying = false;
unsigned Surveyed = 0;

// The stub an unimplemented-but-NAMED CRT routine gets under --survey: return
// to the caller as cdecl with EAX = 0 instead of trapping, so one run reaches
// past it and the whole frontier can be enumerated rather than one item per
// rebuild-and-run cycle.
//
// It returns a LIE - zero - and that is the entire reason a surveyed run is not
// a boot. A caller that checks the result takes the failure branch; one that
// does not carries a null onward. Either way what follows is not the program.
void crt_survey_skip(OpensmacxStaticRecompileState &s) {
    // The state carries no instruction pointer, so the stub cannot name itself;
    // opensmacx_crt_dispatch prints the address when it hands this out, which is
    // the one place that has both the address and the decision.
    ++Surveyed;
    ret(s, 0U);
}

}  // namespace

OpensmacxLiftedFunction opensmacx_crt_dispatch(uint32_t address) {
    const OpensmacxCrtEntry *entry = entry_for(address);
    if (!entry) {
        return nullptr;
    }
    OpensmacxLiftedFunction shim = shim_for_name(entry->name);
    if (shim) {
        return shim;
    }
    if (Surveying) {
        // Named, unimplemented, and we were asked to keep going. Logged HERE
        // because this is the only point holding both the address and the
        // decision - the stub gets no instruction pointer, and the trap that
        // would otherwise fire is never entered. Dispatch runs once per distinct
        // call site, so a routine reached from twenty sites prints up to twenty
        // times and the driver deduplicates.
        std::printf("SURVEY-REACHED %#010x %s\n", unsigned(address),
                    entry->name);
        std::fflush(stdout);
        return &crt_survey_skip;
    }
    return nullptr;
}

void opensmacx_crt_survey(bool enabled) {
    Surveying = enabled;
}

unsigned opensmacx_crt_surveyed(void) {
    return Surveyed;
}

const char *opensmacx_crt_name(uint32_t address) {
    const OpensmacxCrtEntry *entry = entry_for(address);
    return entry ? entry->name : nullptr;
}

void opensmacx_crt_coverage(unsigned *implemented, unsigned *catalogued) {
    unsigned count = 0;
    for (unsigned i = 0; i < OpensmacxCrtTableSize; ++i) {
        if (shim_for_name(OpensmacxCrtTable[i].name)) ++count;
    }
    if (implemented) *implemented = count;
    if (catalogued) *catalogued = OpensmacxCrtTableSize;
}
