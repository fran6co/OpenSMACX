/*
 * <stdint.h> for cl 12.00.8168, which predates C99 and so does not ship one.
 *
 * A SHIM DIRECTORY RATHER THAN AN EDIT TO EACH INCLUDER. The tree's `#include
 * <stdint.h>` lines are correct C++ and stay correct when this compiler is
 * eventually retired; putting the header VC6 is missing on its include path
 * fixes them all without a single source change to undo later. The directory
 * is attached to the target only when the compiler is old enough to need it,
 * so a modern build never sees this file and keeps its own.
 *
 * The typedefs themselves live in vc6_compat.h, next to the rest of what this
 * compiler lacks, so there is exactly one definition of each and no way for
 * the two to drift apart.
 */
#pragma once

#include "vc6_compat.h"
