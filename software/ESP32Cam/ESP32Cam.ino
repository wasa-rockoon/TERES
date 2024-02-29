
// #define V1
#define V2
// #define V3

#if defined V1 || defined V2
#include <SD.h>            // SD Card ESP32
#include <SdFat.h>
#elif defined V3
#define SOC_SDMMC_HOST_SUPPORTED 1
#include <SD_MMC.h>
#endif

#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include "AVIWriter.h"
#include <EEPROM.h>            // read and write from flash memory
#include <SPI.h>
#include <esp_task_wdt.h>
#include <TaskScheduler.h>

#include <UARTBus.hpp>
#include <Indicator.h>
#include <Datatypes.h>

#define NODE 'c'

#define OV5640

#define SPI0_SCK 11
#define SPI0_MOSI 10
#define SPI0_MISO 12
#define SPI0_CS 9
#define SDCARD_MOSI_PIN SPI0_MOSI
#define SDCARD_MISO_PIN SPI0_MISO
#define SDCARD_SS_PIN SPI0_CS
#define SDCARD_SCK_PIN SPI0_SCK

// #define SDMMC_CLK_PIN 11
// #define SDMMC_CMD_PIN 10
// #define SDMMC_D0_PIN 12
// #define SDMMC_D1_PIN 8
// #define SDMMC_D2_PIN 18
// #define SDMMC_D3_PIN 9


#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
#include "camera_pins.h"


/* #define LED_STATUS_PIN 13 */
#if defined V1
#define LED_STATUS_PIN 14
#define LED_ERROR_PIN 19
// #define SWITCH_PIN 0
#define TX0_PIN 2
#define RX0_PIN 20
#define TX1_PIN 16
#define RX1_PIN 17

#elif defined V2 || defined V3
#define LED_STATUS_PIN 13
#define LED_ERROR_PIN 14
#define TX0_PIN 2
#define RX0_PIN 44
#define TX1_PIN 16
#define RX1_PIN 10
#define CAMERA_POWER_PIN 43

#endif

#define MIC_PIN 1

// Constants
#define SERIAL_BAUD 115200
#define BUS_FIFO_SIZE 4096

#define EEPROM_SIZE 5

#define SEND_TLM_FREQ 1

#define CHANGE_FOLDER_FRAMES (30 * 5)


Scheduler scheduler;

void send_tlm();
Task task_send_tlm(
  TASK_SECOND / SEND_TLM_FREQ, TASK_FOREVER,
  &send_tlm, &scheduler, false);



#if defined V1 || defined V2
typedef SdFat32 sd_t;
typedef File32 file_t;
sd_t sd;
#define SPI_CLOCK SD_SCK_MHZ(50)
#define SD_CONFIG SdSpiConfig(SDCARD_SS_PIN, DEDICATED_SPI, SPI_CLOCK)
#elif defined V3
#define sd SD_MMC
#define SD SD_MMC
typedef File file_t;
#endif

AVIWriter avi;


uint8_t framebuffer_static[128 * 1024];
//struct tm lt;


TaskHandle_t thp[2];


uint16_t pictureNumber = 0;
// unsigned frame = 33;

file_t file;

bool camera_ok = false;
bool sd_ok = false;
bool recording = true;

unsigned long start_millis;
unsigned frame_count;
unsigned actual_frame_count;
unsigned wrote_bytes;


UARTBus bus(NODE, Serial2, Serial1);
Indicator indicator(LED_STATUS_PIN, LED_ERROR_PIN);

Shared<FlightMode> flight_mode(FlightMode::STANDBY);

unsigned bus_error_count_;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  // delay(5000);

  Serial1.begin(SERIAL_BAUD, SERIAL_8N1, RX0_PIN, TX0_PIN);
  Serial2.begin(SERIAL_BAUD, SERIAL_8N1, RX1_PIN, TX1_PIN);

  bus.begin();
  bus.subscribe(flight_mode, TELEMETRY, 'B', 'm');

  indicator.begin();

  bus_error_count_ = bus.getErrorCount();

#ifdef V2
  pinMode(CAMERA_POWER_PIN, OUTPUT);
  digitalWrite(CAMERA_POWER_PIN, HIGH);
#endif

  EEPROM.begin(EEPROM_SIZE); 

  /* pinMode(SWITCH_PIN, INPUT_PULLUP); */

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
#ifdef OV5640
  config.xclk_freq_hz = 54000000;
#else
  config.xclk_freq_hz = 28000000;
#endif
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
#ifdef OV5640
    config.frame_size = FRAMESIZE_XGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    avi.video.width = 1024;
    avi.video.height = 768;
    avi.video.fps = 25;
#else
    config.frame_size = FRAMESIZE_SVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    avi.video.width = 800;
    avi.video.height = 600;
    avi.video.fps = 25;
#endif
    config.jpeg_quality = 12;
    config.fb_count = 8;

  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM; 
    avi.video.width = 800;
    avi.video.height = 600;
    avi.video.fps = 25;
  }

// Init Camera
  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  camera_ok = true;

  #if defined V1 || defined V2

  SPI.begin(SPI0_SCK, SPI0_MISO, SPI0_MOSI, SPI0_CS);

  if (!sd.begin(SD_CONFIG)) {
    Serial.printf("SD Fail");
    sd.initErrorHalt(&Serial);
  }

  #elif defined V3

  Serial.println("SD V2");

  pinMode(SDMMC_CLK_PIN, INPUT_PULLUP);
  pinMode(SDMMC_CMD_PIN, INPUT_PULLUP);

  if (!sd.setPins(SDMMC_CLK_PIN, SDMMC_CMD_PIN, SDMMC_D0_PIN, SDMMC_D1_PIN, SDMMC_D2_PIN, SDMMC_D3_PIN)) {
    Serial.println("Pin Set Fail");
  }

  pinMode(SDMMC_D0_PIN, INPUT_PULLUP);
  pinMode(SDMMC_D1_PIN, INPUT_PULLUP);
  pinMode(SDMMC_D2_PIN, INPUT_PULLUP);
  pinMode(SDMMC_D3_PIN, INPUT_PULLUP);

  while (!sd.begin("/sdcard", true)) {
    Serial.println("SD Fail");
    delay(1000);
  }
  #endif

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD Card attached");
    // return;
  }
  sd_ok = true;

  if (recording) new_capture();

  Serial.println("Start");

  xTaskCreatePinnedToCore(core0, "core0", 8192, NULL, 10, &thp[0], 0);
  xTaskCreatePinnedToCore(core1, "core1", 8192, NULL, 10, &thp[1], 1);

  task_send_tlm.enable();

  indicator.clearError();
}

void new_capture() {
  EEPROM.get(1, pictureNumber);
  pictureNumber++;

  char file_name[16];
  sprintf(file_name,"/%d.avi", pictureNumber);

  if (!file.open(file_name, O_RDWR | O_CREAT)) {
      Serial.print(" Open failed, ");
  } 
  else {
    Serial.printf("Opened %s\n", file_name);
    //file.write(fb->buf, fb->len); // payload (image), payload lengt
      // file.write(framebuffer_static, fb->len);
      // Serial.printf(" Saved %d kB, ", fb->len / 1000);
    avi.start(file);
  }

  // char dir_name[16];
  // sprintf(dir_name,"/c%d",pictureNumber);
  // if (sd.mkdir(dir_name)) {
  //   Serial.printf("Make dir: %s\n", dir_name);
  // }
  // else {
  //   Serial.printf("Make dir failed: %s\n", dir_name);
  // }

  EEPROM.put(1, pictureNumber);
  EEPROM.commit();

  // captured_millis = millis();
  start_millis = millis();
  frame_count = 0;
  actual_frame_count = 0;
}

void core0(void* argv) {

  for(;;) {
    loop0();
    vTaskDelay(1);
  }
}

void core1(void* argv) {

  for(;;) {
    loop1();
    vTaskDelay(1);
  }
}

void loop() {

}

void loop0() {
  bus.update();

  // Serial1.printf("1");
  // Serial2.printf("2");

  // Serial.printf("core1: %d\n", xPortGetCoreID());

  // Serial.printf(".");

  // Serial.println(analogRead(MIC_PIN));

  bus.sanity(1, camera_ok);
  bus.sanity(2, sd_ok);

  if (bus.getErrorCount() != bus_error_count_) {
    indicator.errorEvent();
    bus_error_count_ = bus.getErrorCount();
  }

  scheduler.execute();
}

void send_tlm() {
  static unsigned last_frame_count;
  static unsigned last_actual_frame_count;
  static unsigned last_wrote_bytes;

  if (camera_ok && sd_ok && recording) {

    unsigned fps = (frame_count - last_frame_count) * SEND_TLM_FREQ;
    unsigned actual_fps = (actual_frame_count - last_actual_frame_count) * SEND_TLM_FREQ;
    unsigned Bps = (wrote_bytes - last_wrote_bytes) * SEND_TLM_FREQ;

    last_frame_count = frame_count;
    last_actual_frame_count = actual_frame_count;
    last_wrote_bytes = wrote_bytes;

    uint8_t buf[BUF_SIZE(4)];
    Packet cam(buf, sizeof(buf));
    cam.set(TELEMETRY, 'c');
    cam.begin()
      .append('C', frame_count)
      .append('F', fps)
      .append('f', actual_fps)
      .append('B', Bps);
    bus.send(cam);

    Serial.printf("%ds: %d (%d) fps, %d kBps, %d %s %d\n", 
      (millis() - start_millis) / 1000, fps, actual_fps, Bps / 1000,
      bus_error_count_, bus.error_code_, flight_mode.value());

    indicator.blink(500);
  }
  else indicator.blink(10);
}

void loop1() {

  // Serial.printf("core0: %d\n", xPortGetCoreID());

  /* if (!digitalRead(SWITCH_PIN)) recording = true; */

  // unsigned long start_millis = millis();

  if (flight_mode.value() == FlightMode::RECOVERY) {
    recording = false;
#ifdef V2
    digitalWrite(CAMERA_POWER_PIN, LOW);
#endif
  }
  else {
    if (!recording) {
        recording = true;
#ifdef V2
        digitalWrite(CAMERA_POWER_PIN, HIGH);
#endif
        new_capture();
      }
  }

  if (camera_ok && sd_ok && recording) {
    camera_fb_t * fb = NULL;
    // Take Picture with Camera
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    // memcpy(framebuffer_static, fb->buf, fb->len);

  // unsigned frame_millis = millis() - captured_millis;
  // captured_millis += 1000.0f / avi.video.fps

// Path where new picture will be saved in SD Card
    // String path = "/c" + String(pictureNumber) + "/pic" + String(i) + ".jpg";

    // fs::FS &fs = SD;
    // Serial.print(path);

#if defined V1 || defined V2
    // unsigned long makefile_millis = millis();

    avi.addVideoFrame(fb->buf, fb->len);
    // avi.updateParameters();

    // if (!file.open(path.c_str(), O_RDWR | O_CREAT)) {
    //   Serial.print(" Open failed, ");
    // } else {
    //   //file.write(fb->buf, fb->len); // payload (image), payload lengt
    //   makefile_millis = millis();
    //   file.write(framebuffer_static, fb->len);
      // Serial.printf("%d kB, ", fb->len / 1000);
    // }
    // file.close();
#elif defined V3
    File file = SD_MMC.open(path.c_str(), FILE_WRITE);
    if (!file) {
      Serial.print(" Open failed, ");
    }else {
      //file.write(fb->buf, fb->len); // payload (image), payload lengt
      file.write(framebuffer_static, fb->len);
      // Serial.printf(" Saved %d kB, ", fb->len / 1000);
    }
    file.close();
#endif

    esp_camera_fb_return(fb);

    frame_count++;
    actual_frame_count++;

    float frame_per_ms = avi.video.fps / 1000.0;
    float frame_ms = 1000.0 / avi.video.fps;

    // Serial.printf("%d %f %f\n", millis() - start_millis, frame_count / frame_per_ms, frame_ms);

    while (millis() - start_millis > (unsigned)(frame_count / frame_per_ms + frame_ms)) {
      frame_count++;
      avi.skipFrame();
      // Serial.print("skip ");
    }
    while (millis() - start_millis < (unsigned)(frame_count / frame_per_ms - frame_ms)) {
      delay(1);
    }

    wrote_bytes += fb->len;

    // unsigned long captured_millis = millis();

    // frame_count++;
    // i++;
    // unsigned long end_millis = millis();

    // unsigned long m = end_millis - start_millis;

    // long sleep_millis = frame - m;

    // if (m > frame) { // skip frame
    //   // avi.skipFrame();
    //   sleep_millis = sleep_millis % frame;
    //   i++;
    // }

    // Serial.printf("%ld cap, %ld write, %ld sleep, skip:%d\n",
    //               captured_millis - start_millis,
    //               // makefile_millis - captured_millis,
    //               end_millis - captured_millis,
    //               sleep_millis, m > frame);

    // delay(sleep_millis);

    // indicator.blink(1);
  }
}
