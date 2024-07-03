#include "FS.h"
#include "SPIFFS.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define FORMAT_SPIFFS_IF_FAILED true

const char* ssid = "GHOSTLAP";
const char* password = "Ghostlap";
const char* server = "http://api.thingspeak.com/channels/YOUR_CHANNEL_ID/bulk_update.json";
const char* writeAPIKey = "EY53TXWC5D8S4L0N";

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("- message appended");
  } else {
    Serial.println("- append failed");
  }
  file.close();
}

String generateRandomLatLong() {
  float lat = random(-90000000, 90000000) / 1000000.0;
  float lon = random(-180000000, 180000000) / 1000000.0;
  char buffer[50];
  sprintf(buffer, "%.6f,%.6f", lat, lon);
  return String(buffer);
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
}

void bulkPushToThingSpeak(String data) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server);
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    String postData = data;
    int httpResponseCode = http.POST(postData);
    if (httpResponseCode > 0) {
      Serial.printf("HTTP Response code: %d\n", httpResponseCode);
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.printf("Error code: %d\n", httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void setup() {
  Serial.begin(115200);
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  connectToWiFi();

  listDir(SPIFFS, "/", 0);
  writeFile(SPIFFS, "/data.txt", ""); // Create an empty file to store data

  // Store random lat long data every 30 seconds for 2 minutes
  for (int i = 0; i < 4; i++) {
    String latLong = generateRandomLatLong();
    Serial.print("Generated LatLong: ");
    Serial.println(latLong);
    String latLongWithNewline = latLong + "\n";
    appendFile(SPIFFS, "/data.txt", latLongWithNewline.c_str());
    delay(30000); // 30 seconds
  }

  // Read all data from the file
  File file = SPIFFS.open("/data.txt");
  String data = "";
  if (file) {
    while (file.available()) {
      data += char(file.read());
    }
    file.close();
  }

  Serial.print("Data to be pushed: ");
  Serial.println(data);

  // Prepare data for bulk push
  String bulkData = "{\"write_api_key\":\"" + String(writeAPIKey) + "\", \"updates\":[";
  String entry = "";
  for (int i = 0; i < data.length(); i++) {
    if (data[i] == '\n') {
      bulkData += "{\"field1\":\"" + entry + "\"},";
      entry = "";
    } else {
      entry += data[i];
    }
  }

  if (entry.length() > 0) {
    bulkData += "{\"field1\":\"" + entry + "\"}";
  } else {
    bulkData.remove(bulkData.length() - 1); // Remove the last comma
  }

  bulkData += "]}";

  Serial.print("Bulk Data to be pushed: ");
  Serial.println(bulkData);

  // Bulk push to ThingSpeak
  bulkPushToThingSpeak(bulkData);

  Serial.println("Test complete");
}

void loop() {}
