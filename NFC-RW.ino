#include <SPI.h>
#include <MFRC522.h>

/*
* Signal      Pin          Pin
* -----------------------------
* RST/Reset   RST          9
* SPI SS      SDA(SS)      10
* SPI MOSI    MOSI         11
* SPI MISO    MISO         12
* SPI SCK     SCK          13
*/

#define SS_PIN 10
#define RST_PIN 9

#define rLed 5
#define gLed 4
#define bLed 6

#define writeBtn 2
#define NEW_UID {0xFF, 0xFF, 0xFF, 0xFF}
MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

byte nuidPICC[3];
int ledState[2];

void setup() {
	Serial.begin(9600);
	pinMode(writeBtn, INPUT_PULLUP);

	pinMode(rLed, OUTPUT);
	pinMode(gLed, OUTPUT);
	pinMode(bLed, OUTPUT);
	
	SPI.begin();
	mfrc522.PCD_Init();

	for (byte i = 0; i < 6; i++) {
		key.keyByte[i] = 0xFF;
	}

	intro();
	digitalWrite(bLed, 1);
}

void loop() {
	readSerial();
	if (isWritePressed()) {
		mifareWrite();
		return;
	}
		mifareRead();
}

void notifyRead(String type) {
	if (type == "SET") {
		//Serial.print("S");
		Serial.print("Type: " + type);
		} else {
		Serial.print("Type: " + type);
	}
	Serial.println();
	Serial.print("UID:");
	for (byte i = 0; i < 4; i++) {
		Serial.print(nuidPICC[i] < 0x10 ? " 0" : " ");
		Serial.print(nuidPICC[i], HEX);
	}
	Serial.println();
}

void mifareRead() {
	if (!mfrc522.PICC_IsNewCardPresent())
	return;

	if (!mfrc522.PICC_ReadCardSerial())
	return;

	MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
	
	if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
	piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
	piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
		Serial.println(F("Unsupported card type"));
		return;
	}

	for (byte i = 0; i < 4; i++) {
		nuidPICC[i] = mfrc522.uid.uidByte[i];
	}

	notifyRead(mfrc522.PICC_GetTypeName(piccType));
	ledBlink('G');

	mfrc522.PICC_HaltA();
	mfrc522.PCD_StopCrypto1();
}

void mifareWrite() {
	led('R',1);
	if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial()) {
		delay(50);
		led('R',0);
		return;
	}
	led('R',0);

	ledBlink('Y');
	if ( mfrc522.MIFARE_SetUid(nuidPICC, (byte)4, true) ) {
		ledBlink('G');
		Serial.println(F("Wrote new UID to card."));
		} else {
		ledBlink('R');
	}

	mfrc522.PICC_HaltA();
	if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() ) {
		return;
	}

	delay(1000);
}

boolean isWritePressed() {
	if (digitalRead(writeBtn) == LOW) {
		return true;
		} else {
		return false;
	}
}


// LED DIODE
void turnOffLeds() {
	digitalWrite(rLed, 0);
	digitalWrite(gLed, 0);
	digitalWrite(bLed, 0);
}

void saveLedState() {
	ledState[0] = digitalRead(rLed);
	ledState[1] = digitalRead(gLed);
	ledState[2] = digitalRead(bLed);
	turnOffLeds();
}

void loadLedState() {
	digitalWrite(rLed, ledState[0]);
	digitalWrite(gLed, ledState[1]);
	digitalWrite(bLed, ledState[2]);
}

void led(char led, int state) {
	switch (led) {
		case 'R':
		state == HIGH ? saveLedState() : loadLedState();
		digitalWrite(rLed, state);
		break;
		case 'G':
		state == HIGH ? saveLedState() : loadLedState();
		digitalWrite(gLed, state);
		break;
		case 'B':
		state == HIGH ? saveLedState() : loadLedState();
		digitalWrite(bLed, state);
		break;
		case 'Y':
		state == HIGH ? saveLedState() : loadLedState();
		digitalWrite(rLed, state);
		digitalWrite(gLed, state);
		break;
	}
}

void intro() {
	for (int i=0; i<8; i++) {
		led('R',1);
		delay(30);
		led('R',0);
		led('G',1);
		delay(30);
		led('G',0);
		led('B',1);
		delay(30);
		led('B',0);
	}
}

void ledBlink(char led) {
	saveLedState();
	turnOffLeds();
	for (int i=0; i<4; i++) {
		switch(led) {
			case 'R':
			digitalWrite(rLed, 1);
			break;
			case 'G':
			digitalWrite(gLed, 1);
			break;
			case 'B':
			digitalWrite(bLed, 1);
			break;
			case 'Y':
			digitalWrite(rLed, 1);
			digitalWrite(gLed, 1);
			break;
		}
		delay(50);

		switch(led) {
			case 'R':
			digitalWrite(rLed, 0);
			break;
			case 'G':
			digitalWrite(gLed, 0);
			break;
			case 'B':
			digitalWrite(bLed, 0);
			break;
			case 'Y':
			digitalWrite(rLed, 0);
			digitalWrite(gLed, 0);
			break;
		}
		delay(50);
	}
	loadLedState();
}
// ------------------------------

void readSerial() {
	if (Serial.available() >= 4) {
		nuidPICC[0]=Serial.read();
		nuidPICC[1]=Serial.read();
		nuidPICC[2]=Serial.read();
		nuidPICC[3]=Serial.read();

		notifyRead("SET");

		while(Serial.available() > 0) {
			Serial.read();
		}

		ledBlink('Y');
	}
}


