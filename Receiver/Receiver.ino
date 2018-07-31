
#include <Servo.h>


/*-----( Declare Variables )-----*/

unsigned long engineCommandCounter = 0;

const byte DEBUG_LEVEL_NONE = 0;
const byte DEBUG_LEVEL_INFO = 1;
const byte DEBUG_LEVEL_VERBOSE = 2;

const byte CUR_DEBUG_LEVEL =  DEBUG_LEVEL_NONE ;

const int NUMBER_OF_CHANNELS = 6;

Servo servo_1, servo_2, servo_3, servo_4 , servo_5, ESC;



//unsigned long receivedPackCount = 0;

void setup()   /****** SETUP: RUNS ONCE ******/
{
  //Serial3.begin(115200);
  Serial.begin(115200);   // MUST reset the Serial Monitor to 115200 (lower right of window )
  //printf_begin();
  /*-----( Set up servos )-----*/
  ESC.attach(9);
  servo_1.attach(2);
  servo_2.attach(3);
  servo_3.attach(4);
  servo_4.attach(5);
  servo_5.attach(6);

  ESC.write (1);
  /*---------------------------*/

  //pinMode( A0, INPUT);
}//--(end setup )---

unsigned long byteCounter = 0;
unsigned long validMsgCounter = 0;
byte serialBuffer[10];
int i = 6;
void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  ReadSerial();


  // A_A

  // ***

  if ( millis() - engineCommandCounter > 1000)
  {
    engineCommandCounter = millis();
    
    Serial.println("total msgs : " + String(byteCounter / 6));
    Serial.println("total bytes : " + String(byteCounter) );
    Serial.println("valid msgs per second : " + String(validMsgCounter) );
    validMsgCounter = 0;
    //msgCounter = 0;


  }

  // Routine for when radio has failed
  // or no command received for some time
  if ( millis() - engineCommandCounter > 2000)
  {
    // No commands for 5 seconds
    EmergencyRoutine();

  }

}//--(end main loop )---

/*-----( Declare User-written Functions )-----*/
void WriteToServo()
{
  ESC.write ((int)serialBuffer[1]);
  servo_1.write ((int)serialBuffer[2]);
  servo_2.write ((int)serialBuffer[3]);
  servo_3.write ((int)serialBuffer[4]);
  servo_4.write ((int)serialBuffer[5]);
  //servo_5.write ((int)myData.servo5);

  // save last commands time
  engineCommandCounter = millis();
}


void EmergencyRoutine()
{
  ESC.write (0);
  // TODO :
  // autopilot needs to level the aircraft and prepare for crash landing
  if (CUR_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE)
  {
    Serial.println("EMERGENCY!");
  }
}
//*********( THE END )***********


void ReadSerial()
{
  if (Serial.available() > 0) //radio.available())
  {

    // bool commandStarted = false;
    while (Serial.available() > 0)
    {
      byte ntemp = Serial.read();
      byteCounter++;
      if ( ntemp == 244 )
      {
        if (i < NUMBER_OF_CHANNELS+1)
           if (CUR_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE)
              Serial.println("Some Bytes missed");

        i = 0;
        serialBuffer[0] = ntemp;
      }
      else
      {
        i++;
        //Serial.println ( " i : " + String(i));
        if (i < NUMBER_OF_CHANNELS+1)
        {
          serialBuffer[i] = ntemp;
        }
        else if (i == NUMBER_OF_CHANNELS+1)
        {
          serialBuffer[i] = ntemp;
          if ( checksum())
          {
            validMsgCounter++;
            // write only if all bytes are valid
            //WriteToServo();
            //Serial.println("Checks OUT");
            //Serial.println(" << CheckSum valid >> ");
            if (CUR_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE)
            {
            
              Serial.print("ESC : " + String((int)serialBuffer[1]));
              Serial.print("servo1 : " + String((int)serialBuffer[2]));
              Serial.print("servo2 : " + String((int)serialBuffer[3]));
              Serial.print("servo3 : " + String((int)serialBuffer[4]));
              Serial.println("servo4 : " + String((int)serialBuffer[5]));
            }
          }
          else if (CUR_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE)
            Serial.println("CheckSum Invalid");
        }
        else
        {
          if (CUR_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE)
            Serial.println("Invalid Bytes");
        }
      }
    }

    //Serial.println("READING IN WHILE");

    //return;
    
  }
}

bool checksum()
{
  byte sum = 0;
  for ( int i = 1; i < NUMBER_OF_CHANNELS+1; i++)
  {
    sum = sum ^ serialBuffer[i];
  }
  return ( sum == serialBuffer[NUMBER_OF_CHANNELS+1]);
}
