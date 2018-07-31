
#include <Servo.h>


/*-----( Declare Variables )-----*/
byte addresses[][6] = {"remot", "pilot"}; // These will be the names of the "Pipes"
unsigned long engineCommandCounter = 0;

const byte DEBUG_LEVEL_NONE = 0;
const byte DEBUG_LEVEL_INFO = 1;
const byte DEBUG_LEVEL_VERBOSE = 2;

const byte CUR_DEBUG_LEVEL = 2;



Servo servo_1, servo_2, servo_3, servo_4 , servo_5, ESC;



//unsigned long receivedPackCount = 0;

void setup()   /****** SETUP: RUNS ONCE ******/
{ Serial3.begin(38400);
  Serial.begin(115200);   // MUST reset the Serial Monitor to 115200 (lower right of window )
  //printf_begin();
  /*-----( Set up servos )-----*/
  ESC.attach(9);
  servo_1.attach(2);
  servo_2.attach(3);
  servo_3.attach(4);
  servo_4.attach(5);
  servo_5.attach(6);

  ESC.write (0);
  /*---------------------------*/

  pinMode( A0, INPUT);
}//--(end setup )---


byte serialBuffer[64];
int i = 0;
int msgCnt = 0;
unsigned long lastWrite;
void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{

  if (Serial3.available() > 0) //radio.available())
  {
    i = 0;
    while ( Serial3.available() > 0  && i < 64)
    {
      serialBuffer[i] = Serial3.read();
      //Serial.print(String((int)(serialBuffer[i])) + "  ");
      //delay(1);
      i++;
    }
    

    // Serial.println();

    /*

    */
    // A_A

    // ***
    WriteToServo();


    /*
    */
  } // END radio available
  else
  {
    // Routine for when radio has failed
    // or no command received for some time
    if ( millis() - engineCommandCounter > 3000)
    {
      // No commands for 5 seconds
      EmergencyRoutine();

    }
  }
  if ( millis() - lastWrite > 1000)
  {
    // No commands for 5 seconds
    // EmergencyRoutine();
    lastWrite = millis();
    Serial.println(msgCnt);
    msgCnt = 0;
  }
}//--(end main loop )---

/*-----( Declare User-written Functions )-----*/
int commandpos = 0;
void WriteToServo()
{
  for ( commandpos = 0; commandpos < 5; commandpos++)
  {
    if ( serialBuffer[commandpos] == 244)
    {

      ESC.write ((int)serialBuffer[commandpos + 1]);
      servo_1.write ((int)serialBuffer[commandpos + 2]);
      servo_2.write ((int)serialBuffer[commandpos + 3]);
      servo_3.write ((int)serialBuffer[commandpos + 4]);
      servo_4.write ((int)serialBuffer[commandpos + 5]);
      //servo_5.write ((int)myData.servo5);
      engineCommandCounter = millis();
      msgCnt++;
      if (CUR_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE)
      {
        Serial.print("ESC : " + String((int)serialBuffer[commandpos + 1]));
        Serial.print(" servo1 : " + String((int)serialBuffer[commandpos + 2]));
        Serial.print(" servo2 : " + String((int)serialBuffer[commandpos + 3]));
        Serial.print(" servo3 : " + String((int)serialBuffer[commandpos + 4]));
        Serial.println(" servo4 : " + String((int)serialBuffer[commandpos + 5]));
      }
      break;
    }
    // save last commands time

  }
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


