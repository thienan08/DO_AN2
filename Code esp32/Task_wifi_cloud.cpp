#include <Task_wifi_cloud.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

//firebase object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

//Timestamp
String ts;
String ds;

void TaskWifiCloud(void *pvParameters) {
    DEBUG_PRINTLN("[WiFi] Task WiFi");
    config.api_key = cfg.fbapi;
    config.database_url = cfg.fburl;
    auth.user.email = cfg.email;
    auth.user.password = cfg.emailpass;
    config.token_status_callback = tokenStatusCallback;
    fbData fbDataReceive;
    for (;;) {
        xEventGroupWaitBits(
            sysEvent,
            EVT_WIFI_MODE,
            pdFALSE,
            pdTRUE,
            portMAX_DELAY
        ); 

        while (xEventGroupGetBits(sysEvent) & EVT_WIFI_MODE) {
            if (!wifiStarted) {
                DEBUG_PRINTLN("[WiFi] WiFi begin");
                WiFi.begin(wssid, wpassword);
                wifiStarted = true;
            }

            if (WiFi.status() != WL_CONNECTED) {
                DEBUG_PRINTLN("[WiFi] DISCONNECTED");
                WiFi.reconnect();
                vTaskDelay(3000 / portTICK_PERIOD_MS);
                continue;
            }

            if (!Firebase.ready()) {
                Firebase.begin(&config, &auth);
                Firebase.reconnectWiFi(true);
            }
            if (xQueueReceive(firebaseUpload, &fbDataReceive, pdMS_TO_TICKS(500)) == pdPASS) {
                ds = String(fbDataReceive.dayStamp);
                ts = String(fbDataReceive.timeStamp);

                // >> upload Firebase
                // > ID1
                Firebase.RTDB.setFloat(
                    &fbdo,
                    "/" + String(ID1) + "/Data/" + ds + "/" + ts,
                    fbDataReceive.energy_1
                );

                Firebase.RTDB.setInt(
                    &fbdo,
                    "/" + String(ID1) + "/Voltage",
                    fbDataReceive.voltage_1
                );
                
                // > ID2
                Firebase.RTDB.setFloat(
                    &fbdo,
                    "/" + String(ID2) + "/Data/" + ds + "/" + ts,
                    fbDataReceive.energy_2
                );

                Firebase.RTDB.setInt(
                    &fbdo,
                    "/" + String(ID2) + "/Voltage",
                    fbDataReceive.voltage_2
                );
            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        DEBUG_PRINTLN("[WiFi] Exit WiFi mode");
        wifiStarted = false;
    }
}