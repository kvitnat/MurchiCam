#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class display
{
	Adafruit_SSD1306* display_;

	const int SCREEN_WIDTH = 128; // OLED display width, in pixels
	const int SCREEN_HEIGHT = 64; // OLED display height, in pixels
public:
	display();
	~display();

	void initDisplay();
	void initTextStyle();

	void setCursor(int x, int y);
	void drawWifiSymbol();
	void printLineToScreen(const char* str);

	void clearArea(short x1, short y1, short x2, short y2);
};

//#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 'wifi_icon', 12x12px
const unsigned char wifi_icon [] = {
	0x00, 0xf0, 0x03, 0x80, 0x0e, 0x00, 0x18, 0x30, 0x30, 0xe0, 0x61, 0x80, 0x47, 0x00, 0xc4, 0x30, 
	0x8c, 0xe0, 0x88, 0x80, 0x99, 0xb0, 0x91, 0x30};


