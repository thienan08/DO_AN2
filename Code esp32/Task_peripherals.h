#ifndef TASK_PERIPHERALS_H
#define TASK_PERIPHERALS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <Wire.h>
#include <string>
#include <ctype.h>
#include <Preferences.h>
#include "config.h"
#include <PZEM004Tv30.h>
#include "ST7789_extend.h"
#include <RTClib.h>
#include <config.h>

extern Preferences prefs;
extern SystemConfig cfg;
extern QueueHandle_t firebaseUpload;

typedef struct {
    int limit;
    int price;   
} ElectricityTier;
static long tieredElectricCalculate(int kwh, ElectricityTier tiers[], int size, bool includeVAT);
void TaskPeripherals(void *pvParameters);


#endif