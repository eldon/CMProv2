#include "Adafruit_Soundboard.h"


// Choose any two pins that can be used with SoftwareSerial to RX & TX
#define SFX_TX TX1
#define SFX_RX RX0
#define BTN 6

// Connect to the RST pin on the Sound Board
#define SFX_RST A3

// You can also monitor the ACT pin for when audio is playing!

// we'll be using software serial
//SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);

// pass the software serial to Adafruit_soundboard, the second
// argument is the debug port (not used really) and the third 
// arg is the reset pin
// Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);
// can also try hardware serial with
Adafruit_Soundboard sfx = Adafruit_Soundboard(&Serial1, NULL, SFX_RST);

bool lastButtonState = HIGH;

void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit Sound Board!");
  
  // softwareserial at 9600 baud
  //Serial1.begin(9600);
  Serial1.begin(9600);


  pinMode(BTN, INPUT_PULLUP);

  if (!sfx.reset()) {
    Serial.println("Not found");
    while (1) {
    }
  }
  Serial.println("SFX board found");
  pinMode(13, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(BTN) != lastButtonState && digitalRead(BTN) == LOW) {
    //play music
    Serial.println("button pushed");
    sfx.playTrack(1);
  }
  lastButtonState = digitalRead(BTN);


}
