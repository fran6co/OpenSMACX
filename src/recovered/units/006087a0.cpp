// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x006087A0
// name           ?sort@StringList@@QAEXH@Z
// size           480 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006087a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?sort@StringList@@QAEXH@Z  at 0x006087A0  (480 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;

class StringList;
class StringStruct;
struct StringStructEntry;

struct StringStructEntry {
    uint32_t abi_word;
    int id;
    int payload;
    StringStructEntry * next;
    StringStructEntry * previous;
    uint32_t secondary_abi_word;
    void * allocation_owner;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
class StringStruct { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;
    int seek_pos(int);
};
extern "C" int abs(int);
extern "C" int strcmp(const char *, const char *);

class StringList { public:
    uint32_t primary_abi_word_;
    uint32_t virtual_base_abi_word_;
    StringStructEntry * head_;
    StringStructEntry * current_;
    int entry_count_;
    int current_position_;
    void * allocator_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t allocation_base_abi_word_;
    void * allocation_owner_;

    void sort(int);
};

void StringList::sort(int a1) {
    int i = 0;
    int count = entry_count_;
    int bound = count - 1;
    int nextI;
    if (bound > 0) {
        do {
            int pos;
            if (i <= bound) {
                current_ = head_;
                if (i < 0) {
                    count = entry_count_;
                    int n = abs(i);
                    if (count < n) goto after_seek;
                    n = abs(i);
                    if (n > 0) {
                        do {
                            n = n - 1;
                            current_ = current_->previous;
                        } while (n != 0);
                    }
                    count = entry_count_;
                    pos = count + i;
                } else {
                    int n = i;
                    pos = i;
                    if (i > 0) {
                        do {
                            n = n - 1;
                            current_ = current_->next;
                        } while (n != 0);
                    }
                }
                current_position_ = pos;
            }
after_seek:
            char *bestStr;
            if (head_ == 0) {
                bestStr = 0;
            } else {
                bestStr = *reinterpret_cast<char **>(current_->payload + 4);
            }
            nextI = i + 1;
            int bestPos = i;
            int j = nextI;
            if (nextI < count) {
                do {
                    if (head_ != 0) {
                        int newPos = current_position_ + 1;
                        current_ = current_->next;
                        current_position_ = newPos;
                        if (newPos == count) {
                            current_position_ = 0;
                        }
                    }
                    char *candStr;
                    if (current_ == 0) {
                        candStr = 0;
                    } else {
                        candStr = *reinterpret_cast<char **>((head_ ? current_->payload : 0) + 4);
                    }
                    count = strcmp(candStr, bestStr);
                    if (count < 0) {
                        if (head_ == 0) {
                            bestStr = 0;
                        } else {
                            bestStr = *reinterpret_cast<char **>(current_->payload + 4);
                        }
                        bestPos = current_position_;
                    }
                    count = entry_count_;
                    j = j + 1;
                } while (j < count);
                if (bestPos != i && bestPos <= count - 1) {
                    current_ = head_;
                    if (bestPos < 0) {
                        count = abs(bestPos);
                        if (entry_count_ < count) goto after_swap;
                        count = abs(bestPos);
                        if (count > 0) {
                            do {
                                count = count - 1;
                                current_ = current_->previous;
                            } while (count != 0);
                        }
                        bestPos = bestPos + entry_count_;
                    } else {
                        count = bestPos;
                        if (bestPos > 0) {
                            do {
                                count = count - 1;
                                current_ = current_->next;
                            } while (count != 0);
                        }
                    }
                    StringStructEntry *minNode = current_;
                    current_position_ = bestPos;
                    int ok = reinterpret_cast<StringStruct *>(this)->seek_pos(i);
                    if (ok != 0) {
                        StringStructEntry *node2 = current_;
                        unsigned id1 = static_cast<unsigned>(minNode->id) ^ static_cast<unsigned>(node2->id);
                        minNode->id = id1;
                        unsigned id2 = static_cast<unsigned>(node2->id) ^ id1;
                        node2->id = id2;
                        minNode->id = minNode->id ^ id2;
                        int tmpPayload = minNode->payload;
                        minNode->payload = node2->payload;
                        node2->payload = tmpPayload;
                    }
                }
            }
after_swap:
            count = entry_count_;
            bound = count - 1;
            i = nextI;
        } while (nextI < bound);
    }
}
