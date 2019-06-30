/*
  Copyright (c) 2018, Amplified IT
  See the full description at http://labs.amplifiedit.com/centipede

  Support forums are available at https://plus.google.com/communities/100599537603662785064

  Published under an MIT License https://opensource.org/licenses/MIT

*/

#include <Keyboard.h>
//#include <Mouse.h>
#include <Adafruit_NeoPixel.h>

/* NeoPixel variables */

#define PIN 4 
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
unsigned int rgbColour[3];

/* Modify the following definitions to fit your wireless and enrollment credentials. */

#define device_version 73 // Change to the ChromeOS version you expect to use with Centipede; Changes have been reported in the following ranges 58-68, 69, 70

#define wifi_name "" // Define SSID for your wireless connection.
#define wifi_pass "" // Define the password for your wireless connection.
#define wifi_security 2 //[0,1,2] Set to 0 for open, 1 for WEP, 2 for WPA
#define username "" // Define the user name for enrolling the device.
#define password "" // The password for the enrollment GAFE account.

// Use these options to deter mine if you want to disable analytics, skip asset ID, or if you need to slow down the Centipede

#define sendUsageToGoogle 1 //[0,1] Set to 0 if you want to un-check the box to send usage analytics to Google
#define skipAssetIdScreen 0 //[0,1] Set to 0 if you want Centipede to stop at the asset ID and location screen
#define selectedOnBoot 0 //[0,1] Set to 1 if the language box is selected when you first power on the device

/* These are advanced options. The defaults should be fine, but feel free to tweak values below. */

#define setWiFi true //[true,false] Set to false for devices that already have WiFi setup and have accepted Terms of Service (ToS) 

// Use this area for advanced network setup options
#define advancedNetworkSetup false //[true,false] Set to true for EAP configuration, and fill out the definitions below
#define eapMethod "LEAP" // Valid options are "LEAP" "PEAP" "EAP-TLS" or "EAP-TTLS" - Note that they require the quotes to work properly
#define phaseTwoAuthentication 2 //[0,1,2,3,4,5,6] Set to 0 for automatic, 1 for EAP-MD5, 2 for MSCHAP(v2 pre v69; v1 V69+, 3 for MSCHAPv2, 4 for PAP, 5 for CHAP, 6 for GTC; v69+)
#define serverCaCertificateCheck 0 //[0,1] 0 is default, 1 is "Do not check"
#define subjectMatch "" // Fill in subject match here if needed for advanced wireless
#define identity "identity" // Fill in identity here if needed for advanced wireless
#define anonymousIdentity "" // Fill in anonymous identity here for advanced wireless
#define saveIdentityAndPassword 0 //[0,1] Set to 1 to save identity and password. NOT RECOMMENDED
#define sso 0 //[0,1] Set to 1 if using Single Sign On - NOTE: May need additional configuration in Advanced Network Setup around line 182.

// Use this section for additional non-traditional methods
#define longer_enrollment_time 45 // Set to additional seconds to wait for Device Configuration and Enrollment
#define update_via_guest 0 //[0,1] Set to 1 to go into Guest Mode and navigate to chrome://chrome and Check for Updates
#define powerwash 0 //[0,1] Powerwash the device BEFORE enrollment is completed - NOTE: Will not work on ENROLLED devices. Used for Setting up Centipede.
#define sign_in 0 //[0,1] Set to 1 to sign-in to the device after enrollment - NOTE: Will not sign-in if update_via_guest or powerwash is set to true;
#define remove_enrollment_wifi 0 //[0,1] Set to 1 to remove the enrollment wifi network. *sign_in also must be true* - NOTE: Only set to true when Chrome Device Network has been pulled down
#define enroll_device_cert 0 //[0,1] Set to 1 if enrolling device wide certificate *sign_in also must be true* - NOTE: Works best if user _*only*_ has Certificate Enrollment extension force installed

#define slowMode 0 // [0,1] Set to 1 if Centipede appears to be moving too quickly at any screen. This will slow down the entire process
#define update_wait_time 90 // Set to seconds to wait for Update with update_via_guest before exiting guest mode.  Update will continue while device is online.

/* Do not modify anything below this line unless you're confident that you understand how to program Arduino or C */

// Version Defination
#define VERSION_69 (device_version >= 69)
#define VERSION_70 (device_version >= 70)

// Special characters definite
#define KEY_LEFT_CTRL   0x80
#define KEY_LEFT_SHIFT  0x81
#define KEY_LEFT_ALT    0x82
#define KEY_RIGHT_CTRL  0x84
#define KEY_RIGHT_SHIFT 0x85
#define KEY_RIGHT_ALT   0x86
#define KEY_UP_ARROW    0xDA
#define KEY_DOWN_ARROW  0xD9
#define KEY_LEFT_ARROW  0xD8
#define KEY_RIGHT_ARROW 0xD7
#define KEY_BACKSPACE   0xB2
#define KEY_TAB         0xB3
#define KEY_ENTER       0xB0
#define KEY_ESC         0xB1
#define KEY_CAPS_LOCK   0xC1

//TODO:
#define in_developer_mode 0 // Set to 1 if device is in developer mode 

int RXLED = 17;
bool stopEnrollment = false;
static uint8_t __clock_prescaler = (CLKPR & (_BV(CLKPS0) | _BV(CLKPS1) | _BV(CLKPS2) | _BV(CLKPS3)));
String uuid = "";

//Function declarations
void writeString(String stringData);
void commandHandler(String command);
void enrollDevice(String command);
void bootLoop();
void showSuccess();
void repeatKey(byte key, int num);
void blink();
void wait(int cycles);
void enterUuid(String uuid);
void enterCredentials();
void enterEnrollment();
void ToS();
void wifiConfig();
void shutDown();
void setupAdvancedNetworkConfig();
void updateViaGuest();
void reloadPolicies();
void removeEnrollmentWifi();
void newTab();
void closeTab();
void showVersion();
void Powerwash();
void certificateEnrollment();
void setPrescaler();
void setNeoPixel(String command);


void setup()
{
  setPrescaler(); // Set prescaler to highest clock speed
  Keyboard.begin(); // Start they keyboard emulator
  Serial.begin(9600);
  Serial1.begin(115200);
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)

  pinMode(RXLED, OUTPUT); // Configure the on-board LED
  digitalWrite(RXLED, LOW);
  TXLED1;
  wait(5); // Wait for all services to finish loading
}

void loop() { // Main Function - workflow is called within loop();
  if (Serial1.available() > 0)
  {
    String input = Serial1.readStringUntil(';');
    writeString(input);
    commandHandler(input);
  }
  //bootLoop();
}

void writeString(String stringData) { // Used to serially push out a String with Serial.write()

  for (int i = 0; i < stringData.length(); i++)
  {
    Serial.write(stringData[i]);
    Serial1.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
  }
  Serial.println();
  Serial1.println(";");
}

void commandHandler(String command) {
  if(command.startsWith("RGB"))
  {
    setNeoPixel(command);
  }
  else if(command.startsWith("Enroll"))
  {
    if (command.substring(command.indexOf(':') + 1) == "Start")
    {
      enrollDevice(command);
    }
    else if(command.substring(command.indexOf(':') + 1) == "Stop")
    {
      stopEnrollment = true;
    }
    else
    {
      uuid = command.substring(command.indexOf(':') + 1);
    }
    
  }
  
}

void enrollDevice(String command) {
  stopEnrollment = false;
  showVersion();
  // Mouse.press(MOUSE_LEFT);
  // wait(1);
  // if (stopEnrollment)
  // {
  //   return;
  // }
  // Mouse.release(MOUSE_LEFT);
  if (!update_via_guest){ // Guestmode not available for devices tagged for enrollment
    enterEnrollment();
  }
  if (setWiFi){
    wifiConfig(); // Enter the wifi configuration method (written down below)
    ToS(); // Accept Terms of Service
  }   
  TXLED1; // Toggle the TX on-board LED
  wait(15 + longer_enrollment_time); // Wait device to download configuration
  if (stopEnrollment)
  {
    return;
  }
  TXLED0;
  if (update_via_guest){
    updateViaGuest(); // Enrollment keypress at the end (around line 447)to continue the enrollment process
  }
  enterCredentials(); // Max progress with powerwash set to true - Will Powerwash after typing the password but before submitting
  wait(50 + longer_enrollment_time); // wait for Enrollment to complete
  if (stopEnrollment)
  {
    return;
  }
  
  if (sign_in && skipAssetIdScreen){ // Do not sign-in if "skipAssetIdScreen" is false
    Keyboard.write(KEY_ENTER);
    wait(10);
    if (stopEnrollment)
    {
      return;
    }
    enterCredentials();
    wait(90); // Wait for profile to load
    if (stopEnrollment)
    {
      return;
    }
    if (enroll_device_cert){
      certificateEnrollment(); // Enroll Device wide Certificate
    }
    if (remove_enrollment_wifi){
      removeEnrollmentWifi(); // Remove non-managed Enrollment WiFi
    }
  }
  if (skipAssetIdScreen) {
    shutDown();
  }
  else
  {
    enterUuid(uuid);
  }
  
  showSuccess();
}

void bootLoop() {
  //      digitalWrite(RXLED, LOW);   // set the LED on
  TXLED0; //TX LED is not tied to a normally controlled pin
  delay(200);              // wait for a second
  TXLED1;
  delay(200);
  TXLED0; //TX LED is not tied to a normally controlled pin
  delay(200);              // wait for a second
  TXLED1;
  delay(800);
}

void showSuccess() {
  digitalWrite(RXLED, HIGH);  // set the LED off
  for(int j = 0; j < 10; j++)
  {
    for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, 0, 255, 0);
  
      pixels.show();   // Send the updated pixel colors to the hardware.
  
      //delay(DELAYVAL); // Pause before next pass through loop
    }
    delay(250);
    for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, 0, 0, 0);
  
      pixels.show();   // Send the updated pixel colors to the hardware.
  
      //delay(DELAYVAL); // Pause before next pass through loop
    }
    delay(100);
    for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(rgbColour[0], rgbColour[1], rgbColour[2]));
  
      pixels.show();   // Send the updated pixel colors to the hardware.
  
      //delay(DELAYVAL); // Pause before next pass through loop
    }
    delay(250);
  }
  writeString("Enroll:Success");
}

void repeatKey(byte key, int num) {
  for (int i = 0; i < num; i++) {
    Keyboard.write(key);
    wait(1);
  }
}

void blink() {
  digitalWrite(RXLED, LOW);
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, 0, 0, 0);

    pixels.show();   // Send the updated pixel colors to the hardware.
  }
  //  TXLED1;
  delay(250);
  digitalWrite(RXLED, HIGH);
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(rgbColour[0], rgbColour[1], rgbColour[2]));

    pixels.show();   // Send the updated pixel colors to the hardware.
  }
  //  TXLED0;
  delay(250);
}

void wait(int cycles) {
  for (int i = 0; i < cycles; i++) {
    if (Serial1.available() > 0)
    {
      String input = Serial1.readStringUntil(';');
      writeString(input);
      commandHandler(input);
    }
    blink();
    if (slowMode) {
      delay(250);
    }
  }
}

void enterUuid(String uuid)
{
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.write('a');
  Keyboard.release(KEY_LEFT_CTRL);
  Keyboard.print(uuid);
  repeatKey(KEY_TAB, 3);
  Keyboard.write(KEY_ENTER);
}

void enterCredentials() {
  wait(5);
  Keyboard.print(username);
  wait(3);
  Keyboard.write(KEY_ENTER);
  wait(8);
  if (sso){
   Keyboard.write(KEY_TAB);

   Keyboard.print(username);
   Keyboard.write(KEY_TAB);
   wait(1);
  }
  Keyboard.print(password);
  if (powerwash){
    wait(5);
    Powerwash();
  }
  wait(3);
  Keyboard.write(KEY_ENTER);
  wait(3);
}

void enterEnrollment() {
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.write('e');
  Keyboard.release(KEY_LEFT_ALT);
  Keyboard.release(KEY_LEFT_CTRL);
  wait(1);
}

void ToS(){
// Terms of Service screen
  wait(5);
  repeatKey(KEY_TAB, 2);
  if (!sendUsageToGoogle) {
    Keyboard.write(KEY_ENTER);
    wait(1);
  }
  repeatKey(KEY_TAB, 3);
  wait(1);
  Keyboard.write(KEY_ENTER);
}

void wifiConfig() {
  // Access the Network option from the system tray (Status Area).
  Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.write('s');
  Keyboard.release(KEY_LEFT_ALT);
  Keyboard.release(KEY_LEFT_SHIFT);
  wait(2);
  //to select the Network
  repeatKey(KEY_TAB, 3 + VERSION_70);  // 3 for pre v70, 4 for ver 70 (black menu)
  wait(1);
  Keyboard.write(KEY_ENTER);
  wait(1);
  //to select the 'add Wifi' icon
  repeatKey(KEY_TAB, 3);
  Keyboard.write(KEY_ENTER);
  wait(1);
  // SSID
  Keyboard.print(wifi_name);
  wait(1);
  // TAB
  Keyboard.write(KEY_TAB);
  wait(1);
  if (wifi_security == 0) {
    repeatKey(KEY_TAB, 2);
  } else {
      if (advancedNetworkSetup) {
        setupAdvancedNetworkConfig();
      }
      else{
        repeatKey(KEY_DOWN_ARROW, wifi_security); //[1]WEP, [2]PSK (WPA or RSN), [3]EAP;
      
    // TAB
    Keyboard.write(KEY_TAB); //[1,2]password, [3]EAP method;
    wait(1);
    // type wifi password
    Keyboard.print(wifi_pass);
    repeatKey(KEY_TAB, 3);
   }
  }
  wait(1);
  // Enter
  Keyboard.write(KEY_ENTER); // Connect
  // Delay 15 seconds to connect
  wait(15);
  repeatKey(KEY_TAB, 3 - selectedOnBoot);
  wait(2);
  Keyboard.write(KEY_ENTER); // Click "Let's Go"
  wait(1);
  repeatKey(KEY_TAB, 2 + VERSION_70); // 3 for version 70+ (black menu)
  // After connecting, enter the enrollment key command to skip checking for update at this point in the process
  if (!update_via_guest){
    enterEnrollment();
  }
  wait(1);
  Keyboard.write(KEY_ENTER); // Click "Next"
}

void shutDown() { // Shutdown if not signed in, Sign out if signed in
  // Access the Network option from the system tray (Status Area).
  Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.write('s');
  Keyboard.release(KEY_LEFT_ALT);
  Keyboard.release(KEY_LEFT_SHIFT);
  wait(2);
  repeatKey(KEY_TAB, 1 + sign_in);
  repeatKey(KEY_ENTER, 1);
}

void setupAdvancedNetworkConfig() {
  //Starting at Security box
  if (VERSION_69 == 1){
    repeatKey(KEY_DOWN_ARROW, 3); // Select Security "EAP" (v69);
        Keyboard.write(KEY_TAB);
  }else{
    //ARROW_DOWN x3 WEP, PSK, EAP
    repeatKey(KEY_TAB, 2);
    Keyboard.write(KEY_ENTER);
    wait(1);
    //SSID (again);
    Keyboard.print(wifi_name);
    Keyboard.write(KEY_TAB);
    //@EAP Method
  }

  if (eapMethod == "LEAP") {
    // Default is LEAP v69+
    repeatKey(KEY_DOWN_ARROW, 1 - VERSION_69);
    Keyboard.write(KEY_TAB);
    // Identity
    Keyboard.print(identity);
    Keyboard.write(KEY_TAB);
    wait(1);
    Keyboard.print(wifi_pass);
    repeatKey(KEY_TAB, 2);
    wait(1);
    Keyboard.write(KEY_ENTER); // Save Identity and Password (true);
    repeatKey(KEY_TAB, 2);
    Keyboard.write(KEY_ENTER); // Connect;
  } else if (eapMethod == "PEAP") {
    // Select PEAP method
    repeatKey(KEY_DOWN_ARROW, 2 - VERSION_69);
    Keyboard.write(KEY_TAB);
    wait(1);
    // EAP Phase 2 authentication
    // If phase two authentication is defined, select it
    if (phaseTwoAuthentication) {
      repeatKey(KEY_DOWN_ARROW, phaseTwoAuthentication); // [0]Automatic, [1]EAP-MD5, [2]MSCHAP(v2 pre-v69;v1 v69+, [3]MSCHAPv2, [4]PAP, [5]CHAP, [6]GTC : v69)
    }
    Keyboard.write(KEY_TAB);
    // Server CA Certificate
    if (serverCaCertificateCheck) {
      Keyboard.write(KEY_DOWN_ARROW);//change to "DO NOT CHECK"
    }
    Keyboard.write(KEY_TAB);

    // Identity
    Keyboard.print(identity);
    Keyboard.write(KEY_TAB);
    wait(1);
    Keyboard.print(wifi_pass);
    repeatKey(KEY_TAB, 2);

    // Anonymous Identity
    Keyboard.print(anonymousIdentity);
    Keyboard.write(KEY_TAB);
    Keyboard.write(KEY_ENTER); //Save ID and PW
    repeatKey(KEY_TAB, 1 + VERSION_69); //End on Connect  /v69+
  } else if (eapMethod ==  "EAP-TLS") {
    // Select EAP-TLS method
    repeatKey(KEY_DOWN_ARROW, 2);
    Keyboard.write(KEY_TAB);
    //EAP Phase 2 authentication
    // If phase two authentication is defined, select it
    if (phaseTwoAuthentication) {
      repeatKey(KEY_DOWN_ARROW, phaseTwoAuthentication); // [0]Automatic, [1]EAP-MD5, [2]MSCHAP(v2 pre-v69;v1 v69+, [3]MSCHAPv2, [4]PAP, [5]CHAP, [6]GTC : v69)
    }
    Keyboard.write(KEY_TAB);
    // Server CA Certificate
    if (serverCaCertificateCheck) {
      Keyboard.write(KEY_DOWN_ARROW); // Change to "DO NOT CHECK"
    }
    Keyboard.write(KEY_TAB);

    // Subject match
    Keyboard.print(subjectMatch);
    Keyboard.write(KEY_TAB);

    // Identity
    Keyboard.print(identity);
    repeatKey(KEY_TAB, 3);

  } else if (eapMethod == "EAP-TTLS") {
    repeatKey(KEY_DOWN_ARROW, 4);
    Keyboard.write(KEY_TAB);

    // If phase two authentication is defined, select it
    if (phaseTwoAuthentication) {
      repeatKey(KEY_DOWN_ARROW, phaseTwoAuthentication);
    }
    Keyboard.write(KEY_TAB);

    // Server CA Certificate
    if (serverCaCertificateCheck) {
      Keyboard.write(KEY_DOWN_ARROW);
    }

    // Identity
    Keyboard.print(identity);
    Keyboard.write(KEY_TAB);
    Keyboard.print(wifi_pass);
    repeatKey(KEY_TAB, 2);

    // Anonymous Identity
    Keyboard.print(anonymousIdentity);
    repeatKey(KEY_TAB, 2);
  } else if (eapMethod ==  "EAP-TLS") {
    // Select EAP-TLS method
    repeatKey(KEY_DOWN_ARROW, 3);
    Keyboard.write(KEY_TAB);

    // Server CA Certificate
    if (serverCaCertificateCheck) {
      Keyboard.write(KEY_DOWN_ARROW);
    }
    Keyboard.write(KEY_TAB);

    // Subject match
    Keyboard.print(subjectMatch);
    Keyboard.write(KEY_TAB);

    // Identity
    Keyboard.print(identity);
    repeatKey(KEY_TAB, 3);

    // Anonymous Identity
    Keyboard.print(anonymousIdentity);
    repeatKey(KEY_TAB, 2);
  }
}

void updateViaGuest(){ // Guest mode not available after enrollment keys pressed
  wait(3);
  repeatKey(KEY_TAB, 6); // from "Enter Email Address"
  wait(2);
  Keyboard.write(KEY_ENTER);
  wait(15);
  newTab();
  Keyboard.print("chrome://chrome");
  Keyboard.write(KEY_ENTER);
  wait(3);
  repeatKey(KEY_TAB, 1); // Move to "Check for Updates"
  wait(1);
  Keyboard.write(KEY_ENTER);
  wait(update_wait_time);
  //exit Guest Mode
  Keyboard.press(KEY_RIGHT_SHIFT);
  Keyboard.press(KEY_RIGHT_ALT);
  Keyboard.print("s");
  wait(1);
  Keyboard.release(KEY_RIGHT_SHIFT);
  Keyboard.release(KEY_RIGHT_ALT);
  wait(1);
  Keyboard.write(KEY_TAB);
  wait(1);
  Keyboard.write(KEY_ENTER);
  wait(15);
//  enterEnrollment(); // Comment out to prevent enrolling after guestmode;
  wait(2);
}

void reloadPolicies(){
  wait(3);
  newTab();
  Keyboard.print("chrome://policy");
  Keyboard.write(KEY_ENTER);
  wait(2);
  repeatKey(KEY_TAB, 1);
  wait(1);
  Keyboard.write(KEY_ENTER);
  wait(1);
  closeTab();
}

void removeEnrollmentWifi(){
  wait(7);
  reloadPolicies();
  newTab();
  Keyboard.print("chrome://settings/knownNetworks?type=WiFi");
  Keyboard.write(KEY_ENTER);
  wait(5);
  repeatKey(KEY_TAB, 3); // Select the Top Network's "More options"
  wait(1);
  Keyboard.write(KEY_ENTER);
  wait(2);
  repeatKey(KEY_DOWN_ARROW, 3); // Select "Forget";
  wait(2);
  Keyboard.write(KEY_ENTER);
  wait(5);
  closeTab();
}

void newTab(){
  Keyboard.press(KEY_RIGHT_CTRL);
  Keyboard.print("n");
  wait(1);
  Keyboard.release(KEY_RIGHT_CTRL);
  wait(2);
}
void closeTab(){
  Keyboard.press(KEY_RIGHT_CTRL);
  Keyboard.print("w");
  wait(1);
  Keyboard.release(KEY_RIGHT_CTRL);
  wait(1);
}
void showVersion(){
  Keyboard.press(KEY_RIGHT_ALT);
  Keyboard.print("v");
  wait(1);
  Keyboard.release(KEY_RIGHT_ALT);
}
void Powerwash(){
  Keyboard.press(KEY_RIGHT_SHIFT);
  Keyboard.press(KEY_RIGHT_CTRL);
  Keyboard.press(KEY_RIGHT_ALT);
  Keyboard.print("r");
  wait(1);
  Keyboard.release(KEY_RIGHT_SHIFT);
  Keyboard.release(KEY_RIGHT_CTRL);
  Keyboard.release(KEY_RIGHT_ALT);
  wait(1);
  Keyboard.write(KEY_ENTER);
  wait(2);
  Keyboard.write(KEY_TAB);
  Keyboard.write(KEY_ENTER);
}
void certificateEnrollment() {
  wait(5);
  repeatKey(KEY_TAB, 2);
  Keyboard.print(username); //Enter Username for Certificate Enrollment
  Keyboard.write(KEY_TAB);
  wait(1);
  Keyboard.print(password); //Enter Password for Certificate Enrollment
  Keyboard.write(KEY_TAB);
  wait(1);
  Keyboard.write(KEY_ENTER); //Enable Device Wide certificate for Certificate Enrollment
  wait(1);
  repeatKey(KEY_TAB, 4);
  wait(1);
  Keyboard.write(KEY_ENTER);
  wait(40);
}
void setPrescaler() {
  // Disable interrupts.
  uint8_t oldSREG = SREG;
  cli();

  // Enable change.
  CLKPR = _BV(CLKPCE); // write the CLKPCE bit to one and all the other to zero

  // Change clock division.
  CLKPR = 0x0; // write the CLKPS0..3 bits while writing the CLKPE bit to zero

  // Copy for fast access.
  __clock_prescaler = 0x0;

  // Recopy interrupt register.
  SREG = oldSREG;
}

void setNeoPixel(String command)
{
  if (command.indexOf('r') != -1)
  {
    rgbColour[0] = command.substring(command.indexOf('r')+1, command.indexOf('r')+4).toInt();
    //writeString(String(rgbColour[0]));
  }
  if (command.indexOf('g') != -1)
  {
    rgbColour[1] = command.substring(command.indexOf('g')+1, command.indexOf('g')+4).toInt();
    //writeString(String(rgbColour[1]));
  }
  if (command.indexOf('b') != -1)
  {
    rgbColour[2] = command.substring(command.indexOf('b')+1, command.indexOf('b')+4).toInt();
    //writeString(String(rgbColour[2]));
  }
  
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
        // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(rgbColour[0], rgbColour[1], rgbColour[2]));
    
        pixels.show();   // Send the updated pixel colors to the hardware.
    
        //delay(DELAYVAL); // Pause before next pass through loop
  }
}
