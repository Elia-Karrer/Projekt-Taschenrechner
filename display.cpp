#include "display.h"


Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);


void init_display(void)
{
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
        Serial.println("Display Error");
    
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("OLED initialized!");
    display.display();
}
