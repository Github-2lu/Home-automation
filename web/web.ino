#include<WiFi.h>
#include<ESPmDNS.h>
#include<ESPAsyncWebServer.h>
#include<WebSocketsServer.h>
#include<ArduinoJson.h>

AsyncWebServer server(80);
WebSocketsServer websockets(81);

#define LED1 2
#define LED2 5

char *ssid = "h409";
char *password = "";

char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <title>Home Automation</title>
    </head>
    <script>
        var connection = new WebSocket('ws://'+location.hostname+':81/');

        LED1_status = 0;
        LED2_status=0;
        function LED1_toggle(){
            if(LED1_status==0){
                LED1_status=1;
                document.getElementById("LED1").innerHTML="OFF";
                console.log("LED1 is ON");
                send_data();
            }
            else{
                LED1_status=0;
                document.getElementById("LED1").innerHTML="ON";
                console.log("LED1 is OFF");
                send_data();
            }
        }
        function LED2_toggle(){
            if(LED2_status==0){
                LED2_status=1;
                document.getElementById("LED2").innerHTML="OFF";
                console.log("LED2 is ON");
                send_data();
            }
            else{
                LED2_status=0;
                document.getElementById("LED2").innerHTML="ON";
                console.log("LED2 is OFF");
                send_data();
            }
        }
        function send_data(){
            var full_data = '{"LED1":'+LED1_status+',"LED2":'+LED2_status+'}';
            connection.send(full_data);
        }
    </script>
    <body>
        <center>
            <h1>Home Automation</h1>
            <h3>LED1</h3>
            <button id="LED1", onclick="LED1_toggle()">ON</button>
            <h3>LED2</h3>
            <button id="LED2", onclick="LED2_toggle()">ON</button>
        </center>
    </body>
</html>
)=====";

void found(AsyncWebServerRequest *request){
  request->send_P(200, "text/html", webpage);
}

void notFound(AsyncWebServerRequest *request){
  request->send(404, "text/plain", "Page Not Found.");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length){
  switch(type){
    case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected.\n", num);
    break;
    case WStype_CONNECTED:{
      IPAddress ip = websockets.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      websockets.sendTXT(num, "Connected from server");
    }
    break;
    case WStype_TEXT:
    Serial.printf("[%u] get Text: %s\n", num, payload);
    String message= String((char*)(payload));
    Serial.println(message);

    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, message);

    if(error){
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }
    int LED1_status = doc["LED1"];
    int LED2_status = doc["LED2"];
    digitalWrite(LED1, LED1_status);
    digitalWrite(LED2, LED2_status);
    break;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  WiFi.softAP(ssid, password);
  Serial.print("ssid = ");
  Serial.println(ssid);
  Serial.print("Password = ");
  Serial.println(password);
  Serial.println(WiFi.softAPIP());

  if(MDNS.begin("home")){
    Serial.println("Go to home.local");
  }

  server.on("/", found);
/*
  server.on("/led1/on", [](AsyncWebServerRequest *request){
    digitalWrite(LED1, HIGH);
    request->send_P(200, "text/html", webpage);
  });

  server.on("/led1/off", [](AsyncWebServerRequest *request){
    digitalWrite(LED1, LOW);
    request->send_P(200, "text/html", webpage);
  });
*/
  server.onNotFound(notFound);

  server.begin();
  websockets.begin();
  websockets.onEvent(webSocketEvent);
}

void loop() {
  // put your main code here, to run repeatedly:
  websockets.loop();
}
