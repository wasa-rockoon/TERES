
#pragma once

#include <SdFat.h>

#define AVI_IDX1_INITIAL_COUNT 256
#define AVI_KEYFRAME 16

#define AVI_FRAME_CACHE 64

class AVIWriter {
public:
  struct VideoFormat {
    unsigned width;
    unsigned height;
    float fps;
  };

  struct AudioFormat {
    unsigned channels;
    unsigned samples;
    unsigned bits;
  };

  VideoFormat video;
  AudioFormat audio;

  AVIWriter();

  bool start(FatFile& file);
  bool addVideoFrame(const uint8_t* data, unsigned size);
  bool skipFrame();
  bool sync();

  unsigned getFileSize() const;

private:
  FatFile* file_;

  bool initialized_;

  unsigned idx1_start_pos_;
  unsigned movi_start_pos_;
  unsigned movi_size_;
  unsigned frame_count_;
  unsigned frame_max_bytes_;

  unsigned last_frame_offset_;
  unsigned last_frame_size_;

  unsigned synced_frame_count_;
  unsigned frame_offset_cache_[AVI_FRAME_CACHE];
  unsigned frame_size_cache_[AVI_FRAME_CACHE];

  bool writeHeader();

  uint32_t readWord();
  bool write4CC(const char fourCC[]);
  bool writeWord(uint32_t word);
  bool writeEmpty(unsigned len = 1); 
};