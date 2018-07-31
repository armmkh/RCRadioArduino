/*
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

*/
#include <SPI.h>
#include <SD.h>


// #define DEBUG_MODE digitalRead(22)   // pin 22 HIGH means we are in debug mode
boolean DEBUG_MODE = 0;
boolean logPinStatus = false;

File sdLogFile;
String logFileName = String();

long logTimer = 0;

const int NUMBER_OF_CHANNELS = 5;
const uint8_t commandChar = 244;
uint8_t data[NUMBER_OF_CHANNELS];
uint8_t inputs[4]; // RAW analog inputs

uint8_t escmin = 1 ;
uint8_t escmax = 180;
uint8_t max1 = 140;
uint8_t max2 = 160;
uint8_t max3 = 145;
uint8_t max4 = 155;
uint8_t min1 = 40;
uint8_t min2 = 60;
uint8_t min3 = 45;
uint8_t min4 = 25;


uint8_t ail;
uint8_t elev;
int ailDeflect;


int i = 0;


int offsets[NUMBER_OF_CHANNELS];

void setup() {

  pinMode(8, INPUT);
  offsets[1] = -27;
  offsets[2] = -3;
  offsets[3] = 22;
  offsets[4] = 10;

  // Open serial communications and wait for port to open:
  Serial.begin(9600);    // usb debug
  Serial2.begin(9600);   // Bluetooth to android
  Serial3.begin(115200); // Zigbee, Aircraft Comm
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  if ( DEBUG_MODE)
    Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    if ( DEBUG_MODE)
      Serial.println("initialization failed!");
    return;
  }
  else if ( DEBUG_MODE)
    Serial.println("SD initialization done.");

  if ( DEBUG_MODE)
    Serial.println("End of SETUP");
  i = 0;

  //generateFiles();
}

void loop() {
  // First get all 4 analog channels
  inputs[0] = map( analogRead(A6) , 0, 1023, escmax, escmin);  // ESC Stick
  inputs[1] = map( analogRead(A4) , 0, 1023, min1, max1);      // Rudder
  inputs[2] = map( analogRead(A7) , 0, 1023, min2, max2);      // Elevator
  inputs[3] = map( analogRead(A5) , 0, 1023, min3, max3);      // Aileron

  if ( DEBUG_MODE)
  {
    Serial.print("RAW inputs: ");
    Serial.println("ESC : " + String (inputs[0]) + " Rudder : " + String(inputs[1])
                   + " Elev : " + String(inputs[2]) + " Aileron : " + String(inputs[3])  );
  }


  // **** we have all analog inputs, lets map them **** //
  //ESC out
  data[0] = inputs[0]; // mapping is enough

  elev = map(inputs[2] , min2, max2, max2, min2);

  // elevon
  ail = map(inputs[3] , min1, max1, min1, max1);
  ailDeflect = (int)ail - (int)(max1 + min1 ) / 2;
  data[2] = elev;
  data[1] = inputs[2];
  //if (ailDeflect >= 0)
  data[1] += ailDeflect;
  //else
  data[2]   += ailDeflect;
  if ( data[1] > max1 + 30) data[1] = max1 + 30;
  if ( data[1] < min1 + 20) data[1] = min1 + 20 ;
  if ( data[2] > max2) data[2] = max2;
  if ( data[2] < min2) data[2] = min2;

  data[3] = 0;
  data[4] = 0;


  // apply offsets
  data[1] += offsets[1];
  data[2] += offsets[2];
  data[3] += offsets[3];
  data[4] += offsets[4];

  data[5] = 0b00000000;  // this byte is reserved for special commands, never change MSB to 1
  // every bit in this byte will represent a special key being pressed or released

  SendData();


  if (digitalRead( 8 ) == HIGH && false )  // disabled code for now
  {
    if (logPinStatus)
    {
      //Continue logging on same file
      if ( DEBUG_MODE)
        Serial.println("continuing " + logFileName);


      // if the file is available, write to it:
      if (sdLogFile)
      {
        if ( DEBUG_MODE)
          Serial.println("Loging " + String(logPinStatus));
        sdLogFile.print( String( millis()) + " ");
        for ( i = 0; i < NUMBER_OF_CHANNELS ; i++)
        {
          // Send Channel Data and calculate checksum
          sdLogFile.print( String( int (data[i]) ) + " ");
        }
        sdLogFile.println();

      }

    }
    else
    {
      // Create new file
      if ( DEBUG_MODE)
        Serial.println("Creating new File" + String(logPinStatus));
      for ( i = 0; i < 32000 ; i++)
      {
        logFileName = "l" + String(i) + ".txt" ;
        if (!SD.exists(logFileName))
          break;
        if ( DEBUG_MODE)
          Serial.println("File : " + logFileName + "exists");
      }
      char charFileName[logFileName.length() + 1];
      logFileName.toCharArray(charFileName, sizeof(charFileName));
      sdLogFile =  SD.open(charFileName, FILE_WRITE);
      i = 0;
      logPinStatus =   digitalRead(8);
    }

  }
  else
  {
    if (logPinStatus)
    {
      sdLogFile.close();
      //Serial.println("Closing" );
      logPinStatus =   digitalRead(8);
    }
  }


  SendToAndroid();
  if ( DEBUG_MODE)
  {
    if ( Serial2.available())
    {
      Serial.write(Serial2.read());
    }
  }
  delay(2);


}



void SendData()
{
  uint8_t checksum = 0;
  int i ;

  // Send Command start byte
  Serial3.write(commandChar);

  for ( i = 0; i < NUMBER_OF_CHANNELS ; i++)
  {
    // Send Channel Data and calculate checksum
    Serial3.write( data[i]);
    checksum =  checksum ^ data[i];

  }

  // Send Checksum
  Serial3.write(checksum);
  if ( DEBUG_MODE)
  {
    Serial.print("Sending : ");

    for ( i = 0; i < NUMBER_OF_CHANNELS ; i++)
    {
      // Send Channel Data and calculate checksum
      Serial.print( String( int (data[i]) ) );
    }
    Serial.println( String( int (checksum )) );
  }

}

void SendToAndroid()
{
  uint8_t checksum = 0;
  if (DEBUG_MODE)
    Serial.println ( "sending to android");
  Serial2.write(commandChar);

  for ( i = 0; i < 4 ; i++)
  {
    Serial2.write(inputs[i]);
    checksum =  checksum ^ inputs[i];

  }
  for ( i = 0; i < NUMBER_OF_CHANNELS; i++)
  {
    Serial2.write(data[i]);
    checksum =  checksum ^ data[i];

  }

  //Serial2.write(checksum);
  Serial2.write((char)245);
}





