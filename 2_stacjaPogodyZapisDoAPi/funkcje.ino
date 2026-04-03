String formatDate(const String& dateStr) {
  // Sprawdzamy, czy data jest w formacie ISO 8601
  if (dateStr.length() >= 19) {
    // Wyciągamy poszczególne części daty i czasu
    String datePart = dateStr.substring(0, 10);   // YYYY-MM-DD
    String timePart = dateStr.substring(11, 16);  // HH:MM

    // Zmiana formatu na DD-MM-YYYY HH:MM
    String day = datePart.substring(8, 10);
    String month = datePart.substring(5, 7);
    String year = datePart.substring(0, 4);

    return day + "-" + month + "-" + year + " " + timePart;
  }
  return "Niepoprawny format daty";
}

String getCurrentISOTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return String("1970-01-01T00:00:00.000Z");
  }
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S.000Z", &timeinfo);
  return String(buffer);
}

void setCurrentData(float internalTemp, float externalTemp, float humidity) {
  lastInternalTemp = internalTemp;
  lastExternalTemp = externalTemp;
  lastHumidity = humidity;
}


void handleData() {
  String json = "{";
  json += "\"temp_zew\": " + String(lastExternalTemp, 1) + ",";
  json += "\"temp_wew\": " + String(lastInternalTemp, 1) + ",";
  json += "\"humidity\": " + String(lastHumidity, 1) + ",";
  json += "\"eq_temp_wew\": " + String(eQinternalTemp, 1) + ",";
  json += "\"eq_temp_zew\": " + String(eQexternalTemp, 1) + ",";
  json += "\"eq_humidity\": " + String(eQumidity, 1);
  json += "}";

  server.send(200, "application/json", json);
}