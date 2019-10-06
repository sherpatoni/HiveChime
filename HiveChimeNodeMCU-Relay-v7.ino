// This string defines the ID for the device
const char* ChimeID  = "HC0002"; // to keep track of origin on UDP packets

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

WiFiUDP Udp;

const char* ssid = "replace with ssid";
const char* password = "replace with password";



//const char* DestinIP = "192.168.0.147";
//unsigned int localUdpPort = 4210;  // local port to send on

const char* DestinIP = "34.216.40.20";
unsigned int localUdpPort = 9095;  // local port to send on

const long utcOffsetInSeconds = -7*3600; // this is to offset the UTC time given by the NTP server
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);



// Connect left board connector (C1) to Adruino 5V
// Connect second from left board connector (C2) to Adruino GND
// Board connector (C3) is left open 
int clockPin        = 14; // connects to Clock pin of shift registers : (C4) next to empty/middle connector on sensor board
int ploadPin        = 12; // connects to Parallel load aka latch pin  : (C5) middle connector on sensor board
int dataPin         = 13; // connects to the Q7 aka data pin          : (C6) outermost connector on sensor board
int RelaySensor     = 2; 
int RelayPower      = 0; 
const int Temp_sens = A0;        // A0



int RelayCounter = 1; // to count how many measurements we do before pausing
#define COOLDOWN 10000//00 // milliseconds we leave for cooling
#define NumSensings 10000 // number of sensings between cooling periods

#define NUMBER_SENSOR_BOARDS   5 // adjust to number of boards plugged in. this determines the lenght of the integer used to store the state of the sensors
#define SENSOR_PER_BOARDS   8 // boards are 4 gates, with two sensors per gate
#define DELAY_MSEC   20  // Delay between sensor reads.
#define BOARD_READING int // type used to write the state of the 8 sensors of a board. convert to binary to decimal to read the states
#define TEMP_READING float

int ChimeState[NUMBER_SENSOR_BOARDS]={}; // array used to record the current state of all the sensors on all the boards. Each element of the array corresponds to a board
int oldChimeState[NUMBER_SENSOR_BOARDS]={}; // array used to find changes in state of all sensors on all boards
bool Change_Sensors=false; // boolean to indicate that a sensor has changed state
char UDP_StateString[400]; //This is used to build the string to send to UDP and display state of sensors


float temp_read_1;
float temp_read_2;
// This function is a shift-in routine reading the serial Data from one shift register chip. 
//  Note that you need a latch signal on the ploadpin before calling this function
// Note also that this only reads one board at a time
BOARD_READING read_shift_regs(int readnum)
{
    long bitVal;
    BOARD_READING bytesVal = 0;

    for(int i = 0; i < SENSOR_PER_BOARDS; i++)
    {
        bitVal = digitalRead(dataPin);   // note: the ploadPin is pulled down from high to latch the data on outside this function to be able to read several boards in a row
        bitVal = max(bitVal*readnum ,(1-bitVal)*(1-readnum));
        //Serial.print(bitVal);
        bytesVal |= (bitVal << ((SENSOR_PER_BOARDS-1) - i)); // Set the corresponding bit in bytesVal.
        digitalWrite(clockPin, HIGH); // pulse data to get shift
        digitalWrite(clockPin, LOW);
    }
    return(bytesVal);
}



TEMP_READING read_temps()
{
  delay(50);  
  temp_read_2 = analogRead(Temp_sens);
  delay(50);
  digitalWrite(RelaySensor, LOW);
  delay(50);
  temp_read_1 = analogRead(Temp_sens);  
  delay(50);     
  digitalWrite(RelaySensor, HIGH);
  float r1 = (1023-temp_read_1)/temp_read_1*10000;
  float r2 = (1023-temp_read_2)/temp_read_2*10000;
  float t1 = 1/(0.00261218301987081 +2.48322305591471E-05*log(r1) + 9.35934616507308E-07*pow(log(r1),3))-273.15;
  float t2 = 1/(0.00260120932091315 +2.98185218458834E-05*log(r2) + 9.02662281140301E-07*pow(log(r2),3))-273.15;

  sprintf(UDP_StateString, "%s%s",ChimeID,";tempe;t1:");
  sprintf(UDP_StateString,"%s%.2f%s%.2f%s",UDP_StateString,t1,".t2:",t2,"."); 
  Udp.beginPacket(DestinIP, localUdpPort);
  Udp.write(UDP_StateString);
  Udp.endPacket();
  Serial.println(UDP_StateString);  
}


void setup() {
  
//  ESPSerial.begin(9600); // initialize the serial com with the ESP. Note that the ESP baud must be set to 9600 (usually 115200 by default)
  Serial.begin(9600);  // initialize the serial com with the computer for debugging when Arduino is USB connected
  delay(1000); // delay to make sure the serials are set up. I changed this to 1 second from 2. Might need to be put back...

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  Udp.begin(localUdpPort);
  Serial.printf("Now sending at IP %s, UDP port %d\n", DestinIP, localUdpPort);


 // timeClient.begin();

  delay(200);

  // declade the mode of the pins that talk to the sensor boards. 
    pinMode(ploadPin, OUTPUT); 
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, INPUT);
      
    pinMode(RelaySensor, OUTPUT);
    digitalWrite(RelaySensor, HIGH);

    pinMode(RelayPower, OUTPUT);
    digitalWrite(RelayPower, HIGH);
    
    digitalWrite(clockPin, LOW); // initialize the state of the clock pin
    digitalWrite(ploadPin, HIGH); // intitialize the state of the load pin


    digitalWrite(ploadPin, LOW); // Latch the data from sensors to shift registers with low pulse
    digitalWrite(ploadPin, HIGH);
    for(int k = 0; k < NUMBER_SENSOR_BOARDS; k++)  // set the reference state of the sensors by reading the current state
    { oldChimeState[k]  = ChimeState[k]  = read_shift_regs(1);  }
    
//    timeClient.update();
//
//    String datestringtest = timeClient.getFormattedTime();
//    char datestring[50];
//    datestringtest.toCharArray(datestring, 50);
//    Serial.println(datestringtest); 
//    Serial.println(datestring);   

// event ID for time stamp is "stamp"


       sprintf(UDP_StateString, "%s%s",ChimeID,";Init.");
      // sprintf(UDP_StateString, "%s%s%s",ChimeID,datestring ,"-InitialBConnect");
       Udp.beginPacket(DestinIP, localUdpPort);
       Udp.write(UDP_StateString);
       Udp.endPacket();
   Serial.println(UDP_StateString);   
   Serial.println(" Initial connect packet sent");
   Serial.println("Each Sensor loop prints one dot");
   
}


// This the main loop with repeated sensing and writing
void loop() {

    read_temps();
    RelayCounter = 0;
    

    digitalWrite(RelayPower, LOW);
while (RelayCounter < NumSensings) {
  // do something repetitive 200 times

    Serial.print(".");   

    Change_Sensors=false;  //Reset the indicator of change in sensors to false
    digitalWrite(ploadPin, LOW); // Latch the data from sensors to shift registers 
    digitalWrite(ploadPin, HIGH);

    for(int k = 0; k < NUMBER_SENSOR_BOARDS; k++) // read the boards one by one
    {
        ChimeState[k]  = read_shift_regs(1); // read state of sensors one board at a time 
        Change_Sensors =  ((ChimeState[k] != oldChimeState[k]) || Change_Sensors); // track change in sensors 
    }
        
    if(Change_Sensors)    {  // if change, send UDP and reset reference state

       sprintf(UDP_StateString, "%s%s%s",ChimeID,";IO;");
       for(int n = 0; n < NUMBER_SENSOR_BOARDS; n++) // write the state of sensors one board at a time
       {sprintf(UDP_StateString,"%s%d%s",UDP_StateString,ChimeState[n],"."); } // adding each board to the string
       
       Udp.beginPacket(DestinIP, localUdpPort);
       Udp.write(UDP_StateString);
       Udp.endPacket();
       
       Serial.println(UDP_StateString); 

       for(int k = 0; k < NUMBER_SENSOR_BOARDS; k++){ oldChimeState[k]  = ChimeState[k]; } // set the oldChimeState to new 
    
    }
    delay(DELAY_MSEC);
      RelayCounter++;
}
  digitalWrite(RelayPower, HIGH);
  
 delay(COOLDOWN);
 
}
