#include "display.h"
#include <Wire.h>

display::display()
{
	display_ = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
}

display::~display()
{
	delete display_;
	display_ = nullptr;
}

void display::drawWifiSymbol()
{
	display_->drawBitmap(112, 0, wifi_icon, 12, 12, WHITE);
}

void display::initDisplay()
{
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display_->begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
    }

    display_->clearDisplay();
    display_->display();
    initTextStyle();
}

void display::initTextStyle()
{
    display_->setTextSize(1);      // Normal 1:1 pixel scale
    display_->setTextColor(WHITE); // Draw white text
    display_->setCursor(0, 0);     // Start at top-left corner
    display_->cp437(true);         // Use full 256 char 'Code Page 437' font
}

void display::setCursor(int x, int y)
{
	display_->setCursor(x, y);
}

//void display::printLineToScreen(const char* str)
//{
//    int i = 0;
//    while(str[i] != '\0')
//    {
//        display_->write(str[i++]);
//    }
//
//    display_->write('\n');
//    display_->display();
//}

void display::printLineToScreen(const std::string& str)
{
    for (const char& ch : str)
        display_->write(ch);
    
    display_->write('\n');
    display_->display();
}


void display::clearArea(int x1, int y1, int x2, int y2)
{
    for (int i = x1; i < x2; i++)
        for (int j = y1; j < y2; j++)
            display_->drawPixel(i, j, SSD1306_BLACK);
}