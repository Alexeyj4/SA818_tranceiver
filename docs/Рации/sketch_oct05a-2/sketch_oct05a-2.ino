#include "U8glib.h"         // библиотека дисплея
#include "SA818.h"          // библиотека для радиомодуля
#include "bits.h"           // картинки для OLED дисплея
#include "ChannelsTable.h"  // таблица каналов
#include <EEPROM.h>         // библиотека для работы с EEPROM
#include "GyverButton.h"    // библиотека для работы с кнопками

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);    // I2C / TWI 
// 0-14 - yellow        дисплей двухцветный. Верхний рад желтого цвета размерами 128х15 пикселей, начало координат 0х0 - 127х14
// 15-63 - blue         нижний ряд синего цвета. Начало координат: 0х15 - 127х63

#define SA818_AUDIO_ON  8   // пин AUDIO_ON модуля SA818
//#define SA818_PTT 3       // пин PTT
#define SA818_POW 10        // пин H/L, для переключения мощности модуля

//SA818 intercom(SA818_UHF, SA818_PTT, SA818_PD, SA818_POW);
SA818 intercom(SA818_UHF, SA818_POW);

#define Bttn_PTT    3       // Кнопка PTT
#define Bttn_Menu   5       // Кнопка Меню / Ок
#define Bttn_Dn     6       // Кнопка Вниз / Уменьшение
#define Bttn_Up     7       // Кнопка Вверх / Увеличение
GButton b_PTT(Bttn_PTT);
GButton b_Menu(Bttn_Menu);
GButton b_Dn(Bttn_Dn);
GButton b_Up(Bttn_Up);

#define BattPin     A0      // Analog input 
#define ChargPin    2       // определение подключения питания для заряда аккумулятора
float Voltage   = 0.0;      // напряжение на аккумуляторе
bool Charging   = false;    // 

char Buffer[] = "Hello String";

byte Channel    = 12;       // Текущий канал
bool BandWidth  = 1;        // 0 - PMR 12.5 KHz         1 - LDP 25 KHz
bool Manualfreq = 0;        // 0 - каналы LDP и PMR     1 - ручная настройка частоты
byte Volume     = 4;        // уровень громкости 1, 4 и 8
byte SQ         = 4;        // уровен шумоподавления 0-8
bool TailTone   = 1;        // 0 - отключение setTailTone    1 - включение setTailTone
bool Power      = 1;        // 0 - низкая мощность      1 - максимальная мощность
byte TX_CTCSS   = 38;
#define CTCSS_Max 38
byte RX_CDCSS   = 166;
#define CDCSS_Max 166
bool RSSI_read  = 0;
byte RSSI_level = 0;
byte RSSI_time  = 1;        // Частота проверки RSSI в минутах
bool FILTER_EMPHASIS = 1;
bool FILTER_HIGHPASS = 1;
bool FILTER_LOWPASS  = 1;

bool PPT_is_Presed  = 0;
bool Menu_is_Presed = 0;
bool Menu_Settings  = 0;
bool Menu_is_Press  = 0;
bool Dn_is_Press    = 0;
bool Up_is_Press    = 0;
byte Menu_item      = 1;
byte Menu_item_select = 0;
byte Menu_Pointer   = 0;
#define ITEMS       15
bool flag           = true;    // Флаг выбора
bool ManualFrequencySet = 0;
byte nPoint = 0;

byte event = true;
unsigned long timing;
unsigned long timing2;

                            // Переменые для настройки частоты (первая часть в MHz)
int FrequencyH = 400;       // Верхняя граница частоты в MHz
int FrequencyL = 9;         // Нижняя граница частоты в KHz
#define FrequencyH_Min 400  // Нижняя граница частоты в MHz
#define FrequencyH_Max 480  // Верхняя граница частоты в MHz
#define FrequencyL_Max 9999 // Верхняя граница частоты в KHz

#define addr_Channel        0
#define addr_BandWidth      1
#define addr_Manualfreq     2
#define addr_Volume         3
#define addr_SQ             4
#define addr_TailTone       5
#define addr_Power          6
#define addr_TX_CTCSS       7
#define addr_RX_CDCSS       8
#define addr_FrequencyH     9
#define addr_FrequencyL     11
#define addr_RSSI_read      13
#define addr_FILTER_EMPHASIS      14
#define addr_FILTER_HIGHPASS      15
#define addr_FILTER_LOWPASS      16
#define addr_RSSI_time      17



void draw(void) {
    if(Volume == 8){                        // отображение уровня громкости 1, 4 и 8
        u8g.drawXBMP(32, 48, 16, 15, Vol_3_bits);
    }else if(Volume == 4){
        u8g.drawXBMP(32, 48, 16, 15, Vol_2_bits);
    }else{
        u8g.drawXBMP(32, 48, 16, 15, Vol_1_bits);
    }

    if(Power){                              // отображение значка повышенной мощности передатчика
        u8g.drawXBMP(0, 48, 16, 16, Antenna_2_bits);
        //u8g.drawXBMP(0, 0, 16, 16, Antenna_2_bits);
    }
    
    //u8g.drawXBMP(0, 48, 16, 16, Settings_bits); // отображение значка меню настройки
    
    if (RSSI_level > 191){
        u8g.drawXBMP(1, 1, 16, 14, RSSI_4_bits);   // отображение значка RSSI
        //u8g.drawXBMP(21, 1, 16, 14, RSSI_4_bits);   // отображение значка RSSI
    }else if (RSSI_level > 127){
        u8g.drawXBMP(1, 1, 16, 14, RSSI_3_bits);   // отображение значка RSSI
        //u8g.drawXBMP(21, 1, 16, 14, RSSI_3_bits);   // отображение значка RSSI
    }else if (RSSI_level > 63){
        u8g.drawXBMP(1, 1, 16, 14, RSSI_2_bits);   // отображение значка RSSI
        //u8g.drawXBMP(21, 1, 16, 14, RSSI_2_bits);   // отображение значка RSSI
    }else if (RSSI_level > 0){
        u8g.drawXBMP(1, 1, 16, 14, RSSI_1_bits);   // отображение значка RSSI
        //u8g.drawXBMP(21, 1, 16, 14, RSSI_1_bits);   // отображение значка RSSI
    }

    
    if(TailTone){                                   // отображение значка TailTone  
        u8g.drawXBMP(64, 48, 16, 15, Tone_bits);
    }


    u8g.setFont(u8g_font_fur11r);                   // отображение типа канала (LDP / PMR) и его номера или уведомления о ручной настройке канала
    sprintf(Buffer,"%s-%d", (BandWidth ? "LDP" : "PMR"),Channel);
    u8g.drawStr(30, 14, (Manualfreq ? "Manual" : Buffer));
    //u8g.drawStr(43, 14, (Manualfreq ? "Manual" : Buffer));

    
    sprintf(Buffer,"SQ%d", SQ);                     // отображение уровня шумоподавления 1-8
    u8g.drawStr(96, 63, Buffer);


    if (PPT_is_Presed){
        u8g.drawXBMP(43, 16, 40, 30, PPT_bits);
    }else{
                                                        // отображение текущей частоты:
        u8g.setFont(u8g_font_fub20);
        sprintf(Buffer,"%d.%04d", FrequencyH, FrequencyL);
        u8g.drawStr( 10, 44, Buffer);
    }

    

    if (Charging){                          // отображения уровня заряда аккумулятора или подключенной зарядки
        u8g.drawXBMP(96, 0, 32, 15, battery_ch_bits);
    }else if(Voltage > 4.0){
        u8g.drawXBMP(96, 0, 32, 15, battery_100_bits);
    }else if(Voltage > 3.7){
        u8g.drawXBMP(96, 0, 32, 15, battery_66_bits);
    }else if(Voltage > 3.4){
        u8g.drawXBMP(96, 0, 32, 15, battery_33_bits);
    }else{
        u8g.drawXBMP(96, 0, 32, 15, battery_0_bits);
    }
}

void draw_Setup(void) {
    u8g.setFont(u8g_font_fur11r);       // Список меню
    if (Menu_Pointer < 5) {
        u8g.drawStr(12, 15, "Channel:");          sprintf(Buffer,"%d", Channel);        u8g.drawStr(96, 15, Buffer);
        u8g.drawStr(12, 27, "BandW:");                                                  u8g.drawStr(96, 27, (BandWidth ? "LDP" : "PMR"));
        u8g.drawStr(12, 39, "M.Freq:");                                                 u8g.drawStr(96, 39, (Manualfreq ? "Man." : "L/P"));
        u8g.drawStr(12, 51, "Frequency");
        u8g.drawStr(12, 63, "Volume:");           sprintf(Buffer,"%d", Volume);         u8g.drawStr(96, 63, Buffer);
        u8g.drawStr(0, (Menu_Pointer*12)+15, (flag ? ">" : "#"));
     } else if (Menu_Pointer < 10) {    // Вторая страница
        u8g.drawStr(12, 15, "SQ:");               sprintf(Buffer,"%d", SQ);             u8g.drawStr(96, 15, Buffer);
        u8g.drawStr(12, 27, "TailTone:");                                               u8g.drawStr(96, 27, (TailTone ? "Yes" : "No"));
        u8g.drawStr(12, 39, "Power:");                                                  u8g.drawStr(96, 39, (Power ? "Max" : "Min"));
        u8g.drawStr(12, 51, "TX_CTCSS:");         sprintf(Buffer,"%d", TX_CTCSS);       u8g.drawStr(96, 51, Buffer);
        u8g.drawStr(12, 63, "RX_CDCSS:");         sprintf(Buffer,"%d", RX_CDCSS);       u8g.drawStr(96, 63, Buffer);
        u8g.drawStr(0, (Menu_Pointer*12)-45, (flag ? ">" : "#"));
    } else {                            // третья страница
        u8g.drawStr(12, 15, "RSSI level:");                                             u8g.drawStr(96, 15, (RSSI_read ? "Yes" : "No"));
        u8g.drawStr(12, 27, "EMPHASIS:");                                               u8g.drawStr(96, 27, (FILTER_EMPHASIS ? "Yes" : "No"));
        u8g.drawStr(12, 39, "HIGHPASS:");                                               u8g.drawStr(96, 39, (FILTER_HIGHPASS ? "Yes" : "No"));
        u8g.drawStr(12, 51, "LOWPASS:");                                                u8g.drawStr(96, 51, (FILTER_LOWPASS ? "Yes" : "No"));
        u8g.drawStr(12, 63, "RSSI read:");        sprintf(Buffer,"%d", RSSI_time);      u8g.drawStr(96, 63, Buffer);
        u8g.drawStr(0, (Menu_Pointer*12)-105, (flag ? ">" : "#"));
    }
}

void block() { for(;;) {} }

void setup() {
    b_PTT.setTickMode(AUTO);
    b_PTT.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
    b_PTT.setTimeout(50);        // настройка таймаута на удержание (по умолчанию 500 мс)
    b_PTT.setClickTimeout(100);   // настройка таймаута между кликами (по умолчанию 300 мс)

    b_Menu.setTickMode(AUTO);
    b_Menu.setDebounce(40);        // настройка антидребезга (по умолчанию 80 мс)
    b_Menu.setTimeout(1000);        // настройка таймаута на удержание (по умолчанию 500 мс)
    b_Menu.setClickTimeout(50);   // настройка таймаута между кликами (по умолчанию 300 мс)

    b_Dn.setTickMode(AUTO);
    b_Dn.setDebounce(40);        // настройка антидребезга (по умолчанию 80 мс)
    b_Dn.setTimeout(300);        // настройка таймаута на удержание (по умолчанию 500 мс)
    b_Dn.setClickTimeout(80);   // настройка таймаута между кликами (по умолчанию 300 мс)

    b_Up.setTickMode(AUTO);
    b_Up.setDebounce(40);        // настройка антидребезга (по умолчанию 80 мс)
    b_Up.setTimeout(300);        // настройка таймаута на удержание (по умолчанию 500 мс)
    b_Up.setClickTimeout(80);   // настройка таймаута между кликами (по умолчанию 300 мс)
    
    analogReference(EXTERNAL);   // внешнее опорное напряжение TL431 на 2.5 вольт
    pinMode(ChargPin, INPUT);
    pinMode(4 ,INPUT);

    pinMode(SA818_AUDIO_ON, INPUT); // отслеживание шины AUDIO_ON, включающей аудиоусилитель
                                    // появление низкого уровня на шине сигнализирует о том что есть входящий сигнал

    //intercom.powerUp();
        
    intercom.begin();
    if(!intercom.handshake(true)) {
        u8g.setFont(u8g_font_fur11r); 
        u8g.firstPage();  
        do {
            u8g.drawStr(0, 14, "SA818.handshake: ");
            u8g.drawStr(0, 28, "failed");
        } while( u8g.nextPage() );
        block();
    }
    Restore_from_EEPROM();
    intercom.setPower(Power);
    Display_Update();
    timing = millis(); 
}

void loop() {
    
    if (digitalRead(SA818_AUDIO_ON) == LOW) {
        event = true;
        PPT_is_Presed = true;
        if (millis() - timing2 > 500){ 
            timing2 = millis(); 
            RSSI_level = intercom.readRSSI();
            Display_Update();
        }
    }
    
    if (RSSI_read){
        
        if (millis() - timing2 > (RSSI_time * 60000)){ // 5 минут
            timing2 = millis(); 
            RSSI_level = intercom.readRSSI();
        }
    }
        
    if (event){
        if (millis() - timing > 120000){ // 120 секунд 120000
            timing = millis(); 
            event = false;
        }
        
        if (!PPT_is_Presed){
            battery();
        }

        if (b_Menu.isHolded()){
            timing = millis(); 
            event = true;
            Menu_is_Presed = !Menu_is_Presed;
            if (Menu_is_Presed){
                Menu_Settings = 1;
            }else{
                Menu_Settings = 0;      // выход из режима настройки
                Frequency_Set();
                EEPROM_Save();          // сохраряем настройки в EEPROM
                intercom_Set();         // устанавливаем настройки в передатчике
            }
            Display_Update();
        }
        if (ManualFrequencySet){
            Manual_frequency_set();
        }else{
            if (!Menu_Settings){
                PPT_is_Presed = b_PTT.isHold();
                if (PPT_is_Presed){
                    intercom.enableTX();
                }else{
                    intercom.disableTX();
                }
                Display_Update();
            }else{
                buttons();
            }
        }
    }else{
        if (b_Up.isClick() or b_Up.isHold()) {                // Если кнопку нажали или удерживают
            timing = millis(); 
            event = true;
        }
      
        if (b_Dn.isClick() or b_Dn.isHold()) {
            timing = millis(); 
            event = true;
        }
    
        if (b_Menu.isClick() or b_Menu.isHold()) {         // Нажатие на ОК - переход в пункт меню
            timing = millis(); 
            event = true;
        }
        
        if (b_PTT.isClick() or b_PTT.isHold()) {         // Нажатие на ОК - переход в пункт меню
            timing = millis(); 
            event = true;
        }
        
        u8g.firstPage();  
        do {
        } while( u8g.nextPage() );
    }
}

void Restore_from_EEPROM(){
    Channel = EEPROM.read(addr_Channel);
    BandWidth = EEPROM.read(addr_BandWidth);
    Manualfreq = EEPROM.read(addr_Manualfreq);
    Volume = EEPROM.read(addr_Volume);
    SQ = EEPROM.read(addr_SQ);
    TailTone = EEPROM.read(addr_TailTone);
    Power = EEPROM.read(addr_Power);
    TX_CTCSS = EEPROM.read(addr_TX_CTCSS);
    RX_CDCSS = EEPROM.read(addr_RX_CDCSS);
    FrequencyH = readIntFromEEPROM(addr_FrequencyH);
    FrequencyL = readIntFromEEPROM(addr_FrequencyL);
    RSSI_read = EEPROM.read(addr_RSSI_read);
    RSSI_time = EEPROM.read(addr_RSSI_time);
    

    FILTER_EMPHASIS = EEPROM.read(addr_FILTER_EMPHASIS);
    FILTER_HIGHPASS = EEPROM.read(addr_FILTER_HIGHPASS);
    FILTER_LOWPASS = EEPROM.read(addr_FILTER_LOWPASS);
}

void Frequency_Set(){
    if(!Manualfreq){
        FrequencyH = (BandWidth ? ((Channel<LDPprefixStep) ? LDPprefix1 : LDPprefix2) : PMRprefix1);
        FrequencyL = (BandWidth ? ((Channel>37)?((LDPstep*(Channel-1))+LDPstart-10000):((LDPstep*(Channel-1))+LDPstart)) : ((PMRstep*(Channel-1))+PMRstart));
    }
}

void EEPROM_Save(){
    EEPROM.update(addr_Channel, Channel);
    EEPROM.update(addr_BandWidth, BandWidth);
    EEPROM.update(addr_Manualfreq, Manualfreq);
    EEPROM.update(addr_Volume, Volume);
    EEPROM.update(addr_SQ, SQ);
    EEPROM.update(addr_TailTone, TailTone);
    EEPROM.update(addr_Power, Power);
    EEPROM.update(addr_TX_CTCSS, TX_CTCSS);
    EEPROM.update(addr_RX_CDCSS, RX_CDCSS);
    EEPROM.update(addr_RSSI_read, RSSI_read);
    EEPROM.update(addr_RSSI_time, RSSI_time);
    
    writeIntIntoEEPROM(addr_FrequencyH, FrequencyH);
    writeIntIntoEEPROM(addr_FrequencyL, FrequencyL);

    EEPROM.update(addr_FILTER_EMPHASIS, FILTER_EMPHASIS);
    EEPROM.update(addr_FILTER_HIGHPASS, FILTER_HIGHPASS);
    EEPROM.update(addr_FILTER_LOWPASS, FILTER_LOWPASS);
}

void intercom_Set(){
    u8g.firstPage();  
    do {
        u8g.drawStr(0, 14, "Apply Config: ");
    } while( u8g.nextPage() );
        
    sprintf(Buffer,"%d.%04d", FrequencyH, FrequencyL);
    intercom.setPower(Power);
    intercom.setTXFrequency(Buffer);
    intercom.setRXFrequency(Buffer);
    intercom.setBandwidth(BandWidth);
    intercom.setSQueltch(SQ);
    intercom.setRX_CTCSS(RX_CDCSS);
    intercom.setTX_CTCSS(TX_CTCSS);

    if(intercom.applyConfig(true)) {
        do {
            u8g.drawStr(0, 14, "Apply Config: ");
            u8g.drawStr(0, 28, "Ok");
        } while( u8g.nextPage() );
    } else {
        u8g.firstPage();  
        do {
            u8g.drawStr(0, 14, "Apply Config: ");
            u8g.drawStr(0, 28, "failed");
        } while( u8g.nextPage() );
        block();
    }
    
    delay(100);
    
    intercom.setVolume(Volume);
    delay(100);
    intercom.setFilter(FILTER_EMPHASIS, FILTER_HIGHPASS, FILTER_LOWPASS);
    delay(100);
    intercom.setTailTone(!TailTone);
    delay(100);

    u8g.firstPage();  
    do {
        u8g.drawStr(0, 14, "Done");
    } while( u8g.nextPage() );
}

void Display_Update(){
    if (ManualFrequencySet){
        u8g.firstPage();  
        do {
            u8g.setFont(u8g_font_fur11r);
            switch (nPoint) {
                case 0:
                    u8g.drawStr( 109, 55, "^");  break;
                case 1:
                    u8g.drawStr( 94, 55, "^");   break;
                case 2:
                    u8g.drawStr( 79, 55, "^");   break;
                case 3:
                    u8g.drawStr( 64, 55, "^");   break;
                case 4:
                    u8g.drawStr( 41, 55, "^");   break;
                case 5:
                    u8g.drawStr( 26, 55, "^");   break;
            }
            
            u8g.setFont(u8g_font_fub20);
            sprintf(Buffer,"%d.%04d", FrequencyH, FrequencyL);
            u8g.drawStr( 10, 44, Buffer);
        } while( u8g.nextPage() );
    }else{
        u8g.firstPage();  
        do {
            if (Menu_Settings){
                draw_Setup();
            }else{
                draw();
            }
        } while( u8g.nextPage() );
    }
}

void buttons(){
    if (b_Up.isClick() or b_Up.isHold()) {                // Если кнопку нажали или удерживают
        timing = millis(); 
        event = true;
        if (flag) {                                       // Если флаг установлен
            Menu_Pointer = constrain(Menu_Pointer - 1, 0, ITEMS - 1); // Двигаем указатель в пределах дисплея
        } else {                                          // Иначе
            but_Plus();                                // Увеличиваем параметр/меню
        }
        Display_Update();
    }
  
    if (b_Dn.isClick() or b_Dn.isHold()) {
        timing = millis(); 
        event = true;
        if (flag) {                                       // Если флаг установлен
            Menu_Pointer = constrain(Menu_Pointer + 1, 0, ITEMS - 1);
        } else {                                          // Иначе
            but_Minus();                                // Уменьшаем параметр/меню
        }
        Display_Update();
    }

    if (b_Menu.isClick()) {         // Нажатие на ОК - переход в пункт меню
        timing = millis(); 
        event = true;
        flag = !flag;
        if (Menu_Pointer == 3){
            ManualFrequencySet = 1;
        }
        Display_Update();
    }
}

void Manual_frequency_set(){
    nPoint = 0;
    
    do{
        if (b_Menu.isClick()) {         // Нажатие на ОК - переход в пункт меню
            timing = millis(); 
            event = true;
            nPoint++;
        }

        if (b_Up.isClick() or b_Up.isHold()) {                // Если кнопку нажали или удерживают
            timing = millis(); 
            event = true;
            switch (nPoint) {     // По номеру указателей располагаем вложенные функции (можно вложенные меню)
                case 0:
                    FrequencyL++;
                    if (FrequencyL > FrequencyL_Max){
                        FrequencyL = 0;
                        FrequencyH++;
                    }
                    SPlus();
                    break;          
                case 1:
                    FrequencyL = FrequencyL + 10;
                    if (FrequencyL > FrequencyL_Max){
                        FrequencyL = FrequencyL - 10000;
                        FrequencyH++;
                    }
                    SPlus();
                    break;
                case 2:
                    FrequencyL = FrequencyL + 100;
                    if (FrequencyL > FrequencyL_Max){
                        FrequencyL = FrequencyL - 10000;
                        FrequencyH++;
                    }
                    SPlus();
                    break;
                case 3:
                    FrequencyL = FrequencyL + 1000;
                    if (FrequencyL > FrequencyL_Max){
                        FrequencyL = FrequencyL - 10000;
                        FrequencyH++;
                    }
                    SPlus();
                    break;
                case 4:
                    FrequencyH++;
                    SPlus();
                    break;
                case 5:
                    FrequencyH = FrequencyH + 10;
                    SPlus();
                    break;       
            }
        }

        if (b_Dn.isClick() or b_Dn.isHold()) {
            timing = millis(); 
            event = true;
            switch (nPoint) {     // По номеру указателей располагаем вложенные функции (можно вложенные меню)
                case 0:                    
                    if (FrequencyL > 0){
                        FrequencyL--;
                    }else{
                        if (FrequencyH > FrequencyH_Min){
                            FrequencyL = FrequencyL_Max;
                            FrequencyH--;
                        }else{
                            FrequencyL = 0;
                        }
                    }
                    break;          
                case 1:
                    if (FrequencyL > 9){
                        FrequencyL = FrequencyL - 10;
                    }else{
                        if (FrequencyH > FrequencyH_Min){
                            FrequencyL = 9990 + FrequencyL;
                            FrequencyH--;
                        }else{
                            FrequencyL = 0;
                        }
                    }
                    break;
                case 2:
                    if (FrequencyL > 99){
                        FrequencyL = FrequencyL - 100;
                    }else{
                        if (FrequencyH > FrequencyH_Min){
                            FrequencyL = 9900 + FrequencyL;
                            FrequencyH--;
                        }else{
                            FrequencyL = 0;
                        }
                    }
                    break;
                case 3:
                    if (FrequencyL > 999){
                        FrequencyL = FrequencyL - 1000;
                    }else{
                        if (FrequencyH > FrequencyH_Min){
                            FrequencyL = 9000 + FrequencyL;
                            FrequencyH--;
                        }else{
                            FrequencyL = 0;
                        }
                    }
                    break;
                case 4:
                    if (FrequencyH > FrequencyH_Min){
                        FrequencyH--;
                    }else{
                        FrequencyH = FrequencyH_Min;
                        FrequencyL = 0;
                    }
                    break;
                case 5:
                    if (FrequencyH > (FrequencyH_Min+9)){
                        FrequencyH = FrequencyH - 10;
                    }else{
                        FrequencyH = FrequencyH_Min;
                        FrequencyL = 0;
                    }
                    break;      
            }
        }
        Display_Update();
    } while (nPoint < 6);
    
    flag = !flag;
    ManualFrequencySet = 0;
    Display_Update();
}

void SPlus(){
    if (FrequencyH > FrequencyH_Max){
        FrequencyH = FrequencyH_Max;
        FrequencyL = 0;
    }
}

void but_Plus(){
    switch (Menu_Pointer) {     // По номеру указателей располагаем вложенные функции (можно вложенные меню)
        case 0:
            if (BandWidth){
                if (Channel < LDPchanells){
                    Channel++;
                }else{
                    Channel = LDPchanells;
                }
            }else{
                if (Channel < PMRchanells){
                    Channel++;
                }else{
                    Channel = PMRchanells;
                }
            }
            break;          // По нажатию на ОК при наведении на 0й пункт вызвать функцию
        case 1:
            BandWidth = !BandWidth;
            if (BandWidth){
                if (Channel > LDPchanells){
                    Channel = LDPchanells;
                }
            }else{
                if (Channel > PMRchanells){
                    Channel = PMRchanells;
                }
            }
            break;
        case 2:
            Manualfreq = !Manualfreq;
            break;
        case 3:
            ManualFrequencySet = 1;
            break;
        case 4:
            if (Volume < 4){
                Volume = 4;
            }else{
                Volume = 8;
            }
            break;
        case 5:
            if (SQ < 8){
                SQ++;
            }else{
                SQ = 8;
            }
            break;
        case 6:
            TailTone = !TailTone;
            break;
        case 7:
            Power = !Power;
            break;
        case 8:
            if (TX_CTCSS < CTCSS_Max){
                TX_CTCSS++;
            }else{
                TX_CTCSS = CTCSS_Max;
            }
            break;
        case 9:
            if (RX_CDCSS < CDCSS_Max){
                RX_CDCSS++;
            }else{
                RX_CDCSS = CDCSS_Max;
            }
            break;
        case 10:
            RSSI_read = !RSSI_read;
            break;
        case 11:
            FILTER_EMPHASIS = !FILTER_EMPHASIS;
            break;
        case 12:
            FILTER_HIGHPASS = !FILTER_HIGHPASS;
            break;
        case 13:
            FILTER_LOWPASS = !FILTER_LOWPASS;
            break;
        case 14:
            if (RSSI_time<15){
                RSSI_time++;
            }else{
                RSSI_time = 15;
            }
            break;
            
    }
}

void but_Minus(){
    switch (Menu_Pointer) {     // По номеру указателей располагаем вложенные функции (можно вложенные меню)
        case 0:
            if (Channel > 1){
                Channel--;
            }else{
                Channel = 1;
            }
            break;          // По нажатию на ОК при наведении на 0й пункт вызвать функцию
        case 1:
            BandWidth = !BandWidth;
            if (BandWidth){
                if (Channel > LDPchanells){
                    Channel = LDPchanells;
                }
            }else{
                if (Channel > PMRchanells){
                    Channel = PMRchanells;
                }
            }
            break;
        case 2:
            Manualfreq = !Manualfreq;
            break;
        case 3:
            ManualFrequencySet = 1;
            break;
        case 4:
            if (Volume > 4){
                Volume = 4;
            }else{
                Volume = 1;
            }
            break;
        case 5:
            if (SQ > 0){
                SQ--;
            }else{
                SQ = 0;
            }
            break;
        case 6:
            TailTone = !TailTone;
            break;
        case 7:
            Power = !Power;
            break;
        case 8:
            if (TX_CTCSS > 0){
                TX_CTCSS--;
            }else{
                TX_CTCSS = 0;
            }
            break;
        case 9:
            if (RX_CDCSS > 0){
                RX_CDCSS--;
            }else{
                RX_CDCSS = 0;
            }
            break;
        case 10:
            RSSI_read = !RSSI_read;
            break;
        case 11:
            FILTER_EMPHASIS = !FILTER_EMPHASIS;
            break;
        case 12:
            FILTER_HIGHPASS = !FILTER_HIGHPASS;
            break;
        case 13:
            FILTER_LOWPASS = !FILTER_LOWPASS;
            break;
        case 14:
            if (RSSI_time>1){
                RSSI_time--;
            }else{
                RSSI_time = 1;
            }
    }
}


void battery(){
    float sample = 0.0;
    // 2.5 = 1024

    for(int i=0;i< 20;i++)
    {
        sample = sample + analogRead(BattPin); //read the voltage from the divider circuit
        delay (3);
    }
    sample = sample / 20; 
    Voltage = 2 * (sample * 2.46 / 1024.0);     // ----------------------------<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    Charging = digitalRead(ChargPin);
}

void writeIntIntoEEPROM(int address, int number)
{ 
    EEPROM.update(address, number >> 8);
    EEPROM.update(address + 1, number & 0xFF);
}

int readIntFromEEPROM(int address)
{
    byte byte1 = EEPROM.read(address);
    byte byte2 = EEPROM.read(address + 1);
    return (byte1 << 8) + byte2;
}
