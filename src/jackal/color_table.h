#pragma once

// One source palette index selects a row of 256 destination palette indices.
// Sprite5E9289..5E928D shows source -> BH, destination -> BL, table[EBX].
// The pointer type expresses the row stride; it does not add index checking.
typedef unsigned char ColorTable[256];
