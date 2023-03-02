
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include <SD.h>            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory
#include <SPI.h>
#include "SdFat.h"
#include <esp_task_wdt.h>
#include <TaskScheduler.h>

#include "Bus.h"
#include "Indicator.h"

/* #define OV5640 */

#define SPI0_SCK 11
#define SPI0_MOSI 10
#define SPI0_MISO 12
#define SPI0_CS 9
#define SDCARD_MOSI_PIN SPI0_MOSI
#define SDCARD_MISO_PIN SPI0_MISO
#define SDCARD_SS_PIN SPI0_CS
#define SDCARD_SCK_PIN SPI0_SCK
#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
#include "camera_pins.h"


/* #define LED_STATUS_PIN 13 */
#define LED_STATUS_PIN 14
#define LED_ERROR_PIN 19
#define SWITCH_PIN 0

#define TX0_PIN 2
#define RX0_PIN 20
#define TX1_PIN 16
#define RX1_PIN 17

// Constants
#define SERIAL_BAUD 115200
#define BUS_FIFO_SIZE 4096

#define EEPROM_SIZE 1

#define SEND_TLM_FREQ 1


Scheduler scheduler;

void send_tlm();
Task task_send_tlm(
  TASK_SECOND / SEND_TLM_FREQ, TASK_FOREVER,
  &send_tlm, &scheduler, false);



typedef SdFat32 sd_t;
typedef File32 file_t;
uint8_t framebuffer_static[128 * 1024];
#define SPI_CLOCK SD_SCK_MHZ(50)
#define SD_CONFIG SdSpiConfig(9, DEDICATED_SPI, SPI_CLOCK)
//struct tm lt;


TaskHandle_t thp[1];


int pictureNumber = 0;
int i;
unsigned frame = 40;


sd_t sd;
file_t file;

bool camera_ok = false;
bool recording = false;


Bus bus(Serial2, Serial1);
Indicator indicator(LED_STATUS_PIN, LED_ERROR_PIN);

unsigned bus_error_count_;

unsigned frame_count = 0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  Serial1.begin(SERIAL_BAUD, SERIAL_8N1, RX0_PIN, TX0_PIN);
  Serial2.begin(SERIAL_BAUD, SERIAL_8N1, RX1_PIN, TX1_PIN);

  bool bus_ok = bus.begin();
  bool indicator_ok = indicator.begin();

  bus_error_count_ = bus.getErrorCount();

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
#else
    config.frame_size = FRAMESIZE_SVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
#endif
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

// Init Camera
  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  SPI.begin(SPI0_SCK, SPI0_MISO, SPI0_MOSI, SPI0_CS);

  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD Card attached");
    /* return; */
  }

  if (recording) new_capture();

  camera_ok = true;

  Serial.println("Start");

  xTaskCreatePinnedToCore(core0, "core0", 8192, NULL, 3, &thp[0], 0);

  indicator.clearError();
}

void new_capture() {
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;
  char dir_name[16];
  sprintf(dir_name,"/cam%d",pictureNumber);
  if (sd.mkdir(dir_name)) {
    Serial.printf("Make dir: %s\n", dir_name);
  }
  else {
    Serial.printf("Make dir failed: %s\n", dir_name);
  }
  EEPROM.write(0, pictureNumber);
  EEPROM.commit();
}

void core0(void* argv) {
  task_send_tlm.enable();

  bus.listen('m', changeMode);

  for(;;) {
    loop1();
    vTaskDelay(1);
  }
}

void changeMode(const Message& message) {
  /* if (message.get('F')) { */
    if (!recording) {
      recording = true;
      new_capture();
    }
  /* } */
  /* else if (message.get('S')) { */
  /*   recording = false; */
  /* } */
}


void loop() {
  bus.receive();

  if (bus.getErrorCount() != bus_error_count_) {
    indicator.errorEvent();
    bus_error_count_ = bus.getErrorCount();
  }
}

void send_tlm() {
  static unsigned last_frame_count;
  if (camera_ok && recording) {

    unsigned fps = (frame_count - last_frame_count) * SEND_TLM_FREQ;

    last_frame_count = frame_count;

    Message message('r', 0, 1);
    message.entries[0].set('f', (uint32_t)fps);
    bus.send(message);
  }
}

void loop1() {

  /* if (!digitalRead(SWITCH_PIN)) recording = true; */

  unsigned long start_millis = millis();

  if (camera_ok && recording) {
    camera_fb_t * fb = NULL;
    // Take Picture with Camera
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    memcpy(framebuffer_static, fb->buf,  fb->len);

    unsigned long captured_millis = millis();

// Path where new picture will be saved in SD Card
    String path = "/cam" + String(pictureNumber) + "/pic" + String(i) + ".jpg";

    fs::FS &fs = SD;
    Serial.print(path);
    if (!file.open(path.c_str(), O_RDWR | O_CREAT)) {
      Serial.print(" Open failed, ");
    }else {
      //file.write(fb->buf, fb->len); // payload (image), payload lengt
      file.write(framebuffer_static, fb->len);
      Serial.printf(" Saved %d kB, ", fb->len / 1000);
    }
    file.close();

    esp_camera_fb_return(fb);

    i++;
    unsigned long end_millis = millis();

    unsigned long m = end_millis - start_millis;

    long sleep_millis = frame - m;

    if (m > frame) {
      sleep_millis = sleep_millis % frame;
      i++;
    }


    Serial.printf("%d cap, %d sd, %d sleep, skip:%d\n",
                  captured_millis - start_millis,
                  end_millis - captured_millis,
                  sleep_millis, m > frame);

    delay(sleep_millis);

    indicator.blink(10);
  }
}
