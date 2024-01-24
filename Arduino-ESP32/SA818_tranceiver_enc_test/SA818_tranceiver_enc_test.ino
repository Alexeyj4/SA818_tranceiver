#define OLED_DISPLAY_TYPE 96
#include <Oled.h>
#include <Ble.h>
#include <ESP32Encoder.h>
const gpio_num_t btn_pin=GPIO_NUM_25; //иначе не работает выход из спящего режима, если просто int 25 указать //encoder Button pin
const int OLEDVCC_pin=4;
const int enc_a_pin=27; //encoder A pin
const int enc_b_pin=26; //encoder B pin

int enc_oldPosition  = 0; //encoder old position

Oled oled;
Ble ble;
ESP32Encoder enc;

void setup() { 
  pinMode(OLEDVCC_pin, OUTPUT);
  pinMode(enc_a_pin, INPUT_PULLUP);//A
  pinMode(enc_b_pin, INPUT_PULLUP);//B
  pinMode(btn_pin,INPUT_PULLUP);
  digitalWrite(OLEDVCC_pin,1);
  Serial.begin(115200);   
  ble.begin();
  oled.begin();  
  enc.attachFullQuad(enc_a_pin, enc_b_pin);
  enc.clearCount();
}

void loop() {
  if(ble.recvd()!=""){
    oled.print(1,ble.recvd());
    ble.clr();
  }
  oled.update();  
  delay(50);
  if(digitalRead(btn_pin)==0){
    ble.send("Нажата кнопка");    
    delay(1000);
  }       
oled.print(2,String((int32_t)(floor(enc.getCount()/4)))); 

}
