#ifndef TASK_BLUETOOTH_H
#define TASK_BLUETOOTH_H

#include <BluetoothSerial.h>
#include <esp_bt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Task_wifi_cloud.h"
#include <config.h>

extern Preferences prefs;
extern SystemConfig cfg;
extern String wssid, wpassword;
extern TaskHandle_t wifiCloud_handle;
extern EventGroupHandle_t sysEvent;
extern volatile bool wifiConfigured;

String parseSSID(String msg);
String parsePassword(String msg);
static bool btInited = false;
void TaskBluetooth(void *pvParameters);

#endif