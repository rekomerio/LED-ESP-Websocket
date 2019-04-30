
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <FS.h>

#define LED_RED     12  //D6
#define LED_GREEN   13  //D7
#define LED_BLUE    15  //D8

#define USE_SERIAL Serial


ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      USE_SERIAL.printf("[%u] Message: %s\n", num, payload);
      readData(payload);
      break;
  }

}

uint8_t r = 0;
uint8_t g = 0;
uint8_t b = 0;
uint8_t program = 0;
uint8_t interval = 5;
uint32_t lastMillis = 0;
uint32_t currentM;
float i = 0;
bool reversed = false;

void setup() {

  USE_SERIAL.begin(115200);
  //USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  digitalWrite(LED_RED, 1);
  digitalWrite(LED_GREEN, 1);
  digitalWrite(LED_BLUE, 1);

  WiFiMulti.addAP("Wifi", "pass");

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  SPIFFS.begin();
  {
    Serial.println("SPIFFS contents:");

    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
    }
    Serial.printf("\n");
  }
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  if (MDNS.begin("esp8266")) {
    USE_SERIAL.println("MDNS responder started");
  }

  // handle index
  server.serveStatic("/", SPIFFS, "/index.html", "max-age=86400");
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
  server.begin();

  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);

  digitalWrite(LED_RED, 0);
  digitalWrite(LED_GREEN, 0);
  digitalWrite(LED_BLUE, 0);

}

void loop() {
  webSocket.loop();
  server.handleClient();
  ledProgram();
}

void serveIndex() {
  File file = SPIFFS.open("/index.html", "r");
  server.streamFile(file, "text/html");
  file.close();
}


String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void readData(uint8_t payload[]) {
  uint32_t data = (uint32_t) strtol((const char *) &payload[1], NULL, 16);

  switch (payload[0]) {
    case '#':
      r = ((data >> 16) & 0xFF);
      g = ((data >> 8) & 0xFF);
      b = ((data >> 0) & 0xFF);
      break;
    case '!':
      program = data;
      break;
  }
}
void ledProgram() {
  switch (program) {
    case 0: none();
      break;
    case 1: fire();
      break;
    case 2: alarm();
      break;
    default: none();
      break;
  }
}

void none() {
  analogWrite(LED_RED, r * 4);
  analogWrite(LED_GREEN, g * 4);
  analogWrite(LED_BLUE, b * 4);
}

void fire() {
  currentM = millis();
  if ((uint32_t)(currentM - lastMillis) >= interval) {
    lastMillis = currentM;
    analogWrite(LED_RED, (r * 4) * i);
    analogWrite(LED_GREEN, (g * 4)* i);
    analogWrite(LED_BLUE, (b * 4) * i);
    i -= 0.01;
    if (i < 0.4) {
      interval = random(2, 20);
      i = 1;
    }
  }
}
void alarm() {
  currentM = millis();
  if ((uint32_t)(currentM - lastMillis) >= 15) {
    lastMillis = currentM;
    analogWrite(LED_RED, (r * 4) * i);
    analogWrite(LED_GREEN, (g * 4)* i);
    analogWrite(LED_BLUE, (b * 4) * i);
    if (reversed)
      i -= 0.01;
    else
      i += 0.01;
    if (i <= 0.0 || i >= 1) {
      reversed = !reversed;
    }
  }
}

