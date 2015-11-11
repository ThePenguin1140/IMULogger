#include <DS1302.h> //clock

#define RST 9
#define DAT 8
#define CLK 7

boolean setClock = false;
DS1302 rtc(RST, DAT, CLK);

void setup() {
  Serial.begin(9600);
  
  //set clock if selected
  rtc.halt(false);
  if (setClock) {
    Serial.print("Setting Clock...");
    rtc.writeProtect(false);
    Time t(2015, 10, 9, 12, 00, 00, Time::kFriday);
    rtc.time(t);
    Serial.println("DONE");
  }
  
  //TODO setup IMU
  //TODO setup IR Remote
  //TODO setup various pins (LEDs)
}

//TODO add vars for logging

void loop() {
  //TODO monitor IR remote for OK and # press
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
