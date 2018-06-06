/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h" // DHT22 and other sensors in the DHT range
#include <Servo.h> 
// Update these with values suitable for your network.
Servo myservo;
const char* ssid = "Thales";
const char* password = "lele1234";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
const int lamp = 14;

/************************* DHT22 Setup *************************************/
#define DHTPIN 5  // GPIO 15 (D1) ***** what digital pin we're connected to
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);



void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String teste;
  String messageTemp;
  int pos;

  teste = String(topic);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    messageTemp+= (char)payload[i];
  }
 

  if(teste == "room/lamp"){
      Serial.print("Changing Room lamp to ");
      if(messageTemp == "on"){
        digitalWrite(lamp, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){
        digitalWrite(lamp, LOW);
        Serial.print("Off");
      }
  }
   if(teste == "room/lampAll"){
      Serial.print("Changing all lamp to ");
      if(messageTemp == "on"){
        digitalWrite(lamp, HIGH);
        digitalWrite(13, HIGH);
        digitalWrite(12, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){
        digitalWrite(lamp, LOW);
        digitalWrite(13, LOW);
        digitalWrite(12, LOW);
        Serial.print("Off");
      }
  }
  if(teste == "taiko/wifiServo"){
     myservo.write(messageTemp.toInt());
    }
  
  Serial.println();
    
  }
 

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
  //    client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("room/lamp");
      client.subscribe("room/lampAll");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(lamp, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(13, OUTPUT); 
  pinMode(12, OUTPUT); 
  /************************* SERVO ******************************************/
  myservo.attach(15);
  
  /************************* DHT22 Setup *************************************/
  Serial.println("DHTxx test!"); // Prints out to the serial monitor console
  dht.begin();// Starts the DHT sensor

  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 75, "hello world #%ld", value);
   // Serial.print("Publish message: ");
  //  Serial.println(msg);
    client.publish("outTopic", msg);

/************************* DHT22 *************************************/
    // start working...
  Serial.println("=================================");
  Serial.println("Sample DHT22...");

  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(temperature, humidity, false);
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print("Sample OK: ");
  Serial.print((int)temperature); Serial.print(" *C, ");
  Serial.print((int)humidity); Serial.println(" %");
/************************* Publish String MQTT ************************/
  client.publish("wifiTemp", String(temperature).c_str());
  client.publish("wifiHumidity", String(humidity).c_str());

  // DHT22 sampling rate is 1HZ.
  delay(1000);

  }
}