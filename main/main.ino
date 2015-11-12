#include <DS1302.h> //clock
#include "IRremote.h" //IR

//TODO add remaining pin definitions

#define RST 9
#define DAT 8
#define CLK 7
#define RECV_PIN 2
#define START "FF02FD"
#define STOP "FF52AD"

boolean setClock = false;
IRrecv irrecv(RECV_PIN);
decode_results results;
DS1302 rtc(RST, DAT, CLK);

void setup() {
  Serial.begin(9600);

  //configure ethernet shield
  Serial.print("Disable Ethernet...");
  pinMode(10, OUTPUT);
  Serial.print("...");
  digitalWrite(10, HIGH);
  Serial.println("DONE");

  //set clock if selected
  rtc.halt(false);
  if (setClock) {
    Serial.print("Setting Clock...");
    rtc.writeProtect(false);
    Time t(2015, 10, 9, 12, 00, 00, Time::kFriday);
    rtc.time(t);
    Serial.println("DONE");
  }

  //init SD
  Serial.print("Initializing SD card communications...");
  if ( !SD.begin(4) ) {
    Serial.println("FAILED");
    return;
  }
  Serial.println("DONE");

  //Init Log file
  Serial.print("Searching for log file...");
  //TODO figure out how to use fileName as a parameter
  if ( SD.exists("accelbus.csv") ) {
    Serial.println("FOUND");
    Serial.print("Removing file...");
    SD.remove("accelbus.csv");
    Serial.println("DONE");
  } else {
    Serial.println("DONE");
  }

  Serial.print("Creating new Log File...");
  File logFile = SD.open("accelbus.csv", FILE_WRITE);
  Serial.print("...");
  if (logFile) {
    Serial.println("DONE");
    logFile.close();
  } else {
    Serial.println("ERROR");
  }

  //TODO setup IMU
  
  irrecv.enableIRIn(); // Start the IR receiver
  
  //TODO setup various pins (LEDs)
}

float x_avg = 0;
float x_min = 0;
float x_max = 0;
float y_avg = 0;
float y_min = 0;
float y_max = 0;
float z_avg = 0;
float z_min = 0;
float z_max = 0;
int seqnum = 0;
int count = 0;

void loop() {

  // IR monitoring
  String input;
  if(irrecv.decode(&results)){
	input = String(results.value, HEX);
	input.toUpperCase();
	irrecv.resume(); 
  }
  
  if(input==START){ //remote button OK has been hit
  
  }
  if(input==STOP){ //remote button # has been hit
  
  }
  

  //TODO write general logic as to when to record
  //make a if/else that checks the above var and then
  //calls updateWithValues every loop with the new IMU values

  //TODO write general logic of writing to SD every 2 seconds
  //make an if statment that will write to the SD every 2 seconds
  //if the system is recording.
}

//TODO write function that writes to SD

void updateWithValues(float x, float, y, float z) {
  if (x < x_min) x_min = x;
  if (y < y_min) y_min = y;
  if (z < z_min) z_min = z;

  if (x > x_max) x_max = x;
  if (y > y_max) y_max = y;
  if (z > z_max) z_max = z;

  x_avg = ((x_avg * count) + x) / (count + 1);
  y_avg = ((y_avg * count) + y) / (count + 1);
  z_avg = ((z_avg * count) + z) / (count + 1);

  count++;
}

String createLogTimeStamp(Time t) {
  return
    String(t.yr) +
    "-" +
    intToMonth(t.mon) +
    "-" +
    makeDoubleDigit(t.date) +
    " " +
    makeDoubleDigit(t.hr) +
    ":" +
    makeDoubleDigit(t.min) +
    ":" +
    makeDoubleDigit(t.sec);
}

String makeDoubleDigit(int i) {
  if (i > 9) {
    return String(i);
  } else {
    String d = "0" + String(i);
    return d;
  }
}

String intToMonth(int m) {
  switch (m) {
    case 1:
      return "JAN";
    case 2:
      return "FEB";
    case 3:
      return "MAR";
    case 4:
      return "APR";
    case 5:
      return "MAY";
    case 6:
      return "JUN";
    case 7:
      return "JUL";
    case 8:
      return "AUG";
    case 9:
      return "SEP";
    case 10:
      return "OCT";
    case 11:
      return "NOV";
    case 12:
      return "DEC";
    default:
      return "ERR";
  }
}
