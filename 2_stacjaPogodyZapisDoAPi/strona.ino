void handleRoot() {
  eQPomiarow();
  setCurrentData(internalTemp, externalTemp, humidity);

  String html = "<!DOCTYPE html><html><head><title>Pomiary Pogody</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                "<style>"
                "* { margin: 0; padding: 0; box-sizing: border-box; }"
                "body { font-family: 'Arial', sans-serif; line-height: 1.6; margin: 0; padding: 20px; background-color: #f4f4f9; color: #333; min-height: 100vh; }"
                "h1 { font-size: 2.5em; margin-bottom: 15px; text-align: center; color: #007BFF; border-bottom: 1px solid #ddd; }"
                "h2 { font-size: 3em; text-align: center; color: #001B4F; border-bottom: 1px solid #ddd; }"
                "label { display: block; margin-bottom: 10px; font-weight: bold; }"
                "table { width: 100%; border-collapse: collapse; margin-top: 8px; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1); border-radius: 8px; overflow: hidden; }"
                "th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }"
                "th { background-color: #007BFF; color: white; }"
                "tr:hover { background-color: #f1f1f1; }"
                "tr:nth-child(even) { background-color: #f9f9f9; }"
                "tr:nth-child(odd) { background-color: #ffffff; }"
                ".form-container { display:block; align-items: center; margin-bottom: 15px; background-color: white; border: 1px solid #ccc; padding: 10px; padding-left: 25px; padding-right: 25px; padding-bottom: 25px; border-radius: 8px; box-shadow: 0 4px 20px rgba(0, 0, 0, 0.1); }"
                ".container { max-width: 1000px; margin: 20px auto; padding: 20px; }"
                ".temperature-display { width: 55%; text-align: center; font-size: 10rem; font-weight: bold; color: #d9534f; padding-left: 25px; padding-top: 5px; display: flex; justify-content: center; align-items: center; height: 100%; }"
                ".details-display { width: 40%; font-size: 1rem; color: #555; display: flex; flex-direction: column; justify-content: center; align-items: center; text-align: center; height: 100%; }"
                ".details-display p { margin: 10px 0; line-height: 1.1; }"
                ".form-content { display: flex; justify-content: space-between; align-items: flex-start; }"
                ".refresh-indicator { font-size: 0.8em; color: #888; text-align: right; margin-top: 6px; }"
                "@media (max-width: 768px) { .form-container { width: 90%; padding: 15px; }"
                ".form-content { flex-direction: column; align-items: center; }"
                ".temperature-display { width: 100%; font-size: 2.5rem; margin-bottom: 15px; }"
                ".details { width: 100%; font-size: 0.9rem; text-align: center; } }"
                "</style></head><body>";

  html += "<div class='container'>";

  // Sekcja aktualnych pomiarów
  html += "<div id='buttonForm' class='form-container'>";
  html += "<div class='current'><h1>Aktualne Pomiary</h1></div>";
  html += "<div class='form-content'>";

  html += "<div class='temperature-display'>";
  html += "<p id='ext-temp'>" + String((float)lastExternalTemp, 1) + "°</p>";
  html += "</div>";

  html += "<div class='details-display'>";
  html += "<p>Temperatura wewnętrzna: <h2 id='int-temp'>" + String(lastInternalTemp) + "°C</h2></p>";
  html += "<p>Wilgotność: <h2 id='humidity'>" + String(lastHumidity) + "%</h2></p>";
  html += "</div>";

  html += "</div>";
  html += "<p class='refresh-indicator'>Odświeżono: <span id='last-update'>teraz</span> &nbsp;|&nbsp; następne za <span id='countdown'>60</span>s</p>";
  html += "</div>";

  // Sekcja średnich pomiarów
  html += "<div id='buttonForm' class='form-container'>";
  html += "<h1>Średnie z dzisiejszych pomiarów</h1>";
  html += "<p>Temperatura wewnętrzna: <h2 id='eq-int-temp'>" + String(eQinternalTemp) + "°C</h2></p>";
  html += "<p>Temperatura zewnętrzna: <h2 id='eq-ext-temp'>" + String(eQexternalTemp) + "°C</h2></p>";
  html += "<p>Wilgotność: <h2 id='eq-humidity'>" + String(eQumidity) + "%</h2></p>";
  html += "</div>";

  // Tabela
  html += "<h1>Pomiary z API</h1>"
          "<table><tr>"
          "<th>ID</th><th>Temperatura wewnętrzna</th><th>Temperatura zewnętrzna</th>"
          "<th>Wilgotność</th><th>Data pomiaru</th><th>Opis pomiaru</th><th>Is complite</th>"
          "</tr>";
  html += getTableData();
  html += "</table>";

  html += "</div>";

  // JavaScript – automatyczne odświeżanie danych co 60 sekund przez /data
  html += "<script>"
          "var countdown = 60;"
          "function updateData() {"
          "  fetch('/data')"
          "    .then(function(r){ return r.json(); })"
          "    .then(function(d){"
          "      document.getElementById('ext-temp').textContent = d.temp_zew.toFixed(1) + '°';"
          "      document.getElementById('int-temp').textContent = d.temp_wew.toFixed(2) + '°C';"
          "      document.getElementById('humidity').textContent = d.humidity.toFixed(2) + '%';"
          "      document.getElementById('eq-int-temp').textContent = d.eq_temp_wew.toFixed(2) + '°C';"
          "      document.getElementById('eq-ext-temp').textContent = d.eq_temp_zew.toFixed(2) + '°C';"
          "      document.getElementById('eq-humidity').textContent = d.eq_humidity.toFixed(2) + '%';"
          "      var now = new Date();"
          "      document.getElementById('last-update').textContent ="
          "        now.getHours().toString().padStart(2,'0') + ':' +"
          "        now.getMinutes().toString().padStart(2,'0') + ':' +"
          "        now.getSeconds().toString().padStart(2,'0');"
          "      countdown = 60;"
          "    })"
          "    .catch(function(e){ console.error('Błąd pobierania danych:', e); });"
          "}"
          "setInterval(updateData, 60000);"
          "setInterval(function(){"
          "  countdown--;"
          "  if(countdown < 0) countdown = 60;"
          "  document.getElementById('countdown').textContent = countdown;"
          "}, 1000);"
          "</script>"
          "</body></html>";

  server.send(200, "text/html", html);
}
