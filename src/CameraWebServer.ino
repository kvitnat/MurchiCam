#include "esp_camera.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "motor_controller.h"
#include <vector>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 'wifi_icon', 12x12px
const unsigned char wifi_icon [] = {
	0x00, 0xf0, 0x03, 0x80, 0x0e, 0x00, 0x18, 0x30, 0x30, 0xe0, 0x61, 0x80, 0x47, 0x00, 0xc4, 0x30, 
	0x8c, 0xe0, 0x88, 0x80, 0x99, 0xb0, 0x91, 0x30};

// Select camera model
// #define CAMERA_MODEL_WROVER_KIT
// #define CAMERA_MODEL_ESP_EYE
// #define CAMERA_MODEL_M5STACK_PSRAM
// #define CAMERA_MODEL_M5STACK_WIDE
// #define CAMERA_MODEL_AI_THINKER
#define CAMERA_MODEL_XIAO_ESP32S3

#include "camera_pins.h"
#include "esp32-hal-ledc.h"



float joystick_x = 0;
float joystick_y = 0;

void startCameraServer();

motion_controller m_controller;
driver_controller d_controller;

void initTextStyle()
{
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0, 16);     // Start at top-left corner
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
}

void printLineToScreen(const char* str, int size)
{
    for (int16_t i = 0; i < size; i++)
        display.write(str[i]);
    
    display.write('\n');
    display.display();
}

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
    }

    display.clearDisplay();
    display.display();
    initTextStyle();

    d_controller.init_driver_pins();
    
    pinMode(BUILTIN_LED, OUTPUT);
    digitalWrite(BUILTIN_LED, LOW);
    delay(1000);

    digitalWrite(BUILTIN_LED, HIGH);

    delay(7000);

    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    const char *ssid = "bvd23";
    const char *key = "5164g77r8c";
    const char *responderName = "espcam";

    File configFile = SPIFFS.open("/config.json", "r");
    DynamicJsonDocument doc(1024);
    if (!configFile)
    {
        Serial.println("Failed to open config.json for reading");
        return;
    }
    else
    {
        DeserializationError error = deserializeJson(doc, configFile);
        if (error)
        {
            Serial.print("Error parsing config.json [");
            Serial.print(error.c_str());
            Serial.println("]");
        }
        else
        {
            ssid = doc["ssid"];
            key = doc["key"];
            responderName = doc["mDNS"];
        }

        configFile.close();
    }
    SPIFFS.end();

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
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 23000000;
    config.pixel_format = PIXFORMAT_JPEG;
    // init with high specs to pre-allocate larger buffers
    if (psramFound())
    {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

#if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
#endif

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID)
    {
        s->set_vflip(s, 1);       // flip it back
        s->set_brightness(s, 1);  // up the blightness just a bit
        s->set_saturation(s, -2); // lower the saturation
    }
    // drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
#endif

    WiFi.begin(ssid, key);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    digitalWrite(BUILTIN_LED, LOW);
    Serial.println("");
    Serial.println("WiFi connected");
    IPAddress localIp = WiFi.localIP();
    Serial.println(localIp);

    display.drawBitmap(112, 0, wifi_icon, 12, 12, WHITE);

    printLineToScreen("WiFi CONNECTED", 15);
    printLineToScreen("", 0);
    printLineToScreen(localIp.toString().c_str(), localIp.toString().length());

    int responderStarted = 0;
    if (!MDNS.begin(responderName))
    {
        Serial.println("Error setting up mDNS responder");
    }
    else
    {
        responderStarted = 1;
        Serial.print("mDNS responder started: ");
        Serial.print(responderName);
        Serial.println(".local");
    }

    startCameraServer();

    Serial.print("Camera Ready! Use 'http://");
    if (responderStarted)
    {
        Serial.print(responderName);
        Serial.print(".local");
    }
    else
    {
        Serial.print(localIp);
    }
    Serial.println("' to connect");

}

void loop()
{
    if (m_controller.is_input_changed(joystick_x, joystick_y))
    {
        // calculate speed of each wheel 
        auto speed = m_controller.input(joystick_x, joystick_y);

        // feed calculated values into driver controller
        d_controller.set_speed(speed);

        if (d_controller.needs_starting_voltage())
        {
            d_controller.calculate_driver_values();
            d_controller.write_values_to_driver();
            delay(50);
        }

        d_controller.calculate_driver_values();
        d_controller.write_values_to_driver();
        
        Serial.print(d_controller.get_debug_info());
    }
}
