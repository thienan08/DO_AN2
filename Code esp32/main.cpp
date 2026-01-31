#include <Preferences.h>
#include <Wire.h>
#include <SPI.h>
#include <string>
#include <ctype.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <math.h>
#include <RTClib.h>
#include <WiFi.h>
#include "time.h"
#include "config.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Task_peripherals.h"
#include "Task_wifi_cloud.h"
#include "Task_bluetooth.h"

AsyncWebServer server(80);
Preferences prefs;

//Task handle
TaskHandle_t peripherals_handle = NULL;
TaskHandle_t wifiCloud_handle = NULL;
TaskHandle_t bluetooth_handle = NULL;

//Queue
QueueHandle_t firebaseUpload;

// >> button interrupt (start)
volatile bool btnpressed = false;
volatile bool wifiConfigured = false;
TimerHandle_t btnTimer;
volatile uint32_t btnPressTick = 0;

// >> interrupt
void IRAM_ATTR buttonISR() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (digitalRead(BUTTON) == LOW) {
        btnpressed = true;
        btnPressTick = xTaskGetTickCountFromISR();
        xTimerStartFromISR(btnTimer, &xHigherPriorityTaskWoken);
        ets_printf("[ISR] Button PRESSED\n");
    } 
    else {
        btnpressed = false;
        xTimerStopFromISR(btnTimer, &xHigherPriorityTaskWoken);
        ets_printf("[ISR] Button RELEASED\n");
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

EventGroupHandle_t sysEvent;
void btnTimerCallback(TimerHandle_t xTimer) {
    if (!btnpressed) {
        DEBUG_PRINTLN("[TIMER] Button released before 5s -> IGNORE");
        return;
    }

    if (!wifiConfigured) {
        DEBUG_PRINTLN("[BTN] WiFi not configured → BT LOCKED");
        return; // LOCKED
    }

    EventBits_t bits = xEventGroupGetBits(sysEvent);

    if (bits & EVT_BT_MODE) {
        DEBUG_PRINTLN("[MODE] BT -> WIFI");
        xEventGroupClearBits(sysEvent, EVT_BT_MODE);
        xEventGroupSetBits(sysEvent, EVT_WIFI_MODE);
    }
    else if (bits & EVT_WIFI_MODE) {
        DEBUG_PRINTLN("[MODE] WIFI -> BT");
        xEventGroupClearBits(sysEvent, EVT_WIFI_MODE);
        xEventGroupSetBits(sysEvent, EVT_BT_MODE);
    }
    else {
        DEBUG_PRINTLN("[MODE] NONE → BT");
        xEventGroupSetBits(sysEvent, EVT_BT_MODE);
    }
}

// >> checking configurations
bool isConfigSaved() {
    if (!prefs.begin("config", true)) {
        prefs.end();
        return false;
    }

    bool hasFBurl     = prefs.isKey("fburl");
    bool hasFBapi     = prefs.isKey("fbapi");
    bool hasEmail     = prefs.isKey("email");
    bool hasEmailPass = prefs.isKey("emailpass");
    bool hasRoom1     = prefs.isKey("room1");
    bool hasRoom2     = prefs.isKey("room2");
    bool hasUnitTier  = prefs.isKey("unitOrTier");
    bool hasunitPrice = prefs.isKey("unitPrice");

    prefs.end();

    return (
        hasFBurl && hasFBapi &&
        hasEmail && hasEmailPass &&
        hasRoom1 && hasRoom2 &&
        hasUnitTier && hasunitPrice
    );
}

// >> Access Point init
void startAP(const String &ssid, const String &password) {
    DEBUG_PRINTLN("[AP] Starting AP mode...");

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), password.c_str());

    DEBUG_PRINTLN("[AP] AP IP: ");
    DEBUG_PRINTLN(WiFi.softAPIP());

    if (!LittleFS.begin(true)) {
        DEBUG_PRINTLN("[LittleFS] LittleFS mount failed!");
        ESP.restart();
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", "text/html");
    });

    server.serveStatic("/", LittleFS, "/");

    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
        prefs.begin("config");
        prefs.putString("fburl",      request->arg("fburl"));
        prefs.putString("fbapi",      request->arg("fbapi"));
        prefs.putString("email",      request->arg("email"));
        prefs.putString("emailpass",  request->arg("emailpass"));
        prefs.putString("room1",      request->arg("room1"));
        prefs.putString("room2",      request->arg("room2"));
        prefs.putString("unitOrTier", request->arg("unitOrTier"));
        prefs.putString("unitPrice",  request->arg("unitPrice"));
        prefs.end();

        request->send(200, "text/plain", "Saved! Rebooting...");
        DEBUG_PRINTLN("[Config] Config saved -> Restarting");
        delay(500);
        ESP.restart();
    });

    server.on("/exit", HTTP_GET, [](AsyncWebServerRequest *request) {
        DEBUG_PRINTLN("[AP] Exiting");
        request->send(200, "text/plain", "Exiting setup mode...");
        ESP.restart();
    });

    server.begin();
    DEBUG_PRINTLN("[AP] AP server started");
}

SystemConfig cfg;

// >> Load Configurations
bool loadConfig() {
    if (!prefs.begin("config", true)) {
        prefs.end();
        return false;
    }

    cfg.fburl      = prefs.getString("fburl", "");
    cfg.fbapi      = prefs.getString("fbapi", "");
    cfg.email      = prefs.getString("email", "");
    cfg.emailpass  = prefs.getString("emailpass", "");
    cfg.room1      = prefs.getString("room1", "");
    cfg.room2      = prefs.getString("room2", "");
    cfg.unitOrTier = prefs.getString("unitOrTier", "");
    cfg.unitPrice  = prefs.getString("unitPrice", "");

    prefs.end();
    return true;
}

ST7789_extend *tft = new ST7789_extend(TFT_CS, TFT_DC, TFT_RST);
String wssid, wpassword;

void setup() {
    Serial.begin(115200);
    firebaseUpload = xQueueCreate(30, sizeof(fbData));

    DEBUG_PRINTLN("[Main] CHECKING CONFIG!");
    tft->init(240, 320);
    tft->invertDisplay(0);
    tft->setRotation(1);
    if (!isConfigSaved()) {
        DEBUG_PRINTLN("[Main] Config not found");
        startAP("config", "12345678");
        tft->fillScreen(ST77XX_BLACK);
        tft->print("Waiting for config...", 1, 1, 2, ST77XX_WHITE);
        while(!isConfigSaved()) {
            delay(500);
        }
    }
    else {
        loadConfig();
    }
    
    tft->fillScreen(ST77XX_BLACK);
    delete tft;
    tft = nullptr;

    sysEvent = xEventGroupCreate();

    DEBUG_PRINTLN("[Main] CHECKING WIFI!");
    prefs.begin("wifi", true);
    if (!prefs.isKey("ssid") || !prefs.isKey("password")) {
        DEBUG_PRINTLN("[Main] No Wifi");
        xEventGroupSetBits(sysEvent, EVT_BT_MODE);
        wifiConfigured = false;
    } else {
        wssid = prefs.getString("ssid", "");
        wpassword = prefs.getString("password", "");
        DEBUG_PRINTLN(wssid);
        DEBUG_PRINTLN(wpassword);
        xEventGroupSetBits(sysEvent, EVT_WIFI_MODE);
        wifiConfigured = true;
    }
    prefs.end();
    DEBUG("[Main] Begin tasks");

    btnTimer = xTimerCreate(
        "btnTimer",
        pdMS_TO_TICKS(HOLD_TIME_MS),
        pdFALSE,
        NULL,
        btnTimerCallback
    );

    //Button config
    pinMode(BUTTON, INPUT_PULLUP);
    attachInterrupt(BUTTON, buttonISR, CHANGE);

    xTaskCreatePinnedToCore(
        TaskBluetooth,
        "Bluetooth",
        10000,
        NULL,
        3,
        &bluetooth_handle,
        1
    );

    xTaskCreatePinnedToCore(
        TaskWifiCloud,
        "Wifi Cloud",
        10000,
        NULL,
        3,
        &wifiCloud_handle,
        1
    );

    xTaskCreatePinnedToCore(
        TaskPeripherals,
        "Normal Operation",
        4096,
        NULL,
        6,
        &peripherals_handle,
        0
    );
}

void loop() {}