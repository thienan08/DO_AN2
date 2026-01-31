#include "ST7789_extend.h"
#include <string.h>

void ST7789_extend::fillScreen(uint16_t color) {
    Adafruit_ST7789::fillScreen(color);
    stored_screenColor = color;
}

void ST7789_extend::setTextSize(uint8_t size) {
    Adafruit_ST7789::setTextSize(size);
    stored_textSize = size;
}

void ST7789_extend::setCursor (uint16_t x, uint16_t y) {
    Adafruit_ST7789::setCursor(x, y);
    stored_x = x;
    stored_y = y;
}

void ST7789_extend::setTextColor(uint16_t color) {
    Adafruit_ST7789::setTextColor(color);
    stored_textColor = color;
}


void ST7789_extend::println(const String &text, uint16_t x, uint16_t y, uint8_t size, uint16_t color, uint16_t dst) {
    setCursor(x, y);
    setTextSize(size);
    setTextColor(color);
    Adafruit_ST7789::println(text);
    stored_text = text;
    cursor_x += stored_x;
    cursor_y += (dst == 1)? 0 : dst;
    Serial.println(cursor_y);
}

void ST7789_extend::println(const String &text, uint16_t dst, int color) {
    setTextColor((color != -1) ? color : stored_textColor);
    Adafruit_ST7789::println(text);
    stored_text = text;
    cursor_x += stored_x;
    cursor_y += (dst == 1)? 0 : dst;
    Serial.println(cursor_y);
}

void ST7789_extend::deleteText(uint16_t x, uint16_t y, uint8_t size, uint16_t textLength) {
    fillRect
    (
        x, 
        y, 
        6 * textLength * size, 
        8 * size, 
        stored_screenColor
    );
}

void ST7789_extend::drawTable(int x, int y, int w, int h, int rows, int cols,
            uint16_t color, int thickness, int gridThickness) {
    int cellW = w / cols;
    int cellH = h / rows;

    for (int i = 0; i < thickness; i++) {
        drawRect(x + i, y + i, w - 2 * i, h - 2 * i, color);
    }

    for (int c = 1; c < cols; c++) {
        int lineX = x + c * cellW;
        for (int i = 0; i < gridThickness; i++) {
            drawFastVLine(lineX + i, y + thickness, h - 2 * thickness, color);
        }
    }

    for (int r = 1; r < rows; r++) {
        int lineY = y + r * cellH;
        for (int i = 0; i < gridThickness; i++) {
            drawFastHLine(x + thickness, lineY + i, w - 2 * thickness, color);
        }
    }
}
void ST7789_extend::initTable(int x, int y, int w, int h, int rows, int cols,
            uint16_t color, int thickness, int gridThickness) {
    drawTable(x, y, w, h, rows, cols, color, thickness, gridThickness);
    tbDat = tableData(x, y + h - thickness, w, h, rows, cols, thickness, gridThickness);
}

void ST7789_extend::addTable(int h, int rows, int cols, uint16_t color, int thickness, int gridThickness) {
    drawTable(
        tbDat.x, 
        tbDat.y, 
        tbDat.w, h, rows, cols, color, 
        thickness, 
        gridThickness);
    tbDat.y = tbDat.y + h - tbDat.thickness;
    tbDat.h += h;
}