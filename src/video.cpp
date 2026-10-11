// RECOVERY_GENERATED: v1 vtable 0x66e790 0x66e82c ??_7VideoSoundState@@6B@ 0x004c8620
// Video playback, composed resources and Sound-derived audio ownership.
// The Sound prefix owns table 0x66E790 and its own deleting thunk 0x4C9360.
// Do not add an outer virtual deleter: that thunk calls prefix 0x4C86D0 only.
#include "video.h"
#include "wave.h"
#include "general.h"
#include <stdio.h>
// RECOVERY_IMPORT: v1 __imp__ICClose@4 MSVFW32.dll name:ICClose
// RECOVERY_IMPORT: v1 __imp__ICOpen@12 MSVFW32.dll name:ICOpen

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

// ORIGINAL: 0x004C8620 sub_4c8620 0x004C8620-0x004C86C9;0x004C8450-0x004C8457;0x00659F5E-0x00659F70 FILE
// RECOVERY_BINDING: v1 {"evidence": ["image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf native636400/636530 member construction/destruction;66E790 prefix table and4C9360->4C86D0.", "native636960 full loader jump table636F9C;637530 SDK and Indeo packet calls.", "Native spans and all historical claims/lessons retained; close6365B0 broad-span and4C8910 truncated-span debts remain explicit."], "reason": "Video owns composed Sound prefix, real Palette/queue/Buffer members, SDK media resources and native observed source globals; scalar deletion belongs only to the prefix."}
// symbol    ??0VideoSoundState@@QAE@XZ
// TRIED: byte-exact - the SEH frame prologue (push -1/handler; mov fs:[0]) has no C++ source form to reproduce without a real try/catch that this Sound-constructor body doesn't need (documented wall). Landed as a __thiscall ctor-shaped member returning `this`, with the progressive vtable_storage_ reassignment (0x66e444 -> 0x66e3c0 -> 0x66e790) and the self-clobbering dead `if (device_ != 0)` check (device_ is zeroed immediately before the read) transcribed as-is.
VideoSoundState::VideoSoundState() : Sound() {
  // Native 0x4C86AB installs the derived table before 0x4C86B1 calls
  // Sound::set_type(4). Compiler-owned construction supplies these table stages.
  Sound::set_type(4);
}

// ORIGINAL: 0x004C86D0 sub_4c86d0 0x004C86D0-0x004C87A2;0x004C8450-0x004C8457;0x00659F70-0x00659F8A FILE
// symbol    ??1VideoSoundState@@UAE@XZ
// TRIED: literal `call 0x64557f`/SEH __try/__finally reproduction - the span at 0x004C8450-0x004C8457 is COMDAT-folded with src/wave.cpp's Wave::~Wave and src/ambience.cpp's Ambience::~Ambience (both list it in their own spans too), so classify_body reports SHARED_TAIL for this address regardless of body content - no per-function verdict is well defined here, confirmed by running the bare verifier against the empty placeholder before writing anything.
VideoSoundState::~VideoSoundState() {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    device->destroy_device(1);
    device_ = 0;
  }
  // Sound::~Sound then owns filename/device-release/chain teardown and the
  // compiler emits base/root vtable descent. Do not duplicate Sound cleanup.
}

// RECOVERY_SHARED: v1 0x406b30 0x406b33 ?unload@VideoSoundState@@UAEHXZ 0x4c8620
int VideoSoundState::unload() {
  return 0;
}

// ORIGINAL: 0x004C88B0 sub_4c88b0 0x004C88B0-0x004C88BF BYTE_EXACT
// symbol    ?stop@VideoSoundState@@UAEHXZ
int VideoSoundState::stop() {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    device->stop();
  }
  return 0;
}

// ORIGINAL: 0x004C8840 sub_4c8840 0x004C8840-0x004C8888 FILE BYTE_EXACT
// symbol    ?release@VideoSoundState@@UAEHXZ
// LEVER: the g_0090db28 function pointer is __cdecl, not __stdcall - the original has `add esp,4` after that call; __stdcall dropped it and cost a MISMATCH at the trailing `add`/`jmp`.
int VideoSoundState::release() {
  if (device_ != 0) {
    if (static_cast<VideoSoundDevice *>(device_)->can_release() == 0) {
      if (WaveDeviceReleaseGuard) {
        (SoundXOrdinal2DeleteSound())(device_);
      }
    } else {
      // The query can call back into the owner. Native 4C886B re-reads
      // the field; do not retain the pre-query device across that callback.
      static_cast<VideoSoundDevice *>(device_)->release();
    }
    device_ = 0;
  }
  detach();
  return 0;
}

// RECOVERY_SHARED: v1 0x406b30 0x406b33 ?get_game_hwnd@VideoSoundState@@UAEHXZ 0x4c8620
int VideoSoundState::get_game_hwnd() {
  return 0;
}

// RECOVERY_SHARED: v1 0x4c93e0 0x4c93e8 ?set_attrib@VideoSoundState@@UAEHK@Z 0x4c8620
int VideoSoundState::set_attrib(unsigned long) {
  return 0x0B;
}

// RECOVERY_SHARED: v1 0x406b30 0x406b33 ?get_attrib@VideoSoundState@@UAEHXZ 0x4c8620
int VideoSoundState::get_attrib() {
  return 0;
}

// ORIGINAL: 0x004C8950 sub_4c8950 0x004C8950-0x004C8953 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; VideoSoundState primary table 0x0066E790 slot 29 at 0x0066E804 relocates to 0x004C8950", "the pre-proposal src/video.cpp RECOVERY_SHARED record already bound this exact span to VideoSoundState::get_time; this replaces that one shared record with the canonical identity", "src/video.cpp owns the actual int VideoSoundState::get_time() method body returning zero; no Time class or global timer ownership is inferred", "historical catalog facts retained: sub_4c8950 is a three-byte zero-return leaf; its name, span and claim state are carried from the parsed source record by the native writer", "retired source symbol retained as historical evidence: ?nullsub_004c8950_redirect@@YIHPAX0@Z"], "reason": "Promote the existing VideoSoundState::get_time shared-body mapping to the unique canonical identity. Keep its real typed method and avoid a duplicate annotation or adapter."}
// symbol    ?get_time@VideoSoundState@@UAEHXZ
// kind      game
int VideoSoundState::get_time() {
  return 0;
}

// ORIGINAL: 0x004C87B0 sub_4c87b0 0x004C87B0-0x004C8839 FILE
// symbol    ?configure_streams@VideoSoundState@@UAEHPBX00H@Z
int VideoSoundState::configure_streams(const void *audio_stream,
                                       const void *video_stream,
                                       const void *audio_format,
                                       int flags) {
  VideoSoundDevice *device =
      static_cast<VideoSoundDevice *>(device_);
  int result = 0;
  if (device == 0) {
    if (!WaveDeviceReleaseGuard) {
      return 1;
    }
    result = (SoundXOrdinal1CreateSound())(&device_, 0, 4);
    if (result != 0) {
      return result;
    }
    device = static_cast<VideoSoundDevice *>(device_);
  }

  // Native 0x4C87E9..0x4C87ED allows success without a device object.
  if (device == 0) return result;

  result = device->configure_streams(audio_stream, video_stream,
                                     audio_format, flags);
  if (result != 0) {
    if (WaveDeviceReleaseGuard) {
      (SoundXOrdinal2DeleteSound())(device_);
    }
    device_ = 0;
  }

  attach();
  return result;
}

// ORIGINAL: 0x004C8890 sub_4c8890 0x004C8890-0x004C88AC FILE BYTE_EXACT
// symbol    ?set_target_frame@VideoSoundState@@UAEHH@Z
int VideoSoundState::set_target_frame(int frame) {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    device->set_target_frame(frame);
  }
  // Native wrapper 0x4C8890 discards the device result and returns zero.
  return 0;
}

// ORIGINAL: 0x004C88C0 sub_4c88c0 0x004C88C0-0x004C88E0 BYTE_EXACT
// symbol    ?acquire_audio_buffer@VideoSoundState@@UAEPAXH@Z
void *VideoSoundState::acquire_audio_buffer(int bytes) {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    return device->acquire_audio_buffer(bytes);
  }
  return 0;
}

// ORIGINAL: 0x004C88E0 sub_4c88e0 0x004C88E0-0x004C8903 FILE BYTE_EXACT
// symbol    ?submit_audio_buffer@VideoSoundState@@UAEHPAX@Z
int VideoSoundState::submit_audio_buffer(void *record) {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    return device->submit_audio_buffer(record);
  }
  return 0x13;
}

// ORIGINAL: 0x004C8910 sub_4c8910 0x004C8910-0x004C8923 FILE
// symbol    ?current_frame@VideoSoundState@@UAEHXZ
// TRIED: the tail-called vtable slot37 matches exactly (mov/test/je/mov/jmp, 5/5 mnemonics), so the divergence is entirely the null branch: the original's `return -1` JUMPS OUT of this function into a shared tail block at 0x4C891F, four bytes past its own 15, and no C++ spelling reaches another function's interior. This body inlines `or eax,-1; ret` instead and is 4 bytes long. Not a lever - it names why the remainder cannot close from here.
// Original4C8915's null branch selects4C891F..4C8923 directly. That
// four-byte return tail belongs to this complete source behavior; the IDA
// underflow label does not establish another function. The historical TRIED
// interpretation above is retained, not a current boundary proof.
int VideoSoundState::current_frame() {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    return device->current_frame();
  }
  // Native4C891F..22: no device is a -1 frame, not zero.
  return -1;
}

// ORIGINAL: 0x004C8930 sub_4c8930 0x004C8930-0x004C8942 BYTE_EXACT
// symbol    ?frame_status@VideoSoundState@@UAEHXZ
int VideoSoundState::frame_status() {
  VideoSoundDevice *const device =
      static_cast<VideoSoundDevice *>(device_);
  if (device != 0) {
    return device->frame_status();
  }
  return 0;
}

// ORIGINAL: 0x0063E520 sub_63e520 0x0063E520-0x0063E534 BYTE_EXACT
// symbol    ??0VideoFrameQueue@@QAE@XZ
VideoFrameQueue::VideoFrameQueue() : head_(0), tail_(0), count_(0), limit_(-1) {}
// ORIGINAL: 0x006381E0 Video_unk16 0x006381E0-0x006381E5 FILE BYTE_EXACT
// symbol    ??1VideoFrameQueue@@QAE@XZ
// LEVER: tail call to the unnamed sub via a plain extern "C" declaration
VideoFrameQueue::~VideoFrameQueue() { clear(); }
// ORIGINAL: 0x0063E540 sub_63e540 0x0063E540-0x0063E58F FILE
// symbol    ?set_limit@VideoFrameQueue@@QAEXH@Z
void VideoFrameQueue::set_limit(int limit) { clear(); limit_ = limit; }

// ORIGINAL: 0x0063E6E0 sub_63e6e0 0x0063E6E0-0x0063E7EE FILE
// symbol    ?allocate@VideoFrameQueue@@QAEHPAPAEHHI@Z
// TRIED: the given definition head (`__stdcall`, no `this`) contradicts the disassembly - `ecx` does real field work (`self[0..3]`) with no matching stack slot, so this is really a `__thiscall` member with 4 stack params, not a 4-arg stdcall free function. Rewritten as a member on a synthetic class - landed FILE-mode because BODY mode's naive brace-balance extraction (`extract_forward_text`) stops at the first top-level `{...}` it meets, which is the class's own declaration block, truncating the out-of-line method definition entirely (measured: NO_COMPILE, "found 0" external symbols). Reproduces the redundant `if (headOld != 0)` re-test before the second `free()` (the original keeps it even though it is statically known true) since dropping it did not change the divergence. Landed at mnemonic_similarity 0.971, byte count already matches (270/270); only the free-path register scheduling around instruction 79 still differs.
int VideoFrameQueue::allocate(unsigned char **payload_out, int byte_count,
                              int frame_id, unsigned int frame_flags) {
    if (payload_out == 0) return 3;
    *payload_out = 0;
    if (count_ == limit_) return 0x14;

    VideoFrameNode *node;
    if (head_ == 0) {
        node = static_cast<VideoFrameNode *>(mem_get(sizeof(VideoFrameNode)));
        head_ = node;
        if (node == 0) return 4;
    } else {
        VideoFrameNode *const appended = static_cast<VideoFrameNode *>(mem_get(sizeof(VideoFrameNode)));
        tail_->next = appended;
        node = tail_->next;
        if (node == 0) return 4;
    }
    tail_ = node;
    node->next = 0;
    node->frame_flags = frame_flags;
    node->frame_id = frame_id;
    // Native zero-byte branch leaves bytes/byte_count untouched.
    // Optional initialization is NOT authorized; malloc state may be invalid.
    if (byte_count != 0) {
        node->bytes = static_cast<unsigned char *>(mem_get(byte_count));
        if (node->bytes == 0) {
            discard_head();
            return 4;
        }
        node->byte_count = static_cast<unsigned int>(byte_count);
    }
    ++count_;
    *payload_out = node->bytes;
    return 0;
}

// ORIGINAL: 0x0063E5E0 sub_63e5e0 0x0063E5E0-0x0063E6DD FILE
// symbol    ?append@VideoFrameQueue@@QAEHPBEHHI@Z
// TRIED: real `__thiscall` member (C0063e5e0::fill(int,int,int,int)) - `this[2]==this[3]` full-guard, mem_get(0x14) in BOTH the empty-list and grow-list arms matching the ASM exactly, a linked-list node {id,val,buf,buflen,next} appended at `this[1]`, and the alloc-failure path unwinding the node it just linked (free buf, unlink, free node, count--). Reproduces the whole body; MISMATCH #3 is the same "no ebp frame" prologue wall seen across this batch (0x005D4890, 0x005DEA20) plus a `cmp [mem],reg` vs `mov reg,[mem]; cmp reg,reg` operand-form choice neither a named local nor inlining changed.
int VideoFrameQueue::append(const unsigned char *source, int byte_count,
                            int frame_id, unsigned int frame_flags) {
    if (count_ == limit_) return 0x14;
    VideoFrameNode *node;
    if (head_ == 0) {
        node = static_cast<VideoFrameNode *>(mem_get(sizeof(VideoFrameNode)));
        head_ = node;
        if (node == 0) return 4;
    } else {
        tail_->next = static_cast<VideoFrameNode *>(mem_get(sizeof(VideoFrameNode)));
        node = tail_->next;
        if (node == 0) return 4;
    }
    tail_ = node;
    node->next = 0;
    node->frame_flags = frame_flags;
    node->frame_id = frame_id;
    if (byte_count != 0) {
        node->bytes = static_cast<unsigned char *>(mem_get(byte_count));
        if (node->bytes == 0) {
            discard_head();
            return 4;
        }
        if (source != 0) memcpy(node->bytes, source, byte_count);
        node->byte_count = static_cast<unsigned int>(byte_count);
    }
    ++count_;
    return 0;
}

// ORIGINAL: 0x0063E7F0 sub_63e7f0 0x0063E7F0-0x0063E81C SEMANTIC
// symbol    ?peek@VideoFrameQueue@@QBEPAEPAHPAI@Z
// TRIED: plateaus at MNEMONIC_ONLY 11/18 - eax/edx swap on the `first` branch (image keeps `first` in eax, node-reload in edx; this tree does the opposite). Caching the loaded front value in a named local, and `!=nullptr` vs bare pointer truthiness, both measured with no change - register allocation, not source shape.
unsigned char *VideoFrameQueue::peek(int *next_frame_id,
                                     unsigned int *frame_flags) const {
    if (head_ == 0) return 0;
    if (frame_flags != 0) *frame_flags = head_->frame_flags;
    if (next_frame_id != 0) *next_frame_id = head_->frame_id;
    return head_->bytes;
}

// ORIGINAL: 0x0063E820 sub_63e820 0x0063E820-0x0063E85B FILE
// symbol    ?discard_head@VideoFrameQueue@@QAEXXZ
void VideoFrameQueue::discard_head() {
    VideoFrameNode *const node = head_;
    if (node != 0) {
        if (node->bytes != 0) {
            free(node->bytes);
            node->bytes = 0;
        }
        head_ = head_->next;
        if (node != 0) free(node);
        --count_;
    }
}

// ORIGINAL: 0x0063E590 sub_63e590 0x0063E590-0x0063E5DF FILE
// symbol    ?clear@VideoFrameQueue@@QAEXXZ
void VideoFrameQueue::clear() {
    while (head_ != 0) {
        VideoFrameNode *const node = head_;
        VideoFrameNode *const next = node->next;
        if (node->bytes != 0) {
            free(node->bytes);
            node->bytes = 0;
        }
        free(node);
        head_ = next;
    }
    tail_ = 0;
    count_ = 0;
}


// ORIGINAL: 0x00636400 ??0Video@@QAE@XZ 0x00636400-0x0063652E;0x00663890-0x006638C9 FILE
// symbol    ??0Video@@QAE@XZ
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

// ORIGINAL: 0x00636530 ??1Video@@QAE@XZ 0x00636530-0x006365A6;0x006638C9-0x00663902 FILE
// symbol    ??1Video@@QAE@XZ
// TRIED: MISMATCH #1 push/mov - no SEH __try/__finally frame emitted; Video/Buffer/Palette have no dtor chain in the scaffold so the compiler treats it as trivial-unwind and skips the frame
Video::~Video() {
  close();
  // Compiler-owned destruction follows the native order: Buffer, queue,
  // Palette, then the Sound-derived prefix. The prefix's separate deleting
  // thunk is not this outer object's deletion path.
}

// Historical FILE coverage spans multiple entries through 0x637310; it is
// retained as whole-owner provenance, not the extent of this close body.
// The native close return ends at 0x636732. No original span is narrowed here.
// ORIGINAL: 0x006365B0 Video_unk1 0x006365B0-0x00637310 FILE
// symbol    ?close@Video@@QAEXXZ
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

  memset(&child_chunk_, 0, 0x14);
  buffered_frame_ = 0;
  target_frame_ = 0;
  frame_count_ = 0;
  video_chunk_id_ = 0;
  audio_chunk_id_ = 0;
  memset(&parent_chunk_, 0, 0x14);
  decode_output_flags_ = 0;
  last_tick_ = 0;
  decode_output_ = 0;
  decode_source_ = 0;
  viewport_x_ = 0;
  viewport_y_ = 0;
  source_rect_.left = 0;
  source_rect_.top = 0;
  source_rect_.right = 0;
  source_rect_.bottom = 0;
  output_buffer_.close();
}

// ORIGINAL: 0x00636FC0 ?process@Video@@QAEXXZ 0x00636FC0-0x0063706F FILE BYTE_EXACT
// symbol    ?process@Video@@QAEXXZ
// LEVER: one 32-bit `flags` load reused for the first `& 0x8000`/`& 0x10` pair (matches the original's single `mov eax` feeding both `test ah,0x80` and `test al,0x10`), fresh byte reloads for every later `& 0x10` check (matches the repeated `mov al, [esi+0x4ac]` after each intervening call), and `while (r != 0 && (r = decode_frame(0)) != 0) r = play_frame();` for the tail loop. First try, byte exact.
void Video::process() {
  const unsigned int flags = flags_;
  if ((flags & 0x8000) != 0) {
    if ((flags & 0x10) == 0) {
      if (play_frame() != 0) return;
      log_say("decode_frame_process", "Entering",
              static_cast<int>(timeGetTime()), 0, 0);
      if ((static_cast<unsigned char>(flags_) & 0x10) == 0) {
        play_frame();
        if (decode_frame(0) != 0) return;
      }
    }
    log_say("read_frame_process", "Entering",
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
int Video::play() {
  if (media_file_ == 0) return 7;
  if ((static_cast<unsigned char>(flags_) & 0x10) != 0) {
    read_frame();
  } else {
    while (read_frame() != 0) {}
    decode_frame(0);
  }
  log_say("play()", "Starting", static_cast<int>(timeGetTime()), 0, 0);
  if ((static_cast<unsigned char>(flags_) & 1) != 0) {
    last_tick_ = timeGetTime();
    flags_ |= 0x8000;
    return 0;
  }
  sound_state_.set_target_frame(target_frame_);
  flags_ |= 0x8000;
  return 0;
}

// ORIGINAL: 0x00637870 ?decode_frame_process@Video@@QAEXXZ 0x00637870-0x006378B0 FILE BYTE_EXACT
// symbol    ?decode_frame_process@Video@@QAEHXZ
int Video::decode_frame_process() {
  log_say("decode_frame_process", "Entering",
          static_cast<int>(timeGetTime()), 0, 0);
  if ((static_cast<unsigned char>(flags_) & 0x10) != 0) return 0;
  play_frame();
  return decode_frame(0);
}

// ORIGINAL: 0x006378B0 ?read_frame_process@Video@@QAEXXZ 0x006378B0-0x00637906 FILE BYTE_EXACT
// symbol    ?read_frame_process@Video@@QAEXXZ
void Video::read_frame_process() {
  log_say("read_frame_process", "Entering",
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
  for (;;) {
    if (mmioDescend(media_file_, &child_chunk_, &parent_chunk_, 0) != 0)
      return 1;
    if (child_chunk_.ckid == video_chunk_id_) {
      ++buffered_frame_;
      log_say("Exiting advance_frame",
              VideoAdvanceVideoLogContext,
              static_cast<int>(timeGetTime()), 0, 0);
      return 0;
    }
    if (child_chunk_.ckid == audio_chunk_id_) {
      const int byte_count = static_cast<int>(child_chunk_.cksize);
      log_say("advance_frame()", "Advancing audio.",
              static_cast<int>(timeGetTime()), byte_count, 0);
      VideoAudioBufferRecord *const record = static_cast<VideoAudioBufferRecord *>(
          sound_state_.acquire_audio_buffer(byte_count));
      if (record == 0) {
        log_say("No mmio!!!",
                VideoAudioBufferLogContext, 0, 0, 0);
      } else {
        const LONG bytes_read = mmioRead(media_file_,
            reinterpret_cast<HPSTR>(record->data), byte_count);
        if (bytes_read != byte_count) return 1;
        record->end = record->data + byte_count;
        sound_state_.submit_audio_buffer(record);
      }
    }
    if (child_chunk_.ckid != FOURCC_LIST &&
        mmioAscend(media_file_, &child_chunk_, 0) != 0) return 1;
  }
}

// ORIGINAL: 0x00637C30 ?read_frame@Video@@QAEHXZ 0x00637C30-0x00637D62
// symbol    ?read_frame@Video@@QAEHXZ
// TRIED: original pushes an extra callee-saved register (ecx-slot, ebx, esi, edi - 4 total) this body's control flow only needed 3 of (no ebx); not chased further. `sub_63e6e0` is called here as the scaffold's plain __stdcall(int,int,int,int), dropping the `lea ecx,[esi+0xa5c]` receiver the disassembly sets right before the call - Ghidra's own decompile drops the same thing (`the decompiler drops the this pointer on __thiscall functions`), so it reads as a genuine 5th (thiscall) param this body does not reproduce.
int Video::read_frame() {
  unsigned char *position = 0;
  int result;
  if (child_chunk_.cksize == 0) {
    result = frame_queue_.allocate(&position, 0,
                                  static_cast<unsigned int>(buffered_frame_), 1);
    if (result != 0) return 0;
    log_say("read_frame()", "Read data--still frame", buffered_frame_,
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
      position += bytes_read;
      remaining -= static_cast<unsigned int>(bytes_read);
    }
    log_say("read_frame()", "Read data--normal frame", buffered_frame_,
            frame_queue_.count(), 0);
  }
  advance_frame();
  return 1;
}

// ORIGINAL: 0x00637910 ?decode_frame@Video@@QAEHH@Z 0x00637910-0x00637C26 FILE
// symbol    ?decode_frame@Video@@QAEHH@Z
// TRIED: MISMATCH #42 'mov' vs 'and' - transcribed the Ghidra decompile directly, sub_63e7f0/sub_63e820 redeclared to take the receiver (self+0xA5C) as an explicit int since a free-function thiscall can't be spelled without a helper class; matches structurally through the first ~40 instructions before diverging, not chased further.
int Video::decode_frame(int destination_index) {
  if (codec_ == 0) return 0;
  if ((flags_ & 0x1000) != 0) return 0;
  if (frame_queue_.count() == 0) return 0;

  if (viewport_y_ < 0) {
    viewport_y_ = 0;
  } else {
    int height = main_header_.dwHeight;
    int compared_height = height;
    const int output_height = output_buffer_.dib_.bmiHeader.biHeight;
    if ((flags_ & 4) != 0) compared_height *= 2;
    if (-(compared_height + output_height) < viewport_y_) {
      if ((flags_ & 4) != 0) height *= 2;
      viewport_y_ = -(output_height + height);
    }
  }

  unsigned int queue_flags = 0;
  const unsigned char *const frame_data = frame_queue_.peek(&target_frame_, &queue_flags);
  decode_output_flags_ = queue_flags;
  if (target_frame_ == frame_count_ - 1) {
    flags_ &= 0xFFFF7FFFU;
  } else {
    log_say("decode_frame()", static_cast<int>(timeGetTime()),
            target_frame_, 0);
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

      const RECT previous = source_rect_;
      int x_offset = source_dib_.bmiHeader.biWidth;
      int y_offset = source_dib_.bmiHeader.biHeight;
      if ((flags_ & 4) != 0) {
        x_offset *= 2;
        y_offset *= 2;
      }
      int height = main_header_.dwHeight;
      if ((flags_ & 4) != 0) height <<= 1;

      ICDECOMPRESSEX decompress;
      memset(&decompress, 0, sizeof decompress);
      decompress.dwFlags = destination_index ? 0x80000000UL : 0;
      decompress.lpbiSrc = &source_dib_.bmiHeader;
      decompress.lpSrc = const_cast<unsigned char *>(frame_data);
      decompress.lpbiDst = &output_buffer_.dib_.bmiHeader;
      decompress.lpDst = output_buffer_.get_data();
      decompress.xDst = viewport_x_;
      decompress.yDst = -(output_buffer_.dib_.bmiHeader.biHeight +
                          viewport_y_ + height);
      decompress.dxDst = x_offset;
      decompress.dyDst = y_offset;
      decompress.xSrc = 0;
      decompress.ySrc = 0;
      decompress.dxSrc = source_dib_.bmiHeader.biWidth;
      decompress.dySrc = source_dib_.bmiHeader.biHeight;

      if (ICSendMessage(codec_, ICM_DECOMPRESSEX,
              reinterpret_cast<DWORD>(&decompress), sizeof decompress) != 0) {
        log_say("decode_frame()", "Decode failed!",
                static_cast<int>(timeGetTime()), 0, 0);
        return 0;
      }

      const int viewport_width = VideoViewportCodec.width;
      const int viewport_height = VideoViewportCodec.height;
      int new_left = viewport_x_ + VideoViewportCodec.origin_x;
      int new_top = viewport_y_ + VideoViewportCodec.origin_y;
      int width = viewport_width;
      int height_out = viewport_height;
      if ((flags_ & 4) != 0) {
        width *= 2;
        height_out *= 2;
      }
      source_rect_.left = new_left;
      source_rect_.top = new_top;
      source_rect_.right = new_left + width;
      source_rect_.bottom = new_top + height_out;
      UnionRect(&VideoDirtyRect, &previous, &source_rect_);
    }
    frame_queue_.discard_head();
    flags_ |= 0x1000;
  }
  return 1;
}

// ORIGINAL: 0x00637D70 ?play_frame@Video@@QAEHXZ 0x00637D70-0x0063808D FILE
// symbol    ?play_frame@Video@@QAEHXZ
// TRIED: prologue register order (large 797-byte function, many branches); landed as a structural transcription rather than chasing every register-allocation choice. Div-by-1000 modelled as plain unsigned int/1000 (VC6's own reciprocal-multiply idiom), not the 64-bit division Ghidra's decompile implied.
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
  log_say("play_frame()", "Entering", static_cast<int>(timeGetTime()),
          current_frame - target_frame_, 0);
  if (current_frame > target_frame_) {
    if (current_frame == target_frame_ + 1) {
      if (frame_queue_.count() == 0) {
        read_frame();
        flags_ &= 0xFFFFEFFFU;
        goto finish;
      }
      flags_ &= 0xFFFFEFFFU;
    } else {
      const int nearest = next_key_frame(current_frame);
      if (nearest == -1) {
        int frame = target_frame_;
        while (frame < current_frame) {
          flags_ &= 0xFFFFEFFFU;
          if (decode_frame(0) == 0) {
            read_frame();
            decode_frame(0);
          }
          frame = target_frame_;
        }
      }
      log_say("play_frame()", "BEHIND AUDIO", current_frame,
              target_frame_, nearest);
      int frame = target_frame_;
      if (nearest - current_frame <= current_frame - frame) {
        if (buffered_frame_ - 1 < nearest) {
          frame_queue_.set_limit(0x28);
          int buffered = buffered_frame_;
          while (buffered < nearest) {
            advance_frame();
            buffered = buffered_frame_;
          }
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
        decode_frame(0);
        goto finish;
      }
      while (frame < current_frame - 1) {
        flags_ &= 0xFFFFEFFFU;
        if (decode_frame(1) == 0) read_frame();
        frame = target_frame_;
      }
      flags_ &= 0xFFFFEFFFU;
      if (decode_frame(0) != 0) goto finish;
      read_frame();
    }
    decode_frame(0);
  }

finish:
  if ((decode_output_flags_ & 1) == 0) {
    const int height = (flags_ & 4) ? main_header_.dwHeight * 2
                                     : main_header_.dwHeight;
    const int width = (flags_ & 4) ? main_header_.dwWidth * 2
                                    : main_header_.dwWidth;
    HDC const source = output_buffer_.get_hdc();
    HDC const destination = Win::get_hdc();
    BitBlt(destination, viewport_x_, viewport_y_, width, height, source,
           viewport_x_, viewport_y_, SRCCOPY);
    Win::release_hdc();
    output_buffer_.release_hdc(1);
  }
  if (target_frame_ == frame_count_ - 1) flags_ &= 0xFFFF7FFFU;
  flags_ &= 0xFFFFEFFFU;
  log_say("play_frame()", "Exiting", static_cast<int>(timeGetTime()), 0, 0);
  return 1;
}


// ORIGINAL: 0x00637310 Video_unk12 0x00637310-0x00637395 FILE
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

// ORIGINAL: 0x00638090 Video_unk14 0x00638090-0x00638128 FILE
// symbol    ?previous_key_frame@Video@@QAEHH@Z
// TRIED: MISMATCH #0 mov/push, prologue register choice differs; the receiver is the Video object (ecx reads with no stack slot).
int Video::previous_key_frame(int frame) {
  if (media_file_ == 0 || frame_index_table_ == 0) return -1;
  if (frame >= frame_count_ || frame < 0) return -1;
  AVIINDEXENTRY *record = frame_index_table_;
  while (record->ckid != video_chunk_id_) ++record;
  if (frame > 0) {
    int remaining = frame;
    do {
      do { ++record; } while (record->ckid != video_chunk_id_);
      --remaining;
    } while (remaining != 0);
    while (frame > 0 && (record->dwFlags & 0x10) == 0) {
      do { --record; } while (record->ckid != video_chunk_id_);
      --frame;
    }
  }
  // Native638114 rejects even a sync frame at index0; do not normalize it.
  if (frame == 0) return -1;
  current_index_record_ = record;
  return frame;
}

// ORIGINAL: 0x00638130 Video_unk15 0x00638130-0x006381DD FILE
// symbol    ?next_key_frame@Video@@QAEHH@Z
int Video::next_key_frame(int frame) {
  if (media_file_ == 0 || frame_index_table_ == 0) return -1;
  if (frame >= frame_count_ || frame < 0) return -1;
  AVIINDEXENTRY *record = frame_index_table_;
  while (record->ckid != video_chunk_id_) ++record;
  if (frame > 0) {
    int remaining = frame;
    do {
      do { ++record; } while (record->ckid != video_chunk_id_);
      --remaining;
    } while (remaining != 0);
  }
  while (frame < frame_count_ - 1 && (record->dwFlags & 0x10) == 0) {
    do { ++record; } while (record->ckid != video_chunk_id_);
    ++frame;
  }
  if (frame == frame_count_ - 1) {
    if ((record->dwFlags & 0x10) == 0) return -1;
  } else if (frame == frame_count_) {
    return -1;
  }
  current_index_record_ = record;
  return frame;
}

// Native registration permits index10. The eleventh dword physically aliases
// codec origin_x at9BE668, not an eleventh owned registry pointer. This defined
// bit-copy projection preserves that native corrupting effect rather than
// clamping it or forming an out-of-bounds C++ pointer store. Coordinator review
// is still required; this is a private behavior/storage proposal, not publication.
static inline void video_store_registry_entry(int index, Video *video) {
  if (index == 10) {
    // Copy the pointer object representation itself, avoiding signed/integer
    // conversion. The real x86 header constrains these two storage widths.
    memcpy(&VideoViewportCodec.origin_x, &video,
           sizeof VideoViewportCodec.origin_x);
  } else {
    VideoRegistry[index] = video;
  }
}

// ORIGINAL: 0x00636850 Video_unk3 0x00636850-0x006368C8 FILE
// symbol    ?register_window@Video@@QAEHHHPAVGraphicWin@@I@Z
int Video::register_window(int x, int y, GraphicWin *window,
                           unsigned int flags) {
  if (window == 0) return 0x10;
  close();
  viewport_x_ = x;
  decode_output_ = window;
  viewport_y_ = y;
  flags_ = flags & 0xFFFF7FFFU;
  if ((flags & 0x20) != 0) flags_ |= 4;
  const int count = VideoRegistryCount;
  if (count > 10) return 1;
  video_store_registry_entry(count, this);
  VideoRegistryCount = count + 1;
  return 0;
}

// ORIGINAL: 0x006368D0 Video_unk4 0x006368D0-0x00636954 FILE
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
  if ((flags & 0x20) != 0) flags_ |= 4;
  const int count = VideoRegistryCount;
  if (count > 10) return 1;
  video_store_registry_entry(count, this);
  VideoRegistryCount = count + 1;
  return 0;
}

// The ordinary registry span is ten pointers. For the native error state
// count11, the pump consumes the codec origin_x word as the eleventh receiver.
// Recover that observed effect explicitly, without asserting array[10] is valid.
static inline Video *video_registry_receiver(int index) {
  if (index == 10) {
    Video *video = 0;
    memcpy(&video, &VideoViewportCodec.origin_x,
           sizeof VideoViewportCodec.origin_x);
    return video;
  }
  return VideoRegistry[index];
}

// ORIGINAL: 0x00636300 ?do_video@@YAXXZ 0x00636300-0x006363EE FILE
// symbol    ?do_video@@YAXXZ
// TRIED: nothing yet - MISMATCH #2 mov/push, mnemonic similarity 0.908. for-loop and do/while shaped identically; not chased further.
void __cdecl do_video() {
  // Native636300 inlines the same pipeline, but its actual call targets are
  // play_frame/decode_frame/read_frame, not Video::process. Keep those calls.
  // Reload the count after each iteration; callbacks can rethread the registry.
  for (int index = 0; index < VideoRegistryCount; ++index) {
    Video *const video = video_registry_receiver(index);
    if ((video->flags_ & 0x8000) == 0) continue;
    if ((video->flags_ & 0x10) == 0) {
      if (video->play_frame() != 0) continue;
      log_say("decode_frame_process", "Entering",
              static_cast<int>(timeGetTime()), 0, 0);
      if ((video->flags_ & 0x10) == 0) {
        video->play_frame();
        if (video->decode_frame(0) != 0) continue;
      }
    }
    log_say("read_frame_process", "Entering",
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

// ORIGINAL: 0x006371D0 Video_unk10 0x006371D0-0x006372F7
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

// ORIGINAL: 0x00636740 Video_unk2 0x00636740-0x0063684D FILE
// symbol    ?reset@Video@@QAEXXZ
// TRIED: not chased to byte-exact (MISMATCH from instruction 1, 276 vs 269 bytes). Landed for the field-write order and call order, which match the disassembly one-for-one, including the `and ch,0x7f` / `test al,1` pair - a mask-after-copy on the SAME dword (`bit0` read before the 0x8000 bit is cleared).
void Video::reset() {
  // Unlike close(), reset preserves registration, coordinates and flags
  // other than the playback bit. This distinction is load-bearing in load().
  frame_queue_.clear();
  state_4a8_ = 0;
  const bool had_audio = (flags_ & 1) == 0;
  flags_ &= 0xFFFF7FFFU;
  if (had_audio) sound_state_.release();
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
  buffered_frame_ = target_frame_ = frame_count_ = 0;
  video_chunk_id_ = audio_chunk_id_ = 0;
  memset(&child_chunk_, 0, sizeof child_chunk_);
  memset(&parent_chunk_, 0, sizeof parent_chunk_);
  decode_output_flags_ = last_tick_ = 0;
  memset(&source_rect_, 0, sizeof source_rect_);
  output_buffer_.close();
}

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
  if ((flags_ & 0xC) == 0) {
    // Native uses the output DIB's original BGR colours, not the codec's
    // palette-query result. The query result's fourth byte is preserved.
    const unsigned char *original = reinterpret_cast<const unsigned char *>(
        &output_buffer_.dib_) + sizeof(BITMAPINFOHEADER);
    unsigned char *shuffled = reinterpret_cast<unsigned char *>(&destination)
        + sizeof(BITMAPINFOHEADER);
    int palette_index;
    for (palette_index = 0; palette_index < 256; ++palette_index) {
      shuffled[palette_index * 4] = original[palette_index * 4];
      shuffled[palette_index * 4 + 1] = original[palette_index * 4 + 1];
      shuffled[palette_index * 4 + 2] = original[palette_index * 4 + 2];
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
    codec_state_tail_.selector = 4;
    codec_state_tail_.mode = 2;
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
    codec_state_tail_.selector = 4;
    codec_state_tail_.mode = 2;
    codec_state_tail_.flags = 0x80000008UL;
    codec_state_tail_.option_28 = 1;
    ICSendMessage(codec_, 0x5001, reinterpret_cast<DWORD>(&codec_state_tail_), 0x2C);
  }
  ICDECOMPRESSEX begin;
  memset(&begin, 0, sizeof begin);
  begin.lpbiSrc = &source_dib_.bmiHeader;
  begin.lpbiDst = &destination.bmiHeader;
  begin.xDst = VideoDirtyRect.left;
  begin.yDst = VideoDirtyRect.top;
  begin.dxDst = source_dib_.bmiHeader.biWidth;
  begin.dyDst = source_dib_.bmiHeader.biHeight;
  if ((flags_ & 4) != 0) { begin.dxDst *= 2; begin.dyDst *= 2; }
  begin.dxSrc = source_dib_.bmiHeader.biWidth;
  begin.dySrc = source_dib_.bmiHeader.biHeight;
  return ICSendMessage(codec_, ICM_DECOMPRESSEX_BEGIN,
           reinterpret_cast<DWORD>(&begin), sizeof begin) != 0;
}

// ORIGINAL: 0x004C9360 ??_GVideo@@UAEPAXI@Z 0x004C9360-0x004C9381 FILE BYTE_EXACT
// symbol    ??_GVideoSoundState@@UAEPAXI@Z
// Compiler-owned prefix scalar destructor mapping anchor.
