/***************************************************
  Chris Woodle
  MIT license
 ****************************************************/

#include <ArduinoJson.h>      // JSON parsing library

#include <Adafruit_GFX.h>     // Core graphics library
#include "Adafruit_EPD.h"

#define EPD_CS      0
#define EPD_DC      15
#define SRAM_CS     16
#define EPD_RESET   -1 // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY    -1 // can set to -1 to not use a pin (will wait a fixed delay)

/* 2.13" tricolor EPD */
Adafruit_IL0373 display(212, 104, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);


// Reference sketch: HTTPRequestCaCertAxTLS from ESP8826 bsp
#define USING_AXTLS
#include <time.h>
#include <ESP8266WiFi.h>

// force use of AxTLS (BearSSL is now default)
#include <WiFiClientSecureAxTLS.h>
using namespace axTLS;

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-psk"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

const char* host = "www.googleapis.com";

// Root certificate used by www.googleapis.com.
// Defined in "CACert" tab.
extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;

const char* channelId = "your-channel-id";
const char* apiKey = "your-google-api-key";

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored  "-Wdeprecated-declarations"
WiFiClientSecure client;
#pragma GCC diagnostic pop

float p = 3.1415926;

StaticJsonDocument<620> doc;

void setup(void) {
  Serial.begin(115200);
  Serial.print("Hello! EPD Test");

  display.begin();

  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Initialized");

  // Synchronize time useing SNTP. This is necessary to verify that
  // the TLS certificates offered by the server are currently valid.
  Serial.print("Setting time using SNTP");
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

  // Load root certificate in DER format into WiFiClientSecure object
  bool res = client.setCACert_P(caCert, caCertLen);
  if (!res) {
    Serial.println("Failed to load root CA certificate!");
    while (true) {
      yield();
    }
  }

  //  display.clearBuffer();
  //  display.fillRect(display.width() / 3, 0, display.width() / 3, display.height(), EPD_RED);
  //  display.fillRect((display.width() * 2) / 3, 0, display.width() / 3, display.height(), EPD_BLACK);
  //  display.display();
  //
  //  delay(15 * 1000);

  Serial.println("done");
}

void loop() {
  delay(500);

  const int httpsPort = 443;
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  // Verify validity of server's certificate
  if (client.verifyCertChain(host)) {
    Serial.println("Server certificate verified");
  } else {
    Serial.println("ERROR: certificate verification failed!");
    return;
  }

  // We now create a URI for the request
  String url = "/youtube/v3/channels?part=statistics";
  url += "&id=";
  url += channelId;
  url += "&key=";
  url += apiKey;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
    if (line.indexOf("subscriberCount") > -1) {
      
      // JSON Parse
      DeserializationError error = deserializeJson(doc, line);

      // Test if parsing succeeds.
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }

      int pageInfo_totalResults = doc["pageInfo"]["totalResults"]; // 1
      int pageInfo_resultsPerPage = doc["pageInfo"]["resultsPerPage"]; // 1

      JsonObject items_0 = doc["items"][0];

      JsonObject items_0_statistics = items_0["statistics"];
      const char* items_0_statistics_viewCount = items_0_statistics["viewCount"]; // "28169526"
      const char* items_0_statistics_commentCount = items_0_statistics["commentCount"]; // "0"
      const char* items_0_statistics_subscriberCount = items_0_statistics["subscriberCount"]; // "207906"
      bool items_0_statistics_hiddenSubscriberCount = items_0_statistics["hiddenSubscriberCount"]; // false
      const char* items_0_statistics_videoCount = items_0_statistics["videoCount"]; // "363"
      Serial.println(items_0_statistics_subscriberCount);
      display.clearBuffer();
      display.fillRect(0, 0, display.width() / 3, display.height(), EPD_RED);
      display.setCursor(display.width() / 3 + 5, display.height() / 2 - 10);
      display.fillScreen(EPD_WHITE);
      display.setTextColor(EPD_BLACK);
      display.setTextSize(3);
      display.println(items_0_statistics_subscriberCount);
      display.setTextSize(1);
      display.setTextColor(EPD_RED);
      display.display();
    }

  }
}

void testlines(uint16_t color) {
  display.clearBuffer();
  display.fillScreen(EPD_WHITE);
  for (int16_t x = 0; x < display.width(); x += 6) {
    display.drawLine(0, 0, x, display.height() - 1, color);
  }
  for (int16_t y = 0; y < display.height(); y += 6) {
    display.drawLine(0, 0, display.width() - 1, y, color);
  }

  display.fillScreen(EPD_WHITE);
  for (int16_t x = 0; x < display.width(); x += 6) {
    display.drawLine(display.width() - 1, 0, x, display.height() - 1, color);
  }
  for (int16_t y = 0; y < display.height(); y += 6) {
    display.drawLine(display.width() - 1, 0, 0, y, color);
  }

  display.fillScreen(EPD_WHITE);
  for (int16_t x = 0; x < display.width(); x += 6) {
    display.drawLine(0, display.height() - 1, x, 0, color);
  }
  for (int16_t y = 0; y < display.height(); y += 6) {
    display.drawLine(0, display.height() - 1, display.width() - 1, y, color);
  }

  display.fillScreen(EPD_WHITE);
  for (int16_t x = 0; x < display.width(); x += 6) {
    display.drawLine(display.width() - 1, display.height() - 1, x, 0, color);
  }
  for (int16_t y = 0; y < display.height(); y += 6) {
    display.drawLine(display.width() - 1, display.height() - 1, 0, y, color);
  }
  display.display();
}

void testdrawtext(char *text, uint16_t color) {
  display.clearBuffer();
  display.setCursor(5, 5);
  display.setTextColor(color);
  display.setTextWrap(true);
  display.print(text);
  display.display();
}

void testfastlines(uint16_t color1, uint16_t color2) {
  display.clearBuffer();
  display.fillScreen(EPD_WHITE);
  for (int16_t y = 0; y < display.height(); y += 5) {
    display.drawFastHLine(0, y, display.width(), color1);
  }
  for (int16_t x = 0; x < display.width(); x += 5) {
    display.drawFastVLine(x, 0, display.height(), color2);
  }
  display.display();
}

void testdrawrects(uint16_t color) {
  display.clearBuffer();
  display.fillScreen(EPD_WHITE);
  for (int16_t x = 0; x < display.width(); x += 6) {
    display.drawRect(display.width() / 2 - x / 2, display.height() / 2 - x / 2 , x, x, color);
  }
  display.display();
}

void testfillrects(uint16_t color1, uint16_t color2) {
  display.clearBuffer();
  display.fillScreen(EPD_WHITE);
  for (int16_t x = display.width() - 1; x > 6; x -= 6) {
    display.fillRect(display.width() / 2 - x / 2, display.height() / 2 - x / 2 , x, x, color1);
    display.drawRect(display.width() / 2 - x / 2, display.height() / 2 - x / 2 , x, x, color2);
  }
  display.display();
}

void testfillcircles(uint8_t radius, uint16_t color) {
  display.clearBuffer();
  for (int16_t x = radius; x < display.width(); x += radius * 2) {
    for (int16_t y = radius; y < display.height(); y += radius * 2) {
      display.fillCircle(x, y, radius, color);
    }
  }
  display.display();
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  display.clearBuffer();
  for (int16_t x = 0; x < display.width() + radius; x += radius * 2) {
    for (int16_t y = 0; y < display.height() + radius; y += radius * 2) {
      display.drawCircle(x, y, radius, color);
    }
  }
  display.display();
}

void testtriangles() {
  display.clearBuffer();
  display.fillScreen(EPD_WHITE);
  int color = EPD_BLACK;
  int t;
  int w = display.width() / 2;
  int x = display.height() - 1;
  int y = 0;
  int z = display.width();
  for (t = 0 ; t <= 15; t++) {
    display.drawTriangle(w, y, y, x, z, x, color);
    x -= 4;
    y += 4;
    z -= 4;
    if (t == 8) color = EPD_RED;
  }
  display.display();
}

void testroundrects() {
  display.clearBuffer();
  display.fillScreen(EPD_WHITE);
  int color = EPD_BLACK;
  int i;
  int t;
  for (t = 0 ; t <= 4; t += 1) {
    int x = 0;
    int y = 0;
    int w = display.width() - 2;
    int h = display.height() - 2;
    for (i = 0 ; i <= 16; i += 1) {
      display.drawRoundRect(x, y, w, h, 5, color);
      x += 2;
      y += 3;
      w -= 4;
      h -= 6;
      if (i == 7) color = EPD_RED;
    }
    color = EPD_BLACK;
  }
  display.display();
}



void epdPrintTest() {
  display.clearBuffer();
  display.setCursor(5, 5);
  display.fillScreen(EPD_WHITE);
  display.setTextColor(EPD_BLACK);
  display.setTextSize(2);
  display.println("Hello World!");
  display.setTextSize(1);
  display.setTextColor(EPD_RED);
  display.print(p, 6);
  display.println(" Want pi?");
  display.println(" ");
  display.print(8675309, HEX); // print 8,675,309 out in HEX!
  display.println(" Print HEX!");
  display.println(" ");
  display.setTextColor(EPD_BLACK);
  display.println("Sketch has been");
  display.println("running for: ");
  display.setTextColor(EPD_RED);
  display.print(millis() / 1000);
  display.setTextColor(EPD_BLACK);
  display.print(" seconds.");
  display.display();
}

void mediabuttons() {
  display.clearBuffer();
  // play
  display.fillScreen(EPD_WHITE);
  display.fillRoundRect(25, 10, 78, 60, 8, EPD_BLACK);
  display.fillTriangle(42, 20, 42, 60, 90, 40, EPD_RED);
  // pause
  display.fillRoundRect(25, 90, 78, 60, 8, EPD_BLACK);
  display.fillRoundRect(39, 98, 20, 45, 5, EPD_RED);
  display.fillRoundRect(69, 98, 20, 45, 5, EPD_RED);
  display.display();
}
