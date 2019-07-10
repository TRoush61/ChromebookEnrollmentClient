#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <EEPROM.h>
#include <PubSubClient.h>

//Set to false when deploying with an Arduino intended to receive the messages
//That way we're not sending garbage to the arduino it doesn't need
#define DEBUG false

//for LED status
#include <Ticker.h>
Ticker ticker;

// select which pin will trigger the configuration portal when set to LOW
// ESP-01 users please note: the only pins available (0 and 2), are shared 
// with the bootloader, so always set them HIGH at power-up
#define TRIGGER_PIN 0

char clientId[6];
int clientIdStoreAddress = 1;
IPAddress enrollmentServerIp;
int enrollmentServerIpStoreAddress = 8;

bool eepromHasData = false;

//Mqtt variables
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void writeDebugMsg(const char *msg)
{
  if (DEBUG)
  {
    Serial.println(msg);
  }
}

void writeDebugMsg(String msg)
{
  if (DEBUG)
  {
    Serial.println(msg);
  }
}

void writeDebugMsg(IPAddress msg)
{
  if (DEBUG)
  {
    Serial.println(msg);
  }
}

void tick()
{
  //toggle state
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  writeDebugMsg("Entered config mode");
  writeDebugMsg(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  writeDebugMsg(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void startConfigPortal(bool resetConfig = false)
{
  if (!eepromHasData)
  {
    //Get a random client Id if we don't already have one
    int rand = random(10000,99999);
    itoa(rand, clientId, 10);
    EEPROM.write(0, 1);
    EEPROM.put(clientIdStoreAddress, clientId);
  }
  else
  {
    EEPROM.get(clientIdStoreAddress, clientId);
  }
  

  //set led pin as output
  pinMode(LED_BUILTIN, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  char ip[16];
  ip[0] = '\0';
  WiFiManagerParameter custom_enrollment_server("server", "Enrollment Server IP", ip, 20);
  wifiManager.addParameter( &custom_enrollment_server);
  //reset settings - for testing
  if (resetConfig)
  {
    wifiManager.resetSettings();
  }

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
  

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  char apSsid[16];
  strcpy(apSsid, "Enroller-");
  strcat(apSsid, clientId);
  if (!wifiManager.autoConnect(apSsid)) {
    writeDebugMsg("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }
  if (custom_enrollment_server.getValue()[0] != '\0')
  {

    strncpy(ip, custom_enrollment_server.getValue(), 20);
    enrollmentServerIp.fromString(ip);
    EEPROM.put(enrollmentServerIpStoreAddress, enrollmentServerIp);
    EEPROM.write(0, 1);
  }
  else if (eepromHasData)
  {
    EEPROM.get(enrollmentServerIpStoreAddress, enrollmentServerIp);
  }

  //if you get here you have connected to the WiFi
  writeDebugMsg("connected...yeey :)");
  ticker.detach();

  writeDebugMsg(WiFi.localIP());
  writeDebugMsg("Entered enrollment server IP: ");
  writeDebugMsg(enrollmentServerIp);
  //keep LED on
  digitalWrite(LED_BUILTIN, LOW);
  EEPROM.commit();
}

void callback(char* topic, byte* payload, unsigned int length) {
  writeDebugMsg("Message arrived [");
  writeDebugMsg(topic);
  writeDebugMsg("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  writeDebugMsg("");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    writeDebugMsg("Attempting MQTT connection...");
    // Create a random client ID
    // Attempt to connect
    if (client.connect(clientId)) {
      writeDebugMsg("connected");
      // Once connected, publish an announcement...
      client.publish("newClient", clientId);
      // ... and resubscribe
      char commandTopic[6];
      strcpy(commandTopic, clientId);
      strcat(commandTopic, "-commands");
      client.subscribe(commandTopic);
    } else {
      writeDebugMsg("failed, rc=");
      writeDebugMsg(client.state());
      writeDebugMsg(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  EEPROM.begin(512);

  clientId[0] = '\0';

  int value = EEPROM.read(0);
  if (value == 1)
  {
    writeDebugMsg("EEPROM has data we can read from");
    eepromHasData = true;
  }
  else
  {
    writeDebugMsg("EEPROM has no data");
  }
  
  startConfigPortal();

  pinMode(TRIGGER_PIN, INPUT);

  //Init Mqtt
  client.setServer(enrollmentServerIp, 1883);
  client.setCallback(callback);
}

void loop() {
  //Echo whatever we read on serial up to the server
  if (Serial.available() > 0)
  {
    String input = Serial.readStringUntil(';');
    input = input + ';';
    client.publish(clientId, (char*) input.c_str());
  }

  // put your main code here, to run repeatedly:
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    startConfigPortal();
  }

  //Mqtt
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}