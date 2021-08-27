/*
    SmartFarm with Arduino nano by racuca
    Version 0.1
    2021.08.24 first version
*/


#include <DHT.h>
#include <DHT_U.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>


#define DEBUG true
// PIN Define
// Analog
#define SoilMoistureSensor A0  // A0 for Soil moisture sensor
#define PhotoCdsSensor A2      // A2 for the potentiometer

// Digital
#define TempHumidSensor 2     // D2 for TempHumid Sensor
#define ESPRx 4
#define ESPTx 5
#define LEDBarRelay 7         // D7 as Relay Control for LED bar


SoftwareSerial ESPserial(4, 5); // RX | TX
DHT dht(2, DHT11);
LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

// Threshold value define
int SoilThreshold = 700;
int PhotoThreshold = 500;
float TempThreshold = 30;
float HumidThreshold = 60;

// sensor variables
int soilsensor = 0;
int photosensor = 0;
float temp = 0;
float humid = 0;

// wifi variables
boolean alreadyConnected = false;
int sensorcheckcount = 100;


// lcd display variables
int displaytypes = 0;
int cursorpos = 0;
void lcddisplay(String msg)
{  
  lcd.setCursor(0,cursorpos);
  lcd.print(msg);

  cursorpos = 1- cursorpos;
}

void setup() {
  String res = "";
  
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcddisplay("Initialize...");

  Serial.begin(9600);
  // Start the software serial for communication with the ESP8266
  ESPserial.begin(9600);

  //pinMode(SoilMoistureSensor, INPUT);
  //pinMode(PhotoCdsSensor, INPUT);
  pinMode(LEDBarRelay, OUTPUT);

  dht.begin();

  res = sendData("AT+RST\r\n", 5000, DEBUG); // Reset the ESP8266
  while(!(res.indexOf("OK") >= 0 || res.indexOf("CONNECTED") >= 0))
  {
    String res = sendData("AT+RST\r\n", 5000, DEBUG); // Reset the ESP8266
    delay(3000);   
    
    sendData("AT+CWMODE=1\r\n", 5000, DEBUG); //Set station mode Operation
    sendData("AT+CWJAP=\"olleh_WiFi_F6C4\",\"0000006394\"\r\n", 5000, DEBUG);//Enter your WiFi network's SSID and Password.
    while (!ESPserial.find("OK"))
    {
      Serial.println("Waiting");
      delay(1000);
      sendData("AT+CWJAP=\"olleh_WiFi_F6C4\",\"0000006394\"\r\n", 5000, DEBUG);//Enter your WiFi network's SSID and Password.
    }
    sendData("AT+CIFSR\r\n", 5000, DEBUG);//You will get the IP Address of the ESP8266 from this command.
  }
  
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
      Serial.println("Receive data from Web : " + msg);
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

  // 10초에 한번 센서 측정
  delay(100);

  if (sensorcheckcount <= 0)
  {
    sensorcheckcount = 100;
  }
  else
  {
    sensorcheckcount--;
    return;
  }

  // measure sensor value
  soilsensor = analogRead(SoilMoistureSensor);
  photosensor = analogRead(PhotoCdsSensor);
  temp = dht.readTemperature();     // read temperature
  humid = dht.readHumidity();        // read humidity

  Serial.println("Sensor");
  Serial.print("Soil : ");  Serial.println(soilsensor);
  Serial.print("Photo : ");  Serial.println(photosensor);
  Serial.print("Temp : ");  Serial.println(temp);
  Serial.print("Humid : ");  Serial.println(humid);

  lcd.clear();
  cursorpos = 0;
  if (displaytypes == 0) {
    lcddisplay("Temp   Humid");
    lcddisplay(String(temp) + "   " + String(humid));
  }
  else if (displaytypes == 1) {
    lcddisplay("Soil   Photo");
    lcddisplay(String(soilsensor) + "   " + String(photosensor));
  }
  displaytypes = 1- displaytypes;
  
  if (soilsensor > SoilThreshold)
  {
    // 흙의 수분이 마름. 물 공급 필요
  }
  else {
    // 흙에 수분이 충분함. 물 공급 중단
  }

  //
  if (temp > TempThreshold) {
    // LED 바 끄기
    digitalWrite(LEDBarRelay, LOW);
    Serial.println("LED OFF");
  }
  else {
    // LED 바 켜기
    digitalWrite(LEDBarRelay, HIGH);
    Serial.println("LED ON");
  }

  // 습도가 너무 높다. 팬 가동
  if (humid > HumidThreshold) {

  }
  else {

  }

  postsensordata();
}

/*
  Name: sendData
  Description: Function used to send data to ESP8266.
  Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
  Returns: The response from the esp8266 (if there is a reponse)
*/
String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  ESPserial.print(command); // send the read character to the esp8266
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (ESPserial.available())
    {
      // The esp has data so display its output to the serial window
      char c = ESPserial.read(); // read the next character.
      response += c;
    }
  }

  if (debug)
  {
    Serial.print(response);
  }

  return response;
}


void postsensordata()
{
  String server = "192.168.0.51";
  //String uri = "/test/smartfarm/";
  String uri = "/";
  
  String res = "";
  Serial.println("Send Sensor Data to Web Server...");
  String data = "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(humid) +"}";

  String postRequest =
    "POST " + uri + " HTTP/1.1\r\n" +
    "Host: " + server + "\r\n" +
    "Accept: */*\r\n" + 
    "Content-Length: " + data.length() + "\r\n" +
    "Content-Type: application/json\r\n" + 
    "\r\n" + data + "\r\n";
  
  res = sendData("AT+CIPSTART=0,\"TCP\",\"" + server + "\",5000\r\n", 3000, DEBUG);
  if (res.indexOf("OK") < 0)
    return;
  
  //determine the number of caracters to be sent.
  String sendCmd = "AT+CIPSEND=0," + String(postRequest.length()) + "\r\n";
  res = sendData(sendCmd, 5000, DEBUG);
  if (res.indexOf(">")) {
    sendData(postRequest, 5000, DEBUG);
  }

  sendData("AT+CIPCLOSE\r\n", 3000, DEBUG);

}
