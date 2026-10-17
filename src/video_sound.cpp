// RECOVERY_GENERATED: v1 vtable 0x66e790 0x66e82c ??_7VideoSoundState@@6B@ 0x004c8620
// Video playback, composed resources and Sound-derived audio ownership.
// The Sound prefix owns table 0x66E790 and its own deleting thunk 0x4C9360.
// Do not add an outer virtual deleter: that thunk calls prefix 0x4C86D0 only.
#include "video.h"

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
    delete device;
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
    if (static_cast<VideoSoundDevice *>(device_)->is_playing() == 0) {
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
// RECOVERY_BINDING: v1 {"evidence": ["pinned image SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; VideoSoundState primary table 0x0066E790 slot 29 at 0x0066E804 relocates to 0x004C8950", "the pre-proposal src/video.cpp RECOVERY_SHARED record already bound this exact span to VideoSoundState::get_time; this replaces that one shared record with the canonical identity", "src/video_sound.cpp owns the actual int VideoSoundState::get_time() method body returning zero; no Time class or global timer ownership is inferred", "historical catalog facts retained: sub_4c8950 is a three-byte zero-return leaf; its name, span and claim state are carried from the parsed source record by the native writer", "retired source symbol retained as historical evidence: ?nullsub_004c8950_redirect@@YIHPAX0@Z"], "reason": "Promote the existing VideoSoundState::get_time shared-body mapping to the unique canonical identity. Keep its real typed method and avoid a duplicate annotation or adapter."}
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

// ORIGINAL: 0x004C9360 ??_GVideo@@UAEPAXI@Z 0x004C9360-0x004C9381 FILE BYTE_EXACT
// symbol    ??_GVideoSoundState@@UAEPAXI@Z
// Compiler-owned prefix scalar destructor mapping anchor.
