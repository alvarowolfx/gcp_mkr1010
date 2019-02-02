#include <Arduino.h>
#include <ArduinoECCX08.h>
#include <WiFiNINA.h>
#include <Adafruit_TCS34725.h>
#include "arduino_secrets.h" 

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;
unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10L * 1000L; // delay between updates, in milliseconds

// Initialize the Wifi client library
WiFiClient client;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void connectWifi(){  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Initialize TCS Sensor
  if (tcs.begin()) {
    //Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }


  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // Attempt to connect to wifi network
  connectWifi();
  // you're connected now, so print out the status:
  printWifiStatus();

  // Check ECC508 chip
  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  String serialNumber = ECCX08.serialNumber();

  Serial.print("ECCX08 Serial Number = ");
  Serial.println(serialNumber);
  Serial.println();

  /* if (!ECCX08.locked()) {
    Serial.println("The ECC508/ECC608 is not locked!");
    while (1);
  } */
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the Nina module
  client.stop();

  // if there's a successful connection:
  if (client.connect(SERVER_HOST, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:

    uint16_t red, green, blue, light;  
    tcs.setInterrupt(false);  // turn on LED
    delay(60);  // takes 50ms to read
    tcs.getRawData(&red, &green, &blue, &light);

    String params = "";
    params += "deviceId=" + ECCX08.serialNumber();
    params += "&";
    params += "red=" + String(red);
    params += "&";
    params += "blue=" + String(blue);
    params += "&";
    params += "green=" + String(green);
    params += "&";
    params += "light=" + String(light);

    Serial.print("Params = ");
    Serial.println(params);

    String getRequest = "GET " + String(SERVER_PATH) + "?" + params + " HTTP/1.1";
    Serial.println(getRequest);

    client.println(getRequest);
    client.println("Host: " + String(SERVER_HOST));
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

void loop() {
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {    
    httpRequest();
  }
}