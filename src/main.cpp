#include <ESP8266WiFi.h>
#include "../.pio/libdeps/nodemcuv2/ESPAsyncWebServer-esphome/src/ESPAsyncWebServer.h"
#include "../.pio/libdeps/nodemcuv2/DHT sensor library/DHT.h"
#include "../.pio/libdeps/nodemcuv2/DHT sensor library/DHT_U.h"
#include "../.pio/libdeps/nodemcuv2/Adafruit Unified Sensor/Adafruit_Sensor.h"


// Replace with your network credentials
const char *ssid = "B1Digital";
const char *password = "merhabaB1";

bool relay1 = false;
bool relay2 = false;
bool relay3 = false;
bool relay4 = false;
float humidity = 0;
float temperature = 0;


const int pin1 = 2;
const int pin2 = 0;
const int pin3 = 4;
const int pin4 = 5;
#define DHTPIN 14
#define DHTTYPE    DHT11     // DHT 11

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0f8b8d;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>ESP WebSocket Server</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>Output - GPIO 2</h2>
      <p class="state">state: <span id="state">%STATE%</span></p>
      <p><button id="button" class="button">Toggle</button></p>
    </div>
  </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    if (event.data == "1"){
      state = "ON";
    }
    else{
      state = "OFF";
    }
    document.getElementById('state').innerHTML = state;
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }
</script>
</body>
</html>
)rawliteral";

String sendState(bool relay, String relayName) {
    String returnValue;
    returnValue = "{";
    returnValue += "\"" + relayName + "\":" + String(relay);
    returnValue += "}";
    return returnValue;

}

// Get the state of the GPIO pins
String getState() {
    String returnValue;
    returnValue = "{";
    returnValue += "\"relay1\":" + String(!relay1) + ",";
    returnValue += "\"relay2\":" + String(!relay2) + ",";
    returnValue += "\"relay3\":" + String(!relay3) + ",";
    returnValue += "\"relay4\":" + String(!relay4) + ",";
    returnValue += "\"humidity\":" + String(humidity) + ",";
    returnValue += "\"temperature\":" + String(temperature);
    returnValue += "}";
    return returnValue;
}

// sendTemperatureAndHumidity : send the temperature and humidity to the client
String sendTemperatureAndHumidity(float Humidity, float Temperature) {
    String returnValue;
    returnValue = "{";
    returnValue += "\"humidity\":" + String(Humidity);
    returnValue += ",";
    returnValue += "\"temperature\":" + String(Temperature);
    returnValue += ",";
    returnValue += "\"time\":" + String(millis());
    returnValue += "}";
    return returnValue;
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *) arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        if (strcmp((char *) data, "toggleCh1") == 0) {
            relay1 = !relay1;
            ws.textAll(sendState(!relay1, "relay1"));
        }
        if (strcmp((char *) data, "toggleCh2") == 0) {
            relay2 = !relay2;
            ws.textAll(sendState(!relay2, "relay2"));
        }
        if (strcmp((char *) data, "toggleCh3") == 0) {
            relay3 = !relay3;
            ws.textAll(sendState(!relay3, "relay3"));
        }
        if (strcmp((char *) data, "toggleCh4") == 0) {
            relay4 = !relay4;
            ws.textAll(sendState(!relay4, "relay4"));
        }
        if (strcmp((char *) data, "getState") == 0) {
            ws.textAll(getState());
        }
    }
}


void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(),
                          client->remoteIP().toString().c_str());

            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

String processor(const String &var) {
    Serial.println(var);
    if (var == "STATE") {
        if (relay1) {
            return "ON";
        } else {
            return "OFF";
        }
    }
    return String();
}

void sendHumidityAndTemperature() {
    delay(delayMS);
    sensors_event_t temperatureEvent;
    sensors_event_t humidityEvent;
    dht.temperature().getEvent(&temperatureEvent);
    dht.humidity().getEvent(&humidityEvent);

    if (isnan(temperatureEvent.temperature) && isnan(humidityEvent.relative_humidity)) {
        Serial.println(F("Error reading temperature and humidity!"));
    } else {
        Serial.print(F("Temperature: "));
        Serial.print(temperatureEvent.temperature);
        Serial.println(F("°C"));
        Serial.print(F("Humidity: "));
        Serial.print(humidityEvent.relative_humidity);
        Serial.println(F("%"));
        humidity = humidityEvent.relative_humidity;
        temperature = temperatureEvent.temperature;
        ws.textAll(sendTemperatureAndHumidity(humidityEvent.relative_humidity, temperatureEvent.temperature));
    }
}

void setup() {
    // Serial port for debugging purposes
    Serial.begin(115200);

    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    pinMode(pin3, OUTPUT);
    pinMode(pin4, OUTPUT);
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    digitalWrite(pin3, LOW);
    digitalWrite(pin4, LOW);




    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }



    // Print ESP Local IP Address
    Serial.println(WiFi.localIP());

    initWebSocket();

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html, processor);
    });

    // Start server
    server.begin();
    dht.begin();
    Serial.println(F("DHTxx Unified Sensor Example"));
    // Print temperature sensor details.
    sensor_t sensor;
    dht.temperature().getSensor(&sensor);
    Serial.println(F("------------------------------------"));
    Serial.println(F("Temperature Sensor"));
    Serial.print(F("Sensor Type: "));
    Serial.println(sensor.name);
    Serial.print(F("Driver Ver:  "));
    Serial.println(sensor.version);
    Serial.print(F("Unique ID:   "));
    Serial.println(sensor.sensor_id);
    Serial.print(F("Max Value:   "));
    Serial.print(sensor.max_value);
    Serial.println(F("°C"));
    Serial.print(F("Min Value:   "));
    Serial.print(sensor.min_value);
    Serial.println(F("°C"));
    Serial.print(F("Resolution:  "));
    Serial.print(sensor.resolution);
    Serial.println(F("°C"));
    Serial.println(F("------------------------------------"));
    // Print humidity sensor details.
    dht.humidity().getSensor(&sensor);
    Serial.println(F("Humidity Sensor"));
    Serial.print(F("Sensor Type: "));
    Serial.println(sensor.name);
    Serial.print(F("Driver Ver:  "));
    Serial.println(sensor.version);
    Serial.print(F("Unique ID:   "));
    Serial.println(sensor.sensor_id);
    Serial.print(F("Max Value:   "));
    Serial.print(sensor.max_value);
    Serial.println(F("%"));
    Serial.print(F("Min Value:   "));
    Serial.print(sensor.min_value);
    Serial.println(F("%"));
    Serial.print(F("Resolution:  "));
    Serial.print(sensor.resolution);
    Serial.println(F("%"));
    Serial.println(F("------------------------------------"));
    // Set delay between sensor readings based on sensor details.
    delayMS = sensor.min_delay / 1000;
}

void loop() {
    sendHumidityAndTemperature();
    ws.cleanupClients();
    digitalWrite(pin1, relay1);
    digitalWrite(pin2, relay2);
    digitalWrite(pin3, relay3);
    digitalWrite(pin4, relay4);
}