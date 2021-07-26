#include <SPI.h>
#include <ArduinoJson.h>
#include <UIPEthernet.h>
#include <PubSubClient.h> 
#include <Wire.h>
#include <MCP3424.h>
#include <DHT.h>

MCP3424 MCP(0x68); // Declaration of MCP3424

#define DHTTYPE DHT22
#define DHTPIN 27
DHT dht(DHTPIN, DHTTYPE);

#define adcpin 36
#define adcpin2 39
#define GPI1 21
#define lightPin 2

long lastMsg = 0;
char msg[50];
int value = 0;

float t = 0;
float t2 = 0;
float h = 0;
float c = 0;
float v = 0;
float p = 0;

String MQTT_Server = "esp32/randTH";

// JSON
char msgJson[80];

// Update these with values suitable for your network.
byte mac[] = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 157);
IPAddress server(192, 168, 0, 92);  // WiFi
// IPAddress server(192, 168, 0, 171); // LAN


// Callback function header
// void callback(char* topic, byte* payload, unsigned int length);

UIPEthernetClass ether1;
EthernetClient ethClient;
// PubSubClient client(server, 1883, callback, ethClient);
PubSubClient client(server, 1883, ethClient);

// Callback function
// void callback(char* topic, byte* payload, unsigned int length) {
//    
//  //turn the LED ON if the payload is '1' and publish to the MQTT server a confirmation message
//  if(payload[0] == '1'){
//    digitalWrite(lightPin, HIGH);
//    client.publish("outTopic", "Light On"); }
//    
//  //turn the LED OFF if the payload is '0' and publish to the MQTT server a confirmation message
//  if (payload[0] == '0'){
//    digitalWrite(lightPin, LOW);
//    client.publish("outTopic", "Light Off");
//    }
//} // void callback

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("esp32/randTH")) {
      Serial.println("connected");
      digitalWrite(lightPin, LOW);
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      digitalWrite(lightPin, HIGH);
     Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

float makeJson(float temp, float humi, float curr, float vibe, float phot) 
{ // function data Json data format
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["Temp"] = temp;
  root["Humi"] = humi;
  root["curr"] = curr;
  root["vibe"] = vibe;
  root["phot"] = phot;

  // Serial.print("Json data : ");
  root.printTo(Serial);
  Serial.println();
  root.printTo(msgJson); //json data -> char msgJson[] input msgJson
}

void setup() {
  Serial.begin(115200);
  delay(3000);
  
  pinMode(lightPin, OUTPUT);
//  digitalWrite(lightPin, LOW);
  
  // PT100 Setup - MCP3424
  Wire.begin(32, 33);
  MCP.Configuration(2, 18, 0, 1);
  
  // DHT Setup - PIN 27
  dht.begin();
  
  // TLP181 - PIN 21
  pinMode(GPI1, INPUT);

  // Ethernet Setup -Ch1
  Ethernet.init(5);
  Ethernet.begin(mac, ip);
}

void loop() {
//  delay(3000);
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;
    MCP.NewConversion(); // New conversion is initiated
    // Temp - PT100
    t = roundf((MCP.Measure() / pow(10, 3) - 100) / 0.39093 * 100) / 100; // 소수점 둘째
    // Temp2 - DHT22
    // t2 = dht.readTemperature();  
    // Humd - DHT22
    h = dht.readHumidity();
    // Current - ADC1(36)
    c = map(analogRead(adcpin)/4095 * 3.9, 0.6, 3.0, 4, 20);
    // Vibration - ADC2(39)
    v = map(analogRead(adcpin2)/4095 * 3.9, 0.6, 3.0, 4, 20);
    // Photo - TLP181
    p = digitalRead(GPI1);

  // t = random(21, 30);
    h = random(0, 10);
  // c = random(0, 10);
  // v = random(0, 10);
  // p = random(0, 10);
  
    makeJson(t, h, c, v, p);
    client.publish("esp32/randTH", msgJson);
  }
}
