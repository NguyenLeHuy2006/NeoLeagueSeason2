/* Edge Impulse Arduino examples */

#include <smart_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"

#include "esp_camera.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

Servo myServo;

/* ---------------- CAMERA MODEL ---------------- */
#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#endif

/* ---------------- CONSTANTS ---------------- */

#define EI_CAMERA_RAW_FRAME_BUFFER_COLS 320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS 240
#define EI_CAMERA_FRAME_BYTE_SIZE 3

#define I2C_SDA 15
#define I2C_SCL 14
#define SERVO_PIN 12

TwoWire I2Cbus = TwoWire(0);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Cbus, OLED_RESET);

/* ---------------- VARIABLES ---------------- */

static bool debug_nn = false;
static bool is_initialised = false;
uint8_t *snapshot_buf;

/* ---------------- CAMERA CONFIG ---------------- */

static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,

    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,

    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_QVGA,
    .jpeg_quality = 12,
    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

/* ---------------- SETUP ---------------- */

void setup() {

    Serial.begin(115200);

    I2Cbus.begin(I2C_SDA, I2C_SCL, 100000);

    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

    myServo.attach(SERVO_PIN);
    myServo.write(90);

    while (!Serial);

    Serial.println("Edge Impulse Smart Dustbin");

    ei_camera_init();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("SMART BIN READY");
    display.display();

    ei_sleep(2000);
}

/* ---------------- LOOP ---------------- */

void loop() {

    if (ei_sleep(5) != EI_IMPULSE_OK) return;

    snapshot_buf = (uint8_t*)malloc(
        EI_CAMERA_RAW_FRAME_BUFFER_COLS *
        EI_CAMERA_RAW_FRAME_BUFFER_ROWS *
        EI_CAMERA_FRAME_BYTE_SIZE);

    if(snapshot_buf == nullptr) {
        Serial.println("Buffer alloc failed");
        return;
    }

    ei::signal_t signal;
    signal.total_length =
        EI_CLASSIFIER_INPUT_WIDTH *
        EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    if (!ei_camera_capture(
            EI_CLASSIFIER_INPUT_WIDTH,
            EI_CLASSIFIER_INPUT_HEIGHT,
            snapshot_buf)) {

        free(snapshot_buf);
        return;
    }

    ei_impulse_result_t result = {0};
    run_classifier(&signal, &result, debug_nn);

/* =====================================================
   OBJECT DETECTION RESULT
=====================================================*/

#if EI_CLASSIFIER_OBJECT_DETECTION == 1

    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {

        ei_impulse_result_bounding_box_t bb =
            result.bounding_boxes[i];

        if (bb.value == 0) continue;

        String label = String(bb.label);

        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0,20);
        display.print(label);
        display.display();

/* =====================================================
   TRASH GROUPING SYSTEM
=====================================================*/

        bool HARD_TRASH = false;
        bool SOFT_TRASH = false;

        // HARD ITEMS
        if (label == "plastic" ||
            label == "metal") {
            HARD_TRASH = true;
        }

        // SOFT ITEMS
        if (label == "paper" ||
            label == "cardboard") {
            SOFT_TRASH = true;
        }

/* =====================================================
   SERVO ACTION
=====================================================*/

if (HARD_TRASH) {

    Serial.println("HARD TRASH - LEFT");

    display.clearDisplay();
    display.setCursor(0,20);
    display.setTextSize(2);
    display.print("HARD");
    display.display();

    myServo.write(0);     // LEFT BIN (HARD)
    ei_sleep(2000);

    myServo.write(90);    // RETURN HORIZONTAL
}

else if (SOFT_TRASH) {

    Serial.println("SOFT TRASH - RIGHT");

    display.clearDisplay();
    display.setCursor(0,20);
    display.setTextSize(2);
    display.print("SOFT");
    display.display();

    myServo.write(180);   // RIGHT BIN (SOFT)
    ei_sleep(2000);

    myServo.write(90);    // RETURN HORIZONTAL
}
    }

#endif

    free(snapshot_buf);
}

/* ---------------- CAMERA FUNCTIONS ---------------- */

bool ei_camera_init(void) {

    if (is_initialised) return true;

    if (esp_camera_init(&camera_config) != ESP_OK)
        return false;

    is_initialised = true;
    return true;
}

bool ei_camera_capture(uint32_t img_width,
                       uint32_t img_height,
                       uint8_t *out_buf) {

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) return false;

    bool converted =
        fmt2rgb888(fb->buf, fb->len,
                   PIXFORMAT_JPEG,
                   snapshot_buf);

    esp_camera_fb_return(fb);

    if(!converted) return false;

    ei::image::processing::crop_and_interpolate_rgb888(
        out_buf,
        EI_CAMERA_RAW_FRAME_BUFFER_COLS,
        EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
        out_buf,
        img_width,
        img_height);

    return true;
}

static int ei_camera_get_data(size_t offset,
                              size_t length,
                              float *out_ptr) {

    size_t pixel_ix = offset * 3;

    for(size_t i=0;i<length;i++){
        out_ptr[i] =
        (snapshot_buf[pixel_ix+2]<<16) +
        (snapshot_buf[pixel_ix+1]<<8) +
        snapshot_buf[pixel_ix];
        pixel_ix+=3;
    }
    return 0;
}