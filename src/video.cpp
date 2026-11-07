#include "video.h"
#include "wave.h"
#include "general.h"
#include "rectangle_operations.h"
#include <stdio.h>

// Each phase has its own original entry-message object. The process methods
// and registry pump share these with the corresponding standalone helper.
// The names and constness are current representations; full original bounds
// and original source declarations remain unproved.
namespace {
// RECOVERY_OBJECT: v1 0x00698870 _video_decode_phase_entering
const char video_decode_phase_entering[] = "Entering";
// RECOVERY_OBJECT: v1 0x00698894 _video_read_phase_entering
const char video_read_phase_entering[] = "Entering";
// read_frame's still-frame log names its own "read_frame()" copy; a pooled
// literal lets VC6 merge that push into the normal-frame tail.
// RECOVERY_OBJECT: v1 0x00698918 _video_read_frame_still_name
const char video_read_frame_still_name[] = "read_frame()";
}
// RECOVERY_IMPORT: v1 __imp__ICClose@4 MSVFW32.dll name:ICClose
// RECOVERY_IMPORT: v1 __imp__ICOpen@12 MSVFW32.dll name:ICOpen
// RECOVERY_IMPORT: v1 __imp__mmioDescend@16 WINMM.dll name:mmioDescend
// RECOVERY_IMPORT: v1 __imp__mmioSeek@12 WINMM.dll name:mmioSeek

// Real source-owned storage; original allocation extents beyond the observed
// API spans are unknown. No executable-address aliases or pointer stand-ins.
// RECOVERY_OBJECT: v2 0x9be640 0x9be668 ?VideoRegistry@@3PAPAVVideo@@A 0x636850
Video *VideoRegistry[10];
// RECOVERY_OBJECT: v2 0x9be68c 0x9be690 ?VideoRegistryCount@@3HA 0x636850
int VideoRegistryCount;
// RECOVERY_OBJECT: v2 0x9be668 0x9be678 ?VideoViewportCodec@@3UVideoViewportCodecParameters@@A 0x637530
VideoViewportCodecParameters VideoViewportCodec;
// RECOVERY_OBJECT: v2 0x9be678 0x9be688 ?VideoDirtyRect@@3UtagRECT@@A 0x637910
RECT VideoDirtyRect;
// Native log calls pass immediate C-string addresses, not loaded pointers.
// Only each initial NUL byte is evidenced; complete object extents are unknown.
// These minimum consumed spans do not infer ownership from four-byte spacing.
// RECOVERY_OBJECT: v2 0x9be690 0x9be691 ?VideoAdvanceEntryLogContext@@3PADA 0x6373a0
char VideoAdvanceEntryLogContext[1];
// RECOVERY_OBJECT: v2 0x9be698 0x9be699 ?VideoAdvanceVideoLogContext@@3PADA 0x6373a0
char VideoAdvanceVideoLogContext[1];
// RECOVERY_OBJECT: v2 0x9be694 0x9be695 ?VideoAudioBufferLogContext@@3PADA 0x6373a0
char VideoAudioBufferLogContext[1];

// ORIGINAL: 0x006381E0 Video_unk16 0x006381E0-0x006381E5 FILE BYTE_EXACT
// body src/video.h
// symbol    ??1VideoFrameQueue@@QAE@XZ
// LEVER: tail call to the unnamed sub via a plain extern "C" declaration


// ORIGINAL: 0x00636400 ??0Video@@QAE@XZ 0x00636400-0x0063652E;0x00663890-0x006638C9 FILE BYTE_EXACT
// symbol    ??0Video@@QAE@XZ
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memset)
#endif
Video::Video()
    : sound_state_(), palette_(), frame_queue_(), output_buffer_() {
  // Preserve constructor clear boundaries and order after subobject creation.
  log_reset();
  decode_output_ = 0;
  decode_source_ = 0;
  viewport_x_ = 0;
  viewport_y_ = 0;
  state_4a8_ = 0;
  state_1010_ = 0;
  state_1014_ = 0;
  flags_ = 0;
  frame_count_ = 0;
  buffered_frame_ = 0;
  target_frame_ = 0;
  media_file_ = 0;
  video_chunk_id_ = 0;
  audio_chunk_id_ = 0;
  codec_ = 0;
  memset(&child_chunk_, 0, sizeof child_chunk_);
  memset(&parent_chunk_, 0, sizeof parent_chunk_);
  memset(&codec_state_, 0, sizeof codec_state_);
  memset(&codec_state_tail_, 0, sizeof codec_state_tail_);
  frame_index_table_ = 0;
  current_index_record_ = 0;
  decode_output_flags_ = 0;
  last_tick_ = 0;
  memset(&source_rect_, 0, sizeof source_rect_);
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memset)
#endif

// ORIGINAL: 0x00636530 ??1Video@@QAE@XZ 0x00636530-0x006365A6;0x006638C9-0x00663902 FILE BYTE_EXACT
// symbol    ??1Video@@QAE@XZ
// TRIED: MISMATCH #1 push/mov - no SEH __try/__finally frame emitted; Video/Buffer/Palette have no dtor chain in the scaffold so the compiler treats it as trivial-unwind and skips the frame
Video::~Video() {
  close();
  // Compiler-owned destruction follows the native order: Buffer, queue,
  // Palette, then the Sound-derived prefix. The prefix's separate deleting
  // thunk is not this outer object's deletion path.
}

// The former FILE interval through 0x637310 was owner-run provenance.
// Entry-rooted control flow bounds close at 0x636732; later entries, switch
// data and padding are accounted separately without changing their identities.
// ORIGINAL: 0x006365B0 Video_unk1 0x006365B0-0x00636732 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned entry-rooted PE flow has103 reachable instructions ending at RET636731, exclusive636732; IDA and Ghidra agree independently. The next reset begins636740 after14NOPs.", "Historical2f1f617d:src/video.cpp assigned6365B0..637310 as an owner run. Later reset/register/load/process/play/stop/seek/rewind identities remain separate, and UNBOUND637190 retains its original identity/span/claim without implementation.", "The removed region also contains independently recognized padding and load636960 switch-target data636F9C..636FB4 referenced at6369D0; preserve that explicit source association, not close code. No C++ bodies, layout, flags or claims change."], "reason": "Bind Video::close to its complete reachable original cleanup body rather than the former multi-entry owner interval."}
// symbol    ?close@Video@@QAEXXZ
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memset)
#endif
void Video::close() {
  int registry_index = 0;
  while (registry_index < 10 && VideoRegistry[registry_index] != this)
    ++registry_index;
  if (registry_index < 10) {
    if (registry_index < 9) {
      int shift_index = registry_index;
      while (shift_index < 9) {
        VideoRegistry[shift_index] = VideoRegistry[shift_index + 1];
        ++shift_index;
      }
    }
    --VideoRegistryCount;
    if (VideoRegistryCount < 0) VideoRegistryCount = 0;
  }

  state_4a8_ = 0;
  state_1010_ = 0;
  state_1014_ = 0;
  frame_queue_.clear();
  flags_ &= 0xFFFF7FFFU;
  if ((flags_ & 1) == 0) sound_state_.release();
  flags_ = 0;
  palette_.close();

  if (frame_index_table_ != 0) {
    free(frame_index_table_);
    frame_index_table_ = 0;
    current_index_record_ = 0;
  }
  if (codec_ != 0) {
    ICSendMessage(codec_, ICM_DECOMPRESS_END, 0, 0);
    ICClose(codec_);
    codec_ = 0;
  }
  if (media_file_ != 0) {
    mmioClose(media_file_, MMIO_FHOPEN);
    media_file_ = 0;
  }

  buffered_frame_ = 0;
  target_frame_ = 0;
  frame_count_ = 0;
  video_chunk_id_ = 0;
  audio_chunk_id_ = 0;
  memset(&child_chunk_, 0, 0x14);
  memset(&parent_chunk_, 0, 0x14);
  decode_output_flags_ = 0;
  last_tick_ = 0;
  decode_output_ = 0;
  decode_source_ = 0;
  viewport_x_ = 0;
  viewport_y_ = 0;
  set_rectangle_dimensions(&source_rect_, 0, 0, 0, 0);
  output_buffer_.close();
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memset)
#endif

// RECOVERY_LITERAL: v1 0x0069887C 0x00698891 ??_C@_0BF@GKCP@decode_frame_process?$AA@ 0x00636FC0
// RECOVERY_LITERAL: v1 0x006988A0 0x006988B3 ??_C@_0BD@MDIC@read_frame_process?$AA@ 0x00636FC0
// ORIGINAL: 0x00636FC0 ?process@Video@@QAEXXZ 0x00636FC0-0x0063706F FILE BYTE_EXACT
// symbol    ?process@Video@@QAEXXZ
// LEVER: one 32-bit `flags` load reused for the first `& 0x8000`/`& 0x10` pair (matches the original's single `mov eax` feeding both `test ah,0x80` and `test al,0x10`), fresh byte reloads for every later `& 0x10` check (matches the repeated `mov al, [esi+0x4ac]` after each intervening call), and `while (r != 0 && (r = decode_frame(0)) != 0) r = play_frame();` for the tail loop. First try, byte exact.
void Video::process() {
  const unsigned int flags = flags_;
  if ((flags & 0x8000) != 0) {
    if ((flags & 0x10) == 0) {
      if (play_frame() != 0) return;
      log_say("decode_frame_process", video_decode_phase_entering,
              static_cast<int>(timeGetTime()), 0, 0);
      if ((static_cast<unsigned char>(flags_) & 0x10) == 0) {
        play_frame();
        if (decode_frame(0) != 0) return;
      }
    }
    log_say("read_frame_process", video_read_phase_entering,
            static_cast<int>(timeGetTime()), 0, 0);
    if ((static_cast<unsigned char>(flags_) & 0x10) == 0) {
      int result = play_frame();
      while (result != 0 && (result = decode_frame(0)) != 0) {
        result = play_frame();
      }
    }
    read_frame();
  }
}

// ORIGINAL: 0x006370E0 ?play@Video@@QAEHXZ 0x006370E0-0x0063718F FILE
// symbol    ?play@Video@@QAEHXZ
// TRIED: own VCall2 shim (35 slots) for slot34(int) since the scaffold's VCall types it nullary; cached TimeGetTimeFn across both timeGetTime() calls. MISMATCH #2 mov/push, early prologue register-save ordering only.
// LEVER: one shared `flags_ |= 0x8000; return 0;` tail after the if/else. VC6
//        duplicates it per arm, and that is what defers the edi save.
int Video::play() {
  if (media_file_ == 0) return 7;
  if ((static_cast<unsigned char>(flags_) & 0x10) != 0) {
    read_frame();
  } else {
    while (read_frame() != 0) {}
    decode_frame(0);
  }
  log_say("play()", "Starting", static_cast<int>(timeGetTime()), 0, 0);
  if ((static_cast<unsigned char>(flags_) & 1) == 0) {
    sound_state_.set_target_frame(target_frame_);
  } else {
    last_tick_ = timeGetTime();
  }
  flags_ |= 0x8000;
  return 0;
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: zero EAX and return; callee stack cleanup 8 bytes.
// ORIGINAL: 0x00637190 Video_unk7 0x00637190-0x00637195 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x00637190-0x00637195 zero EAX and return; callee stack cleanup 8 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x006370E0 ?play@Video@@QAEHXZ (src/video.cpp) and 0x006371A0 Video_unk8 (src/video.cpp)"]}
// symbol    ?Video_unk7@@YGHHH@Z
// kind      game
int __stdcall Video_unk7(int, int) {
    return 0;
}

// ORIGINAL: 0x00637870 ?decode_frame_process@Video@@QAEXXZ 0x00637870-0x006378B0 FILE BYTE_EXACT
// symbol    ?decode_frame_process@Video@@QAEHXZ
int Video::decode_frame_process() {
  log_say("decode_frame_process", video_decode_phase_entering,
          static_cast<int>(timeGetTime()), 0, 0);
  if ((static_cast<unsigned char>(flags_) & 0x10) != 0) return 0;
  play_frame();
  return decode_frame(0);
}

// ORIGINAL: 0x006378B0 ?read_frame_process@Video@@QAEXXZ 0x006378B0-0x00637906 FILE BYTE_EXACT
// symbol    ?read_frame_process@Video@@QAEXXZ
void Video::read_frame_process() {
  log_say("read_frame_process", video_read_phase_entering,
          static_cast<int>(timeGetTime()), 0, 0);
  if ((static_cast<unsigned char>(flags_) & 0x10) == 0) {
    int result = play_frame();
    while (result != 0 && (result = decode_frame(0)) != 0) {
      result = play_frame();
    }
  }
  read_frame();
}

// ORIGINAL: 0x006373A0 ?advance_frame@Video@@QAEHXZ 0x006373A0-0x00637526 FILE
// symbol    ?advance_frame@Video@@QAEHXZ
// TRIED: none confirmed byte-exact; MISMATCH #0 'push' vs 'sub' - the loop is a for(;;) here where the original threads the two 'return 1' failure exits through one shared epilogue that also differs slightly by which callee-saved registers it pops.
int Video::advance_frame() {
  if (media_file_ == 0) return 7;
  log_say("Entering advance_frame",
          VideoAdvanceEntryLogContext,
          static_cast<int>(timeGetTime()), 0, 0);
  if (child_chunk_.ckid != 0 && mmioAscend(media_file_, &child_chunk_, 0) != 0)
    return 1;
  child_chunk_.ckid = 0;
  while (mmioDescend(media_file_, &child_chunk_, &parent_chunk_, 0) == 0) {
    if (child_chunk_.ckid == video_chunk_id_) {
      ++buffered_frame_;
      log_say("Exiting advance_frame",
              VideoAdvanceVideoLogContext,
              static_cast<int>(timeGetTime()), 0, 0);
      return 0;
    }
    if (child_chunk_.ckid == audio_chunk_id_) {
      const int now = static_cast<int>(timeGetTime());
      log_say("advance_frame()", "Advancing audio.", now,
              child_chunk_.cksize, 0);
      VideoAudioBufferRecord *const record = static_cast<VideoAudioBufferRecord *>(
          sound_state_.acquire_audio_buffer(static_cast<int>(child_chunk_.cksize)));
      if (record != 0) {
        const int byte_count = static_cast<int>(child_chunk_.cksize);
        if (byte_count != mmioRead(media_file_,
                                   reinterpret_cast<HPSTR>(record->data),
                                   byte_count)) return 1;
        record->end = record->data + byte_count;
        sound_state_.submit_audio_buffer(record);
      } else {
        log_say("No mmio!!!",
                VideoAudioBufferLogContext, 0, 0, 0);
      }
    }
    if (child_chunk_.ckid != FOURCC_LIST &&
        mmioAscend(media_file_, &child_chunk_, 0) != 0) return 1;
  }
  return 1;
}

// ORIGINAL: 0x00637C30 ?read_frame@Video@@QAEHXZ 0x00637C30-0x00637D62 BYTE_EXACT
// symbol    ?read_frame@Video@@QAEHXZ
// TRIED: original pushes an extra callee-saved register (ecx-slot, ebx, esi, edi - 4 total) this body's control flow only needed 3 of (no ebx); not chased further. `sub_63e6e0` is called here as the scaffold's plain __stdcall(int,int,int,int), dropping the `lea ecx,[esi+0xa5c]` receiver the disassembly sets right before the call - Ghidra's own decompile drops the same thing (`the decompiler drops the this pointer on __thiscall functions`), so it reads as a genuine 5th (thiscall) param this body does not reproduce.
int Video::read_frame() {
  unsigned char *position;
  int result;
  if (child_chunk_.cksize == 0) {
    result = frame_queue_.allocate(&position, 0,
                                  static_cast<unsigned int>(buffered_frame_), 1);
    if (result != 0) return 0;
    log_say(video_read_frame_still_name, "Read data--still frame", buffered_frame_,
            frame_queue_.count(), 0);
  } else {
    result = frame_queue_.allocate(&position,
        static_cast<int>(child_chunk_.cksize),
        static_cast<unsigned int>(buffered_frame_), 0);
    if (result == 0x14) return 0;
    if (position == 0) {
      log_say("read_frame()", "Null data in fifo::add",
              static_cast<int>(timeGetTime()), 0, 0);
      return 0;
    }
    unsigned int remaining = child_chunk_.cksize;
    while (remaining != 0) {
      const unsigned int chunk = remaining > 0x10000 ? 0x10000 : remaining;
      const LONG bytes_read = mmioRead(media_file_,
          reinterpret_cast<HPSTR>(position), static_cast<LONG>(chunk));
      if (bytes_read == 0) {
        flags_ &= 0xFFFF7FFFU;
        return 0;
      }
      remaining -= static_cast<unsigned int>(bytes_read);
      position += bytes_read;
    }
    log_say("read_frame()", "Read data--normal frame", buffered_frame_,
            frame_queue_.count(), 0);
  }
  advance_frame();
  return 1;
}

// ORIGINAL: 0x00637910 ?decode_frame@Video@@QAEHH@Z 0x00637910-0x00637C26 FILE
// symbol    ?decode_frame@Video@@QAEHH@Z
// REWRITTEN 2026-11-07 from the image (0.22 -> 0.92): peek writes straight
//        into target_frame_/decode_output_flags_, the decode call is vfw.h's
//        inline ICDecompressEx (no memset), and the new rect goes through
//        set_rectangle_dimensions. Residual: ebx/ebp swapped between the
//        rect address and the HURRYUP flag.
// TRIED: MISMATCH #42 'mov' vs 'and' - transcribed the Ghidra decompile directly, sub_63e7f0/sub_63e820 redeclared to take the receiver (self+0xA5C) as an explicit int since a free-function thiscall can't be spelled without a helper class; matches structurally through the first ~40 instructions before diverging, not chased further.
int Video::decode_frame(int destination_index) {
  if (codec_ == 0) return 0;
  if ((flags_ & 0x1000) != 0) return 0;
  if (frame_queue_.count() == 0) return 0;

  if (viewport_y_ < 0) {
    viewport_y_ = 0;
  } else {
    const int doubled = flags_ & 4;
    int height = main_header_.dwHeight;
    const int compared_height = doubled ? height * 2 : height;
    const int output_height = output_buffer_.dib_.bmiHeader.biHeight;
    if (viewport_y_ > -(compared_height + output_height)) {
      if (doubled) height *= 2;
      viewport_y_ = -(output_height + height);
    }
  }

  // The queue writes the frame number and its flags straight into the
  // members the image passes (&target_frame_, &decode_output_flags_).
  const unsigned char *const frame_data =
      frame_queue_.peek(&target_frame_, &decode_output_flags_);
  if (target_frame_ == frame_count_ - 1) {
    flags_ &= 0xFFFF7FFFU;
  } else {
    {
      const int now = static_cast<int>(timeGetTime());
      log_say("decode_frame()", now, target_frame_, 0);
    }
    if ((decode_output_flags_ & 1) == 0) {
      if (frame_data == 0) {
        log_say("decode_frame()", "NULL data", 0, 0, 0);
        return 0;
      }
      if ((flags_ & 2) != 0 && decode_source_ != 0) {
        const int left = source_rect_.left;
        const int top = source_rect_.top;
        decode_source_->copy(&output_buffer_, left, top, left, top,
                            source_rect_.right - left,
                            source_rect_.bottom - top);
      }

      const DWORD decompress_flags = destination_index ? ICDECOMPRESS_HURRYUP : 0;
      const RECT previous = source_rect_;
      int x_offset = source_dib_.bmiHeader.biWidth;
      int y_offset = source_dib_.bmiHeader.biHeight;
      if ((flags_ & 4) != 0) {
        x_offset *= 2;
        y_offset *= 2;
      }
      int height = main_header_.dwHeight;
      if ((flags_ & 4) != 0) height <<= 1;

      // vfw.h's inline ICDecompressEx fills the ICDECOMPRESSEX block.
      if (ICDecompressEx(codec_, decompress_flags,
                         &source_dib_.bmiHeader,
                         const_cast<unsigned char *>(frame_data), 0, 0,
                         source_dib_.bmiHeader.biWidth,
                         source_dib_.bmiHeader.biHeight,
                         &output_buffer_.dib_.bmiHeader,
                         output_buffer_.get_data(), viewport_x_,
                         -(output_buffer_.dib_.bmiHeader.biHeight +
                           viewport_y_ + height),
                         x_offset, y_offset) != 0) {
        log_say("decode_frame()", "Decode failed!",
                static_cast<int>(timeGetTime()), 0, 0);
        return 0;
      }

      int width;
      int height_out;
      if ((flags_ & 4) != 0) {
        height_out = VideoViewportCodec.height * 2;
        width = VideoViewportCodec.width * 2;
      } else {
        height_out = VideoViewportCodec.height;
        width = VideoViewportCodec.width;
      }
      set_rectangle_dimensions(&source_rect_,
                               viewport_x_ + VideoViewportCodec.origin_x,
                               VideoViewportCodec.origin_y + viewport_y_,
                               width, height_out);
      UnionRect(&VideoDirtyRect, &previous, &source_rect_);
    }
    frame_queue_.discard_head();
    flags_ |= 0x1000;
  }
  return 1;
}

// RECOVERY_LITERAL: v1 0x00698978 0x00698981 ??_C@_08IBPL@Entering?$AA@ 0x00637D70
// ORIGINAL: 0x00637D70 ?play_frame@Video@@QAEHXZ 0x00637D70-0x0063808D FILE
// symbol    ?play_frame@Video@@QAEHXZ
// TRIED: prologue register order (large 797-byte function, many branches); landed as a structural transcription rather than chasing every register-allocation choice. Div-by-1000 modelled as plain unsigned int/1000 (VC6's own reciprocal-multiply idiom), not the 64-bit division Ghidra's decompile implied.
// REWRITTEN 2026-11-07 from the image block order (0.992): the remaining
//        diff is ecx vs eax for the merged `flags_ &= ~0x1000` temporaries.
int Video::play_frame() {
  int current_frame;
  if ((flags_ & 0x10) != 0) goto finish;

  if ((flags_ & 1) == 0) {
    current_frame = sound_state_.current_frame();
  } else {
    const unsigned int elapsed = timeGetTime() - last_tick_;
    const unsigned int scale = video_stream_header_.dwScale;
    const unsigned int rate = video_stream_header_.dwRate;
    current_frame = static_cast<int>((rate * elapsed) / scale / 1000);
  }
  if (current_frame < target_frame_) return 0;
  if (current_frame == target_frame_ && (flags_ & 0x1000) == 0) return 0;
  {
    const int now = static_cast<int>(timeGetTime());
    log_say("play_frame()", "Entering", now, current_frame - target_frame_, 0);
  }
  if (current_frame > target_frame_) {
    if (current_frame == target_frame_ + 1) {
      if (frame_queue_.count() != 0) {
        flags_ &= 0xFFFFEFFFU;
      } else {
        read_frame();
        flags_ &= 0xFFFFEFFFU;
        goto finish;
      }
    } else {
      const int nearest = next_key_frame(current_frame);
      if (nearest == -1) {
        while (target_frame_ < current_frame) {
          flags_ &= 0xFFFFEFFFU;
          if (decode_frame(0) == 0) {
            read_frame();
            decode_frame(0);
          }
        }
      }
      log_say("play_frame()", "BEHIND AUDIO", current_frame,
              target_frame_, nearest);
      if (current_frame - target_frame_ < nearest - current_frame) {
        while (target_frame_ < current_frame - 1) {
          flags_ &= 0xFFFFEFFFU;
          if (decode_frame(1) == 0) read_frame();
        }
        flags_ &= 0xFFFFEFFFU;
        if (decode_frame(0) != 0) goto finish;
        read_frame();
      } else {
        if (nearest > buffered_frame_ - 1) {
          frame_queue_.set_limit(0x28);
          while (buffered_frame_ < nearest) advance_frame();
          read_frame();
        } else {
          int queued_frame = 0;
          frame_queue_.peek(&queued_frame, 0);
          while (queued_frame < nearest) {
            frame_queue_.discard_head();
            frame_queue_.peek(&queued_frame, 0);
          }
        }
        flags_ &= 0xFFFFEFFFU;
      }
    }
    decode_frame(0);
  }

finish:
  if ((decode_output_flags_ & 1) == 0) {
    const int doubled = flags_ & 4;
    int height = main_header_.dwHeight;
    if (doubled) height <<= 1;
    int width = main_header_.dwWidth;
    if (doubled) width <<= 1;
    BitBlt(Win::get_hdc(), viewport_x_, viewport_y_, width, height,
           output_buffer_.get_hdc(), viewport_x_, viewport_y_, SRCCOPY);
    Win::release_hdc();
    output_buffer_.release_hdc(1);
  }
  if (target_frame_ == frame_count_ - 1) flags_ &= 0xFFFF7FFFU;
  flags_ &= 0xFFFFEFFFU;
  log_say("play_frame()", "Exiting", static_cast<int>(timeGetTime()), 0, 0);
  return 1;
}


// ORIGINAL: 0x00637310 Video_unk12 0x00637310-0x00637395 FILE BYTE_EXACT
// symbol    ?size_buffer@Video@@QAEHHH@Z
int Video::size_buffer(int width, int height) {
  if (decode_output_ != 0) {
    const RECT &rect = decode_output_->outer_rect();
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
  }
  const int result = output_buffer_.init(width, height, 0, 0);
  if (result != 0) return result;
  if (decode_output_ != 0) output_buffer_.sync_to_palette(PaletteActive);
  if (decode_source_ != 0)
    decode_source_->copy(&output_buffer_, 0, 0, width, height);
  return 0;
}

// ORIGINAL: 0x00638090 Video_unk14 0x00638090-0x00638128 FILE BYTE_EXACT
// symbol    ?previous_key_frame@Video@@QAEHH@Z
// TRIED: MISMATCH #0 mov/push, prologue register choice differs; the receiver is the Video object (ecx reads with no stack slot).
int Video::previous_key_frame(int frame) {
  if (media_file_ == 0 || frame_index_table_ == 0) return -1;
  if (frame >= frame_count_ || frame < 0) return -1;
  AVIINDEXENTRY *record = frame_index_table_;
  while (record->ckid != video_chunk_id_) ++record;
  for (int index = 0; index < frame; ++index) {
    ++record;
    while (record->ckid != video_chunk_id_) ++record;
  }
  while (frame > 0 && (record->dwFlags & 0x10) == 0) {
    --record;
    while (record->ckid != video_chunk_id_) --record;
    --frame;
  }
  // Native638114 rejects even a sync frame at index0; do not normalize it.
  if (frame == 0) return -1;
  current_index_record_ = record;
  return frame;
}

// ORIGINAL: 0x00638130 Video_unk15 0x00638130-0x006381DD FILE BYTE_EXACT
// symbol    ?next_key_frame@Video@@QAEHH@Z
int Video::next_key_frame(int frame) {
  if (media_file_ == 0 || frame_index_table_ == 0) return -1;
  if (frame >= frame_count_ || frame < 0) return -1;
  AVIINDEXENTRY *record = frame_index_table_;
  while (record->ckid != video_chunk_id_) ++record;
  if (frame > 0) {
    int remaining = frame;
    do {
      ++record;
      while (record->ckid != video_chunk_id_) ++record;
      --remaining;
    } while (remaining != 0);
  }
  while (frame < frame_count_ - 1 && (record->dwFlags & 0x10) == 0) {
    ++record;
    while (record->ckid != video_chunk_id_) ++record;
    ++frame;
  }
  if (frame == frame_count_ - 1) {
    if ((record->dwFlags & 0x10) != 0) {
      current_index_record_ = record;
      return frame;
    }
  } else if (frame != frame_count_) {
    current_index_record_ = record;
    return frame;
  }
  return -1;
}

// Native registration permits index 10: the eleventh store lands past the
// ten-entry registry (on codec origin_x at 0x9BE668), as the original does.
// ORIGINAL: 0x00636850 Video_unk3 0x00636850-0x006368C8 FILE BYTE_EXACT
// symbol    ?register_window@Video@@QAEHHHPAVGraphicWin@@I@Z
int Video::register_window(int x, int y, GraphicWin *window,
                           unsigned int flags) {
  if (window == 0) return 0x10;
  close();
  viewport_x_ = x;
  decode_output_ = window;
  viewport_y_ = y;
  flags_ = flags & 0xFFFF7FFFU;
  if ((flags_ & 0x20) != 0) flags_ |= 4;
  const int count = VideoRegistryCount;
  if (count > 10) return 1;
  VideoRegistry[count] = this;
  VideoRegistryCount = count + 1;
  return 0;
}

// ORIGINAL: 0x006368D0 Video_unk4 0x006368D0-0x00636954 FILE BYTE_EXACT
// symbol    ?register_window_buffer@Video@@QAEHHHPAVGraphicWin@@I@Z
int Video::register_window_buffer(int x, int y, GraphicWin *window,
                                  unsigned int flags) {
  if (window == 0) return 0x10;
  close();
  viewport_x_ = x;
  decode_output_ = window;
  decode_source_ = static_cast<Buffer *>(window);
  viewport_y_ = y;
  flags_ = flags & 0xFFFF7FFFU;
  if ((flags_ & 0x20) != 0) flags_ |= 4;
  const int count = VideoRegistryCount;
  if (count > 10) return 1;
  VideoRegistry[count] = this;
  VideoRegistryCount = count + 1;
  return 0;
}

// ORIGINAL: 0x00636300 ?do_video@@YAXXZ 0x00636300-0x006363EE FILE BYTE_EXACT
// symbol    ?do_video@@YAXXZ
// TRIED: nothing yet - MISMATCH #2 mov/push, mnemonic similarity 0.908. for-loop and do/while shaped identically; not chased further.
void __cdecl do_video() {
  // Native636300 inlines the same pipeline, but its actual call targets are
  // play_frame/decode_frame/read_frame, not Video::process. Keep those calls.
  // Reload the count after each iteration; callbacks can rethread the registry.
  for (int index = 0; index < VideoRegistryCount; ++index) {
    // Count 11 reads past the ten-entry registry, as the original does.
    Video *const video = VideoRegistry[index];
    if ((video->flags_ & 0x8000) == 0) continue;
    if ((video->flags_ & 0x10) == 0) {
      if (video->play_frame() != 0) continue;
      log_say("decode_frame_process", video_decode_phase_entering,
              static_cast<int>(timeGetTime()), 0, 0);
      if ((video->flags_ & 0x10) == 0) {
        video->play_frame();
        if (video->decode_frame(0) != 0) continue;
      }
    }
    log_say("read_frame_process", video_read_phase_entering,
            static_cast<int>(timeGetTime()), 0, 0);
    if ((video->flags_ & 0x10) == 0) {
      int result = video->play_frame();
      while (result != 0 && (result = video->decode_frame(0)) != 0)
        result = video->play_frame();
    }
    video->read_frame();
  }
}

// ORIGINAL: 0x006363F0 sub_6363f0 0x006363F0-0x006363FC BYTE_EXACT
// symbol    ?playback_flag@Video@@QBEHXZ
int Video::playback_flag() const { return flags_ & 0x8000U; }

// ORIGINAL: 0x006371A0 Video_unk8 0x006371A0-0x006371C2 FILE BYTE_EXACT
// symbol    ?stop@Video@@QAEXXZ
void Video::stop() {
  if ((flags_ & 1) == 0) sound_state_.stop();
  flags_ &= 0xFFFF7FFFU;
}

// ORIGINAL: 0x00637070 Video_unk6 0x00637070-0x006370D2 FILE BYTE_EXACT
// symbol    ?play_at@Video@@QAEHHH@Z
int Video::play_at(int x, int y) {
  viewport_x_ = x;
  viewport_y_ = y;
  while ((flags_ & 0x1000) == 0 && decode_frame(0) == 0 &&
         buffered_frame_ < frame_count_) read_frame();
  play_frame();
  return target_frame_;
}

// ORIGINAL: 0x00637300 Video_unk11 0x00637300-0x00637308 BYTE_EXACT
// symbol    ?rewind@Video@@QAEHXZ
int Video::rewind() { return seek(0); }

// ORIGINAL: 0x006371D0 Video_unk10 0x006371D0-0x006372F7 BYTE_EXACT
// symbol    ?seek@Video@@QAEHH@Z
// TRIED: reads at 0x4b0/0x4ac/0x100c/0x4c0/0x1000/0x1004 with the receiver cached to esi and no matching stack slot mean __thiscall on a Video object, but Video_unk14 (0x638090) and sub_63e540 (0x63e540) are BOTH called with the SAME receiver in ecx too - a second thiscall convention the scaffold's own nullary/stdcall fallback declarations for those two callees cannot express without conflicting with the declaration already in this translation unit. Landed the calls to those two exactly as the scaffold declared them (stdcall, no receiver), which compiles and keeps the call target addresses right but drops the ecx setup at those two call sites - a real, local divergence rather than a byte-exact ABI. The decode_frame/read_frame calls reuse Video's own catalogued declarations directly. mmioSeek/mmioDescend are read through their import-table globals with locally-declared signatures, since the scaffold does not carry MMIO types.
int Video::seek(int requested_frame) {
  if (media_file_ == 0) return 7;
  if ((flags_ & 1) == 0) return 0;
  const int frame = previous_key_frame(requested_frame);
  if (frame < 0) return 1;
  if (mmioSeek(media_file_, current_index_record_->dwChunkOffset +
      parent_chunk_.dwDataOffset, SEEK_SET) == -1) return 1;
  if (mmioDescend(media_file_, &child_chunk_, &parent_chunk_,
      MMIO_FINDCHUNK) != 0) return 1;
  frame_queue_.set_limit(40);
  buffered_frame_ = frame;
  target_frame_ = 0;
  if (requested_frame - 1 > 0) {
    do {
      flags_ &= 0xFFFFEFFFU;
      if (decode_frame(1) == 0) read_frame();
    } while (target_frame_ < requested_frame - 1);
  }
  flags_ &= 0xFFFFEFFFU;
  if (decode_frame(0) == 0) {
    read_frame();
    decode_frame(0);
  }
  return 0;
}

// ORIGINAL: 0x00636740 Video_unk2 0x00636740-0x0063684D FILE BYTE_EXACT
// symbol    ?reset@Video@@QAEXXZ
// TRIED: not chased to byte-exact (MISMATCH from instruction 1, 276 vs 269 bytes). Landed for the field-write order and call order, which match the disassembly one-for-one, including the `and ch,0x7f` / `test al,1` pair - a mask-after-copy on the SAME dword (`bit0` read before the 0x8000 bit is cleared).
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memset)
#endif
void Video::reset() {
  // Unlike close(), reset preserves registration, coordinates and flags
  // other than the playback bit. This distinction is load-bearing in load().
  state_4a8_ = 0;
  frame_queue_.clear();
  flags_ &= 0xFFFF7FFFU;
  if ((flags_ & 1) == 0) sound_state_.release();
  palette_.close();
  if (frame_index_table_ != 0) {
    free(frame_index_table_);
    frame_index_table_ = 0;
    current_index_record_ = 0;
  }
  if (codec_ != 0) {
    ICSendMessage(codec_, ICM_DECOMPRESS_END, 0, 0);
    ICClose(codec_);
    codec_ = 0;
  }
  if (media_file_ != 0) {
    mmioClose(media_file_, MMIO_FHOPEN);
    media_file_ = 0;
  }
  buffered_frame_ = 0;
  target_frame_ = 0;
  frame_count_ = 0;
  video_chunk_id_ = 0;
  audio_chunk_id_ = 0;
  memset(&child_chunk_, 0, 0x14);
  memset(&parent_chunk_, 0, 0x14);
  decode_output_flags_ = 0;
  last_tick_ = 0;
  set_rectangle_dimensions(&source_rect_, 0, 0, 0, 0);
  output_buffer_.close();
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memset)
#endif

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00636960 0x00636F9C 0x00636FB4 0x006369D0
// ORIGINAL: 0x00636960 Video_unk5 0x00636960-0x00636F99 FILE
// symbol    ?load@Video@@QAEHPADH@Z
// TRIED: __stdcall(int,int) free function - ecx is read as `this` and passed straight into Video::advance_frame(), so recast as a __thiscall Video member; the 6-entry `jmp [eax*4+0x636f9c]` table is data (not disassembled), so its case targets are unknown and every in-range reason falls through to the shared setup path instead of the (unrecoverable-without-the-bytes) per-case returns; field offsets and DirectDraw/AVI enumeration call order transcribed directly from the disasm.
int Video::load(char *filename, int unused) {
  (void)unused; // Native ret8; second argument is not read by this body.
  if (filename == 0) return 16;
  int status = 1;
  reset();
  MMIOINFO info;
  memset(&info, 0, sizeof info);
  media_file_ = mmioOpen(filename, &info, 0);
  if (media_file_ == 0) {
    // Actual image table636F9C, not the seed's missing-case fallback.
    switch (info.wErrorRet) {
    case 0x10B: return 6;
    case 0x110: return 3;
    default: return 1; // 10C..10F and all out-of-table error values.
    }
  }
  MMCKINFO chunk;
  AVIStreamHeader stream;
  int stream_index;
  int width, height;
  chunk.fccType = mmioFOURCC('A','V','I',' ');
  if (mmioDescend(media_file_, &chunk, 0, MMIO_FINDRIFF) != 0) goto fail;
  chunk.fccType = mmioFOURCC('h','d','r','l');
  if (mmioDescend(media_file_, &chunk, 0, MMIO_FINDLIST) != 0) goto fail;
  chunk.ckid = mmioFOURCC('a','v','i','h');
  if (mmioDescend(media_file_, &chunk, 0, MMIO_FINDCHUNK) != 0) goto fail;
  if (chunk.cksize < sizeof main_header_) goto fail;
  memset(&main_header_, 0, sizeof main_header_);
  // Native uses the input chunk length, even if greater than the header.
  // No unapproved file-validation repair has been introduced here.
  if (mmioRead(media_file_, reinterpret_cast<HPSTR>(&main_header_),
      chunk.cksize) != static_cast<LONG>(chunk.cksize)) goto fail;
  if (mmioAscend(media_file_, &chunk, 0) != 0) goto fail;
  if (main_header_.dwStreams > 2) goto fail;
  for (stream_index = 0;
       stream_index < static_cast<int>(main_header_.dwStreams); ++stream_index) {
    chunk.fccType = mmioFOURCC('s','t','r','l');
    if (mmioDescend(media_file_, &chunk, 0, MMIO_FINDLIST) != 0) goto fail;
    chunk.ckid = mmioFOURCC('s','t','r','h');
    if (mmioDescend(media_file_, &chunk, 0, MMIO_FINDCHUNK) != 0) goto fail;
    if (chunk.cksize > sizeof stream) goto fail;
    memset(&stream, 0, sizeof stream);
    if (mmioRead(media_file_, reinterpret_cast<HPSTR>(&stream), chunk.cksize)
        != static_cast<LONG>(chunk.cksize)) goto fail;
    if (mmioAscend(media_file_, &chunk, 0) != 0) goto fail;
    const int lo = stream_index & 15;
    const int hi = (stream_index >> 4) & 15;
    const DWORD digits = (static_cast<DWORD>(lo > 9 ? lo + 0x37 : lo + 0x30) << 8)
        | static_cast<DWORD>(hi > 9 ? hi + 0x37 : hi + 0x30);
    if (stream.fccType == mmioFOURCC('v','i','d','s')) {
      memcpy(&video_stream_header_, &stream, sizeof stream);
      video_chunk_id_ = 0x63640000UL | digits;
    } else if (stream.fccType == mmioFOURCC('a','u','d','s')) {
      memcpy(&audio_stream_header_, &stream, sizeof stream);
      audio_chunk_id_ = 0x62770000UL | digits;
    }
    chunk.ckid = mmioFOURCC('s','t','r','f');
    if (mmioDescend(media_file_, &chunk, 0, MMIO_FINDCHUNK) != 0) goto fail;
    if (stream.fccType == mmioFOURCC('a','u','d','s')) {
      if (chunk.cksize > sizeof audio_format_) goto fail;
      memset(&audio_format_, 0, sizeof audio_format_);
      if (mmioRead(media_file_, reinterpret_cast<HPSTR>(&audio_format_), chunk.cksize)
          != static_cast<LONG>(chunk.cksize)) goto fail;
    } else if (stream.fccType == mmioFOURCC('v','i','d','s')) {
      if (chunk.cksize > sizeof source_dib_) goto fail;
      memset(&source_dib_, 0, sizeof source_dib_);
      if (mmioRead(media_file_, reinterpret_cast<HPSTR>(&source_dib_), chunk.cksize)
          != static_cast<LONG>(chunk.cksize)) goto fail;
    }
    if (mmioAscend(media_file_, &chunk, 0) != 0) goto fail;
  }
  if ((flags_ & 1) == 0) {
    if (audio_chunk_id_ != 0 && sound_state_.configure_streams(
        &audio_stream_header_, &video_stream_header_, &audio_format_, 0) == 0)
      flags_ &= ~1U;
    else flags_ |= 1;
  }
  if (mmioSetBuffer(media_file_, 0, 0x7D000, 0) != 0) goto fail;
  parent_chunk_.fccType = mmioFOURCC('m','o','v','i');
  if (mmioDescend(media_file_, &parent_chunk_, 0, MMIO_FINDLIST) != 0) goto fail;
  chunk.ckid = mmioFOURCC('i','d','x','1');
  if (mmioDescend(media_file_, &chunk, 0, MMIO_FINDCHUNK) != 0) goto fail;
  frame_index_table_ = static_cast<AVIINDEXENTRY *>(mem_get(chunk.cksize));
  if (frame_index_table_ == 0) { status = 4; goto fail; }
  if (mmioRead(media_file_, reinterpret_cast<HPSTR>(frame_index_table_), chunk.cksize)
      != static_cast<LONG>(chunk.cksize)) goto fail;
  current_index_record_ = frame_index_table_;
  // Native636E3B shifts the BYTE length by4, then scans16-byte records.
  // This is an observed invalid-input bound, not inferred AVI entry count.
  {
    const size_t native_scan_bytes = static_cast<DWORD>(chunk.cksize << 4);
    size_t index_offset = 0;
    while (index_offset < native_scan_bytes &&
           static_cast<WORD>(current_index_record_->ckid) !=
           static_cast<WORD>(video_chunk_id_)) {
      ++current_index_record_;
      index_offset += sizeof(AVIINDEXENTRY);
    }
  }
  video_chunk_id_ = current_index_record_->ckid;
  if (mmioSeek(media_file_, parent_chunk_.dwDataOffset + 4, SEEK_SET) == -1) goto fail;
  frame_queue_.set_limit(40);
  status = advance_frame();
  if (status != 0) goto fail;
  buffered_frame_ = target_frame_ = 0;
  width = main_header_.dwWidth;
  height = main_header_.dwHeight;
  if (decode_output_ != 0) {
    const RECT &rect = decode_output_->outer_rect();
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
  }
  status = output_buffer_.init(width, height, 0, 0);
  if (status != 0) goto fail;
  if (decode_output_ != 0) output_buffer_.sync_to_palette(PaletteActive);
  if (decode_source_ != 0) decode_source_->copy(&output_buffer_, 0, 0, width, height);
  status = configure_codec();
  if (status != 0) goto fail;
  frame_count_ = video_stream_header_.dwLength;
  return 0;
fail:
  close();
  return status;
}

// ORIGINAL: 0x00637530 Video_unk13 0x00637530-0x00637865 FILE
// symbol    ?configure_codec@Video@@QAEHXZ
// TRIED: __thiscall Video member with plain if/else transcription of the Ghidra decomp; matches through instr #31, diverges at the byte-shuffle loop / trailing 0x34-byte ICSendMessage struct, whose exact field layout is not recovered
// LEVER: the trailing block is vfw.h's inline ICDecompressExBegin; the colour
//        loop copies rgbRed, rgbGreen, rgbBlue in that order; the 0xC test is
//        two flag tests VC6 merges; mode is stored before selector.
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
int Video::configure_codec() {
  if (media_file_ == 0) return 7;
  if (codec_ != 0) {
    ICSendMessage(codec_, ICM_DECOMPRESS_END, 0, 0);
    ICClose(codec_);
  }
  codec_ = ICOpen(mmioFOURCC('v','i','d','c'), video_stream_header_.fccHandler,
                  ICMODE_DECOMPRESS);
  if (codec_ == 0) return 1;
  Dib destination = output_buffer_.dib_;
  if (ICSendMessage(codec_, ICM_DECOMPRESS_GET_PALETTE,
      reinterpret_cast<DWORD>(&source_dib_),
      reinterpret_cast<DWORD>(&destination)) != 0) return 1;
  if ((flags_ & 4) == 0 && (flags_ & 8) == 0) {
    // Native uses the output DIB's original BGR colours, not the codec's
    // palette-query result. The query result's fourth byte is preserved.
    for (int palette_index = 0; palette_index < 256; ++palette_index) {
      destination.bmiColors[palette_index].rgbRed =
          output_buffer_.dib_.bmiColors[palette_index].rgbRed;
      destination.bmiColors[palette_index].rgbGreen =
          output_buffer_.dib_.bmiColors[palette_index].rgbGreen;
      destination.bmiColors[palette_index].rgbBlue =
          output_buffer_.dib_.bmiColors[palette_index].rgbBlue;
    }
    memcpy(&destination.bmiHeader, &output_buffer_.dib_.bmiHeader,
           sizeof(BITMAPINFOHEADER));
    ICSendMessage(codec_, ICM_DECOMPRESS_SET_PALETTE,
                  reinterpret_cast<DWORD>(&destination), 0);
  } else if (decode_output_ != 0) {
    // The original local Dib has been initialized by the copy/query above.
    // Native passes its colour subspan even when the manual shuffle is skipped.
    palette_.init();
    palette_.set_rgbquad(destination.bmiColors, 0, 256);
    palette_.set();
    output_buffer_.sync_to_palette(&palette_);
    if (decode_source_ != 0) decode_source_->sync_to_palette(&palette_);
  }
  if ((flags_ & 2) != 0) {
    codec_state_tail_.byte_size = 0x2C;
    codec_state_tail_.handler = video_stream_header_.fccHandler;
    codec_state_tail_.version = 0x10001;
    codec_state_tail_.mode = 2;
    codec_state_tail_.selector = 4;
    codec_state_tail_.flags = 0x80000004UL;
    codec_state_tail_.option_24 = 0;
    if (ICSendMessage(codec_, 0x5001, reinterpret_cast<DWORD>(&codec_state_tail_), 0x2C)
        != 0) return 1;
  }
  codec_state_.byte_size = 0x54;
  codec_state_.handler = mmioFOURCC('I','V','4','1');
  codec_state_.version = 0x10001;
  codec_state_.selector = codec_state_.mode = 2;
  codec_state_.flags = 0x80000008UL;
  codec_state_.viewport = &VideoViewportCodec;
  if (ICSendMessage(codec_, 0x5001, reinterpret_cast<DWORD>(&codec_state_), 0x54)
      != 0) return 1;
  if ((flags_ & 0x20) != 0) {
    codec_state_tail_.byte_size = 0x2C;
    codec_state_tail_.handler = video_stream_header_.fccHandler;
    codec_state_tail_.version = 0x10001;
    codec_state_tail_.mode = 2;
    codec_state_tail_.selector = 4;
    codec_state_tail_.flags = 0x80000008UL;
    codec_state_tail_.option_28 = 1;
    ICSendMessage(codec_, 0x5001, reinterpret_cast<DWORD>(&codec_state_tail_), 0x2C);
  }
  int width = source_dib_.bmiHeader.biWidth;
  int height = source_dib_.bmiHeader.biHeight;
  if ((flags_ & 4) != 0) {
    width *= 2;
    height *= 2;
  }
  // vfw.h's inline ICDecompressExBegin fills the ICDECOMPRESSEX block.
  return ICDecompressExBegin(codec_, 0, &source_dib_.bmiHeader, 0, 0, 0,
                             source_dib_.bmiHeader.biWidth,
                             source_dib_.bmiHeader.biHeight,
                             &destination.bmiHeader, 0, VideoDirtyRect.left,
                             VideoDirtyRect.top, width, height) != 0;
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif
