/*
  Jenkins Client 
 */

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  0x90, 0xA2, 0xDA, 0x00, 0x86, 0xE5 };

// Enter a name of Jenkins Server
char serverName[] = "builds.bluetarp.com";
char apiPath[] = "/api/json?tree=jobs[color]";

const int redPin = 22;
const int greenPin = 26;
const int yellowPin = 24;
const int buzzerPin = 52;

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
String result;
int lastResult = -1;


/**
 * Inialize the system.
 */
void setup() {
  
  // start the serial library:
  Serial.begin(9600);
  // start the Ethernet connection:

  // set the digital pin as output:
  pinMode(redPin, OUTPUT);      
  pinMode(greenPin, OUTPUT);     
  pinMode(yellowPin, OUTPUT);

  // give the Ethernet shield a second to initialize:
  Serial.println("initializing...");
  
  Serial.print("dhcp negotiation");
  if (Ethernet.begin(mac)) {
    Serial.print(" successful, local ip: ");
    delay(1000);
    Serial.println(Ethernet.localIP());
  }
  else {
    Serial.println(" failed");
  }
}


/**
 * Connect to the jenkins server and make an api request.
 */
boolean jenkinsConnect() {

  Serial.print("connecting to http://");
  Serial.print(serverName);
  Serial.print(apiPath);
  
  if (client.connect(serverName, 80)) {
    
    Serial.println(" connected, sending request");
    
    // Make a HTTP request:
    client.print("GET ");
    client.print(apiPath);
    client.println(" HTTP/1.0");
    
    // ...use host header in case it's a name based virtual host
    client.print("Host: ");
    client.println(serverName);
    
    // ...empty line to delimit the request.
    client.println();
  } 
  else {
    Serial.println(" failed");
    return false;
  }
  
  return true;
}

/**
 * Reads the result of the jenkins api request and returns true if errors.
 * -1: invalid message
 *  0: all success (green)
 *  1: unstable    (yellow)
 *  2: errors      (red)
 */
int jenkinsResult() {
  
  int resultCode = -1;
  char *resultString = "invalid";
 
  Serial.print("fetching result, ");
 
  while (client.available()) {
    char c = client.read();
    result += c;
  }

  boolean messageComplete = result.endsWith("}]}");    
  boolean hasRed = result.indexOf("{\"color\":\"red\"}") >= 0;
  boolean hasYellow = result.indexOf("{\"color\":\"yellow\"}") >= 0;
  
  result="";

  if (hasRed) {
    resultCode = 2;
    resultString = "red";
  }
  else if (hasYellow) {
    resultCode = 1;
    resultString = "yellow";
  }
  else if (messageComplete) {
    resultCode = 0;
    resultString = "green";
  }

  Serial.print("result: ");
  Serial.println(resultString);
  
  return resultCode;
}

void jenkinsCleanup() {
  client.stop();
  Serial.println("closed connection");
}


void lightRed(boolean light) {
  digitalWrite(redPin, light? HIGH : LOW);
}

void lightGreen(boolean light) {
  digitalWrite(greenPin, light? HIGH : LOW);
}

void lightYellow(boolean light) {
  digitalWrite(yellowPin, light? HIGH : LOW);
}

/**
 * Shows the fetching indicator.
 */
void signalFetch() {
  for (int i=0; i<10; i++) {
    lightRed(i % 3 == 0);
    lightYellow(i % 3 == 1);
    lightGreen(i % 3 == 2);
    delay(100);
  }
  lightRed(true);
  lightYellow(true);
  lightGreen(true);
}

/**
 * Shows the update indicator.
 */
void signalUpdate() {
  for (int i=0; i<5; i++) {
    boolean light = i % 2 == 0;
    lightRed(light);
    lightYellow(light);
    lightGreen(light);
    delay(100);
  }
  lightRed(false);
  lightYellow(false);
  lightGreen(false);
}

/**
 * Shows the result.
 * -1: all lights
 *  0: green
 *  1: yellow
 *  2: red
 */
void signalResult(int result) {
  if (result != lastResult) {
    if (result == 0) { beepOk(); } else { beepFailure(); }
    Serial.println("new result, signalling");
    for (int i=0; i<10; i++) {
      boolean light = i % 2 == 0;
      lightRed(result == -1 || result == 2 && light);
      lightYellow(result == -1 || result == 1 && light);
      lightGreen(result == -1 || result == 0 && light);
      delay(100);
    }
    lastResult = result;
  } else {
    Serial.println("result unchanged");
  }
  lightRed(result == -1 || result == 2);
  lightYellow(result == -1 || result == 1);
  lightGreen(result == -1 || result == 0);
}

void wait(long waitmillis) {
  Serial.print("waiting ");
  Serial.print(waitmillis / 1000);
  Serial.println(" seconds");
  delay(waitmillis);
}

/**
 * Sounds a buzzer to indicate a failure status.
 */
void beepFailure() {
   for (int i = 0; i < 4; i++) {
    tone(buzzerPin, 325);
    delay(250);
    noTone(buzzerPin);
    delay(250);
  }
}

/**
 * Sounds a buzzer to indicate success status.
 */
void beepOk() {
  for (int i = 0; i < 2; i++) {
    tone(buzzerPin, 325);
    delay(100);
    noTone(buzzerPin);
    delay(100);
  }
}

/**
 * The main application loop.
 */
void loop()
{
  signalFetch();
  if (jenkinsConnect()) {
    signalUpdate(); 
    signalResult(jenkinsResult());
  }
  jenkinsCleanup();
  wait(60000);
}


