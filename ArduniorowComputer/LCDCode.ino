//Code for use with a 1602 display with keypad.
//
//-------------------------------------------------------------------
//               reference values for each key on the  keypad:
static int DEFAULT_KEY_PIN = 0; 
static int UPKEY_ARV = 144; 
static int DOWNKEY_ARV = 329;
static int LEFTKEY_ARV = 505;
static int RIGHTKEY_ARV = 0;
static int SELKEY_ARV = 721;
static int NOKEY_ARV = 1023;
static int _threshold = 50;

//-------------------------------------------------------------------
//               custom Character definitions
#define LCDSlowDown 0
#define LCDSpeedUp 1
#define LCDJustFine 3
//-------------------------------------------------------------------
//               KEY index definitions
#define NO_KEY 0
#define UP_KEY 3
#define DOWN_KEY 4
#define LEFT_KEY 2
#define RIGHT_KEY 5
#define SELECT_KEY 1

#ifdef UseLCD
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

byte screenstep=0;                           // int - which part of the display to draw next.

void lcdSetup()
{
    lcd.begin(16, 2);  
    lcd.clear();
}
void writeTimeLeft(long totalSeconds)
{
    lcd.clear();
    lcd.print("Interval ");
    lcd.print(intervals);
    lcd.setCursor(0,1);
    int minutes = totalSeconds/60;
    if(minutes <10) lcd.print ("0");
    lcd.print(minutes);
    int seconds = totalSeconds - (minutes*60);
}

void writeNextScreen()
{
        if(sessionType == DRAGFACTOR)
        {
          lcd.setCursor(0,1);
          lcd.print(k*1000000,0);
          lcd.print("  ");
          return;//no need for other screen stuff.
        }else if (sessionType == RPM)
        {
          lcd.clear();
          lcd.print("R");
          lcd.print(getRpm(0));
          lcd.setCursor(0,1);
          lcd.print("M:");
          int rpms2[3] = {getRpm(0), getRpm(-1), getRpm(-2)};
          lcd.print(median(rpms2,5));
          int x = getKey();
          if(x == SELECT_KEY)
          {
            dumprpms();
          }
          return;//no need for other screen stuff.
        }
        else if(sessionType == WATTS)
        {
          lcd.clear();
          lcd.print(power);
        }
  //Display Format:
  // 2:16  r3.1SPM:35
  //5000m     10:00
  int timemins, timeseconds, splitmin, splits, dm;
  screenstep++;
  switch(screenstep)
  {//only write a little bit to the screen to save time.
    case 0:
    //lcd.clear();
      lcd.setCursor(0,0);
    break;
    case 1:
    {
      splitmin = (int)(split/60);
      int splits = (int)(((split-splitmin*60)));
        if(splitmin < 10)
        {//only display the split if it is less than 10 mins per 500m
            lcd.print(getSplitString());
            //lcd 0->5, 0  used
        }
    }
    break;
    case 2:
      //lcd 10->16,0 used
        lcd.setCursor(12,0);
        lcd.print("S:");
        lcd.print(spm);
        lcd.print("  ");
#ifdef debug
    Serial.print("\tSPM:\t");
    Serial.print(spm);
#endif
    break;
    case 3:
       lcd.setCursor(0,1);
       //Distance in meters:
      if(sessionType == DISTANCE)
      {
        long distanceleft = targetDistance - distancem;
        if(distanceleft <1000) lcd.print("0");
        if(distanceleft <100) lcd.print("0");
        if(distanceleft <10) lcd.print("0");
        lcd.print((int)distanceleft);
        lcd.print("m");
      }
      else
      {
        if(distancem <1000) lcd.print("0");
        if(distancem <100) lcd.print("0");
        if(distancem <10) lcd.print("0");
        lcd.print((int)distancem);
        lcd.print("m");
      }
      
      #ifdef debug
      Serial.print("\tDistance:\t");
      Serial.print(distancem);
      Serial.print("m");
      #endif
      //lcd 0->5, 1 used
      //Drag factor
      /*lcd.print("D:");
      lcd.println(k*1000000);*/
    break;
    case 4:
      //lcd 11->16 used
        lcd.setCursor(11,1);
        lcd.print(getTime());      
      #ifdef debug
        Serial.print("\tTime:\t");
        Serial.print(getTime());
      #endif
      break;
    case 5://next lime
      //lcd 6->9 , 0
       lcd.setCursor(7,0);
       if(RecoveryToDriveRatio > 2.1)
       {
        lcd.print((char)(int)LCDSpeedUp);
       }
       else if (RecoveryToDriveRatio < 1.9)
        {
          lcd.print((char)(int)LCDSlowDown);
        }
        else
        {
          lcd.print((char)(int)LCDJustFine);
        }       
       //lcd.print(RecoveryToDriveRatio,1);
    #ifdef debug
       Serial.print("\tDrag factor:\t");
       Serial.print(k*1000000);
    #endif
      break;
   case 6:
   #ifdef debug
      Serial.print("\tDrive angularve: ");
      Serial.print(driveAngularVelocity);

      Serial.print("\tRPM:\t");
      {
        int rpms1[5] = {getRpm(0), getRpm(-1), getRpm(-2),getRpm(-3),getRpm(-4)};
        Serial.print(median(rpms1,5));
      }
      Serial.print("\tPeakrpm:\t");
      Serial.println(peakrpm);
   #endif 
    break;

    default:
      screenstep = -1;//will clear next time, as 1 is added and if 0 will be cleared.
  }    
}


//Current selection (Distance, Time)
void startMenu()
{
  menuType();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Begin Rowing");
}

void menuType()
{
  int c = NO_KEY;
  while(getKey() == SELECT_KEY) delay(300);
  writeType();
  while(c != SELECT_KEY)
  {
    c = getKey();
    if(c == DOWN_KEY)
    {
      sessionType ++;
      writeType();
      delay(500);
    }
    else if (c == UP_KEY)
    {
      sessionType --;
      writeType();
      delay(500);
    }
  }
  while(c == SELECT_KEY) c = getKey();//wait until select is unpressed.
  delay(200);
  switch(sessionType)
  {
    case DISTANCE:
      menuSelectDistance();
      break;
    case TIME: 
      targetSeconds = menuSelectTime(targetSeconds);
      break;
    case INTERVAL:
     targetSeconds = menuSelectTime(targetSeconds);
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Rest");
     intervalSeconds = menuSelectTime(intervalSeconds);
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Repeats");
     numIntervals = menuSelectNumber(numIntervals);
      break;
    case SETTINGS:
      menuSettings();    
      menuType();  
    default:
      //just row with that as a setting (rpm / drag factor etc..)
    break;
      
  }
}

long menuSelectNumber(long initialValue)
{
  int c = NO_KEY;
  while(getKey() == SELECT_KEY) delay(300);
  c = getKey();
  printNumber(initialValue);
  while(c != SELECT_KEY)
  {
    c = getKey();
      if(c == DOWN_KEY)
    {
      initialValue--;
      printNumber(initialValue);
      delay(500);
    }
    else if (c == UP_KEY)
    {
      initialValue ++;
      printNumber(initialValue);
      delay(500);
    }
  }
  return initialValue;
}

void printNumber(long num)
{
  lcd.setCursor(0,1);
  lcd.print(num);
  lcd.print(" ");
}

void menuSettings()
{
  int c = NO_KEY;
  while(getKey() == SELECT_KEY) delay(300);
  writeSettingsMenu();
  while(c != SELECT_KEY)
  {
    c = getKey();
    if(c == DOWN_KEY)
    {
      sessionType ++;
      writeSettingsMenu();
      delay(500);
    }
    else if (c == UP_KEY)
    {
      sessionType --;
      writeSettingsMenu();
      delay(500);
    }
  }
  while(c == SELECT_KEY) c = getKey();//wait until select is unpressed.
  delay(200);
  switch(sessionType)
  {
    case BACKLIGHT:
      menuSelectBacklight();
      menuSettings();
      break;
    case ERGTYPE:
      menuSelectErgType();
      menuSettings();
      break;
    case BOATTYPE:
      menuSelectBoatType();
      menuSettings();
      break;
    case POWEROFF:
      menuSleep();
      menuSettings();
      break;
    default:
      //back to other menu
    break;
      
  }
}

void menuSelectBoatType()
{
    menuDisplayBoatType();
  int c = NO_KEY;
  while(c!=SELECT_KEY)
  {
    c = getKey();
    if(c==UP_KEY) 
    {
      boatType ++;
      if(boatType > BOAT1) boatType = BOAT4;
      menuDisplayErgType();
      delay(500);
    }
    if(c==DOWN_KEY) 
    {
      boatType --;
      if(boatType < ERGTYPEVFIT) ergType = ERGTYPEC2;
      menuDisplayErgType();
      delay(500);
    }
    setBoatType(boatType);
  }
}

void menuDisplayBoatType()
{
  lcd.setCursor(0,1);
  switch(boatType)
  {
    case BOAT1:
      lcd.print("Single  ");
      break;
    case BOAT4:
      lcd.print("Four    ");
      break;
    case BOAT8:
      lcd.print("Eight   ");
      break;
  }
}

void writeSettingsMenu()
{
    lcd.clear();
    while(getKey() == SELECT_KEY) delay(300);
    if(sessionType <= SETTINGS) sessionType = BACK;
    if(sessionType > BACK) sessionType = BACKLIGHT;
    switch(sessionType)
    {
      case BACKLIGHT:
        menuDisplay("Back Light");
        break;
      case ERGTYPE:
        menuDisplay("Erg Type");
        break;
      case BOATTYPE:
        menuDisplay("Boat Type");
        break;
      case WEIGHT:
        menuDisplay("Weight");
        break;
      case POWEROFF:
        menuDisplay("Sleep");
        break;
      case BACK:
        menuDisplay("Back");
        break;
    }
}

void writeType()
{
  lcd.clear();
  //rollback around.
  if(sessionType <=-1) sessionType = SETTINGS;
  if(sessionType > SETTINGS) sessionType = JUST_ROW;
    switch(sessionType)
    {
      case DISTANCE:
        menuDisplay("Distance");
      break;
      case TIME:
        menuDisplay("Time");
      break;
      case INTERVAL:
        menuDisplay("Interval");
        break;
      case DRAGFACTOR:
        menuDisplay("Drag Factor");
        break;
      case RPM:
        menuDisplay("RPM");
        break;
      case SETTINGS:
        menuDisplay("Settings");
        break;

      case WATTS:
        menuDisplay("Watts");
        break;
      default:
        sessionType = JUST_ROW;
        menuDisplay("Just Row");
        break;
    }
}

void menuSelectErgType()
{
  menuDisplayErgType();
  int c = NO_KEY;
  while(c!=SELECT_KEY)
  {
    c = getKey();
    if(c==UP_KEY) 
    {
      ergType ++;
      if(ergType > ERGTYPEC2) ergType = ERGTYPEVFIT;
      menuDisplayErgType();
      delay(500);
    }
    if(c==DOWN_KEY) 
    {
      ergType --;
      if(ergType < ERGTYPEVFIT) ergType = ERGTYPEC2;
      menuDisplayErgType();
      delay(500);
    }
    setErgType(ergType);
  }
}

void menuDisplayErgType()
{
  lcd.setCursor(0,1);
  switch(ergType)
  {
    case ERGTYPEC2:
      lcd.print("Concept 2");
      break;
    case ERGTYPEVFIT:
      lcd.print("V-Fit    ");
      break;
  }
}

//go to sleep
void menuSleep()
{
  sleep_enable();
  digitalWrite(10,LOW);
  attachInterrupt(0, pin2_isr, LOW);
  /* 0, 1, or many lines of code here */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_bod_disable();
  sei();
  sleep_cpu();
  /* wake up here */
  sleep_disable();
}

//interrupt sleep routine
void pin2_isr()
{
  sleep_disable();
  detachInterrupt(0);
 // pin2_interrupt_flag = 1;
}

void menuSelectBacklight()
{
  pinMode(10,OUTPUT);
  int state = HIGH;
  digitalWrite(10,state);
  showBacklightState(state);
  int c = NO_KEY;
  while(c!=SELECT_KEY)
  {
    c = getKey();
    if(c==UP_KEY || c==DOWN_KEY)
    {
      if(state == LOW) 
      {
        state = HIGH;
      }
      else
      {
        state = LOW;
      }
      showBacklightState(state);
      digitalWrite(10,state);
      delay(500);
    }
  }
}

void showBacklightState(int state)
{
  lcd.setCursor(0,1);
  if(state == LOW)
  {
      lcd.print("Off");
  }
  else
  {
      lcd.print("On ");
  }
}

void menuSelectDistance()
{
  int c = NO_KEY;
  long increment = 1000;
  writeCurrentDistanceamount(increment);
  delay(600);
  while(c!= SELECT_KEY)
  {
    c = getKey();
    if(c==UP_KEY)
    {
      targetDistance += increment;
      if(targetDistance < 0) targetDistance = 0;
      writeCurrentDistanceamount(increment);
    }
    else if(c== DOWN_KEY)
    {
      targetDistance -= increment;
      if(targetDistance < 0) targetDistance = 0;
      writeCurrentDistanceamount(increment);
    }
    else if(c== RIGHT_KEY)
    {
      increment /=10;
      if(increment == 0) increment = 1;
      writeCurrentDistanceamount(increment);
    }
    else if(c==LEFT_KEY)
    {
      increment*=10;
      if(increment >= 10000) increment = 10000;
      writeCurrentDistanceamount(increment);
    }
  }
}

void writeCurrentDistanceamount(int increment)
{
    lcd.setCursor(0,1);
    if(targetDistance < 10000) lcd.print("0");
    if(targetDistance < 1000) lcd.print("0");
    if(targetDistance < 100) lcd.print("0");
    if(targetDistance < 10) lcd.print("0");
    lcd.print(targetDistance);
    switch(increment)
    {
       case 1:
        lcd.setCursor(4,1);
        break;
      case 10:
        lcd.setCursor(3,1);
        break;
      case 100:
        lcd.setCursor(2,1);
        break;
      case 1000:
        lcd.setCursor(1,1);
        break;
      case 10000:
        lcd.setCursor(0,1);
        break;
       default:
        lcd.setCursor(1,0);
        lcd.print(increment);
       break;
    }
    lcd.cursor();
    delay(200);
}

long menuSelectTime(long initialSeconds)
{
  int charpos = 3;
  //charpos is the current selected character on the display, 
  //0 = 10s of hours, 1 = hours, 3 = tens of minutes, 4 = minutes, 6 = tens of seconds, 7 = seconds.
  writeTargetTime(charpos, initialSeconds);
  int c = getKey();
  while(c == SELECT_KEY) 
  {
    c = getKey();
    delay(200);
  }
  while(c!= SELECT_KEY)
  {
    c = getKey();
    long incrementseconds = 0;
    switch (charpos)
    {
      case 0://10s of hours.
        incrementseconds = 36000;
        break;
      case 1://hours
        incrementseconds = 3600;
        break;
      case 3://ten mins
        incrementseconds = 600;
        break;
      case 4://mins
        incrementseconds = 60;
        break;
      case 6://ten seconds
        incrementseconds = 10;
        break;
      case 7:
        incrementseconds = 1;
        break;
      default:
        charpos = 0;
        incrementseconds= 36000;
    }
    if(c==UP_KEY)
    {
      initialSeconds += incrementseconds;
      delay(500);
      writeTargetTime(charpos, initialSeconds);
    }
    else if(c== DOWN_KEY)
    {
      initialSeconds -= incrementseconds;
      if(targetSeconds < 0) 
      {
        targetSeconds = 0;
      }
      delay(500);
      writeTargetTime(charpos, initialSeconds);
    }
    else if (c== RIGHT_KEY)
    {
      charpos ++;
      if(charpos ==2 || charpos == 5) charpos ++;
      delay(500);
      writeTargetTime(charpos, initialSeconds);
    }
    else if (c == LEFT_KEY)
    {
      charpos --;
      if(charpos ==2 || charpos == 5) charpos --;
      delay(500);
      writeTargetTime(charpos, initialSeconds);
    }
  }
  return initialSeconds;
}

void writeTargetTime(int charpos, long numSeconds)
{
  int targetHours= numSeconds /60/60;
    lcd.setCursor(0,1);
    if(targetHours < 10) lcd.print("0");
    lcd.print(targetHours);
    lcd.print(":");
    int targetMins = numSeconds /60 - (targetHours *60);
    if(targetMins < 10) lcd.print("0");
    lcd.print(targetMins);
    lcd.print(":");
    int Seconds = numSeconds - (targetMins*60) - (targetHours*60*60);;
    if(Seconds < 10) lcd.print("0");
    lcd.print(Seconds);
    lcd.setCursor(charpos,1);
    lcd.cursor();
}

int getKey()
{
  int _curInput = analogRead(0);
  //Serial.println(_curInput);
  int _curKey;
  if (_curInput > UPKEY_ARV - _threshold && _curInput < UPKEY_ARV + _threshold ) _curKey = UP_KEY;
      else if (_curInput > DOWNKEY_ARV - _threshold && _curInput < DOWNKEY_ARV + _threshold ) _curKey = DOWN_KEY;
      else if (_curInput > RIGHTKEY_ARV - _threshold && _curInput < RIGHTKEY_ARV + _threshold ) _curKey = RIGHT_KEY;
      else if (_curInput > LEFTKEY_ARV - _threshold && _curInput < LEFTKEY_ARV + _threshold ) _curKey = LEFT_KEY;
      else if (_curInput > SELKEY_ARV - _threshold && _curInput < SELKEY_ARV + _threshold ) _curKey = SELECT_KEY;
      else _curKey = NO_KEY;
  return _curKey;
  delay(100);
}

//

void menuDisplay(char* text)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(text);
}

void graphics() {
  byte SlowDown[8] = {
                    B00100,
                    B00100,
                    B10101,
                    B01110,
                    B00100,
                    B00000,
                    B00000,
                    B00000
                    };
  byte SpeedUp[8] = {
                    B00100,
                    B01110,
                    B10101,
                    B00100,
                    B00100,
                    B00000,
                    B00000,
                    B00000
                    };
  byte JustFine[8] = {
                    B00000,
                    B00100,
                    B00100,
                    B11111,
                    B00100,
                    B00100,
                    B00000,
                    B00000
                    };
  lcd.createChar(LCDSlowDown, SlowDown);
  lcd.createChar(LCDSpeedUp, SpeedUp);
  lcd.createChar(LCDJustFine, JustFine);
}

#endif