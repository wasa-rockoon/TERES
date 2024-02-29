/**
 * Simple AVI library for Arduino
 * LGPL version 2.1 Copyright 2021 Yoshino Taro
 */

#include "AviLibrary.h"

/* constructor */
AviLibrary::AviLibrary():
    m_initialized(false)
  , m_width(0)
  , m_height(0)
  , m_frames(0)
  , m_total_size(0)
  , m_movi_size(0)
  , m_fps(0)
  , m_us_per_frame(0)
  , m_max_bytes_per_sec(0)
  , m_duration_sec(0.0)
  , m_aviFile(nullptr) {}

/* destructor */
AviLibrary::~AviLibrary() {
  if (m_initialized || m_aviFile != nullptr) 
    m_aviFile->close();
}

bool AviLibrary::start(FatFile& aviFile, uint16_t width, uint16_t height, float fps) {
  if (width == 0 || height == 0) return false;
  m_aviFile = &aviFile;
  m_width = width;
  m_height = height;
  m_initialized = true;
  m_fps = fps;
  m_us_per_frame = round(1000000.0f / m_fps);
  writeHeader();
  updateParameters();
  m_start_time = millis();
  return true;
}

bool AviLibrary::addFrame(uint8_t* ImgBuff, uint32_t imgSize) {
  if (!m_initialized || !ImgBuff || !imgSize) return false;

  m_aviFile->seekEnd();
  m_aviFile->write("00dc", 4);
  uint32_write_to_aviFile(imgSize);
  m_aviFile->write(ImgBuff, imgSize);
  m_movi_size += imgSize;
  ++m_frames;
  m_total_size = m_movi_size + 12 * m_frames + 4;
  m_max_bytes_per_sec = m_movi_size * m_fps / m_frames;
  return true;
}

bool AviLibrary::end() {
  if (!m_initialized || !m_aviFile) return false;
  updateParameters();
  m_aviFile->close();
  m_initialized = false;
  m_aviFile = nullptr;
  return true;
}

bool AviLibrary::writeHeader() {
  if (!m_initialized || m_frames != 0) return false;
  uint8_t width_1   = (uint8_t)( m_width  & 0x00ff); 
  uint8_t width_2   = (uint8_t)((m_width  & 0xff00) >> 8); 
  uint8_t height_1  = (uint8_t)( m_height & 0x00ff);
  uint8_t height_2  = (uint8_t)((m_height & 0xff00) >> 8); 
  m_avi_header[ 64] = width_1;
  m_avi_header[ 65] = width_2;
  m_avi_header[ 68] = height_1;
  m_avi_header[ 69] = height_2;
  m_avi_header[168] = width_1;
  m_avi_header[169] = width_2;
  m_avi_header[172] = height_1;
  m_avi_header[173] = height_2;
  m_aviFile->write(m_avi_header, AVI_OFFSET);
  return true;
}

void AviLibrary::uint32_write_to_aviFile(uint32_t v) {
  char value = v % 0x100;
  m_aviFile->write(value);  v = v >> 8; 
  value = v % 0x100;
  m_aviFile->write(value);  v = v >> 8;
  value = v % 0x100;
  m_aviFile->write(value);  v = v >> 8; 
  value = v;
  m_aviFile->write(value); 
}

bool AviLibrary::updateParameters() {
  /* overwrite riff file size */
  m_aviFile->seekSet(0x04);
  uint32_write_to_aviFile(m_total_size);

  /* overwrite hdrl */
  /* hdrl.avih.us_per_frame */
  m_aviFile->seekSet(0x20);
  uint32_write_to_aviFile(m_us_per_frame);
  m_aviFile->seekSet(0x24);
  uint32_write_to_aviFile(m_max_bytes_per_sec);

  /* hdrl.avih.tot_frames */
  m_aviFile->seekSet(0x30);
  uint32_write_to_aviFile(m_frames);
  m_aviFile->seekSet(0x84);
  uint32_write_to_aviFile(m_fps);

  /* hdrl.strl.list_odml.frames */
  m_aviFile->seekSet(0xe0);
  uint32_write_to_aviFile(m_frames);
  m_aviFile->seekSet(0xe8);
  uint32_write_to_aviFile(m_movi_size);  

  return true;
}