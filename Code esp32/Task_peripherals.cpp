#include "Task_peripherals.h"

// >> Tier Calculation (optional)
static long tieredElectricCalculate(int kwh, ElectricityTier tiers[], int size, bool includeVAT) {
    long total = 0;
    int remaining = kwh;

    for (int i = 0; i < size; i++) {
        if (remaining <= 0) break;

        int used = remaining;
        if (tiers[i].limit != -1 && used > tiers[i].limit) {
            used = tiers[i].limit;
        }

        total += (long)used * tiers[i].price;
        remaining -= used;
    }

    if (includeVAT) {
        total += total * 0.08; // vat = 8%
    }

    return total;
}

void TaskPeripherals(void *parameter) {
    // >> TFT init
    ST7789_extend tft(TFT_CS, TFT_DC, TFT_RST);

    // >> Pzem init
    PZEM004Tv30 pzem0(PZEM0_SERIAL, PZEM0_RX_PIN, PZEM0_TX_PIN);
    PZEM004Tv30 pzem1(PZEM1_SERIAL, PZEM1_RX_PIN, PZEM1_TX_PIN);

    //User data
    struct users_data {
        String Room;
        uint8_t voltage;
        float energy;
        uint16_t cost;

        users_data(const String& room, uint8_t voltageVal, float energyVal, uint16_t billVal)
            : Room(room), voltage(voltageVal), energy(energyVal), cost(billVal) {}
    };

    // >> User data init
    users_data A1(cfg.room1, 0, 0, 0);
    users_data A2(cfg.room2, 0, 0, 0);

    // >> RTC init
    RTC_DS1307 rtc;
    const char* daysOfWeek[] = {
    "CN",    // 0
    "T2",    // 1
    "T3",    // 2
    "T4",    // 3
    "T5",    // 4
    "T6",    // 5
    "T7"     // 6
    };

    struct RTC_Data {
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t dayOfWeek;

        RTC_Data()
        : year(0), month(0), day(0),
            hour(0), minute(0), second(0),
            dayOfWeek(0) {}

        RTC_Data(uint16_t y, uint8_t m, uint8_t d,
                uint8_t h, uint8_t min, uint8_t s, uint8_t dow)
        : year(y), month(m), day(d),
            hour(h), minute(min), second(s),
            dayOfWeek(dow) {}
    };

    char timeChar[40];
    char old_time[40];
    uint8_t timeOffset = 100;
    fbData fbDataSend;

    //Tier
    ElectricityTier tiers[] = {
        {50,   1984},   // tier 1
        {50,   2050},   // tier 2
        {100,  2380},   // tier 3
        {100,  2998},   // tier 4
        {100,  3350},   // tier 5
        {-1,   3460},   // tier 6
    };

    // >> Pre-configuration
    Wire.begin(SDA_PIN, SCL_PIN);
    rtc.begin();
    tft.init(240, 320); 
    tft.invertDisplay(0);
    tft.fillScreen(ST77XX_BLACK);
    tft.setRotation(1);
    tft.initTable(0, 0, 320, 50, 1, 1, ST77XX_BLUE);
    tft.addTable(240 - 50, 3, 2, ST77XX_BLUE);
    tft.print(A1.Room, 60, 70, 2, ST77XX_WHITE);
    tft.print(A2.Room, 220, 70, 2, ST77XX_WHITE);

    // >> Write data to Flash
    if (!isnan(pzem0.voltage()) || !isnan(pzem1.voltage())) {
        prefs.begin("storage", false);
        if (!isnan(pzem0.voltage())) {
            prefs.putDouble("energy_KH1", pzem0.energy());
            DEBUG_PRINTLN("[P] Writed energy ID1");
        }

        if (!isnan(pzem1.voltage())) {
            prefs.putDouble("energy_KH2", pzem1.energy());
            DEBUG_PRINTLN("[P] Writed energy ID2");
        }
        prefs.end();
    }

    //Read data from Flash
    prefs.begin("storage", true);
    A1.energy = prefs.getDouble("energy_KH1", 0);
    A2.energy = prefs.getDouble("energy_KH2", 0);
    prefs.end();
    DEBUG_PRINTLN(A1.energy);
    DEBUG_PRINTLN(A2.energy);

    for (;;) {
        DateTime t = rtc.now();
        RTC_Data now(t.year(), t.month(), t.day(),
                    t.hour(), t.minute(), t.second(),
                    t.dayOfTheWeek());
        
        sprintf(timeChar, "%s, %02d/%02d/%04d, %02d:%02d:%02d",
                daysOfWeek[now.dayOfWeek],
                now.day,
                now.month,
                now.year,
                now.hour,
                now.minute,
                now.second
        );
        
        if (strcmp(timeChar, old_time) != 0) {
            // DEBUG_PRINTLN(timeChar);
            tft.deleteText(5, 20, 2, 29);
            tft.print(timeChar, 5, 20, 2, ST77XX_WHITE);
            strcpy(old_time, timeChar);
        }
        
        if (!isnan(pzem0.voltage())) {
            A1.voltage = pzem0.voltage();
            A1.energy = pzem0.energy();
        }
        if (!isnan(pzem1.voltage())) {
            A2.voltage = pzem1.voltage();
            A2.energy = pzem1.energy();
        }

        if (cfg.unitOrTier == "tier") {
            A1.cost = tieredElectricCalculate(A1.energy, tiers, 6, true);
            A2.cost = tieredElectricCalculate(A2.energy, tiers, 6, true);
        }
        else if (cfg.unitOrTier == "unit") {
            A1.cost = A1.energy * cfg.unitPrice.toInt();
            A2.cost = A2.energy * cfg.unitPrice.toInt();
        }

        tft.deleteText(20, 140, 2, 10);
        tft.print(A1.energy, 20, 140, 2, ST77XX_WHITE);
        tft.print(" KWh");
        tft.deleteText(180, 140, 2, 10);
        tft.print(A2.energy, 180, 140, 2, ST77XX_WHITE);
        tft.print(" KWh");
        
        tft.deleteText(10, 200, 2, 12);
        tft.print(A1.cost, 10, 200, 2, ST77XX_WHITE);
        tft.print(" VND");
        tft.deleteText(170, 200, 2, 12);
        tft.print(A2.cost, 170, 200, 2, ST77XX_WHITE);
        tft.print(" VND");
        
        // >> Send every minute with hour timestamp
        if (now.second == 0 && timeOffset != now.minute) {
            fbDataSend.voltage_1 = A1.voltage;
            fbDataSend.voltage_2 = A2.voltage;
            fbDataSend.cost_1 = A1.cost;
            fbDataSend.cost_2 = A2.cost;
            fbDataSend.energy_1 = roundf(A1.energy * 100.0) / 100.0;
            fbDataSend.energy_2 = roundf(A2.energy * 100.0) / 100.0;
            sprintf(fbDataSend.dayStamp, "%04d-%02d-%02d",
                now.year,
                now.month,
                now.day
            );
            sprintf(fbDataSend.timeStamp, "%02d:00",
                now.hour
            );
            DEBUG_PRINTF("[Queue] Daystamp: %s\n", fbDataSend.dayStamp);
            DEBUG_PRINTF("[Queue] TimeStamp: %s\n", fbDataSend.timeStamp);
            xQueueSend(firebaseUpload, &fbDataSend, portMAX_DELAY);
            DEBUG_PRINTLN("[Queue] SENT QUEUE");
            timeOffset = now.minute;
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}