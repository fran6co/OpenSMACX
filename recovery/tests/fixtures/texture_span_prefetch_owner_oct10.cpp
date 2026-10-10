#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

static int texture_span_pixels(int difference) {
    const unsigned int pixels = static_cast<unsigned int>(difference) & 0xffffu;
    return static_cast<int>(pixels - 1u < 0x8001u ? pixels : 1u);
}

// ACTUAL_HELPER_INSERTION
// ACTUAL_KERNEL_INSERTION
// ACTUAL_MAIN_INSERTION

int main() {
    return run_texture_span_prefetch_controls();
}
