#include "lifted_loader.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

// ---------------------------------------------------------------------------
// SHA-256
//
// Self-contained on purpose: the lifted image links -static against nothing
// but libgcc, and pulling in a crypto library to check one constant would be
// the tail wagging the dog.
//
// Verified differentially against Python's hashlib over 633 messages - every
// length from 0 to 199 plus 255, 256, 447, 448, 449, 511, 512, 513, 1000,
// 4096 and 100000, at three seeds each - so both padding branches and the
// length-field boundary are covered, not just the happy path.
// ---------------------------------------------------------------------------

const uint32_t kSha256Round[64] = {
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU,
    0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U, 0xd807aa98U, 0x12835b01U,
    0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U,
    0xc19bf174U, 0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
    0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU, 0x983e5152U,
    0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 0xc6e00bf3U, 0xd5a79147U,
    0x06ca6351U, 0x14292967U, 0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU,
    0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
    0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 0xd192e819U,
    0xd6990624U, 0xf40e3585U, 0x106aa070U, 0x19a4c116U, 0x1e376c08U,
    0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU,
    0x682e6ff3U, 0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
    0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U,
};

inline uint32_t rotr32(uint32_t value, unsigned int count) {
    return (value >> count) | (value << (32U - count));
}

void sha256_block(uint32_t state[8], const unsigned char *block) {
    uint32_t w[64];
    for (unsigned int i = 0; i < 16; ++i) {
        w[i] = (static_cast<uint32_t>(block[i * 4 + 0]) << 24) |
               (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
               (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
               static_cast<uint32_t>(block[i * 4 + 3]);
    }
    for (unsigned int i = 16; i < 64; ++i) {
        const uint32_t s0 =
            rotr32(w[i - 15], 7) ^ rotr32(w[i - 15], 18) ^ (w[i - 15] >> 3);
        const uint32_t s1 =
            rotr32(w[i - 2], 17) ^ rotr32(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
    for (unsigned int i = 0; i < 64; ++i) {
        const uint32_t s1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
        const uint32_t ch = (e & f) ^ (~e & g);
        const uint32_t t1 = h + s1 + ch + kSha256Round[i] + w[i];
        const uint32_t s0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
        const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        const uint32_t t2 = s0 + maj;
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void sha256_hex(const unsigned char *data, size_t size, char out[65]) {
    uint32_t state[8] = {
        0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
        0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U,
    };
    size_t offset = 0;
    for (; offset + 64 <= size; offset += 64) {
        sha256_block(state, data + offset);
    }
    unsigned char tail[128];
    const size_t remainder = size - offset;
    std::memcpy(tail, data + offset, remainder);
    tail[remainder] = 0x80;
    const size_t padded = (remainder < 56) ? 64 : 128;
    std::memset(tail + remainder + 1, 0, padded - remainder - 1 - 8);
    const uint64_t bits = static_cast<uint64_t>(size) * 8U;
    for (unsigned int i = 0; i < 8; ++i) {
        tail[padded - 1 - i] = static_cast<unsigned char>(bits >> (i * 8));
    }
    for (size_t block = 0; block < padded; block += 64) {
        sha256_block(state, tail + block);
    }
    static const char kDigits[] = "0123456789abcdef";
    for (unsigned int i = 0; i < 8; ++i) {
        for (unsigned int byte = 0; byte < 4; ++byte) {
            const unsigned char value =
                static_cast<unsigned char>(state[i] >> (24 - byte * 8));
            out[i * 8 + byte * 2 + 0] = kDigits[value >> 4];
            out[i * 8 + byte * 2 + 1] = kDigits[value & 0x0fU];
        }
    }
    out[64] = '\0';
}

// The one value in this file derived from the original: its identity, already
// pinned as SUPPORTED_EXE_SHA256 in tools/static_recompile_pilot.py.
const char kSupportedSha256[] =
    "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf";

// ---------------------------------------------------------------------------
// Little-endian reads out of the file buffer, bounds-checked against its size
// so a truncated or hostile file cannot walk off the end.
// ---------------------------------------------------------------------------

struct Buffer {
    const unsigned char *data;
    size_t size;

    bool has(size_t offset, size_t length) const {
        return offset <= size && length <= size - offset;
    }
    uint16_t u16(size_t offset) const {
        return static_cast<uint16_t>(data[offset]) |
               static_cast<uint16_t>(static_cast<uint16_t>(data[offset + 1]) << 8);
    }
    uint32_t u32(size_t offset) const {
        return static_cast<uint32_t>(data[offset]) |
               (static_cast<uint32_t>(data[offset + 1]) << 8) |
               (static_cast<uint32_t>(data[offset + 2]) << 16) |
               (static_cast<uint32_t>(data[offset + 3]) << 24);
    }
};

char g_error[512];
char g_sha256[65];
bool g_loaded = false;
uint32_t g_entry_point = 0;
uint32_t g_relocations = 0;
int32_t g_delta = 0;
uint32_t g_reloc_rva = 0;
uint32_t g_reloc_size = 0;

__attribute__((format(printf, 1, 2)))
const char *fail(const char *format, ...) {
    va_list args;
    va_start(args, format);
    std::vsnprintf(g_error, sizeof(g_error), format, args);
    va_end(args);
    return g_error;
}

const char *read_whole_file(const char *path, std::vector<unsigned char> &out) {
    std::FILE *file = std::fopen(path, "rb");
    if (file == nullptr) {
        return fail("cannot open guest image '%s' (set %s)", path,
                    OpensmacxImagePathVariable);
    }
    if (std::fseek(file, 0, SEEK_END) != 0) {
        std::fclose(file);
        return fail("cannot seek '%s'", path);
    }
    const long size = std::ftell(file);
    if (size <= 0) {
        std::fclose(file);
        return fail("'%s' is empty or unseekable", path);
    }
    std::rewind(file);
    out.resize(static_cast<size_t>(size));
    const size_t read = std::fread(out.data(), 1, out.size(), file);
    std::fclose(file);
    if (read != out.size()) {
        // %lu rather than %zu: this links against msvcrt, whose printf does
        // not know the C99 length modifier.
        return fail("short read on '%s': %lu of %lu bytes", path,
                    static_cast<unsigned long>(read),
                    static_cast<unsigned long>(out.size()));
    }
    return nullptr;
}

constexpr uint16_t kMachineI386 = 0x014cU;
constexpr uint16_t kPe32Magic = 0x010bU;
constexpr size_t kSectionHeaderSize = 40;

}  // namespace

const char *opensmacx_resolve_image_path() {
    const char *from_environment = std::getenv(OpensmacxImagePathVariable);
    if (from_environment != nullptr && from_environment[0] != '\0') {
        return from_environment;
    }
    return OPENSMACX_DEFAULT_IMAGE_PATH;
}

const char *opensmacx_load_image(const char *path) {
    if (path == nullptr) {
        path = opensmacx_resolve_image_path();
    }

    std::vector<unsigned char> file;
    if (const char *error = read_whole_file(path, file)) {
        return error;
    }
    const Buffer pe = {file.data(), file.size()};

    // --- identity, before a single byte is copied ---------------------------
    if (!pe.has(0, 2) || pe.data[0] != 'M' || pe.data[1] != 'Z') {
        return fail("'%s' is not a DOS/PE executable (no MZ)", path);
    }
    if (!pe.has(0x3c, 4)) {
        return fail("'%s' is truncated before e_lfanew", path);
    }
    const uint32_t header = pe.u32(0x3c);
    if (!pe.has(header, 24)) {
        return fail("'%s' has an out-of-range PE header offset %#x", path,
                    header);
    }
    if (pe.u32(header) != 0x00004550U) {
        return fail("'%s' has no PE\\0\\0 signature at %#x", path, header);
    }
    const uint16_t machine = pe.u16(header + 4);
    if (machine != kMachineI386) {
        return fail("'%s' is machine %#06x, not i386 (%#06x)", path, machine,
                    kMachineI386);
    }
    sha256_hex(file.data(), file.size(), g_sha256);
    if (std::strcmp(g_sha256, kSupportedSha256) != 0) {
        return fail(
            "'%s' is not the supported build.\n"
            "  expected SHA-256 %s\n"
            "  got      SHA-256 %s\n"
            "A different executable would lower correctly and diverge at run "
            "time, which is indistinguishable from a lowering bug. Refusing.",
            path, kSupportedSha256, g_sha256);
    }

    const uint16_t sections = pe.u16(header + 6);
    const uint16_t optional_size = pe.u16(header + 20);
    const size_t optional = header + 24;
    if (!pe.has(optional, optional_size) || optional_size < 96) {
        return fail("'%s' has an unusable optional header (%u bytes)", path,
                    optional_size);
    }
    if (pe.u16(optional) != kPe32Magic) {
        return fail("'%s' is not PE32 (magic %#06x)", path, pe.u16(optional));
    }

    const uint32_t entry_rva = pe.u32(optional + 16);
    const uint32_t image_base = pe.u32(optional + 28);
    const uint32_t size_of_image = pe.u32(optional + 56);
    const uint32_t size_of_headers = pe.u32(optional + 60);
    const uint32_t directories = pe.u32(optional + 92);
    const uint32_t section_alignment = pe.u32(optional + 32);

    // Used below to round each section's span up to a whole number of pages.
    // A zero or non-power-of-two alignment is not something to paper over with
    // a default: it would make the rounding silently wrong for every section.
    if (section_alignment == 0
            || (section_alignment & (section_alignment - 1U)) != 0) {
        return fail("'%s' has an unusable SectionAlignment %#010x", path,
                    section_alignment);
    }

    if (image_base != OpensmacxImageBase) {
        // Not fatal in principle - that is what the relocation pass below is
        // for - but the generated tree hard-codes OpensmacxImageBase into
        // every absolute operand, so a different ImageBase means the tree was
        // generated from a different file than the one being loaded.
        return fail("'%s' has ImageBase %#010x but the lifted tree was "
                    "generated for %#010x",
                    path, image_base, OpensmacxImageBase);
    }
    if (size_of_image != OpensmacxImageSize) {
        return fail("'%s' has SizeOfImage %#010x but the lifted tree spans "
                    "%#010x",
                    path, size_of_image, OpensmacxImageSize);
    }

    // --- headers ------------------------------------------------------------
    // The Windows loader maps SizeOfHeaders bytes at ImageBase, and guest code
    // that walks its own MZ/PE header (SMACX does, for the resource section)
    // would find zeros otherwise. Cheap, and it makes the mapping match what
    // the real loader produces rather than an approximation of it.
    if (size_of_headers > OpensmacxImageSize || !pe.has(0, size_of_headers)) {
        return fail("'%s' has an unusable SizeOfHeaders %#010x", path,
                    size_of_headers);
    }
    std::memcpy(opensmacx_at(OpensmacxImageBase), file.data(), size_of_headers);

    // --- sections -----------------------------------------------------------
    const size_t table = optional + optional_size;
    if (!pe.has(table, static_cast<size_t>(sections) * kSectionHeaderSize)) {
        return fail("'%s' has a truncated section table", path);
    }
    for (uint16_t index = 0; index < sections; ++index) {
        const size_t entry = table + static_cast<size_t>(index) * kSectionHeaderSize;
        char name[9];
        std::memcpy(name, pe.data + entry, 8);
        name[8] = '\0';

        const uint32_t virtual_size = pe.u32(entry + 8);
        const uint32_t virtual_address = pe.u32(entry + 12);
        const uint32_t raw_size = pe.u32(entry + 16);
        const uint32_t raw_pointer = pe.u32(entry + 20);

        // The span the loader must own is the LARGER of the two: .text has
        // SizeOfRawData 0x263000 against VirtualSize 0x2629c5 (raw padding
        // past the end of the code), while .data has VirtualSize 0x3401f8
        // against SizeOfRawData 0x26000 (3.25 MB of BSS past the end of the
        // file). Taking either one alone is wrong for one of those two.
        const uint32_t span = raw_size > virtual_size ? raw_size : virtual_size;
        if (virtual_address > OpensmacxImageSize ||
            span > OpensmacxImageSize - virtual_address) {
            return fail("section '%s' spans %#010x+%#x, past the %#010x image",
                        name, virtual_address, span, OpensmacxImageSize);
        }
        if (raw_size != 0 && !pe.has(raw_pointer, raw_size)) {
            return fail("section '%s' raw data %#x+%#x is past end of file",
                        name, raw_pointer, raw_size);
        }

        unsigned char *destination = opensmacx_at(image_base + virtual_address);
        if (raw_size != 0) {
            std::memcpy(destination, file.data() + raw_pointer, raw_size);
        }

        // Everything past the raw data is zero, out to the end of the last
        // PAGE the section occupies - not to VirtualSize. Windows commits
        // whole pages, so .data's VirtualSize ends at 0x009c21f8 while the
        // committed section runs to 0x009c3000, and those last 3,592 bytes are
        // zero in the real mapping. Stopping at VirtualSize leaves them
        // holding whatever the previous load left there, which makes a reload
        // non-idempotent in exactly the region a cold start hides: the array
        // is zero-filled at program start, so the bug is invisible until
        // something loads twice.
        //
        // Zeroing is skipped where the RAW span is the longer one (.text has
        // 0x263000 raw against 0x2629c5 virtual): that padding is real mapped
        // content the loader keeps, and page-rounding it changes nothing since
        // raw sizes are already file-aligned.
        uint32_t zero_end =
            (span + section_alignment - 1U) & ~(section_alignment - 1U);
        if (zero_end < span || zero_end > OpensmacxImageSize - virtual_address) {
            // The rounded span leaves the image. Committing only what is
            // actually inside it is right; refusing would reject a file whose
            // final section legitimately rounds up past SizeOfImage.
            zero_end = OpensmacxImageSize - virtual_address;
        }
        // Never round into the next section: it may already have been mapped,
        // and section order in the table is not guaranteed to be by address.
        for (uint16_t other = 0; other < sections; ++other) {
            const uint32_t neighbour =
                pe.u32(table + static_cast<size_t>(other) * kSectionHeaderSize + 12);
            if (neighbour > virtual_address
                    && neighbour - virtual_address < zero_end) {
                zero_end = neighbour - virtual_address;
            }
        }
        if (zero_end > raw_size) {
            std::memset(destination + raw_size, 0, zero_end - raw_size);
        }
    }

    // --- base relocations ---------------------------------------------------
    // The load base IS ImageBase here - the flat array is based at 0x00400000
    // by construction and the check above refuses anything else - so the delta
    // is zero and every fixup would be a no-op. The table is therefore walked
    // only when the delta is non-zero, and the zero case is asserted rather
    // than assumed: g_relocations stays 0 and the verifier checks it.
    // The base relocation directory is entry 5, so reading it needs both that
    // many entries declared AND an optional header long enough to hold them -
    // NumberOfRvaAndSizes is attacker-controlled data, not a promise.
    const bool has_reloc_directory =
        directories > 5 && optional_size >= 96 + 6 * 8;
    g_reloc_rva = has_reloc_directory ? pe.u32(optional + 96 + 5 * 8) : 0;
    g_reloc_size = has_reloc_directory ? pe.u32(optional + 96 + 5 * 8 + 4) : 0;
    g_delta = static_cast<int32_t>(OpensmacxImageBase - image_base);
    g_relocations = 0;
    if (g_delta != 0) {
        if (const char *error =
                opensmacx_apply_relocations(g_delta, &g_relocations)) {
            return error;
        }
    }

    // --- stack --------------------------------------------------------------
    std::memset(opensmacx_at(OpensmacxStackGuardBase), 0xdd,
                OpensmacxStackGuardSize);

    g_entry_point = image_base + entry_rva;
    g_loaded = true;
    g_error[0] = '\0';
    return nullptr;
}

const char *opensmacx_apply_relocations(int32_t delta, uint32_t *applied) {
    uint32_t count = 0;
    if (applied != nullptr) {
        *applied = 0;
    }
    if (g_reloc_rva == 0 || g_reloc_size == 0) {
        return fail("the image is stripped of base relocations but needs a "
                    "%d byte fixup",
                    delta);
    }
    if (g_reloc_rva > OpensmacxImageSize ||
        g_reloc_size > OpensmacxImageSize - g_reloc_rva) {
        return fail("the base relocation directory falls outside the image");
    }
    const unsigned char *block = opensmacx_at(OpensmacxImageBase + g_reloc_rva);
    const unsigned char *end = block + g_reloc_size;
    while (block + 8 <= end) {
        uint32_t page = 0;
        uint32_t block_size = 0;
        std::memcpy(&page, block, 4);
        std::memcpy(&block_size, block + 4, 4);
        // A zero-sized block is the conventional terminator some linkers pad
        // the section with; anything else short is corruption.
        if (block_size == 0) {
            break;
        }
        if (block_size < 8 || block_size > static_cast<uint32_t>(end - block)) {
            return fail("malformed relocation block at page %#010x", page);
        }
        for (const unsigned char *cursor = block + 8;
             cursor + 2 <= block + block_size; cursor += 2) {
            uint16_t item = 0;
            std::memcpy(&item, cursor, 2);
            const unsigned int type = item >> 12;
            const uint32_t target = page + (item & 0x0fffU);
            if (type == 0) {  // IMAGE_REL_BASED_ABSOLUTE: pure padding.
                continue;
            }
            if (type != 3) {  // IMAGE_REL_BASED_HIGHLOW
                return fail("relocation type %u at %#010x, which a 32-bit "
                            "image should never contain",
                            type, OpensmacxImageBase + target);
            }
            if (target > OpensmacxImageSize - 4) {
                return fail("relocation target %#010x is outside the image",
                            OpensmacxImageBase + target);
            }
            uint32_t value = 0;
            unsigned char *slot = opensmacx_at(OpensmacxImageBase + target);
            std::memcpy(&value, slot, 4);
            value = static_cast<uint32_t>(value + static_cast<uint32_t>(delta));
            std::memcpy(slot, &value, 4);
            ++count;
        }
        block += block_size;
    }
    if (applied != nullptr) {
        *applied = count;
    }
    return nullptr;
}

void opensmacx_load_image_or_die(const char *path) {
    if (const char *error = opensmacx_load_image(path)) {
        std::fprintf(stderr, "opensmacx: %s\n", error);
        std::exit(2);
    }
}

bool opensmacx_image_loaded() { return g_loaded; }

uint32_t opensmacx_entry_point() { return g_entry_point; }

uint32_t opensmacx_relocations_applied() { return g_relocations; }

int32_t opensmacx_load_delta() { return g_delta; }

const char *opensmacx_image_sha256() { return g_loaded ? g_sha256 : ""; }

void opensmacx_init_stack(OpensmacxStaticRecompileState &state) {
    // 16 bytes of slack below the very top so that a lifted body reading a
    // stack argument at [esp+n] on entry stays inside the allocation instead
    // of one byte past it.
    state.esp = OpensmacxStackTop - 16U;
    state.ebp = 0;
}

bool opensmacx_stack_guard_intact() {
    const unsigned char *guard = opensmacx_at(OpensmacxStackGuardBase);
    for (uint32_t index = 0; index < OpensmacxStackGuardSize; ++index) {
        if (guard[index] != 0xdd) {
            return false;
        }
    }
    return true;
}
