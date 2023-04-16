#include <esp_now.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ESPAsyncWebSrv.h>

static const char* ssid     = "Muhammad Haiqal's Vivo";
static const char* password = "87654321";

// ThingSpeak settings
String serverName = "api.thingspeak.com";
unsigned long channelID = 2108892;
String apiKey = "9ZL2KOUDOAFFWKZ6";
WiFiClient client;
AsyncWebServer server(80);

// Structure to keep the temperature and humidity data
// Is also required in the client to be able to save the data directly
typedef struct temp_humidity {
  float temperature;
  float humidity;
};

// Create a struct_message called myData
temp_humidity dhtData;
// callback function executed when data is received
void OnRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&dhtData, incomingData, sizeof(dhtData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Temperature: ");
  Serial.println(dhtData.temperature);
  Serial.print("Humidity: ");
  Serial.println(dhtData.humidity);

esp_now_register_recv_cb(OnRecv);
}
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  Serial.println(WiFi.localIP());  
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  // Printout MAC address
  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("There was an error initializing ESP-NOW");
    return;
  }
  
  // Once the ESP-Now protocol is initialized, we will register the callback function
  // to be able to react when a package arrives in near to real time without pooling every loop.
  esp_now_register_recv_cb(OnRecv);

server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    String url = "/update?" + apiKey + "&field1=" + String(analogRead(A0));
    client.connect(serverName, 80);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + serverName + "\r\n" +
                 "Connection: close\r\n\r\n");
    delay(10);
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    client.stop();
  });

  server.begin();  
}
void loop() {
   // Wait for 10 seconds
  delay(10000);

  // Generate a random number between 0 and 100
  int value = random(100);

  // Send the data to ThingSpeak
  if (client.connect(server.c_str(), 80)) {
    String postData = apiKey;
    postData += "&field1=";
    postData += String(value);
    postData += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\r\n");
    client.print("Host: api.thingspeak.com\r\n");
    client.print("Connection: close\r\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\r\n");
    client.print("Content-Type: application/x-www-form-urlencoded\r\n");
    client.print("Content-Length: ");
    client.print(postData.length());
    client.print("\r\n\r\n");
    client.print(postData);
  }
  client.stop();

  Serial.print("Value sent to ThingSpeak: ");
  Serial.println(value);
  
//IFTTT
HTTPClient http;
  String url = "https://maker.ifttt.com/trigger/esp32/with/key/fsnQMgWEBSNAjairgGZz3Cw76nDNmLlO4o5Q1m-D0sx";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.printf("HTTP GET request sent, response code: %d\n", httpCode);
  } else {
    Serial.println("HTTP GET request failed");
  }
  http.end();
  delay(5000);  
}