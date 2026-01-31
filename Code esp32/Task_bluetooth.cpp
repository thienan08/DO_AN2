#include <Task_bluetooth.h>

String parseSSID(String msg) {
    String data = msg.substring(5);
    int sep = data.indexOf('|');
    if (sep < 0) return "";

    return data.substring(0, sep);
}

String parsePassword(String msg) {
    String data = msg.substring(5);   
    int sep = data.indexOf('|');
    if (sep < 0) return "";

    return data.substring(sep + 1);
}

void TaskBluetooth(void *pvParameters) {
    DEBUG_PRINTLN("[BT] Task Bluetooth");
    BluetoothSerial SerialBT;
    String ID = "IDS:" + String(ID1) + "|" + cfg.room1 + "," + String(ID2) + "|" + cfg.room2;
    DEBUG_PRINTLN("[BT] ID: " + ID);
    for (;;) {
        xEventGroupWaitBits(
            sysEvent,
            EVT_BT_MODE,
            pdFALSE,
            pdTRUE,
            portMAX_DELAY
        );

        if (!btInited) {
            esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            DEBUG_PRINTLN("[BT] BT init");
            SerialBT.begin("PMS-2025");
            btInited = true;
        }

        while (xEventGroupGetBits(sysEvent) & EVT_BT_MODE) {
            if (SerialBT.hasClient()) {
                if (SerialBT.available()) {
                    String msg = SerialBT.readStringUntil('\n');
                    msg.trim();

                    DEBUG_PRINTLN("[BT]" + msg);
                    
                    if (msg == "REQUIRE_ID") {
                        SerialBT.println(ID);
                    }

                    if (msg == "ACK_" + String(ID1) || msg == "ACK_" + String(ID2)) {
                        SerialBT.println("READY");
                    }
                    
                    if (msg.startsWith("WIFI:")) {
                        wssid = parseSSID(msg);
                        wpassword = parsePassword(msg);
                        DEBUG_PRINTLN(wssid);
                        DEBUG_PRINTLN(wpassword);
                        SerialBT.println("WIFI_OK");
                        vTaskDelay(1000 / portTICK_PERIOD_MS);
                        prefs.begin("wifi", false);
                        prefs.putString("ssid", wssid);
                        prefs.putString("password", wpassword);
                        prefs.end();
                        break;
                    }
                }
            }
        }
        SerialBT.end();
        esp_bt_controller_disable();
        btInited = false;
        wifiConfigured = true;
        xEventGroupClearBits(sysEvent, EVT_BT_MODE);
        xEventGroupSetBits(sysEvent, EVT_WIFI_MODE);
    }
}