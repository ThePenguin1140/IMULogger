#include <DS1302.h> //clock

#define RST 9
#define DAT 8
#define CLK 7

boolean setClock = false;
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
  
  //TODO setup IMU
  //TODO setup IR Remote
  //TODO setup various pins (LEDs)
}

//TODO add vars for logging
float x_avg = 0, x_min = 0, x_max = 0, y_avg = 0, y_min = 0, y_max = 0, z_avg = 0, z_min = 0, z_max = 0;
int seqnum = 0, count = 0;

void loop() {
  //TODO monitor IR remote for OK and # press
}

void updateWithValues(float x, float, y, float z){
	if(x<x_min) x_min = x;
	if(y<y_min) y_min = y;
	if(z<z_min) z_min = z;
	
	if(x>x_max) x_max = x;
	if(y>y_max) y_max = y;
	if(z>z_max) z_max = z;
	
	x_avg = ((x_avg * count) + x)/(count+1);	
	y_avg = ((y_avg * count) + y)/(count+1);	
	z_avg = ((z_avg * count) + z)/(count+1);
	
	count++;
}

String createTimeStampString(int year, int month, int day, int hour, int minute, int second) {
  String dateStr = String(year - 2000);
  dateStr += "-" + intToMonth(month) + "-" + String(day);
  String timeStr = String(hour) + ":" + String(minute) + ":" + String(second);
  return dateStr + " " + timeStr;
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
