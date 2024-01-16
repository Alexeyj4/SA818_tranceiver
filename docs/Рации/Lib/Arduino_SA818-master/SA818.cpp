/*
  SA818.cpp - Library for controlling the SA818 RF Moules
  Created by TuerleForGaming.
  Shared under the GNU General Public License v3.0
*/

#include "Arduino.h"
#include "SA818.h"

//SA818::SA818(int model,int RXpin, int TXpin, int ptt, int pd)
//SA818::SA818(int model, int ptt, int pd, int hl)
SA818::SA818(int model, int hl)
{
    //_pinCHPD = pd;
    //_pinPTT = ptt;
	_pinPowerHL = hl;
    //_RXpin = RXpin;
    //_TXpin = TXpin;
    _ModuleVersion = model;
    //pinMode(_pinCHPD,OUTPUT);
    //pinMode(_pinPTT ,OUTPUT);
	pinMode(_pinPowerHL,OUTPUT);
    //digitalWrite(_pinCHPD,false);
    //digitalWrite(_pinPTT ,true);
	digitalWrite(_pinPowerHL,false);
}
/*
void SA818::powerUp() {
	digitalWrite(_pinCHPD,true);
	//delay(3000);
	delay(500);
}
void SA818::powerDown() {
	digitalWrite(_pinCHPD,false);
}*/

void SA818::enableTX() {
	//digitalWrite(_pinPTT,false);
	_isTransmitting = true;
}
void SA818::disableTX() {
	//digitalWrite(_pinPTT,true );
	_isTransmitting = false;
}

void SA818::begin() {
	Serial.begin(9600);
}

bool SA818::handshake(bool forceCheck) {
	int cmdsize = 14;
	char verif[cmdsize+1] = {'+','D','M','O','C','O','N','N','E','C','T',':','0','\r','\n'};

	char buff[cmdsize*2];
	int i = 0;

	Serial.print("AT+DMOCONNECT\r\n");
	Serial.flush();

	long start = millis();
	while((start + SA818_CONTACT_TIMEOUT) > millis()) {
		if(i == 0) {
			buff[i] = Serial.read();
		}
		if(buff[i] == verif[0] || i>0) {
			delay((SA818_SERBPS / 1000) * 2 ); //Makes sure that the bit has time to come. Espetially that the implementation is wierd

			i++;
			buff[i] = Serial.read();

			if(buff[i] != verif[i]) {
				return false;
			}
			if(i >= cmdsize) {
				return true;
			}
		}
	}
	return (start + SA818_CONTACT_TIMEOUT) < millis();
}

bool SA818::setVolume(int volume) {
	if(volume<1 || volume>8) { return false; }
	bool wasTXing = _isTransmitting;
	if( _isTransmitting ) { disableTX(); }


	int cmdsize = 16;
	char verif[cmdsize+1] = {'+','D','M','O','S','E','T','V','O','L','U','M','E',':','0','\r','\n'};

	char buff[cmdsize*2];
	int i = 0;

	Serial.print("AT+DMOSETVOLUME="+String(volume)+"\r\n");
	Serial.flush();

	long start = millis();
	while((start + SA818_CONTACT_TIMEOUT) > millis()) {
		if(i == 0) {
			buff[i] = Serial.read();
		}
		if(buff[i] == verif[0] || i>0) {
			delay((SA818_SERBPS / 1000) * 2 ); //Makes sure that the bit has time to come. Espetially that the implementation is wierd

			i++;
			buff[i] = Serial.read();

			if(buff[i] != verif[i]) {
				if( wasTXing ) { enableTX(); }
				return false;
			}
			if(i >= cmdsize) {
				if( wasTXing ) { enableTX(); }
				return true;
			}
		}
	}

	if( wasTXing ) { enableTX(); }
	return (start + SA818_CONTACT_TIMEOUT) < millis();
}

bool SA818::setFilter(int mix) {
	bool empha = mix & SA818_FILTER_EMPHASIS;
	bool lpass = mix & SA818_FILTER_LOWPASS;
	bool hpass = mix & SA818_FILTER_HIGHPASS;

	bool wasTXing = _isTransmitting;
	if( _isTransmitting ) { disableTX(); }


	int cmdsize = 16;
	char verif[cmdsize+1] = {'+','D','M','O','S','E','T','F','I','L','T','E','R',':','0','\r','\n'};

	char buff[cmdsize*2];
	int i = 0;

	Serial.print("AT+SETFILTER="+String(!empha)+","+String(!hpass)+","+String(!lpass)+"\r\n");
	Serial.flush();

	long start = millis();
	while((start + SA818_CONTACT_TIMEOUT) > millis()) {
		if(i == 0) {
			buff[i] = Serial.read();
		}
		if(buff[i] == verif[0] || i>0) {
			delay((SA818_SERBPS / 1000) * 2 ); //Makes sure that the bit has time to come. Espetially that the implementation is wierd

			i++;
			buff[i] = Serial.read();

			if(buff[i] != verif[i]) {
				if( wasTXing ) { enableTX(); }
				return false;
			}
			if(i >= cmdsize) {
				if( wasTXing ) { enableTX(); }
				return true;
			}
		}
	}

	if( wasTXing ) { enableTX(); }
	return (start + SA818_CONTACT_TIMEOUT) < millis();
}

bool SA818::setFilter(bool empha, bool hpass, bool lpass) {

	bool wasTXing = _isTransmitting;
	if( _isTransmitting ) { disableTX(); }


	int cmdsize = 16;
	char verif[cmdsize+1] = {'+','D','M','O','S','E','T','F','I','L','T','E','R',':','0','\r','\n'};

	char buff[cmdsize*2];
	int i = 0;

	Serial.print("AT+SETFILTER="+String(!empha)+","+String(!hpass)+","+String(!lpass)+"\r\n");
	Serial.flush();

	long start = millis();
	while((start + SA818_CONTACT_TIMEOUT) > millis()) {
		if(i == 0) {
			buff[i] = Serial.read();
		}
		if(buff[i] == verif[0] || i>0) {
			delay((SA818_SERBPS / 1000) * 2 ); //Makes sure that the bit has time to come. Espetially that the implementation is wierd

			i++;
			buff[i] = Serial.read();

			if(buff[i] != verif[i]) {
				if( wasTXing ) { enableTX(); }
				return false;
			}
			if(i >= cmdsize) {
				if( wasTXing ) { enableTX(); }
				return true;
			}
		}
	}

	if( wasTXing ) { enableTX(); }
	return (start + SA818_CONTACT_TIMEOUT) < millis();
}

bool SA818::setPower(bool value) {
	//HL to ground - low power, HL float - high power
	if(value){
		pinMode(_pinPowerHL, INPUT);
	}else{
		pinMode(_pinPowerHL, OUTPUT);
		digitalWrite(_pinPowerHL, LOW);
	}
}

bool SA818::setTailTone(bool value) {
	int cmdsize = 14;
	char verif[cmdsize+1] = {'+','D','M','O','S','E','T','T','A','I','L',':','0','\r','\n'};

	char buff[cmdsize*2];
	int i = 0;

	Serial.print("AT+SETTAIL="+String(value)+"\r\n");
	Serial.flush();

	long start = millis();
	while((start + SA818_CONTACT_TIMEOUT) > millis()) {
		if(i == 0) {
			buff[i] = Serial.read();
		}
		if(buff[i] == verif[0] || i>0) {
			delay((SA818_SERBPS / 1000) * 2 ); //Makes sure that the bit has time to come. Espetially that the implementation is wierd

			i++;
			buff[i] = Serial.read();

			if(buff[i] != verif[i]) {
				return false;
			}
			if(i >= cmdsize) {
				return true;
			}
		}
	}

	return (start + SA818_CONTACT_TIMEOUT) < millis();
}

int SA818::readRSSI() {
	int cmdsize = 9;
	char verif[cmdsize+1] = {'R','S','S','I','=','%','%','%','\r','\n'};

	char buff[cmdsize*2];
	int i = 0;

	Serial.print("RSSI?\r\n");
	Serial.flush();

	long start = millis();
	while((start + SA818_CONTACT_TIMEOUT) > millis()) {
		if(i == 0) {
			buff[i] = Serial.read();
		}
		if(buff[i] == verif[0] || i>0) {
			delay((SA818_SERBPS / 1000) * 2 ); //Makes sure that the bit has time to come. Espetially that the implementation is wierd

			i++;
			buff[i] = Serial.read();

			if(buff[i] != verif[i] && verif[i] != '%') {
				return -1;
			}
			if(i >= cmdsize) {
				char tmp[4] = {buff[5], buff[6], buff[7], '\0'};
				int i;
				sscanf(tmp, "%d", &i);
				return i;
			}
		}
	}

	return -1;
}

bool SA818::applyConfig(bool forceCheck) {
    // _CONF_TXfreq     = 433.9000;
    // _CONF_RXfreq     = 433.9000;
    // _CONF_bandwidth  = SA818_BW_25k;
    // int   _CONF_squeltch   = 4;
    // char  _CONF_TXCTCSS[4] = {'0','0','0','0'};
    // char  _CONF_RXCTCSS[4] = {'0','0','0','0'};
    char buffer [50];
    //char txTmp [10];
    //char rxTmp [10];

    //dtostrf(_CONF_TXfreq, 8, 5, txTmp);
    //dtostrf(_CONF_RXfreq, 8, 5, rxTmp);

	int cmdsize = 15;
	char verif[cmdsize+1] = {'+','D','M','O','S','E','T','G','R','O','U','P',':','0','\r','\n'};
	char buff[cmdsize*2];
	int i = 0;
	
	//Serial.println(_CONF_TXCTCSS);
	//Serial.println(_CONF_RXCTCSS);
	
	sprintf(buffer, "AT+DMOSETGROUP=%d,%s,%s,%04d,%d,%04d\r\n", _CONF_bandwidth, _CONF_TXfreq, _CONF_RXfreq, _CONF_TXCTCSS, _CONF_squeltch, _CONF_RXCTCSS);
	//sprintf(buffer, "AT+DMOSETGROUP=%d,%s,%s,%04d,%d,%04d\r\n", _CONF_bandwidth, "433.90000", "433.90000", _CONF_TXCTCSS, _CONF_squeltch, _CONF_RXCTCSS);
	Serial.print(buffer);
	Serial.flush();

	long start = millis();
	while((start + SA818_CONTACT_TIMEOUT) > millis()) {
		if(i == 0) {
			buff[i] = Serial.read();
		}
		if(buff[i] == verif[0] || i>0) {
			delay((SA818_SERBPS / 1000) * 2 ); //Makes sure that the bit has time to come. Espetially that the implementation is wierd

			i++;
			buff[i] = Serial.read();

			if(buff[i] != verif[i]) {
				return false;
			}
			if(i >= cmdsize) {
				return true;
			}
		}
	}

	return (start + SA818_CONTACT_TIMEOUT) < millis();
}
void SA818::printConfig() {
    char buffer [50];
    //char txTmp [11];
    //char rxTmp [11];

    //dtostrf(_CONF_TXfreq, 8, 5, txTmp);
    //dtostrf(_CONF_RXfreq, 8, 5, rxTmp);

	//sprintf(buffer, "AT+DMOSETGROUP=%d,%s,%s,%.4s,%d,%.4s\r\n", _CONF_bandwidth, txTmp, rxTmp, _CONF_TXCTCSS, _CONF_squeltch, _CONF_RXCTCSS);

	sprintf(buffer, "AT+DMOSETGROUP=%d,%s,%s,%04d,%d,%04d\r\n", _CONF_bandwidth, _CONF_TXfreq, _CONF_RXfreq, _CONF_TXCTCSS, _CONF_squeltch, _CONF_RXCTCSS);
	Serial.print(buffer);
}

/*
bool SA818::setTXFrequency(float value) {
	if(_ModuleVersion == SA818_VHF) {
		if (value >134 && value< 174) {
			_CONF_TXfreq = value;
			return true;
		}
		return false;
	}
	if(_ModuleVersion == SA818_UHF) {
		if (value >400 && value< 480) {
			_CONF_TXfreq = value;
			return true;
		}
		return false;
	}
}
bool SA818::setRXFrequency(float value){
	if(_ModuleVersion == SA818_VHF) {
		if (value >134 && value< 174) {
			_CONF_RXfreq = value;
			return true;
		}
		return false;
	}
	if(_ModuleVersion == SA818_UHF) {
		if (value >400 && value< 480) {
			_CONF_RXfreq = value;
			return true;
		}
		return false;
	}
}
*/
bool SA818::setTXFrequency(char value[10]) {
	sprintf(_CONF_TXfreq, "%s", value);
}
bool SA818::setRXFrequency(char value[10]){
	sprintf(_CONF_RXfreq, "%s", value);
}

bool SA818::setBandwidth(int value) {
	if(value != SA818_BW_12_5k && value != SA818_BW_25k) {
		return false;
	}
	_CONF_bandwidth = value;
	return true;
}

bool SA818::setSQueltch(int value) {
	if(value < 0 || value > 8) {
		return false;
	}
	_CONF_squeltch = value;
	return true;
}

/*bool SA818::setRX_CTCSS(char value[4]) {
	sprintf(_CONF_RXCTCSS, "%s", value);
}
bool SA818::setTX_CTCSS(char value[4]) {
	sprintf(_CONF_TXCTCSS, "%s", value);
}*/
bool SA818::setRX_CTCSS(int value) {
	_CONF_RXCTCSS = value;
}
bool SA818::setTX_CTCSS(int value) {
	_CONF_TXCTCSS = value;
}

void SA818::rawSend(String at) {
	Serial.print(at);
	Serial.flush();
	emptySerial();
}

void SA818::emptySerial() {
	for (int i = 0; i < 50; ++i) { Serial.read(); }
}