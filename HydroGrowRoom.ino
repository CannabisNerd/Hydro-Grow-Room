
// A Free cayenne Dashboard account is needed for this sketch, Create account at https://mydevices.com/. Add a device (ESP8266)
//This will create a new dashboard, edit this sketch with the login it provides you. 


#include <TimeLib.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <CayenneMQTTESP8266.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHTesp.h"

#define CAYENNE_PRINT Serial
#define PUMPPIN D5 // Define Digital Pin for Pump Relay
#define DHTPIN D0 // Define Digital Pin for Temp/Humidity readings

DHTesp dht; // Tells DHT commands to use DHTEsp library
OneWire oneWire(D3); // Establishes Onewire communication on D3
DallasTemperature sensors(&oneWire); // Starts communication with Water temp sensor

const int cycle = 1000 * 60 * 7; //Set time Pump stays on for, I run this cycle twice

int lightStatus = 0; // Variable to store Light Status 

const char ssid[] = "YOUR_SSID";  //  your network SSID (name)
const char pass[] = "YOUR_PASSWORD";       // your network password

char username[] = "54cc****-****-*****-*****-****cbcd8710"; // Cayenne MQTT Username
char password[] = "e15b*******************812fe8735d02904"; // Cayenne MQTT Password
char clientID[] = "847***********-*****-********-d8a64eb"; // Cayenne MQTT Client ID


// NTP Servers:
static const char ntpServerName[] = "ca.pool.ntp.org";


const int timeZone = -4;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Atlantic Standard Time (CAN)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
void sendNTPpacket(IPAddress &address);

void setup()
{
  Serial.begin(115200);
  Cayenne.begin(username, password, clientID, ssid, pass); // Starts Cayenne 
  sensors.begin(); // Starts sensors
  dht.setup(DHTPIN); // Connect DHT sensor
  pinMode(PUMPPIN, OUTPUT); // Set PUMP pin to output
  digitalWrite(PUMPPIN, LOW); // Set PUMP pin to low so pump is off at start up

  Alarm.delay(250);

  UpdateNTP(); // Updates Time at boot

  Serial.println("Programming Pump......");
 
 // Schedule All alarms and timers 
 // TimeAlarms.h defaults to a maximum of 6 alarms, ESP devices can handle many more. Edit the TimeAlarms.h file to increase this limit, the more alarms the more RAM used
   
   Alarm.alarmRepeat(8, 5, 0, pumpON);
  Alarm.alarmRepeat(11, 5, 0, pumpON);
  Alarm.alarmRepeat(14, 5, 0, pumpON);
  Alarm.alarmRepeat(17, 5, 0, pumpON);
  Alarm.alarmRepeat(20, 5, 0, pumpON);
  Alarm.alarmRepeat(23, 5, 0, pumpON);
  Alarm.alarmRepeat(3, 5, 0, pumpON);
  Alarm.alarmRepeat(0, 0, 1, UpdateNTP);
  Serial.println("Pump Programmed");
}


void loop()
{
  Alarm.delay(1000); // Use Alarm.delay instead of delay with this sketch
  Cayenne.loop(); // Runs Cayenne loop
  SendData(); // Calls function to update Cayenne dashboard
}

void SendData() {
  sensors.requestTemperatures(); // Reads OneWire temperature probe
  float humidity = dht.getHumidity(); // Reads DHT air Humidity
  float temperature = dht.getTemperature(); //Reads DHT Air Temp
 
  //Prints data to Serial dor Debugging
  Serial.println(temperature);
  Serial.println(humidity);
  Serial.println(sensors.getTempCByIndex(0));

  //Sends all sensor data to Cayenne  
  Cayenne.virtualWrite(0, temperature);
  Cayenne.virtualWrite(1, humidity);
  Cayenne.celsiusWrite(2, sensors.getTempCByIndex(0));
  //
  
  lightStatus = analogRead(A0); // Reads Light sensors analog value
  Serial.print("Light status is: ");

 
  if (lightStatus > 500) // Checks if analog value of light sensor is above what it should be if light is on
  {
    //setting a threshold value
    Cayenne.virtualWrite(4, 1, "digital_sensor", "d"); // Sends digital on command to Cayenne to say light is on
    // Prints Analog and digital values to serial for debugging
    Serial.println("ON"); 
    Serial.println(lightStatus); 
  }

  else {
    Cayenne.virtualWrite(4, 0, "digital_sensor", "d"); // Sends digital off command to cayenne
 // Prints Analog and digital values to serial for debugging
    Serial.println("OFF");
    Serial.println(lightStatus);
  }

}

// This section is for Cayenne in commnds, allows me to remote control my pump if I feel another watering is needed
CAYENNE_IN(3)
{
  int currentValue = getValue.asInt(); // Gets value of digital button on Cayenne
  if (currentValue == 1) // If the Digital button is pushed
  {
    //do whatever you want when you turn on the button on cayenne dashboard
    digitalWrite(PUMPPIN, HIGH);
  }
  else
  {
    //do whatever you want when you turn off the button on cayenne dashboard
    digitalWrite(PUMPPIN, LOW);
  }
}


// functions to be called when an alarm triggers:
void pumpON() {

  // Turns Pump on for the cycle time we set above. I run the cycle twice with a 1 minute delay in between to drain the table.
  // It allows me to use a slightly smaller reservoir. So the cycle set above is actually doubled currently.
  // All other functions will be disabled during this cycle due to the use of delay, 
  // I will be updating at a later date to use millis() instead.
  digitalWrite(PUMPPIN, HIGH);
  Alarm.delay(cycle);
  digitalWrite(PUMPPIN, LOW);
  Alarm.delay(1000 * 60);
  digitalWrite(PUMPPIN, HIGH);
  Alarm.delay(cycle);
  digitalWrite(PUMPPIN, LOW);


}

//Function that updates Time from server
void UpdateNTP() {

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
}


//*-------- NTP code ----------*//

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
