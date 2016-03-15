/*
  Breakin' Beats Web Server
  Allows parental input to control sound from children's crutches via webpage
  (Currently works via local network access point, could be expanded in future)
 
  Based on Arduino Tutorial: WiFi Web Server LED Blink

 A simple  server that lets you play sound on Breakin' Beats crutches via the web.
 This sketch will create a new access point (with no password).
 It will then lauwebnch a new server and print out the IP address
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 13 and generate sound input.

 If the IP address of your shield is yourAddress:
 http://yourAddress/R turns on Right sound via input sent to Soundcard
 http://yourAddress/L turns on Left sound

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 created 25 Nov 2012
 by Tom Igoe
 */

/*
  Found 8 Files
  0 name: T00     OGG size: 10538
  1 name: BONGO1  WAV size: 57338
  2 name: BONGO2  WAV size: 106094
  3 name: GUITARG WAV size: 2429792
  4 name: GUITARF WAV size: 2426392
  5 name: GUITARC WAV size: 2435544
  6 name: GUITARAMWAV size: 2432092
  7 name: COWBELL WAV size: 13284
  ========================
*/

#include <SPI.h>
#include "Adafruit_WINC1500.h"
#include "Adafruit_Soundboard.h"


// Define the WINC1500 board connections below.
// If you're following the Adafruit WINC1500 board
// guide you don't need to modify these:
#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2     // or, tie EN to VCC
// The SPI pins of the WINC1500 (SCK, MOSI, MISO) should be
// connected to the hardware SPI port of the Arduino.
// On an Uno or compatible these are SCK = #13, MISO = #12, MOSI = #11.
// On an Arduino Zero use the 6-pin ICSP header, see:
//   https://www.arduino.cc/en/Reference/SPI

// Setup the WINC1500 connection with the pins above and the default hardware SPI.
Adafruit_WINC1500 WiFi(WINC_CS, WINC_IRQ, WINC_RST);

// Or just use hardware SPI (SCK/MOSI/MISO) and defaults, SS -> #10, INT -> #7, RST -> #5, EN -> 3-5V
//Adafruit_WINC1500 WiFi;

#define LED_PIN  13  // This example assumes you have a LED connected to pin 13

#define DRUM_AND_STRUM_PIN A0
#define DRUM_AND_STRUM_PIN_THRESH 100
#define DRUM_PIN_2 A1
#define DRUM_PIN_2_THRESH 100
#define GUITAR_C_PIN 5
#define GUITAR_F_PIN 6
#define GUITAR_G_PIN 9
#define GUITAR_Am_PIN 10

#define SFX_TX TX1
#define SFX_RX RX0

#define SFX_RST A3

bool cowbell_on = false; // Global variable to check if cowbell should be on or not
bool guitar_c_on = false;
bool guitar_f_on = false;
bool guitar_g_on = false;
bool guitar_am_on = false;

char ssid[] = "feather";      //  created AP name
char pass[] = "wing";         // (not supported yet)

int status = WL_IDLE_STATUS;
Adafruit_Soundboard sfx = Adafruit_Soundboard(&Serial1, NULL, SFX_RST);

Adafruit_WINC1500Server server(80);

void setup() {
#ifdef WINC_EN
  pinMode(WINC_EN, OUTPUT);
  digitalWrite(WINC_EN, HIGH);
#endif

  Serial.begin(115200);
  Serial1.begin(9600);

  Serial.println("Access Point Web Server");

  pinMode(LED_PIN, OUTPUT);      // set the LED pin mode
  
  pinMode(DRUM_AND_STRUM_PIN, INPUT);
  pinMode(DRUM_PIN_2, INPUT);
  pinMode(GUITAR_C_PIN, INPUT_PULLUP);
  pinMode(GUITAR_F_PIN, INPUT_PULLUP);
  pinMode(GUITAR_G_PIN, INPUT_PULLUP);
  pinMode(GUITAR_Am_PIN, INPUT_PULLUP);  

  Serial.println("Initializing sound board.");
  if (!sfx.reset()) {
    Serial.println("Not found");
    while (1);
  }
  Serial.println("SFX board found");

  Serial.println("Initializing wifi board.");
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  Serial.print("Creating Network named: ");
  Serial.println(ssid);                   // print the network name (SSID);
  status = WiFi.beginAP(ssid);
  // wait 10 seconds for connection:
  delay(10000);
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status
  Serial.println("Wifi board initialized.");
}

void loop() {
  Adafruit_WINC1500Client client = server.available();   // listen for incoming clients

  delay(10);

  if (cowbell_on) {
    Serial.println("More cowbell!");
    sfx.playTrack(7);
    cowbell_on = false;
  }

  if (analogRead(DRUM_PIN_2) > DRUM_PIN_2_THRESH) {
    Serial.println("Drum 2 triggered");
    delay(10);
    sfx.playTrack(2);
  }

  if (analogRead(DRUM_AND_STRUM_PIN) > DRUM_AND_STRUM_PIN_THRESH) {
    Serial.println("Drum and strum triggered");
    if (digitalRead(GUITAR_C_PIN) == LOW) {
      Serial.println("C");
      sfx.playTrack(5);
    } else if (digitalRead(GUITAR_F_PIN) == LOW) {
      Serial.println("F");
      sfx.playTrack(5);
    } else if (digitalRead(GUITAR_G_PIN) == LOW) {
      Serial.println("G");
      sfx.playTrack(3);
    } else if (digitalRead(GUITAR_Am_PIN) == LOW) {
      Serial.println("Am");
      sfx.playTrack(6);
    } else {
      Serial.println("Bongo 1");
      sfx.playTrack(1);
    }
  }

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            client.print("<h1 style='color:blue;'>Welcome to Breakin\' Beats!</h1>");
            client.print("<h2 style='color:red;'>Click <a href=\"/C\">here</a> for cowbell</h2>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /C":
        if (currentLine.endsWith("GET /C")) {
          // Send Soundcard signal
          cowbell_on = true;
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
