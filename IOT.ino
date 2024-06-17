/*This Program is aimed at Controlling and monitoring Temperature via  DHT11 Sensor, controlling a Servo motor,
a cooling fan, an OLED display that will serve as a local system monitor and one LED.
All of which will be connected to one Blynk IOT cloud server/application*/

//Author : Malago Achbor Joshua 

//Setting up Blynk account's credentials
// These can be found in Blynk Account's personal details 
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "Template ID"
#define BLYNK_TEMPLATE_NAME "Template Name"
#define BLYNK_AUTH_TOKEN "Authentication Token"

//Libraries to be used
#include"BlynkSimpleEsp32.h"
#include"DHTesp.h"
#include "WiFi.h"
#include"WiFiClient.h"
#include"ESP32Servo.h"
#include"Adafruit_SSD1306.h"
#include <Adafruit_GFX.h>
#include <Wire.h>

//Setting up Blynks Virtual Pin COnstants
#define Temperature V0      
#define Humidity    V1
#define Terminal    V2
#define Slider      V3
#define Led         V4
#define TempStatus  V5
#define Alarm_state V6
#define Screen_width 128
#define Screen_height 64
#define Oled_Adress 0x3C
#define Reset -1   // setting the display's reset pin same as that of the board
#define Relay 25
#define Alarm_switch 5

//Initializing Class Objects
DHTesp dht;
Servo servo;
Adafruit_SSD1306 oled(Screen_width, Screen_height, &Wire, Reset);
BlynkTimer sensor_timer;
WidgetTerminal terminal(V2);

//Setting up Wifi connection Parameters
char wifi_id[]="Wifi Router SSID";
char wifi_psswd[]="Wifi Password";
char token[]="Blynk Account's Token string for connection access";

//Setting up GLobal variables
bool buzzer_state = true, buzzer=false;
int  pos;                //servo motor's Angular position variable
const int buzzer_pin =2;
const int led_pin = 32;


void setup() {
  
  oled.begin(SSD1306_SWITCHCAPVCC, Oled_Adress); 
  Serial.begin(115200);
  dht.setup(13, DHTesp::DHT11);
  Blynk.begin(token, wifi_id, wifi_psswd,"blynk.cloud",80);
  Serial.println("Hello, ESP32!");
  sensor_timer.setInterval(1000L, sensor_data);
  servo.attach(27);

  pinMode(led_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);
  pinMode(Relay,OUTPUT);
  pinMode(Alarm_switch, INPUT);

  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1.5);
  
}


void loop() {
  Blynk.run();
  sensor_timer.run();
  
  //   getting the temperature values in real time from the sensro so as to use them in the condition below.
  TempAndHumidity data= dht.getTempAndHumidity();
  
//This condition will be used to toggle the boolean value "buzzer" which will be used to control the Temperature alarm  
  if(data.temperature > 27 && digitalRead(Alarm_switch)) buzzer = true;
  else{
    buzzer = false;
  }
}


/*This function will Read data from The Blynk cloud's slider 
to control the Servo Motor*/

BLYNK_WRITE(Slider){
  pos = param.asInt();
  servo.write(pos);  
}


/*A function to Read Commands from the Blynk's Terminal
  This will control an LED by means of a text commands.*/  
BLYNK_WRITE(Terminal)
{
  String command = param.asStr();
  if (command == "Lights on" ){
     digitalWrite(led_pin,1);
     Blynk.virtualWrite(Led,"Lights on");
  }
  else if(command == "Lights off"){
     digitalWrite(led_pin,0);
     Blynk.virtualWrite(Led, "Lights off");
  }     
  else if(command =="Clear")  terminal.clear();
  else terminal.println("Invalid command!! Try another Command");
}

/*This function Reads the Humidity and Temperature data measured by
  the DHT sensor after each 1second as per timer's cycle.
  The values will be taken and displayed on the local OLED display
  also on the BLYNK cloud app.*/
  
void sensor_data()
{
  TempAndHumidity data = dht.getTempAndHumidity();
  Blynk.virtualWrite(Temperature, data.temperature);
  Blynk.virtualWrite(Humidity, data.humidity);

  
    oled.clearDisplay();   
    oled.setCursor(0,0);
    oled.print("Temperature:");
    oled.setCursor(80,0);
    oled.print(data.temperature);
    oled.setCursor(0,10);
    oled.print("Humidity: ");
    oled.setCursor(80,10);
    oled.print(data.humidity);
    oled.setCursor(0,20);
    oled.print("Motor Angle:");
    oled.setCursor(80,20);
    oled.print(pos);
    oled.display();
    
    

    /*Conditions to activate the fan and an alarm incase of continued
     increase in temperature */

   if (data.temperature > 25.0){
     digitalWrite(Relay,1);
     Blynk.virtualWrite(TempStatus, "Normal.");
      
      
      /*The below conditions will trigger the alarm and indicate on the dashboard that
        the temperature is rising to "Abnormal" values*/       
   
      if(data.temperature > 27){       
        Blynk.virtualWrite(TempStatus, "Abnormal!");        
        oled.setCursor(0,30);
        oled.print("High Temperature!!");
        oled.display();
        
        
        /*The below condition will operate the Temperature alarm 
        only when the temperature value is reached and the dashboard alarm switch is not turned off*/               
        if(buzzer==false){
          digitalWrite(buzzer_pin, buzzer_state);
          buzzer_state=!buzzer_state;  
          Blynk.virtualWrite(Alarm_state,"Alarm Ringing!");               
        }        
        else{
          digitalWrite(buzzer_pin,0);
          Blynk.virtualWrite(Alarm_state, "Alarm Silent.");
        }
    }
   }
   
   else {
    digitalWrite(Relay,0);
    Blynk.virtualWrite(TempStatus, "Normal.");
    Blynk.virtualWrite(Alarm_state, "Alarm Silent.");      
    }
}
