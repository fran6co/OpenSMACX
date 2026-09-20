/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 *
 * OpenSMACX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once

// Polygon coordinates are signed 32-bit integers. Texture::draw_trans
// reads Y at +4 (0x0061A31E), X at +0 (0x0061A33B), compares both with
// signed branches, and advances by eight bytes at 0x0061A359. Independently,
// polygon scans Y at +4 with signed comparisons and the same eight-byte
// stride at 0x006266BC..0x006266DE. The original PAUVert@@ APIs name a struct.
struct Vert {
    // RECOVERY_MEMBER: v1 Vert x 0x0 0x0061A240
    int x;
    // RECOVERY_MEMBER: v1 Vert y 0x4 0x0061A240
    int y;
};
