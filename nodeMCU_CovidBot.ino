/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>

// Replace with your network credentials
const char* ssid     = "Mi A3";
const char* password = "11235813";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Pins
int motor1pin1 = 5; // Motorpins
int motor1pin2 = 6;
int motor2pin1 = 7;
int motor2pin2 = 8;
int IRSensor1 = 0; // Haanji toh this is the 5 IRSensor array
int IRSensor2 = 1;
int IRSensor3 = 2;
int IRSensor4 = 3;
int IRSensor5 = 4;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

char bedMap[50][50]; // Ha toh bedSize yaha declare karenge. Will change that manually for bigger maps.
                     // Here there's another assumption that there arent a lot of unnecessary turns in the path.

int length(char arr1[])
{
  for (int i = 0; i < 50; i++)
  {
    if (arr1[i] == 'A')
    {
      return i;
    }
  }
}

char presentLocation[50];

char tempAddress[50];

int targetLocations[] = {1, 4, 7, 8, 9, 42}; // Input taken from NodeMCU

int numberTarget; // len(targetLocations)
int currentTurn = 0;
int currentBed = 0;
int Bn = 0;
int Tn = 0;



//the two MODE flags. mapMode and moveMode
int mapFlag = 0;
int moveFlag = 0;
int stationFlag = 0;//flags
int terminusFlag = 0;
int xFlag = 0;
int tStraightFlag = 0;
int tLeftFlag = 0;
int tRightFlag = 0;
int lLeftFlag = 0;
int lRightFlag = 0;
int shortpathFlag = 0;
int dataInputFlag = 0;

// Time constants
int stopTime = 50; // These are the time constants
int turnTime = 50; // toh this is a new variable we have declared. We will fine tune this once the bot is built. it represets the milliseconds required for the bot to take a right turn.
int nodeTime = 50; // This is for differentiation b/w station, junction and terminus

// Path status variables
int statusSensor1;
int statusSensor2;
int statusSensor3;
int statusSensor4;
int statusSensor5;
int prevSensor1;
int prevSensor2;
int prevSensor3;
int prevSensor4;
int prevSensor5;








//The self declared functions 
void readSensor()
{
  statusSensor1 = digitalRead(IRSensor1); // This is for chechking what is under the sensor array in this moment
  statusSensor2 = digitalRead(IRSensor2);
  statusSensor3 = digitalRead(IRSensor3);
  statusSensor4 = digitalRead(IRSensor4);
  statusSensor5 = digitalRead(IRSensor5);
}

// Ha abhi we'll define functions for left right straight.
void turnLeft()
{
  digitalWrite(motor1pin1, HIGH);
  digitalWrite(motor1pin2, LOW);
  delay(turnTime);
  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, LOW);
}

void turnRight()
{
  digitalWrite(motor2pin1, HIGH);
  digitalWrite(motor2pin2, LOW);
  delay(turnTime);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
}

void turnStraight()
{
  digitalWrite(motor1pin1, HIGH);
  digitalWrite(motor1pin2, LOW);
  digitalWrite(motor2pin1, HIGH);
  digitalWrite(motor2pin2, LOW);
  delay(turnTime);
  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, LOW);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
}
void turnBack()
{
  digitalWrite(motor2pin1, HIGH);
  digitalWrite(motor2pin2, LOW);
  delay(2 * turnTime);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
}

void goAhead()
{
  if (statusSensor2 == 1)
  {
    digitalWrite(motor1pin1, LOW);
    digitalWrite(motor1pin2, LOW);
    digitalWrite(motor2pin1, HIGH);
    digitalWrite(motor2pin2, LOW);
  }
  else if (statusSensor4 == 1)
  {
    digitalWrite(motor1pin1, HIGH);
    digitalWrite(motor1pin2, LOW);
    digitalWrite(motor2pin1, LOW);
    digitalWrite(motor2pin2, LOW);
  }
  else
  {
    digitalWrite(motor1pin1, HIGH);
    digitalWrite(motor1pin2, LOW);
    digitalWrite(motor2pin1, HIGH);
    digitalWrite(motor2pin2, LOW);
  }
  delay(stopTime);
  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, LOW);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
}

void goBack()
{
  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, HIGH);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, HIGH);
  delay(stopTime);
  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, LOW);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
  turnStraight;
}

void straightCheck(int checkDummy)
{
  digitalWrite(motor1pin1, HIGH);
  digitalWrite(motor1pin2, LOW);
  digitalWrite(motor2pin1, HIGH);
  digitalWrite(motor2pin2, LOW);
  delay(checkDummy);
  readSensor();
  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, LOW);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
}

void backCheck(int checkDummy)
{
  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, HIGH);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, HIGH);
  delay(checkDummy);
  readSensor();
  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, LOW);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);
}

void detectNode()
{ // This set of functions detect nodes at which the decision making will occur
  if (statusSensor1 == 1 or statusSensor5 == 1)
  {
    prevSensor1 = statusSensor1; // storing IR states as soon as it detects a junction
    prevSensor2 = statusSensor2;
    prevSensor3 = statusSensor3;
    prevSensor4 = statusSensor4;
    prevSensor5 = statusSensor5;
    straightCheck(3 * nodeTime);
    if (statusSensor1 == 1 && statusSensor2 == 1 && statusSensor3 == 1 && statusSensor4 == 1 && statusSensor5 == 1)
    {
      terminusFlag = 1;
    }
    backCheck(3 * nodeTime);
    straightCheck(nodeTime);
    if (statusSensor1 == 1 && statusSensor2 == 1 && statusSensor3 == 1 && statusSensor4 == 1 && statusSensor5 == 1)
    {
      stationFlag = 1;
    }
    else if (statusSensor3 == 0 and prevSensor1 == 0 and prevSensor5 == 1)
    {
      lRightFlag = 1;
    }
    else if (statusSensor3 == 0 and prevSensor1 == 1 and prevSensor5 == 0)
    {
      lLeftFlag = 1;
    }
    else if (statusSensor3 == 0 and prevSensor1 == 1 and prevSensor5 == 1)
    {
      tStraightFlag = 1;
    }
    else if (statusSensor3 == 1 and prevSensor1 == 1 and prevSensor5 == 1)
    {
      xFlag = 1;
    }
    else if (statusSensor3 == 1 and prevSensor1 == 1 and prevSensor5 != 1)
    { // can be removed
      tLeftFlag = 1;
    }
    else if (statusSensor3 == 1 and prevSensor1 != 1 and prevSensor5 == 1)
    { // can be removed
      tRightFlag = 1;
    }
    backCheck(nodeTime);
  }
}

void shortpath()
{
  if (bedMap[Bn][Tn] == 'L' && bedMap[Bn][Tn - 2] == 'L')
  {
    bedMap[Bn][Tn - 2] = 'S';
    bedMap[Bn][Tn - 1] = '_';
    bedMap[Bn][Tn] = '_';
    Tn = Tn - 2;
  }
  else if (bedMap[Bn][Tn] == 'L' && bedMap[Bn][Tn - 2] == 'R')
  {
    bedMap[Bn][Tn - 2] = 'B';
    bedMap[Bn][Tn - 1] = '_';
    bedMap[Bn][Tn] = '_';
    Tn = Tn - 2;
  }
  else if (bedMap[Bn][Tn] == 'L' && bedMap[Bn][Tn - 2] == 'S')
  {
    bedMap[Bn][Tn - 2] = 'R';
    bedMap[Bn][Tn - 1] = '_';
    bedMap[Bn][Tn] = '_';
    Tn = Tn - 2;
  }
  else if (bedMap[Bn][Tn] == 'S' && bedMap[Bn][Tn - 2] == 'L')
  {
    bedMap[Bn][Tn - 2] = 'R';
    bedMap[Bn][Tn - 1] = '_';
    bedMap[Bn][Tn] = '_';
    Tn = Tn - 2;
  }
  else if (bedMap[Bn][Tn] == 'S' && bedMap[Bn][Tn - 2] == 'S')
  {
    bedMap[Bn][Tn - 2] = 'B';
    bedMap[Bn][Tn - 1] = '_';
    bedMap[Bn][Tn] = '_';
    Tn = Tn - 2;
  }
  else if (bedMap[Bn][Tn] == 'R' && bedMap[Bn][Tn - 2] == 'L')
  {
    bedMap[Bn][Tn - 2] = 'B';
    bedMap[Bn][Tn - 1] = '_';
    bedMap[Bn][Tn] = '_';
    Tn = Tn - 2;
  }
  shortpathFlag = 0;
}




void setup() {
  
  Serial.begin(115200);
  
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  // put your setup code here, to run once:
  pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);
  pinMode(motor2pin1, OUTPUT);
  pinMode(motor2pin2, OUTPUT);
  pinMode(IRSensor1, INPUT);
  pinMode(IRSensor2, INPUT);
  pinMode(IRSensor3, INPUT);
  pinMode(IRSensor4, INPUT);
  pinMode(IRSensor5, INPUT);

  for (int i = 0; i < 50; i++)
  { // Putting all 'A's in the bedmap Matrix.
    for (int j = 0; j < 50; j++)
    {
      bedMap[i][j] = 'A';
    }
  }
}


void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              output5State = "on";
              mapFlag = 1;
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              output5State = "off";
              mapFlag = 0;
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              moveFlag = 1;
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              moveFlag = 0;
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Covid Care Bot</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.println("<p>Mapping Mode " + output5State + "</p>");
            // If the output5State is off, it displays the ON button       
            if (output5State=="off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 4  
            client.println("<p>Moving Mode " + output4State + "</p>");
            // If the output4State is off, it displays the ON button       
            if (output4State=="off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
