#include "Arduino.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"


const char * ssid = "CROCS_NETWORK";
const char * pass = "NOTAVLAD";

// Номер пина Arduino с подключенным датчиком
#define PIN_DS18B20 2

// Создаем объект OneWire
OneWire ds(PIN_DS18B20);

DallasTemperature dt(&ds);
DeviceAddress da;

float temperature = 0.0; // Глобальная переменная для хранения значение температуры с датчика DS18B20

void connectToWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    Serial.print("Connecting to "); Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED)
    {
      Serial.print('.');
      delay(500);
    }
    Serial.println("CONNECTED");

}

void printTemperature()
{
    float tempC = dt.getTempC(da);
    if(tempC == DEVICE_DISCONNECTED_C)
    {
        Serial.println("Error: couldn't read temperature data");
        return;
    }
    Serial.print("Temp C: "); Serial.print(tempC); Serial.print(" Temp F: "); Serial.println(DallasTemperature::toFahrenheit(tempC));
}

void setup()
{
    Serial.begin(115200);
    Serial.println("STARTED");

    delay(5000);
    connectToWiFi();
    // to use GPIO2 as input
    //pinMode(0, OUTPUT);
    //digitalWrite(0, LOW);

    Serial.println("Locating devices...");
    dt.begin();
    Serial.print("Found ");
    Serial.println(dt.getDeviceCount());

    if(!dt.getAddress(da, 0)) Serial.println("Unable to find address for Device 0");

    dt.setResolution(da, 12);
}

void loop()
{
    dt.requestTemperatures();
    printTemperature();
    if(!WiFi.isConnected()) connectToWiFi();
    WiFiClient client;
    String ip = WiFi.gatewayIP().toString();
    if(!client.connect("192.168.4.1", 80))
    {
        Serial.println("Connection failed");
        return;
    }
    
    String request = String("/sendtemp?mac=" + WiFi.macAddress() + "&temp=" + String(dt.getTempC(da)));
    Serial.println(request);
    client.print(String("GET " + request + " HTTP/1.1\r\nHost: " + ip + "\r\nConnection: close\r\n\r\n"));
    
    unsigned long timeout = millis();
    while(client.available() == 0)
    {
        if(millis() - timeout > 5000)
        {
            Serial.println("Client timeout///");
            client.stop();
            return;
        }
    }
    delay(3000);
}
