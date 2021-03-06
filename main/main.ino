#include <DS1302.h> //clock
#include <IRremote.h> //IR
#include <Wire.h> //IMU
#include <SD.h> //SD

#define RST 9
#define DAT 8
#define CLK 7

#define START "FF02FD"
#define STOP "FF52AD"

#define BLUE 5
#define YELLOW 3
#define RED 2

#define IR_RECV 6

#define MMA8452_ADDRESS 0x1C // 0x1D if SA0 is high, 0x1C if low
#define OUT_X_MSB 0x01
#define XYZ_DATA_CFG 0x0E
#define WHO_AM_I 0x0D
#define CTRL_REG1 0x2A
#define GSCALE 2 // Full-scale range to +/-2, 4, or 8g.

boolean setClock = false;
IRrecv irrecv(IR_RECV);
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
  if (setClock)
  {
    Time t(2015, 10, 13, 18, 45, 10, Time::kWednesday);
    rtc.time(t);
  }

  rtc.writeProtect(false);
  rtc.halt(false);

  //init SD
  Serial.print("Initializing SD card communications...");
  if ( !SD.begin(4) ) {
    Serial.println("FAILED");
    return;
  }
  Serial.println("DONE");

  //Init Log file
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  if (SD.exists("accelbus.csv"))
  {
    SD.remove("accelbus.csv");
    Serial.println("Previous log removed.");
  }

  //TODO possibly add serial printouts here

  irrecv.enableIRIn(); // Start the IR receiver

  Wire.begin(); //Join the bus as a master

  initMMA8452(); //Test and intialize the MMA8452

  //config various pins
  Serial.print("Setting pin modes...");
  pinMode(RED, OUTPUT);
  Serial.print("...");
  pinMode(BLUE, OUTPUT);
  Serial.print("...");
  pinMode(YELLOW, OUTPUT);
  Serial.println("DONE");
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
boolean recording = false;
boolean isIMUvalid = false; // used to determine a failed IMU read

void loop() {

  // IR monitoring
  String input;
  if (irrecv.decode(&results)) {
    input = String(results.value, HEX);
    input.toUpperCase();
    irrecv.resume();
  }

  if (input == START) { //remote recording OK has been hit
    recording = true;
  }
  if (input == STOP) { //remote recording # has been hit
    recording = false;
  }

  if (recording) {
    int accelCount[3];  // Stores the 12-bit signed value
    readAccelData(accelCount);  // Read the x/y/z adc values

    // Now we'll calculate the accleration value into actual g's
    float accelG[3];  // Stores the real accel value in g's
    for (int i = 0 ; i < 3 ; i++)
    {
      // get actual g value, this depends on what GSCALE was set
      accelG[i] = (float) accelCount[i] / ((1 << 12) / (2 * GSCALE));
      //1<<12 generates 2^12 (two to the power of twelve)
    }

    updateWithValues(accelG[0], accelG[1], accelG[2]);

    if ( millis() % 2000 < 5 ) {
      String line = constructLogEntry();
      File logFile = SD.open("accelbus.csv", FILE_WRITE);
      if (logFile) {
        Serial.println(line);
        logFile.println(line);
        logFile.close();
      } else {
        Serial.println("ERROR PRINTING TO FILE");
        return;
      }
    }
  }
}



void updateWithValues(float x, float y, float z) {
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

String constructLogEntry() {
  seqnum++;
  return String(seqnum) + "," + createLogTimeStamp(rtc.time()) +
         "," + x_avg + "," + x_min + "," + x_max +
         "," + y_avg + "," + y_min + "," + y_max +
         "," + z_avg + "," + z_min + "," + z_max;
}

String createLogTimeStamp(Time t) {
  return
    String(t.yr) +
    "-" +
    makeDoubleDigit(t.mon) +
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
    return "0" + String(i);
  }
}

void readAccelData(int *destination)
{
  byte rawData[6];  // x/y/z accel register data stored here

  readRegisters(OUT_X_MSB, 6, rawData);  // Read the six raw data registers into data array

  // Loop to calculate 12-bit ADC and g value for each axis
  for (int i = 0; i < 3 ; i++)
  {
    int gCount = (rawData[i * 2] << 8) | rawData[(i * 2) + 1]; //Combine the two 8 bit registers into one 12-bit number
    gCount >>= 4; //The registers are left align, here we right align the 12-bit integer

    // If the number is negative, we have to make it so manually (no 12-bit data type)
    if (rawData[i * 2] > 0x7F)
    {
      gCount = ~gCount + 1;
      gCount *= -1;  // Transform into negative 2's complement #
    }

    destination[i] = gCount; //Record this gCount into the 3 int array
  }
}

// Initialize the MMA8452 registers
// See the many application notes for more info on setting all of these registers:
// http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=MMA8452Q
void initMMA8452()
{
  Serial.println("Initialize IMU");
  byte c = readRegister(WHO_AM_I);  // Read WHO_AM_I register
  Serial.println("Reading WHOAMI");
  if (c == 0x2A) // WHO_AM_I should always be 0x2A
  {
    Serial.println("MMA8452Q (GY-45) is online...");
  }
  else
  {
    Serial.print("Could not connect to MMA8452Q (GY-45): 0x");
    Serial.println(c, HEX);
    while (true) ; // stall if communication doesn't happen
  }
  Serial.println("Gyros in standby.");
  MMA8452Standby();  // Must be in standby to change registers

  // Set up the full scale range to 2, 4, or 8g.
  byte fsr = GSCALE;
  if (fsr > 8) fsr = 8; //Easy error check
  fsr >>= 2; // 00 = 2G, 01 = 4A, 10 = 8G
  writeRegister(XYZ_DATA_CFG, fsr);

  //The default data rate is 800Hz;
  //we don't need to modify it in this example code
  Serial.println("Gyros activated.");
  MMA8452Active();  // Set to active to start reading
}

// Sets the MMA8452 to standby mode. It must be in standby to change most register settings
void MMA8452Standby()
{
  byte c = readRegister(CTRL_REG1);
  //Clear the active bit to go into standby
  writeRegister(CTRL_REG1, c & ~(0x01));
}

// Sets the MMA8452 to active mode. Needs to be in this mode to output data
void MMA8452Active()
{
  byte c = readRegister(CTRL_REG1);
  //Set the active bit to begin detection
  writeRegister(CTRL_REG1, c | 0x01);
}

void readRegisters(byte addressToRead, int bytesToRead, byte * dest)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  Wire.endTransmission(false); //endTransmission but keep the connection active

  Wire.requestFrom(MMA8452_ADDRESS, bytesToRead); //Ask for bytes, once done, bus is released by default

  while (Wire.available() < bytesToRead); //Hang out until we get the # of bytes we expect

  for (int x = 0 ; x < bytesToRead ; x++)
    dest[x] = Wire.read();
}

// Read a single byte from addressToRead and return it as a byte
byte readRegister(byte addressToRead)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  Wire.endTransmission(false); //endTransmission but keep the connection active

  Wire.requestFrom(MMA8452_ADDRESS, 1); //Ask for 1 byte, once done, bus is released by default

  isIMUvalid = false;
  byte result = 0;
  long timeOutMicros = micros(); // microsecond clock
  while (!Wire.available() && (micros() - timeOutMicros < 2000)) ; //Wait for the data to come back
  if (Wire.available())
  {
    result = Wire.read(); //save this byte for return
    isIMUvalid = true; // good read!
  }
  return result; // this will be 0 and valid flag false if bad read
}

// Writes a single byte (dataToWrite) into addressToWrite
void writeRegister(byte addressToWrite, byte dataToWrite)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToWrite);
  Wire.write(dataToWrite);
  Wire.endTransmission(); //Stop transmitting
}
