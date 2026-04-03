#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
//#include <NTPClient.h>
#include <WiFiUdp.h>
//#include <html.h>


#define DHTPIN 14        // Pin where DHT11 is connected
#define DHTTYPE DHT11    // DHT 11
#define ONE_WIRE_BUS 12  // Pin where DS18B20 sensors are connected

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// WiFi credentials
const char* ssid = "BS_NET";        // Replace with your network SSID
const char* password = "28062024";  // Replace with your network password

// NTP setup
WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);  // UTC+1 timezone

ESP8266WebServer server(80);
WiFiClient client;

float lastInternalTemp = 0;
float lastExternalTemp = 0;
float lastHumidity = 0;
float internalTemp = 0;
float externalTemp = 0;
float humidity = 0;

float eQinternalTemp = 0;
float eQexternalTemp = 0;
float eQumidity = 0;

unsigned long lastMeasurementTime = 0;
unsigned long lastDatabaseUpdate = 0;
unsigned long lastTerminalUpdate = 0;
unsigned long lastAPIFetch = 0;

// Global variable for today's measurements
// std::vector<JsonObject> todayMeasurements;

// Function declarations
float readDS18B20Temperatures(int nrDs);
void setCurrentData(float internalTemp, float externalTemp, float humidity);
void saveToDatabase(float internalTemp, float externalTemp, float humidity, const char* description);
bool isToday(const String& dateStr);
// float calculateAverage(String field);
String fetchAPIData();
String getTableData();
String fetchEqeMeashure(); 
void handleRoot();

void setup() {
  Serial.begin(115200);
  dht.begin();
  sensors.begin();
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  
}

// *****************************************************************************************************************************************

void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();

  // Take measurements every second
  if (currentMillis - lastMeasurementTime >= 10000) {
    float tempInTmp = readDS18B20Temperatures(0); 
    if ((tempInTmp < 80) and  (tempInTmp > -80)) internalTemp = tempInTmp;
   // internalTemp = readDS18B20Temperatures(0);      ///DS18B20
    delay(2);
    float tempOuTmp = readDS18B20Temperatures(1) + 1;
     if ((tempOuTmp < 80) and  (tempOuTmp > -80)) externalTemp = tempOuTmp;
  //  externalTemp = readDS18B20Temperatures(1) + 1;  ///DS18B20
    if (tempOuTmp == 5003) { externalTemp = dht.readTemperature(); }

    humidity = dht.readHumidity();
    float temDHT = dht.readTemperature();

    internalTemp = round(internalTemp * 100.0) / 100.0;  // Round to two decimal places
    externalTemp = round(externalTemp * 100.0) / 100.0;  // Round to two decimal places
    humidity = round(humidity * 10.0) / 10.0;            // Round to one decimal place

    if ((abs(internalTemp - lastInternalTemp) >= 0.9 || abs(externalTemp - lastExternalTemp) >= 0.9 || abs(humidity - lastHumidity) >= 1) && ((externalTemp < 100) && (externalTemp > -100))) {
     // setCurrentData(internalTemp, externalTemp, humidity);
      saveToDatabase(internalTemp, externalTemp, humidity, "Pomiar zmiany");
      handleRoot();
      lastDatabaseUpdate = currentMillis;
    }

    lastMeasurementTime = currentMillis;
  }

  // Update database every 5 minutes
  if (currentMillis - lastDatabaseUpdate >= 300000) {  // 300000 ms == 5 minutes
    Serial.println(" !!! ******** 5 MINUT ****************** !!! ");
    //setCurrentData(internalTemp, externalTemp, humidity);
    saveToDatabase(lastInternalTemp, lastExternalTemp, lastHumidity, "Pomiar standardowy");
    handleRoot();
    lastDatabaseUpdate = currentMillis;
  }

  // Print readings to terminal every 30 seconds
  if (currentMillis - lastTerminalUpdate >= 30000) {  // 30000 ms == 30 seconds
    Serial.println("----- 30 SEKUND -----");
    lastTerminalUpdate = currentMillis;
  }
}

// *****************************************************************************************************************************************

void eQPomiarow() {
    // Pobierz dane JSON z API
    String eQjsonData = fetchEqeMeashure();
    
    // Utwórz dokument JSON o odpowiednim rozmiarze
    DynamicJsonDocument doc(1024);
    
    // Deserializacja JSON do dokumentu
    DeserializationError error = deserializeJson(doc, eQjsonData);
    if (error) {
        Serial.print("Błąd deserializacji JSON: ");
        Serial.println(error.c_str());
        return;
    }

    // Przypisz dane z JSON do zmiennych
    eQinternalTemp = doc["averageIndoorTemperature"] | 0.0;
    eQexternalTemp = doc["averageOutdoorTemperature"] | 0.0;
    eQumidity = doc["averageHumidity"] | 0.0;

    // Wyświetl dane na Serial Monitor (opcjonalnie)
    Serial.println("Dane z API:");
    Serial.print("Temperatura wewnętrzna: ");
    Serial.println(eQinternalTemp);
    Serial.print("Temperatura zewnętrzna: ");
    Serial.println(eQexternalTemp);
    Serial.print("Wilgotność: ");
    Serial.println(eQumidity);
}

float readDS18B20Temperatures(int nrDs) {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(nrDs);

  if (tempC == DEVICE_DISCONNECTED_C) {
    // Serial.println("Error: Could not read temperature from DS18B20 sensor.");
    return 5002;
  }
  return tempC;
}

String fetchAPIData() {
  Serial.println("--------------- READ API START -----------------------------------");
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, "http://bs.zzux.com:555/Api/pogoda/latest");
    int httpCode = http.GET();
    ////////////////////////////////////////////////////////
    delay(5);
    Serial.println("HTTP request successful.");
    ////////////////////////////////////////////////////////***
    if (httpCode > 0) {
      Serial.println("--------------- HTTP CODE > 0-----------------------------------");
      String payload = http.getString();
      Serial.print("Peyload lenght: ");
      Serial.println(payload.length());  // Spraw

      Serial.println("--------------- SPRAWDZENIE czy > 200 -----------------------------------");
      Serial.print("HTTP Code: ");
      Serial.println(httpCode);
      delay(10);
      http.end();
      return payload;
    } else {
      Serial.println("WiFi not connected");
      return "";
    }
  }
  return "";
}

String fetchEqeMeashure() {
  Serial.println("--------------- Odczyt z api - Średnie pomiary - Start-----------------------------------");
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, "http://bs.zzux.com:555/Api/pogoda/daily");
    int httpCode = http.GET();
    ////////////////////////////////////////////////////////
    delay(5);
    Serial.println("HTTP request successful - Odczyt średnich.");
    ////////////////////////////////////////////////////////***
    if (httpCode > 0) {
      Serial.println("--------------- HTTP CODE > 0-----------------------------------");
      String payload = http.getString();
      Serial.print("Peyload lenght: ");
      Serial.println(payload.length());  // Spraw

      Serial.println("--------------- SPRAWDZENIE czy > 200 -----------------------------------");
      Serial.print("HTTP Code: ");
      Serial.println(httpCode);
      delay(10);
      http.end();
      return payload;
    } else {
      Serial.println("WiFi not connected");
      return "";
    }
  }
  return "";
}

String getTableData() {
  String tableRows;
  tableRows.reserve(1024);

  String jsonData = fetchAPIData();  // Fetch data from API
                                     // Parse JSON data
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, jsonData);

  //  for (JsonObject measurement : todayMeasurements) {
  for (JsonObject elem : doc.as<JsonArray>()) {
    tableRows += "<tr>";
    //  tableRows += "<td>" + String(measurement["id"].as<int>()) + "</td>";
    tableRows += "<td>" + String(elem["id"].as<int>()) + "</td>";

    // tableRows += "<td>" + String(measurement["temperatura_wew"].as<float>(), 2) + "°C</td>";
    //  tableRows += "<td>" + String(measurement["twmperatura_zew"].as<float>(), 2) + "°C</td>";
    tableRows += "<td>" + String(elem["temperatura_wew"].as<float>()) + "</td>";
    tableRows += "<td>" + String(elem["twmperatura_zew"].as<float>()) + "</td>";
    // Formatowanie wilgotności
    // tableRows += "<td>" + String(measurement["wilgotnosc"].as<float>(), 2) + "%</td>";
    tableRows += "<td>" + String(elem["wilgotnosc"].as<float>(), 2) + "%</td>";

    // Obsługa potencjalnych wartości null
    // tableRows += "<td>" + (measurement["dataPomiaru"].isNull() ? "Brak danych" : measurement["dataPomiaru"].as<String>()) + "</td>";
    //   tableRows += "<td>" + (elem["dataPomiaru"].isNull() ? "Brak danych" : elem["dataPomiaru"].as<String>()) + "</td>";
    String formattedDate = elem["dataPomiaru"].isNull() ? "Brak danych" : formatDate(elem["dataPomiaru"].as<String>());
    tableRows += "<td>" + formattedDate + "</td>";

    //    tableRows += "<td>" + (measurement["opisPomiaru"].isNull() ? "Brak opisu" : measurement["opisPomiaru"].as<String>()) + "</td>";
    tableRows += "<td>" + (elem["opisPomiaru"].isNull() ? "Brak opisu" : elem["opisPomiaru"].as<String>()) + "</td>";

    // Obsługa wartości bool
    //tableRows += "<td>" + String(measurement["isCompleted"].as<bool>() ? "true" : "false") + "</td>";
    tableRows += "<td>" + String(elem["isCompleted"].as<bool>() ? "true" : "false") + "</td>";

    tableRows += "</tr>";
  }
  return tableRows;
}


