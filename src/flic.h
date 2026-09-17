/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include "buffer.h"
#include "palette.h"

/*
 * The FLIC on-disk structures the decoder family reads.
 *
 * FliHeader is the file header. Flic::init fetches all 0x80 bytes at a
 * stroke into Flic::header_ (0x00629E50), checks the magic, and every
 * wrap or rewind decision after that reads the two ring offsets. Only
 * the fields a body addresses are named; the rest stays padding.
 *
 * DataChunk is a chunk header: a 4-byte size that counts the 6 header
 * bytes themselves, a 2-byte type, and the 2-byte subchunk count a frame
 * ring (type 0xF1FA) carries. The struct is 16 bytes rather than 6
 * because frame headers are fetched 0x10 at a time (0x0062A070,
 * 0x0062A2C0); the tail is slack those fetches fill and nothing reads.
 */
struct FliHeader {
    uint32_t size_;             // +0x00  size of the whole file
    uint16_t magic_;            // +0x04  0xAF12; Flic::init rejects the file otherwise
    uint16_t frames_;           // +0x06  frames in the animation ring
    uint16_t width_;            // +0x08
    uint16_t height_;           // +0x0A
    uint8_t reserved1_[0x44];   // +0x0C
    uint32_t first_frame_;      // +0x50  file offset of the first frame chunk
    uint32_t second_frame_;     // +0x54  file offset of the loop point
    uint8_t reserved2_[0x28];   // +0x58
};

static_assert(sizeof(FliHeader) == 0x80,
              "the file header Flic::init reads at a stroke is 128 bytes");

struct DataChunk {
    uint32_t size_;             // +0x00  chunk size including the 6-byte header
    uint16_t type_;             // +0x04  0xF1FA frame ring, 7 delta, 0xD black,
                                //       0xF byte run, 0x10 copy
    uint16_t count_;            // +0x06  subchunk count when type_ is 0xF1FA
    uint8_t slack_[8];          // +0x08  slack the 0x10-byte frame fetches fill
};

static_assert(sizeof(DataChunk) == 0x10,
              "frame headers are fetched sixteen bytes at a time");

 /*
  * Flic class
  *
  * No base class: the constructor's first call is straight to
  * `??0Buffer@@QAE@XZ` on `this + 4`, and its second is to
  * `??0Palette@@QAE@XZ` on `this + 0x5BC` - both ordinary members
  * auto-constructing in declaration order, not a base-class chain.
  */
class Flic {
 public:
  Flic();
  ~Flic();
  void close();
  void UNK4();
  void UNK5();
  void UNK6();
  void UNK7();
  void UNK8();
  int init(Buffer *buffer, char *file_name, int x, int y, int transparent);
  int decode_frame(int x, int y);
  int UNK1(int x, int y);
  int UNK2();
  int UNK3(int frame);
  int rewind();
  void decode_byte_run();
  void decode_delta_flc();
  void decode_byte_run_trans();
  void decode_delta_flc_trans();
  void decode_fli_copy();
  void decode_fli_copy_trans();
  void decode_black_trans();
  void decode_chunk(DataChunk *chunk);
  void fill();
  void UNK9();

 private:
  // Only the low byte is ever stored (`mov byte ptr [esi], bl`).
  uint8_t field_0_;  // 0x0
  Buffer buffer_;    // 0x4
  uint32_t field_58C_;
  uint32_t field_590_;
  uint32_t field_594_;
  uint32_t field_598_;
  uint32_t field_59C_;
  uint32_t field_5A0_;
  uint32_t field_5A4_;
  Buffer *buffer2_;
  uint32_t field_5AC_;
  uint32_t field_5B0_;
  uint32_t field_5B4_;
  FILE *file_;
  // A REAL embedded Palette, not a pointer: the constructor
  // placement-constructs it (`lea ecx,[esi+0x5bc]; call ??0Palette@@QAE@XZ`)
  // and the destructor tears it down the same way
  // (`call ??1Palette@@QAE@XZ`), both on `this + 0x5BC` directly.
  Palette palette_;  // 0x5BC, spans to 0xA10 (sizeof(Palette) == 0x454)
  // Was `field_5C0_[329]` before `palette_` became a real object; shifted by
  // sizeof(Palette) - 4 so the total extent this class already claimed
  // (0x5C0 + 329*4 == 0xAE4) is unchanged: 0xA10 + 53*4 == 0xAE4.
  // That blob is now split into the named members below; the extent is
  // unchanged and still ends at 0xAE4.
  uint32_t frame_;           // 0xA10  frames decoded since the last wrap
  FliHeader header_;         // 0xA14  the 128-byte file header
  uint8_t field_A94_[0x20];  // 0xA94  no decoded body addresses this gap
  DataChunk chunk_;          // 0xAB4  header of the frame chunk in flight
  DataChunk first_frame_;    // 0xAC4  saved first-frame chunk header
  DataChunk second_frame_;   // 0xAD4  saved loop-point chunk header
};



static_assert(sizeof(Flic) == 0xae4,
              "Flic size is the span flic1_ occupies in diplopop.h");