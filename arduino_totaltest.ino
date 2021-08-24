#include <DHT.h>
#include <DHT_U.h>
#include <SoftwareSerial.h>

#define DEBUG true

SoftwareSerial ESPserial(4, 5); // RX | TX
DHT dht(2, DHT11);


// PIN Define
// Analog
int SoilMoistureSensor = A0;  // A0 for Soil moisture sensor
int PhotoCdsSensor = A2;      // A2 for the potentiometer

// Digital
int TempHumidSensor = 2;      // D2 for TempHumid Sensor
int ESPRx = 4;
int ESPTx = 5;
int LEDBarRelay = 7;          // D7 as Relay Control for LED bar

// Threshold value define
int SoilThreshold = 700; 
int PhotoThreshold = 500; 
float TempThreshold = 25;
float HumidThreshold = 60;

// sensor variables
int soilsensor = 0;
int photosensor = 0;

// wifi variables
boolean alreadyConnected = false;

int sensorcheckcount = 10;

void setup() {
  Serial.begin(9600);
  // Start the software serial for communication with the ESP8266
  ESPserial.begin(9600);
  
  //pinMode(SoilMoistureSensor, INPUT);
  //pinMode(PhotoCdsSensor, INPUT);
  pinMode(LEDBarRelay, OUTPUT);

  dht.begin();

  sendData("AT+RST\r\n", 5000, DEBUG); // Reset the ESP8266
  sendData("AT+CWMODE=1\r\n", 5000, DEBUG); //Set station mode Operation
  sendData("AT+CWJAP=\"olleh_WiFi_F6C4\",\"0000006394\"\r\n", 5000, DEBUG);//Enter your WiFi network's SSID and Password.
  while(!ESPserial.find("OK")) 
  {
    Serial.println("Waiting");
    delay(1000);
  }
  sendData("AT+CIFSR\r\n", 5000, DEBUG);//You will get the IP Address of the ESP8266 from this command. 
  sendData("AT+CIPMUX=1\r\n", 5000, DEBUG);
  sendData("AT+CIPSERVER=1,80\r\n", 5000, DEBUG);
}

void loop() {

  if (ESPserial.available())
  {
    if (ESPserial.find("+IPD,"))
    {
      String msg;
      ESPserial.find("?");
      msg = ESPserial.readStringUntil(' ');
      String command1 = msg.substring(0, 3);
      String command2 = msg.substring(4);
                  
      if (DEBUG) 
      {
        Serial.println(command1);//Must print "led"
        Serial.println(command2);//Must print "ON" or "OFF"
      }
      delay(100);

      if (command2 == "ON") 
      {
        digitalWrite(LED_BUILTIN, HIGH);
      }
      else 
      {
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
  }


  delay(100);

  if (sensorcheckcount-- <= 0)
    sensorcheckcount = 10;    

  if (sensorcheckcount != 10)
    return;
  
  // measure sensor value 
  soilsensor = analogRead(SoilMoistureSensor);
  photosensor = analogRead(PhotoCdsSensor);
  float temp = dht.readTemperature();     // read temperature
  float humid = dht.readHumidity();        // read humidity

  Serial.println("Sensor");
  Serial.print("Soil : ");  Serial.println(soilsensor);
  Serial.print("Photo : ");  Serial.println(photosensor);
  Serial.print("Temp : ");  Serial.println(temp);
  Serial.print("Humid : ");  Serial.println(humid);
  
  if (soilsensor > SoilThreshold)
  {
    // 흙의 수분이 마름. 물 공급 필요
  }
  else {
    // 흙에 수분이 충분함. 물 공급 중단
  }

  // 
  if (temp > TempThreshold && photosensor > PhotoThreshold) {
     // LED 바 끄기    
     digitalWrite(LEDBarRelay, LOW);
     Serial.println("LED OFF");
  }
  else {
    // LED 바 켜기
    digitalWrite(LEDBarRelay, HIGH);
    Serial.println("LED ON");
  }

  if (humid > HumidThreshold) {
    
  }
  else {
    
  }


}

/*
* Name: sendData
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    ESPserial.print(command); // send the read character to the esp8266
    long int time = millis();
    while( (time+timeout) > millis())
    {
      while(ESPserial.available())
      {
        // The esp has data so display its output to the serial window 
        char c = ESPserial.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}
