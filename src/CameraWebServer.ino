#include "esp_camera.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "motor_controller.h"
#include <vector>

//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
//#define CAMERA_MODEL_AI_THINKER
#define CAMERA_MODEL_XIAO_ESP32S3


#include "camera_pins.h"
#include "esp32-hal-ledc.h"

const uint32_t pwm_frequency = 15000;

const uint8_t pwm_resolution = 8;

// const uint8_t pwm_channel = 0

bool bFlashLed = false;

float joystick_x = 0;
float joystick_y = 0;

int val = 0;
int prev_val = -1;

void startCameraServer();

motor_controller controller;

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    //pinMode(A0, OUTPUT);
    //pinMode(A1, OUTPUT);
    //pinMode(A2, OUTPUT);
    //pinMode(A3, OUTPUT);

    
    ledcSetup(A0, pwm_frequency, pwm_resolution);
    ledcSetup(A1, pwm_frequency, pwm_resolution);
    ledcSetup(A2, pwm_frequency, pwm_resolution);
    ledcSetup(A3, pwm_frequency, pwm_resolution);


    ledcAttachPin(A0, A0);
    ledcAttachPin(A1, A1);
    ledcAttachPin(A2, A2);
    ledcAttachPin(A3, A3);

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
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    //init with high specs to pre-allocate larger buffers
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
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID)
    {
        s->set_vflip(s, 1);       //flip it back
        s->set_brightness(s, 1);  //up the blightness just a bit
        s->set_saturation(s, -2); //lower the saturation
    }
    //drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
#endif

    WiFi.begin(ssid, key);

    while(WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    } 

    digitalWrite(BUILTIN_LED, LOW);
    Serial.println("");
    Serial.println("WiFi connected");
    IPAddress localIp = WiFi.localIP();
    Serial.println(localIp);

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
  /*
    if (bFlashLed)
    {
    	Serial.println("flashing LED...");
    	digitalWrite(BUILTIN_LED, LOW);
        delay(1000);
    	digitalWrite(BUILTIN_LED, HIGH);
        bFlashLed = false;
    }*/

    if (controller.is_input_changed(joystick_x, joystick_y))
    {
        controller.input2(joystick_x, joystick_y);
        
        ledcWrite(A0, controller.RIn1_speed);
        ledcWrite(A1, controller.RIn2_speed);
        ledcWrite(A2, controller.LIn2_speed);
        ledcWrite(A3, controller.LIn1_speed);
       
        Serial.print("Lin1 ");
        Serial.print(controller.LIn1_speed);
        Serial.print(", Lin2 ");
        Serial.print(controller.LIn2_speed);
        Serial.print(", Rin1 ");
        Serial.print(controller.RIn1_speed);
        Serial.print(", Rin2 ");
        Serial.println(controller.RIn2_speed);
    }    
}
