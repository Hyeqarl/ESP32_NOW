#include <esp_now.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ESPAsyncWebSrv.h>

static const char* ssid     = "Muhammad Haiqal's Vivo";
static const char* password = "87654321";

// ThingSpeak settings
// Domain Name with full URL Path for HTTP POST Request
const char* serverName = "http://api.thingspeak.com/update";
// Service API Key
String apiKey = "BL22P1HKXWQ8PMFA";
//AsyncWebServer server(80);

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
unsigned long timerDelay = 100;


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
}
void loop() {
   // Wait for 10 seconds
  delay(10000);

  // Generate a random number between 0 and 100
  int value = random(100);

  // Send the data to ThingSpeak
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
    
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
      
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String temperatureData = "api_key=" + apiKey + "&field1=" + String(random(40));
      String humidityData = "api_key=" + apiKey + "&field2=" + String(random(40));           
      // Send HTTP POST request
      int temperatureCode = http.POST(temperatureData);
      int humidityCode = http.POST(humidityData);
      Serial.print("HTTP Temperature Code: ");
      Serial.println(temperatureCode);
      Serial.print("HTTP Humidity Code: ");
      Serial.println(humidityCode);
        
      // Free resources
      http.end();
    }
    lastTime = millis();
  }
  
//IFTTT
HTTPClient http;
  String url = "https://maker.ifttt.com/trigger/esp32/with/key/fsnQMgWEBSNAjairgGZz3Cw76nDNmLlO4o5Q1m-D0sx";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.printf("HTTP GET request sent, response code: %d\n", httpCode);
  } 
  else {
    Serial.println("HTTP GET request failed");
  }
  http.end();
  delay(5000);  
}
