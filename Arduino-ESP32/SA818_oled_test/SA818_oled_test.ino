//#define OLED_DISPLAY_TYPE 96
#include <config.h>
#include <Oled.h>
const int OLEDVCC_pin=4;

Oled oled;

void setup() { 
  pinMode(OLEDVCC_pin, OUTPUT);
  digitalWrite(OLEDVCC_pin,1);
  delay(100);
  Serial.begin(115200);   
  oled.begin();
  delay(100);
  oled.print(1,"test");
  delay(100);
  oled.update();  
}

void loop() {
}
