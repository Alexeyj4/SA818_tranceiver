#define OLED_DISPLAY_TYPE 130
#include <Oled.h>
const int OLEDVCC_pin=4;

Oled oled;

void setup() { 
  pinMode(OLEDVCC_pin, OUTPUT);
  digitalWrite(OLEDVCC_pin,1);
  delay(100);
  //Serial.begin(115200);   
  oled.begin();
  oled.print(1,"test");
  oled.update();  
}

void loop() {
}
