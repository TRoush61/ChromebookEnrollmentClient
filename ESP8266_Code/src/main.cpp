#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <EEPROM.h>

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

void tick()
{
  //toggle state
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

void startConfigPortal()
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
  //wifiManager.resetSettings();

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
    Serial.println("failed to connect and hit timeout");
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
  Serial.println("connected...yeey :)");
  ticker.detach();

  Serial.println(WiFi.localIP());
  Serial.print("Entered enrollment server IP: ");
  Serial.println(enrollmentServerIp);
  //keep LED on
  digitalWrite(LED_BUILTIN, LOW);
  EEPROM.commit();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  EEPROM.begin(512);

  clientId[0] = '\0';

  int value = EEPROM.read(0);
  if (value == 1)
  {
    Serial.println("EEPROM has data we can read from");
    eepromHasData = true;
  }
  else
  {
    Serial.println("EEPROM has no data");
  }
  

  startConfigPortal();

  pinMode(TRIGGER_PIN, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    startConfigPortal();
  }
}