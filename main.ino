#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>

const char* ssid = "abdallah ben aicha";
const char* password = "99044270";

// IP of PC/Raspberry Pi running Node-RED
const char* serverName = "http://192.168.1.100:1880/ecggps";

HardwareSerial ECGSerial(1);
HardwareSerial GPSSerial(2);

TinyGPSPlus gps;

String ecgData = "";
String gpsData = "";

void setup() {
  Serial.begin(115200);

  ECGSerial.begin(9600, SERIAL_8N1, 10, 9);
  GPSSerial.begin(9600, SERIAL_8N1, 16, 17);

  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());
}

void loop() {

  // Read ECG
  if (ECGSerial.available()) {
    ecgData = ECGSerial.readStringUntil('\n');
  }

  // Read GPS
  while (GPSSerial.available()) {

    gps.encode(GPSSerial.read());

    if (gps.location.isUpdated()) {

      gpsData =
        String(gps.location.lat(), 6) + "," +
        String(gps.location.lng(), 6);

      sendToNodeRed();

      delay(5000);
    }
  }
}

void sendToNodeRed() {

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{";
    jsonData += "\"ID\":\"000000018\",";
    jsonData += "\"ECG\":\"" + ecgData + "\",";
    jsonData += "\"Latitude\":\"" + String(gps.location.lat(), 6) + "\",";
    jsonData += "\"Longitude\":\"" + String(gps.location.lng(), 6) + "\",";
    jsonData += "\"Time\":\"" + getTimeGPS() + "\"";
    jsonData += "}";

    int httpResponseCode = http.POST(jsonData);

    Serial.print("HTTP Response: ");
    Serial.println(httpResponseCode);

    http.end();
  }
}

String getTimeGPS() {

  if (gps.time.isValid()) {

    char buffer[20];

    sprintf(buffer,
            "%02d:%02d:%02d",
            gps.time.hour(),
            gps.time.minute(),
            gps.time.second());

    return String(buffer);
  }

  return "N/A";
}
