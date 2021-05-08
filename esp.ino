#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7
#include <ESP8266WebServer.h>
//-------- Customise these values -----------
// const char* ssid = "TOPNET_19E8";
// const char* password = "u9pHINUEal";
 String ssid = "ddd";
 String password = "ddddddd";

//-------- Customise the above values --------

IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);


char serverr[] = "192.168.1.15";
char clientId[] = "safta001";

const char eventTopic[] = "iot-2/saftamed";
const char cmdTopic[] = "iot-2/saftamed";

bool pwd = true;

WiFiClient wifiClient;
void callback(char* topic, byte* payload, unsigned int payloadLength) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < payloadLength; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
   DynamicJsonDocument doc(512);
deserializeJson(doc, payload);

String action = doc["action"];
 int pin = doc["pin"];
  int value = doc["value"];

  if(action=="D"){
    pinMode(pin,OUTPUT);
    digitalWrite(pin,!value);
  }

}
PubSubClient client(serverr, 1883, callback, wifiClient);
ESP8266WebServer server(80);
int publishInterval = 5000; // 5 seconds//Send adc every 5sc
long lastPublishMillis;

void handleRoot() {
  String response = "<link rel='stylesheet' href='http://192.168.1.15:5000/css/ss.css'><form class='form' action=\"update\"><select name=\"ssid\">";
  byte numSsid = WiFi.scanNetworks();

  for (int thisNet = 0; thisNet<numSsid; thisNet++) {
    response +="<option>"+WiFi.SSID(thisNet) + "</option>";
  } 
   response +="</select><input type='password' name='password'><input type='submit' value='send'></form>";
  server.send(200, "text/html", response);

}
void handleUpdate(){

  ssid = server.arg("ssid");
  password =server.arg("password");
  server.send(200, "text/html", "ok");
     Serial.println(ssid);
   Serial.println(password);
   delay(3000);
  pwd = false;  


}
void serveForPwd(){
    WiFi.mode(WIFI_AP_STA);
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");    
  Serial.print("Configuring access point...");
  WiFi.softAP("safta", "ap_password");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);  
  server.on("/update", handleUpdate);  
  server.begin();
  Serial.println("HTTP server started");
}

void setup() {
  Serial.begin(9600); Serial.println();

  
  serveForPwd();
  while(pwd){
      server.handleClient();
  }

WiFi.softAPdisconnect (true);
  wifiConnect();
  mqttConnect();
}

void loop() {
  /*if (millis() - lastPublishMillis > publishInterval) {
    publishData();
    lastPublishMillis = millis();
    //delay(100);
    //digitalWrite(2,LOW);
  }*/

  if (!client.loop()) {
    mqttConnect();

  }
    server.handleClient();
}

void wifiConnect() {
    if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("nWiFi connected, IP address: "); Serial.println(WiFi.localIP());

}

void mqttConnect() {
  if (!!!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); Serial.println(serverr);
    while (!!!client.connect(clientId)) {
      Serial.print(".");
        server.handleClient();
      delay(500);
    }
    if (client.subscribe(cmdTopic)) {
      Serial.println("subscribe to responses OK");
    } else {
      Serial.println("subscribe to responses FAILED");
    }
    Serial.println();
  }
}


void publishData() {
  // read the input on analog pin 0:
  int sensorValue = 100;

  String payload = "{\"d\":{\"adc\":";
  payload += String(sensorValue, DEC);
  payload += "}}";

  Serial.print("Sending payload: "); Serial.println(payload);

  if (client.publish(eventTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
    //digitalWrite(2,HIGH);
  } else {
    Serial.println("Publish FAILED");
  }
}

