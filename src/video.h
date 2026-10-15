// Video playback owner and the external sound-device contracts it consumes.
// Native zero-size queue behavior is deliberately unchanged.
#ifndef OPENSMACX_VIDEO_H
#define OPENSMACX_VIDEO_H
#include "stdafx.h"
#include "sound.h"
#include "soundx_sound_abi.h"
#include "palette.h"
#include "buffer.h"
#include "graphicwin.h"
#include "win.h"
#include "log.h"
#include <mmsystem.h>
#include <vfw.h>
#include <stddef.h>
#include <cstdlib>
#include <cstring>

// The wrapped SoundX object is an external Sound ABI object followed by
// Video's observed extension; the DLL owns its construction and destruction.
// Pinned mode 4 factory 0x10008070 allocates 0xE8 bytes; its constructor chain
// 0x10001316 -> 0x10005060 -> 0x100011F4 -> 0x1002A520 stages root and Sound
// tables at unadjusted this before table 0x10064168. Slot 2's wrapper
// 0x10005BF0 calls 0x100022B6, tests the flags argument and conditionally
// frees the object. This supports a shared external prefix, not nominal game
// class identity.
class VideoSoundDevice : public SoundDeviceInterface {
public:
  virtual int configure_streams(const void *audio_stream,
                                const void *video_stream,
                                const void *audio_format,
                                int flags) = 0;                       // slot 33
  virtual int set_target_frame(int frame) = 0;                        // slot 34
  virtual void *acquire_audio_buffer(int bytes) = 0;                  // slot 35
  virtual int submit_audio_buffer(void *record) = 0;                  // slot 36
  virtual int current_frame() = 0;                                    // slot 37
  virtual int frame_status() = 0;                                     // slot 38
};

struct VideoFrameNode {
  int frame_id;
  unsigned int frame_flags;
  unsigned char *bytes;
  unsigned int byte_count;
  VideoFrameNode *next;
};

class Video;
struct VideoAudioBufferRecord {
  unsigned char opaque_prefix[0x18];
  unsigned char *data;                       // Native record +18
  unsigned int state_1c_;                    // Unknown native state +1C
  unsigned char *end;                        // Native record +20
};

class VideoFrameQueue {
public:
  VideoFrameQueue();
  ~VideoFrameQueue();
  int count() const { return count_; }
  int allocate(unsigned char **, int, int, unsigned int);
  int append(const unsigned char *, int, int, unsigned int);
  unsigned char *peek(int *, unsigned int *) const;
  void discard_head();
  void clear();
  void set_limit(int limit);
private:
  VideoFrameNode *head_;
  VideoFrameNode *tail_;
  int count_;
  int limit_;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(VideoFrameQueue) == 0x10, "Video queue extent");
static_assert(sizeof(VideoFrameNode) == 0x14, "Video queue node extent");
static_assert(offsetof(VideoAudioBufferRecord, data) == 0x18, "audio payload offset");
static_assert(offsetof(VideoAudioBufferRecord, state_1c_) == 0x1C, "audio intervening state");
static_assert(offsetof(VideoAudioBufferRecord, end) == 0x20, "audio end offset");
#endif

// Sound-derived first member; the compiler owns its 39-slot table.
// Its deleting destructor owns only this prefix, never the outer Video.
// RECOVERY_BASE: v1 VideoSoundState Sound 0x0 0x4c8620
class VideoSoundState : public Sound {
public:
  VideoSoundState();
  virtual ~VideoSoundState();
  virtual int unload();
  virtual int stop();
  virtual int release();
  virtual int get_game_hwnd();
  virtual int set_attrib(unsigned long);
  virtual int get_attrib();
  virtual int get_time();
  virtual int configure_streams(const void *, const void *, const void *, int);
  virtual int set_target_frame(int);
  virtual void *acquire_audio_buffer(int);
  virtual int submit_audio_buffer(void *);
  virtual int current_frame();
  virtual int frame_status();
};

// Four contiguous dwords are handed to the codec as a pointer and read by
// decode_frame. This candidate owner covers that observed API span; it does
// not claim the original allocation ended after these four fields.
struct VideoViewportCodecParameters {
  int origin_x;
  int origin_y;
  int width;
  int height;
};
// Observed Indeo private ICM_USER packet fields, not an SDK ICINFO.
struct VideoCodecPacket {
  DWORD byte_size, handler, version, selector, mode, flags;
  DWORD reserved_18_3b[9];
  VideoViewportCodecParameters *viewport;
  DWORD reserved_40_53[5];
};
struct VideoCodecOptions {
  DWORD byte_size, handler, version, selector, mode, flags;
  DWORD reserved_18_23[3];
  DWORD option_24, option_28;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(VideoViewportCodecParameters) == 0x10,
              "codec viewport parameter span");
#endif
extern VideoViewportCodecParameters VideoViewportCodec;
extern RECT VideoDirtyRect;
extern char VideoAdvanceEntryLogContext[];
extern char VideoAdvanceVideoLogContext[];
extern char VideoAudioBufferLogContext[];
extern Video *VideoRegistry[10];
extern int VideoRegistryCount;

class Video {
public:
  Video();
  ~Video();
  void close();
  void process();
  int play();
  int advance_frame();
  int decode_frame_process();
  void read_frame_process();
  int decode_frame(int);
  int read_frame();
  int play_frame();
  int size_buffer(int width, int height);
  int previous_key_frame(int frame);
  int next_key_frame(int frame);
  int register_window(int x, int y, GraphicWin *window, unsigned int flags);
  int register_window_buffer(int x, int y, GraphicWin *window, unsigned int flags);
  void reset();
  int load(char *filename, int unused);
  int configure_codec();
  int play_at(int x, int y);
  void stop();
  int seek(int frame);
  int rewind();
  int playback_flag() const;

  // RECOVERY_MEMBER: v2 Video sound_state_ VideoSoundState 0x0 0x636400
  VideoSoundState sound_state_;                  // +0x000, 0x54
  // RECOVERY_MEMBER: v2 Video palette_ Palette 0x54 0x636400
  Palette palette_;                              // +0x054, 0x454
  unsigned int state_4a8_;                       // +0x4A8
  unsigned int flags_;                           // +0x4AC
  HMMIO media_file_;                             // +0x4B0
  MMCKINFO parent_chunk_;                        // +0x4B4
  MMCKINFO child_chunk_;                         // +0x4C8
  HIC codec_;                                    // +0x4DC
  MainAVIHeader main_header_;                    // +0x4E0
  AVIStreamHeader video_stream_header_;      // +0x518
  AVIStreamHeader audio_stream_header_;      // +0x558
  DWORD video_chunk_id_;                         // +0x598
  DWORD audio_chunk_id_;                         // +0x59C
  // RECOVERY_MEMBER: v2 Video source_dib_ Dib 0x5a0 0x637530
  Dib source_dib_;                               // +0x5A0, 0x428
  WAVEFORMATEX audio_format_;             // +0x9C8
  unsigned char unknown_9da_9dc_[2];             // +0x9DA
  VideoCodecPacket codec_state_;                 // +0x9DC
  VideoCodecOptions codec_state_tail_;           // +0xA30
  // RECOVERY_MEMBER: v2 Video frame_queue_ VideoFrameQueue 0xa5c 0x636400
  VideoFrameQueue frame_queue_;                  // +0xA5C
  // RECOVERY_MEMBER: v2 Video output_buffer_ Buffer 0xa6c 0x636400
  Buffer output_buffer_;                         // +0xA6C
  unsigned int decode_output_flags_;             // +0xFF4
  int frame_count_;                              // +0xFF8
  unsigned int last_tick_;                       // +0xFFC
  int buffered_frame_;                           // +0x1000
  int target_frame_;                             // +0x1004
  AVIINDEXENTRY *frame_index_table_;                      // +0x1008
  AVIINDEXENTRY *current_index_record_;                   // +0x100C
  int state_1010_;                               // +0x1010
  int state_1014_;                               // +0x1014
  // RECOVERY_MEMBER: v3 Video decode_output_ GraphicWin* 0x1018 0x636850
  GraphicWin *decode_output_;                    // +0x1018, borrowed registration window
  // RECOVERY_MEMBER: v3 Video decode_source_ Buffer* 0x101c 0x6368d0
  Buffer *decode_source_;                        // +0x101C, its borrowed secondary Buffer
  int viewport_x_;                               // +0x1020
  int viewport_y_;                               // +0x1024
  RECT source_rect_;                             // +0x1028
};

void __cdecl do_video();


#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(VideoSoundState) == 0x54, "Sound-derived prefix extent");
static_assert(sizeof(Palette) == 0x454, "Palette member extent");
static_assert(sizeof(Buffer) == 0x588, "Buffer member extent");
// This is the observed field prefix, not proof of complete original allocation.
static_assert(sizeof(Video) == 0x1038, "observed Video field prefix extent");
static_assert(sizeof(Video *) == 4, "native registry pointer width");
static_assert(sizeof(Video *) == sizeof VideoViewportCodec.origin_x,
              "native overflow aliases exactly one codec word");
static_assert(sizeof(ICDECOMPRESSEX) == 0x34, "native decompressor packet extent");
static_assert(sizeof(MainAVIHeader) == 0x38, "native main AVI header extent");
static_assert(sizeof(AVIStreamHeader) == 0x40, "native stream header extent");
static_assert(sizeof(WAVEFORMATEX) == 0x12, "native audio format extent");
static_assert(sizeof(AVIINDEXENTRY) == 0x10, "native index record stride");
static_assert(sizeof(VideoCodecPacket) == 0x54, "Indeo packet extent");
static_assert(offsetof(VideoCodecPacket, viewport) == 0x3C, "Indeo viewport pointer");
static_assert(sizeof(VideoCodecOptions) == 0x2C, "Indeo options extent");
#endif

#endif
