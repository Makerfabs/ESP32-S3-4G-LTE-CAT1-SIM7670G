#include <Arduino.h>
#include "DHT.h"

HardwareSerial mySerial2(2);

String Apikey = "OIXP1RMED1WROHIG";

#define DEBUG true

#define IO_RXD2 47
#define IO_TXD2 48

// change this to match your SD shield or module;
// ESP32-S3 4G LTE: pin 10
const int PIN_SD_SELECT = 10;

#define IO_GSM_PWRKEY 4
#define IO_GSM_RST    5

unsigned long currentTime;
#define DHTPIN 2     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11   // DHT 11


DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  Serial.print(F("Hello! ESP32-S3 AT command V1.0 Test"));
  mySerial2.begin(115200,SERIAL_8N1, IO_RXD2, IO_TXD2);
  pinMode(IO_GSM_RST, OUTPUT);
  digitalWrite(IO_GSM_RST, LOW);
  
  pinMode(IO_GSM_PWRKEY, OUTPUT);
  digitalWrite(IO_GSM_PWRKEY, LOW);
  delay(3000);
  digitalWrite(IO_GSM_PWRKEY, HIGH);
  delay(3000);
  digitalWrite(IO_GSM_PWRKEY, LOW);
  
  Serial.println("ESP32-S3 4G LTE CAT1 ThingSpeak Test Start!");
  delay(2000);
  delay(2000);
  Serial.println("Wait a few minutes for 4G star");
  delay(3000);
  delay(1000);
  sendData("AT", 1000, DEBUG);
  delay(1000);
  sendData("AT+SIMCOMATI", 1000, DEBUG);
  delay(1000);
  sendData("AT+CICCID", 1000, DEBUG);
  sendData("AT+CNUM", 1000, DEBUG);
  sendData("AT+COPS?", 1000, DEBUG);    
  sendData("AT+CPSI?", 1000, DEBUG);
}

void loop() {
  delay(2000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t) ){
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.println(F("%"));
  Serial.print(F("Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
  sendData("AT+HTTPINIT", 1000, DEBUG);
  sendData("AT+HTTPPARA=\"URL\",\"http://api.thingspeak.com/update?api_key=" + Apikey + "&field1=" + (String)t + "&field2=" + (String)h +"\"\r\n", 1000, DEBUG);
  sendData("AT+HTTPACTION=0", 1000, DEBUG);
  sendData("AT+HTTPTERM", 1000, DEBUG);
  currentTime = millis();
}

String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  mySerial2.println(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (mySerial2.available())
    {
      char c = mySerial2.read();
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
  }
  return response;
}
