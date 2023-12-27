#include <Arduino.h>

#include <Main.h>

#include <Adafruit_GFX.h>
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>
#include <ml.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>

#include <FS.h>
#include <LittleFS.h>

#define PIN 2

ESP8266WebServer server(80);

#define mw 40
#define mh 32

#define tx 5
#define ty 4

#define NUMMATRIX (mw * mh)

#define BLUE 0x0F0

#define PIN 2

CRGB matrixleds[NUMMATRIX];

FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(matrixleds, 8, 8, tx, ty,
                                                  NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE +
                                                      NEO_TILE_TOP + NEO_TILE_RIGHT + NEO_TILE_ROWS + NEO_TILE_ZIGZAG);

/*
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(matrixleds, 8, 8, tx, ty,
                                                  NEO_TILE_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE +
                                                      NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS + NEO_TILE_ZIGZAG);
 */


void handleFileRequest() {
    String path = server.uri();
    Serial.println(path);
    if (path.endsWith("/")) { path += "index.html"; }

    if (LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        server.streamFile(file, getContentType(path)); // Get content type dynamically
        file.close();
    } else {
        server.send(404, "text/plain", "File Not Found");
    }
}

String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void setup()
{
  Serial.begin(115200);

  // Initialize LittleFS
  if(!LittleFS.begin()){
      Serial.println("An Error has occurred while mounting LittleFS");
      return;
  }

  FastLED.addLeds<NEOPIXEL, PIN>(matrixleds, NUMMATRIX);
  matrix->begin();
  matrix->setBrightness(255);
 // matrix->setBrightness(16);
  matrix->fillScreen(0x0000);
  matrix->fillScreen(0x0000);

  WiFiManager wifiManager;
  wifiManager.autoConnect("PixelFrame-AP");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  WiFi.hostname("PixelFrame");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("pixelframe"))
  { // Start the mDNS responder for pixelframe.local
    Serial.println("Error setting up MDNS responder!");
  }

  Serial.println("mDNS responder started");

  server.on(
      "/handleFileUpload", handleFileUpload);
  server.onNotFound(handleFileRequest);

  server.begin();
  Serial.println("Web Server started");
}

void loop()
{
  server.handleClient();
}
uint16_t bytes[1280];
void handleFileUpload()
{
  if (server.hasArg("plain") == false)
  {
    server.send(200, "text/plain", "Body not received");
    return;
  }

  String bmpData = server.arg("plain");

  if (server.hasArg("brightness") == true)
  {
    uint16 brightness = server.arg("brightness").toInt();
    if (brightness != 0)
    {
      matrix->setBrightness(brightness);
    }
  }

  int index = 0;

  if (bmpData.length() > 5120)
  {
    server.send(200, "text/plain", "Body must not exceed 5120 HEX characters!");
  }
  else
  {
    for (uint16_t i = 0; i < bmpData.length() - 3; i = i + 4)
    {
      String sub = bmpData.substring(i, i + 4);
      bytes[index] = strtol(sub.c_str(), NULL, 16);
      index++;
      ESP.wdtFeed();
    }

    matrix->clear();
    matrix->drawRGBBitmap(0, 0, bytes, 40, 32);
    matrix->show();

    server.send(200, "text/plain", "Image received");
  }
}