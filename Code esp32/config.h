#ifndef CONFIG_H
#define CONFIG_H

//Serial debug
#define SERIAL_DEBUG 1

#if SERIAL_DEBUG
    #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
    #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(...)
    #define DEBUG_PRINTLN(...)
    #define DEBUG_PRINTF(...)
#endif

// SPI PIN
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15         
#define TFT_DC    2  
#define TFT_RST   4       

//PZEM PIN
#define PZEM0_RX_PIN 16
#define PZEM0_TX_PIN 17
#define PZEM0_SERIAL Serial2

#define PZEM1_RX_PIN 14
#define PZEM1_TX_PIN 13
#define PZEM1_SERIAL Serial1

//RTC DS1307
#define SDA_PIN 27
#define SCL_PIN 26

//NTP
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 7 * 3600
#define DAYLIGHTOFFSET_SEC 0

//BUTTON
#define BUTTON 22
#define HOLD_TIME_MS 5000

//customer id
#define ID1 "C000001"
#define ID2 "C000002"

struct SystemConfig {
    String fburl;
    String fbapi;
    String email;
    String emailpass;
    String room1;
    String room2;
    String unitOrTier;
    String unitPrice;
};

//QUEUE DATAFRAME
typedef struct {
    char dayStamp[20];
    char timeStamp[25];
    uint8_t voltage_1;
    uint8_t voltage_2;
    float energy_1;
    float energy_2;
    long cost_1;
    long cost_2;
} fbData;

//Core 0 flag
#define EVT_WIFI_MODE (1 << 0)
#define EVT_BT_MODE   (1 << 1)

#endif