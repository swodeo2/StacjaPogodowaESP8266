void saveToDatabase(float internalTemp, float externalTemp, float humidity, const char* description) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, "http://bs.zzux.com:555/api/Pogoda");
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["id"] = 0;
    doc["temperatura_wew"] = internalTemp;
    doc["twmperatura_zew"] = externalTemp;
    doc["dataPomiaru"] = getCurrentISOTime();
    doc["wilgotnosc"] = humidity;
    doc["opisPomiaru"] = description;
    doc["isCompleted"] = true;

    String jsonString;
    serializeJson(doc, jsonString);

    int httpResponseCode = http.POST(jsonString);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("Error in WiFi connection");
  }
}