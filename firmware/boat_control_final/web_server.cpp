#include "web_server.h"
#include <WiFi.h>
#include "actuators.h"
#include "autopilot.h"
#include "config.h"
#include "sensors.h"
#include "web_page.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#warning "secrets.h absent : copiez secrets.example.h en secrets.h (valeurs par défaut utilisées)"
#include "secrets.example.h"
#endif

static WiFiServer server(80);

// Wi-Fi 
static void startAccessPoint() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.printf("    SSID: %s\n", AP_SSID);
  Serial.printf("    IP: %s\n", WiFi.softAPIP().toString().c_str());
}

void initWiFi() {
  Serial.println(">>> Configuration WiFi...");
  if (USE_AP_MODE) {
    Serial.println("    Mode: Access Point");
    startAccessPoint();
    return;
  }

  Serial.println("    Mode: Station");
  WiFi.mode(WIFI_STA);
  WiFi.begin(STA_SSID, STA_PASSWORD);
  Serial.print("    Connexion");
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {  // 10 s max
    delay(500);
    Serial.print(".");
    timeout++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" OK");
    Serial.printf("    IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println(" ÉCHEC");
    Serial.println("    Passage en mode AP...");
    startAccessPoint();
  }
}

void initWebServer() {
  server.begin();
  Serial.println("    ✓ Serveur HTTP démarré sur port 80\n");
}

static bool queryParam(const String& req, const char* key, String& value) {
  int idx = req.indexOf(key);
  if (idx < 0) return false;
  value = req.substring(idx + strlen(key));
  int amp = value.indexOf('&');
  value = value.substring(0, amp >= 0 ? amp : value.indexOf(' '));
  return true;
}

static void sendOk(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK\r\nContent-Type:text/plain\r\nConnection:close\r\n\r\nOK");
}

static void sendJson(WiFiClient& client, const String& json) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  client.println(json);
}

// Routes 
static void handleClient(WiFiClient client) {
  String req = client.readStringUntil('\r');
  client.readStringUntil('\n');
  String v;

  if (req.indexOf("GET /servo") >= 0) {
    if (queryParam(req, "angle=", v)) applyServo(v.toInt());
    sendOk(client);
  }

  else if (req.indexOf("GET /dir") >= 0) {
    if (queryParam(req, "d=", v)) {
      direction = v;
      applyMotor();
    }
    sendOk(client);
  }

  else if (req.indexOf("GET /speed?") >= 0) {
    if (queryParam(req, "val=", v)) {
      speedVal = v.toInt();
      applyMotor();
    }
    sendOk(client);
  }

  else if (req.indexOf("GET /speed_data") >= 0) {
    String json = "{";
    json += "\"rpm\":" + String(currentRPM) + ",";
    json += "\"speed\":" + String(currentSpeed, 1) + ",";
    json += "\"current\":" + String(currentAmperes, 2);
    json += "}";
    sendJson(client, json);
  }

  else if (req.indexOf("GET /motor") >= 0) {
    if (queryParam(req, "on=", v)) {
      motorOn = (v == "1");
      applyMotor();
    }
    sendOk(client);
  }

  else if (req.indexOf("GET /autopilot") >= 0) {
    if (queryParam(req, "active=", v)) {
      autopilotActive = (v == "1");
      if (autopilotActive) resetPID();
    }
    if (queryParam(req, "heading=", v)) {
      targetHeading = v.toInt();
      Serial.printf("Autopilot: Active=%d Target=%d°\n", autopilotActive, targetHeading);
    }
    sendOk(client);
  }

  else if (req.indexOf("GET /mag") >= 0) {
    String json = "{";
    json += "\"ok\":" + String(magInitialized ? "true" : "false") + ",";
    json += "\"x\":" + String(magX) + ",";
    json += "\"y\":" + String(magY) + ",";
    json += "\"z\":" + String(magZ) + ",";
    if (magInitialized) {
      currentHeading = smoothHeading(calculateHeading());
      json += "\"heading\":" + String(currentHeading, 1);
    } else {
      json += "\"heading\":0.0";
    }
    json += "}";
    sendJson(client, json);
  }

  else if (req.indexOf("GET /calibrate") >= 0) {
    calibrateNorth();
    sendOk(client);
  }

  else if (req.indexOf("GET / ") >= 0 || req.indexOf("GET /index") >= 0) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=utf-8");
    client.println("Connection: close");
    client.println();
    client.print(WEB_PAGE);
  }

  else {
    client.println("HTTP/1.1 404 Not Found\r\nConnection:close\r\n\r\n404");
  }

  client.stop();
}

void handleWebClients() {
  WiFiClient client = server.available();
  if (client) handleClient(client);
}
