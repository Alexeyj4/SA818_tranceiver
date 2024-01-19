void setup() { 
  pinMode(32, INPUT);//rx1
  pinMode(19, OUTPUT);//ptt
  pinMode(25, INPUT_PULLUP);//btn
  digitalWrite(19,1);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1);
}

void loop() {
if(Serial.available()){
  Serial2.print(Serial.read());
}

if(Serial2.available()){
  Serial.write(Serial2.read());
}   

if(digitalRead(25)==0){
  Serial2.print("RSSI?");
  Serial2.write(13);
  Serial2.write(10);
  digitalWrite(19,0);//ptt on
  pinMode(32, OUTPUT);//rx1
  for(int i=0;i<5000;i++){
    digitalWrite(32,1);
    delay(2);
    digitalWrite(32,0);
    delay(2);
  }  
  pinMode(32, INPUT);//rx1
  digitalWrite(19,1);//ptt off  
}
int l=(int(analogRead(32)/50));
//Serial.println(l);
for(int i=0;i<l;i++){
  Serial.print("*");  
}
Serial.println();
}
