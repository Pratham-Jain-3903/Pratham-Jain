#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal.h>
#include <IRremote.h>
#include <Servo.h>
#include <NewPing.h> // Library for ultrasonic sensor

String ssid = "Simulator Wifi"; // SSID to connect to
String password = ""; // Our virtual wifi has no password
String host = "api.thingspeak.com"; // ThingSpeak API
const int httpPort = 80;
String apiKey = "NY07IPYVSSYPE9WZ"; // ThingSpeak API Key
String uri = "/update?api_key=" + apiKey + "&field1="; // ThingSpeak API update URI

int setupESP8266(void) {
  // Start our ESP8266 Serial Communication
  Serial.begin(115200); // Serial connection over USB to computer
  Serial.println("AT"); // Serial connection on Tx / Rx port to ESP8266
  if (!Serial.find("OK")) return 1;

  // Connect to 123D Circuits Simulator Wifi
  Serial.println("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"");
  if (!Serial.find("OK")) return 2;

  // Open TCP connection to the host:
  Serial.println("AT+CIPSTART=\"TCP\",\"" + host + "\"," + httpPort);
  if (!Serial.find("OK")) return 3;

  return 0;
}

// Initialize ultrasonic sensor pins and object
#define trigPin  4
#define echoPin  5
#define maxDistance 200 // Maximum distance we want to ping for (in centimeters)
NewPing sonar(trigPin, echoPin, maxDistance);

void anydata(int pir_1) {
  // Get distance from ultrasonic sensor
  int distance = sonar.ping_cm();

  // Construct our HTTP call
  String httpPacket = "GET " + uri + String(pir_1) + "&field2=" + String(distance) + " HTTP/1.1\r\nHost: " + host + "\r\n\r\n";
  int length = httpPacket.length();

  // Send our message length
  Serial.print("AT+CIPSEND=");
  Serial.println(length);
  delay(10); // Wait a little for the ESP to respond
  if (!Serial.find(">")) return;

  // Send our http request
  Serial.print(httpPacket);
  delay(10); // Wait a little for the ESP to respond
  if (!Serial.find("SEND OK\r\n")) return;
}

//pins:
#define pingPin  13
#define servoIn  6
#define servoOut 7
#define PIRPin   3
//RGB LED:
#define redLed   0
#define greenLed 1
//neo LED:
#define neoPin   2
#define neoCount 12
//servo constants:
#define barLow   90
#define barHigh  185
#define carDelay 4000
//constants:
#define capacity 12
#define minDistance 100

// initialize the LCD pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// initialize the servo motor
Servo serv_in;
Servo serv_out;
bool en_entry = false;

// initialize the neo LEDs
Adafruit_NeoPixel pixels(neoCount, neoPin, NEO_GRB + NEO_KHZ800);

long pingDistance;
int availableSpots = 2;
int takenSpots = capacity - availableSpots;
int charge; // money charged to drivers


// setup:
void setup() {
  Serial.begin(9600);
    setupESP8266();

  	//servo set up
  	serv_out.attach(servoOut);
  	serv_out.write (barLow);
  	//delay(750);
    serv_in.attach(servoIn);
  	serv_in.write (barLow);
  	
	lcd.clear();
	lcd.home();
	lcd.print("Namaste!!");
	delay(2500);
	lcd.clear();
  	
	// neo leds setup
	// 255 = 100% brightness
	pixels.begin();
	pixels.setBrightness(255); 
  
	//lights takenSpots in red
	for(int i = 0; i < takenSpots; i++){
		pixels.setPixelColor (i, 255, 0, 0);
	}
	//lights availableSpots in green
	for(int i=capacity-availableSpots; i<capacity; i++){
		pixels.setPixelColor (i, 0, 255, 0);
	}
	pixels.show();
  
	// set up the LCD's number of columns and rows
	lcd.begin(16, 2);
  
	// pin mode:
	pinMode(redLed, OUTPUT);
	digitalWrite(redLed, HIGH);
	pinMode(greenLed, OUTPUT);  

	//PIR pinMode
	pinMode(PIRPin, INPUT);
	
	// Enable interrupts of the PIR, and exit-gate
    attachInterrupt(digitalPinToInterrupt(PIRPin), letCarOut, RISING);
}


/*void anydata(int pir_1, int pir_2) {
  
  // Sending cloud data for entry gate
  String httpPacket = "GET" + uri + "&field1="+String(pir_1) + "&field2="+String(pir_2)+"HTTP/1.1\r\nHost:" + host + "\r\n\r\n";
  int length = httpPacket.length();
  // Send our message length
  Serial.print("AT+CIPSEND=");
  Serial.println(length);
  delay(10); // Wait a little for the ESP to respond
  if(!Serial.find("SEND OK\r\n")) return;
}
*/

// program loop:
void loop() {
	// measure ping distace fo the entrance to the parking lot 
	pingDistance = getPingDistance();
    //anydata(20,10);
	// IN
	//checks if there are more free parking spots:  
	if (availableSpots >= 1){
		//checks if the minimun entrance distance condition is met
		if(pingDistance <= minDistance){
						en_entry = true;
						openGate();
						// car enters
						delay(carDelay);
                 		lcd.clear();
                  		en_entry = true;
						closeGate();
						en_entry = false;
          				
		}
		
		// minDistance condition for entrance is not met
		// waitnig for action
		else{
          	lcd.setCursor(11,0);
          	lcd.print("     ");
          	//lcd.clear();
			waitForAction();
		}
	}
  
	//else - no free spots left
	// LCD prints "NO Free Spots"
	else{ 
		lcd.clear();
		lcd.setCursor(1,0);
		lcd.print("Sorry!");
		lcd.setCursor(0,1);
		lcd.print("NO Free Spots");
		delay(50);
	}
}

/*
opens ans closes the exit-gate.
if the parking-lot is empty - 
the exit-gate will NOT open
*/
void letCarOut(){
 	// OUT
	// opens the gate only if NOT empty
  	if(availableSpots < capacity){
			openGate();
            // car exits
			delay(carDelay);
      		lcd.clear();
			closeGate();
    } 
}

/*
sends a ping
get the time it takes for the ping to read
returns the distance in cm
*/
long getPingDistance(){
	// send a ping of 5uSec
	pinMode(pingPin, OUTPUT);
	digitalWrite(pingPin, LOW);
	delayMicroseconds(2);
	digitalWrite(pingPin, HIGH);
	delayMicroseconds(5);
	digitalWrite(pingPin, LOW);
  
	//reads the time
	pinMode(pingPin, INPUT);
  
	// returns distance in cm
	return pulseIn(pingPin, HIGH) / 29 / 2 ;
}



/*
openGate():
checks the en_entry state (true/false)
and performs a series of different actions 
according to the 'en_entry' state:

opens the gate
update the available Spots 
print a message on the LCD
and changes the RGB from red to green
*/
void openGate(){	

	//let car in:
	// open the entry gate
	if(en_entry){ 
      	en_entry = false;
		serv_in.write(barHigh);
		availableSpots--;
		pixels.setPixelColor(capacity-availableSpots-1, 255, 0, 0);
		pixels.show();
      	anydata(availableSpots, charge ,takenSpots);
		// lcd's welcome message
		lcd.clear();
		lcd.setCursor(0, 1);
		lcd.print("you may enter");
      
        lcd.clear();
		lcd.setCursor(0, 1);
      
        charge =1200 - (availableSpots*100);      
        char charge_str[4]; // create a buffer to hold the string
        sprintf(charge_str, "%d", charge); // format the integer as a string
        lcd.print("Charge: "); // print the label
        lcd.print(charge_str); // print the charge value as a string
        delay(1000); // wait for a second
      
        pixels.setPixelColor(capacity-takenSpots-1, 255, 0, 0);
		pixels.show();
      	anydata(availableSpots, charge, takenSpots);
         
	}
	// let car out:
	// open the exit gate
	else			
	{
		serv_out.write(barHigh);
		availableSpots++;
        pixels.setPixelColor(capacity-availableSpots, 0, 255, 0);
        pixels.show();
		// lcd's good-bye message
		lcd.clear();
		lcd.setCursor(0, 1);
		lcd.print("Good-Bye...");
      
        lcd.clear();
		lcd.setCursor(0, 1);
      
        charge =1200 - (availableSpots*100);       
        char charge_str[4]; // create a buffer to hold the string
        sprintf(charge_str, "%d", charge); // format the integer as a string
        lcd.print("Charge: "); // print the label
        lcd.print(charge_str); // print the charge value as a string
        delay(1000); // wait for a second
        pixels.setPixelColor(capacity-takenSpots-1, 255, 0, 0);
		pixels.show();
      	anydata(availableSpots, charge, takenSpots);

         
	}
	//change  RGB-Led from red to green
	digitalWrite(greenLed, HIGH);
	digitalWrite(redLed, LOW);  
}

/*
closes the gate according to
the 'en_entry' state
and changes the RGB from green to red
*/
void closeGate(){
	
	// car got in:
	// close the entry gate
	if(en_entry){ 
		serv_in.write(barLow);
	}
	// car got out:
	// close the exit gate
	else			
	{
		serv_out.write(barLow);
	}
	//change  RGB-Led from green to red
	digitalWrite(greenLed, LOW);
	digitalWrite(redLed, HIGH);
}

/*
if on one comes in or out:
LCD shows availableSpots
*/
void waitForAction(){
	lcd.setCursor(0, 0);
	lcd.print("Free Spots:");
	lcd.setCursor(0,1);
	lcd.setCursor(7, 1);
	lcd.print(availableSpots);
	delay(30);
}

