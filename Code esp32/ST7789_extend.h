#ifndef _ST7789_EXTEND_H
#define _ST7789_EXTEND_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

class ST7789_extend : public Adafruit_ST7789 {
private:
    uint16_t stored_x;
    uint16_t stored_y;
    String stored_text;
    uint8_t stored_textSize;
    uint16_t stored_textColor;
    uint16_t stored_screenColor;

    struct tableData {
        uint16_t x, y, w, h, rows, cols, color, thickness, gridThickness;

        tableData() 
            : x(0), y(0), w(0), h(0), rows(0), cols(0), 
            color(0), thickness(0), gridThickness(0) {}

        tableData(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                uint16_t rows, uint16_t cols, 
                uint16_t thickness, uint16_t gridThickness) {
            this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
            this->rows = rows;
            this->cols = cols;
            this->thickness = thickness;
            this->gridThickness = gridThickness;
        }
    };

    tableData tbDat;

public:
    ST7789_extend(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst)
        : Adafruit_ST7789(cs, dc, mosi, sclk, rst) {}
    ST7789_extend(int8_t cs, int8_t dc, int8_t rst) 
        : Adafruit_ST7789(cs, dc, rst) {}

    using Adafruit_ST7789::print;
    using Adafruit_ST7789::println;
    using Adafruit_ST7789::fillScreen;
    using Adafruit_ST7789::setTextSize;
    using Adafruit_ST7789::setCursor;
    using Adafruit_ST7789::setTextColor;
    
    void fillScreen(uint16_t color);
    void setTextSize(uint8_t size);
    void setCursor (uint16_t x, uint16_t y);
    void setTextColor(uint16_t color);
    using Adafruit_ST7789::print;
    
    template <typename T>
    void print(const T &text, uint16_t x, uint16_t y, uint8_t size, uint16_t color) {
        setCursor(x, y);
        setTextSize(size);
        setTextColor(color);
        Adafruit_ST7789::print(text);
        stored_text = text;
    };

    void println(const String &text, uint16_t x, uint16_t y, uint8_t size, uint16_t color, uint16_t dst = 1);
    void println(const String &text, uint16_t dst = 1, int color = -1);
    void deleteText(uint16_t x, uint16_t y, uint8_t size, uint16_t textLength);
    void drawTable(int x, int y, int w, int h, int rows, int cols,
                uint16_t color = ST77XX_WHITE, int thickness = 1, int gridThickness = 1);
    void initTable(int x, int y, int w, int h, int rows, int cols,
                uint16_t color = ST77XX_WHITE, int thickness = 1, int gridThickness = 1);
    void addTable(int h, int rows, int cols, uint16_t color, int thickness = 1, int gridThickness = 1);
};

#endif