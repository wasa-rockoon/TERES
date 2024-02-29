#include <sys/_stdint.h>
#include <sys/_intsup.h>

#include "AVIWriter.h"

AVIWriter::AVIWriter() {
  initialized_ = false;
  video.width = 1280;
  video.height = 720;
  video.fps = 30.0f;
}

bool AVIWriter::start(FatFile& file) {
  file_ = &file;
  if (!writeHeader()) return false;
  initialized_ = true;
  return true;
}

bool AVIWriter::writeHeader() {
  write4CC("RIFF");
  writeWord(256); // total size, update later
  write4CC("AVI ");
  write4CC("LIST");
  writeWord(208); // size
  write4CC("hdrl");
  write4CC("avih");
  writeWord(56); // size
  writeWord(round(1000000.0f / video.fps)); // us per frame
  writeWord(1000000); // max bytes per sec, update later
  writeWord(0); // padding granularity
  writeWord(16); // flags
  writeWord(0); // total frames, update later
  writeWord(0); // initial frames
  writeWord(1); // streams
  writeWord(0); // suggested buffer size
  writeWord(video.width); // width
  writeWord(video.height); // height
  writeWord(0); // reserved
  writeWord(0);
  writeWord(0);
  writeWord(0);
  write4CC("LIST");
  writeWord(132); // size
  write4CC("strl");
  write4CC("strh");
  writeWord(48); // size
  write4CC("vids");
  write4CC("MJPG");
  writeWord(0); // flags
  writeWord(0); // priority, language
  writeWord(0); // initial frames
  writeWord(1); // scale
  writeWord(video.fps); // rate
  writeWord(0); // start
  writeWord(0); // length, update later
  writeWord(0); // suggested buffer size
  writeWord(0); // quality
  writeWord(0); // sample size
  write4CC("strf");
  writeWord(40); // size
  writeWord(40); // size
  writeWord(video.width); // width
  writeWord(video.height); // height
  writeWord(24 << 16 | 1); // bit count, planes
  write4CC("MJPG");
  writeWord(((video.width * 24 / 8 + 3) & 0xFFFFFFFC) * video.height); // size image
  writeWord(0); // x pels per meter
  writeWord(0); // y pels per meter
  writeWord(0); // clr used
  writeWord(0); // clr important
  write4CC("LIST");
  writeWord(16); // size
  write4CC("odml");
  write4CC("dmlh");
  writeWord(4); // szs
  writeWord(0); // total frames, update later
  
  // idx1
  idx1_start_pos_ = file_->curPosition();
  write4CC("idx1");
  writeWord(AVI_IDX1_INITIAL_COUNT * 16); // size, update later
  writeEmpty(AVI_IDX1_INITIAL_COUNT * 16);

  // movi
  movi_start_pos_ = file_->curPosition();
  write4CC("LIST");
  writeWord(4); // size, update later
  write4CC("movi");
  movi_size_ = 4;
  frame_count_ = 0;
  frame_max_bytes_ = 0;
  synced_frame_count_ = 0;

  file_->sync();

  return true;
}

bool AVIWriter::sync() {
  // Relocate movi
  while (idx1_start_pos_ + 8 + (frame_count_ + 1) * 16 >= movi_start_pos_) {
    Serial.println("RELOCATE");

    // Remove first frame
    file_->seekSet(movi_start_pos_ + 12 + 4);
    unsigned first_frame_size = readWord();
    movi_start_pos_ += 8 + first_frame_size;
    movi_size_ -= 8 + first_frame_size;

    // Write new movi header
    file_->seekSet(movi_start_pos_);
    write4CC("LIST");
    writeWord(movi_size_);
    write4CC("movi");
    file_->seekCur(+4);
    first_frame_size = readWord();

    // Update idx1 header
    file_->seekSet(idx1_start_pos_ + 4);
    writeWord(movi_start_pos_ - idx1_start_pos_ - 4);

    // Rewrite index
    for (int n = 0; n < frame_count_; n++) {
      file_->seekSet(idx1_start_pos_ + 4 + frame_count_ * 16 + 8);
      unsigned offset = readWord();
      if (offset <= movi_start_pos_ + 4) {
        file_->seekCur(-4);
        writeWord(movi_start_pos_ + 4);
        writeWord(first_frame_size);
      }
      else break;
    }
  }

  // Write Index
  file_->seekSet(idx1_start_pos_ + 8 + synced_frame_count_ * 16);
  for (unsigned n = 0; n < frame_count_ - synced_frame_count_; n++) {
    write4CC("00dc");
    writeWord(AVI_KEYFRAME);
    writeWord(frame_offset_cache_[n]); // offset
    writeWord(frame_size_cache_[n]); // size
  }
  synced_frame_count_ = frame_count_;

  // Update parameters
  file_->seekSet(0x04); // total size
  writeWord(movi_start_pos_ - 4 + 12 + movi_size_);
  file_->seekSet(0x24); // max bytes per sec
  writeWord(frame_max_bytes_ * video.fps);
  file_->seekSet(0x30); // total frames
  writeWord(frame_count_);
  file_->seekSet(0x8c); // total frames
  writeWord(frame_count_);
  file_->seekSet(0xe0); // frames
  writeWord(frame_count_);
  file_->seekSet(movi_start_pos_ + 4); // movi size
  writeWord(movi_size_);  

  file_->seekEnd();

  file_->sync();

  return true;
}

bool AVIWriter::addVideoFrame(const uint8_t* data, unsigned size) {
  if (!initialized_) return false;

  // Write frame
  write4CC("00dc");
  unsigned size_pad = size % 2 ? size + 1 : size;
  writeWord(size_pad);
  file_->write(data, size);
  if (size % 2) writeEmpty();

  last_frame_offset_ = movi_start_pos_ + 8 + movi_size_;
  last_frame_size_ = size_pad;
  frame_offset_cache_[frame_count_ - synced_frame_count_] = last_frame_offset_; // offset
  frame_size_cache_[frame_count_ - synced_frame_count_] = last_frame_size_; // size

  movi_size_ += 8 + size_pad;
  frame_count_++;
  if (size_pad > frame_max_bytes_) frame_max_bytes_ = size_pad;

  if (frame_count_ - synced_frame_count_ >= AVI_FRAME_CACHE) sync();

  return true;
}

bool AVIWriter::skipFrame() {
  return addVideoFrame(nullptr, 0);
}

unsigned AVIWriter::getFileSize() const {
  return movi_start_pos_ + 12 + movi_size_;
}

bool AVIWriter::write4CC(const char fourCC[]) {
  return file_->write(fourCC, 4) == 4;
}

bool AVIWriter::writeWord(uint32_t word) {
  char bytes[4];
  memcpy(bytes, &word, 4);
  return file_->write(bytes, 4) == 4;
}

uint32_t AVIWriter::readWord() {
  char bytes[4];
  if (file_->read(bytes, 4) != 4) return 0;
  uint32_t word;
  memcpy(&word, bytes, 4);
  return word;
}

bool AVIWriter::writeEmpty(unsigned len) {
  for (int i = 0; i < len; i++) {
    char empty[] = "\0";
    if (!file_->write(empty, 1)) return false;
  }
  return true;
}