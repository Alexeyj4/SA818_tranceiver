void setup() { 
  pinMode(32, INPUT);//rx1
  pinMode(19, OUTPUT);//ptt
  pinMode(25, INPUT_PULLUP);//btn
  digitalWrite(19,1);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1);
  delay(100);
}

void loop() {
Serial.println("sleep...");
delay(2000);
gpio_wakeup_enable(GPIO_NUM_25,GPIO_INTR_LOW_LEVEL);
esp_sleep_enable_gpio_wakeup();
esp_light_sleep_start();
Serial.println("hello");
delay(5000);

  
if(Serial.available()){
  Serial2.print(Serial.read());
}

if(Serial2.available()){
  Serial.write(Serial2.read());
}   

//if(digitalRead(25)==0){
//  Serial2.print("RSSI?");
//  Serial2.write(13);
//  Serial2.write(10);
//  digitalWrite(19,0);//ptt on
//  pinMode(32, OUTPUT);//rx1
//  for(int i=0;i<5000;i++){
//    digitalWrite(32,1);
//    delay(2);
//    digitalWrite(32,0);
//    delay(2);
//  }  
//  pinMode(32, INPUT);//rx1
//  digitalWrite(19,1);//ptt off  
//}
//int l=(int(analogRead(32)/50));
////Serial.println(l);
//for(int i=0;i<l;i++){
//  Serial.print("*");  
//}
//Serial.println();
}
