#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <FlexiTimer2.h>
#include <EVS_FlowSensor.h>

#define SENSOR_NUMBER (3)
const uint8_t pinHall[SENSOR_NUMBER]={2,5,8};
const uint8_t pinLed[SENSOR_NUMBER]={3,6,0xFF};
const uint8_t pinLcdMode=4, pinAction=7;

#define PIN_VARIABLE (0)
int potentiometer;

LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F or 0x27 for a 16 chars and 2 line display
EVS_FlowSensor flowSensor[SENSOR_NUMBER];
volatile unsigned int counterTimer;  
unsigned int dT;
volatile float T=0,dT_float;

void setupLCD(LiquidCrystal_I2C *lcd, String title, String ver);
void setupFlowSensor(void);
void setupPin(void);
void setupTimer(void);
void doInterruptTimer(void);
volatile int lcdMode=0;

void setup() {
  // put your setup code here, to run once:
  setupSerial();
  setupLCD(&lcd, "EVS_FlowSensor", EVS_FlowSensor_VERSION);
  setupFlowSensor();
  setupTimer();
}

void loop() {
  // put your main code here, to run repeatedly:
  static char s1[16], s2[16], s_float[10];
  unsigned long cntr[SENSOR_NUMBER];
  float q[SENSOR_NUMBER],f[SENSOR_NUMBER];
  int i;

  if( flowSensor[0].isTimeElapsed() )
  {
  Serial.print  ( (String) "Q , F= ");
  for(i=0;i<SENSOR_NUMBER;++i) 
  {
     cntr[i] = flowSensor[i].getCounterTotal();
     if( flowSensor[i].isTimeElapsed() )
     {
        f[i]=flowSensor[i].getFrequency();
        q[i]=flowSensor[i].getFlowRate();
        flowSensor[i].reset();
        Serial.print( (String) q[i] + " , " + f[i] + "\t");
     }
     
  }
  Serial.println((String) "time= " + T);
  }
/*  Serial.print  ( (String) T + "\t");
  for(i=0;i<SENSOR_NUMBER;++i) 
  {
     Serial.print((String) q[i] + "\t");
  }
  Serial.println((String)q[i]);
*/
  switch(lcdMode)
  {
    case 0 :
      lcd.setCursor(0,1);
      lcd.print("Q ");
      for(i=0;i<SENSOR_NUMBER;++i) 
      {
        lcd.print(q[i],1);
        lcd.print(" ");
      }
      lcd.print(" L/min        ");
      break;
    case 1 :
      lcd.setCursor(0,1);
      lcd.print("F ");
      for(i=0;i<SENSOR_NUMBER;++i) 
      {
        lcd.print((int)f[i]);
        lcd.print(" ");
      }
      lcd.print(" Hz         ");
      break;
    case 2 :
      lcd.setCursor(0,1);
      lcd.print("n ");
      for(i=0;i<SENSOR_NUMBER;++i) 
      {
        lcd.print((int)cntr[i]);
        lcd.print(" ");
      }
      lcd.print(" pul         ");
      break;
    default : 
      lcd.setCursor(0,1);
      lcd.print("Unknown mode ");
      break;
  }

  potentiometer = analogRead(PIN_VARIABLE);
  dT_float= int( (float(potentiometer)/1023.)*90)  /10. + 1;
  dT= (dT_float*1000);
  dtostrf(dT_float, 4, 1, s_float);
  sprintf(s1,"%ss",s_float);
  dtostrf(T, 6, 0, s_float);
  sprintf(s1,"%s    %ss",s1,s_float);
  
  for(i=0;i<SENSOR_NUMBER;++i) 
  {
     flowSensor[i].setFreqMeasurePeriod(1000L * dT);
  }

  lcd.setCursor(0,0);
  lcd.print(s1);
  checkReset();
  delay(200);
}

void doInterruptTimer(void) // Interrupt Routine
{
  static int i;
  static unsigned long t_micro,t_micro_old=0;
  static unsigned int t_milli, t_milli_old=0;
  t_micro=micros();
  if( (t_micro-t_micro_old)>=100000L)
  {
    t_micro_old=t_micro;
    T+=0.1;
  }
  ++counterTimer;  
   
  for(i=0;i<SENSOR_NUMBER;++i) 
  {
    if (flowSensor[i].checkPulse() == 2)
    {
      flowSensor[i].setLed(HIGH);
      flowSensor[i].addPulseCounter();
    }
    else
      flowSensor[i].setLed(LOW);
  }
  
}

void setupSerial(void)
{
  Serial.begin(9600);
}

void setupLCD(LiquidCrystal_I2C *lcdI2C, String title, String ver)
{
  lcdI2C->init();                      // initialize the lcd 
  lcdI2C->backlight();
  lcdI2C->setCursor(0,0);
  lcdI2C->print(title);
  lcdI2C->setCursor(0,1);
  lcdI2C->print((String) "Ver: " +  ver);
  delay(2000);
  lcdI2C->clear();
  delay(100);
}

void setupTimer(void)
{
  FlexiTimer2::set(1, 1.0/3000, doInterruptTimer); // call every 500 1ms "ticks"
  // FlexiTimer2::set(500, flash); // MsTimer2 style is also supported
  FlexiTimer2::start();
}

void setupFlowSensor(void)
{
  for(int i=0;i<SENSOR_NUMBER;++i)
  { 
    flowSensor[i].init( pinHall[i], pinLed[i]);
  }
  pinMode(pinLcdMode, INPUT);
  pinMode(pinAction, INPUT);
}

void checkReset(void)
{
  bool modeState,actionState;
  modeState=digitalRead(pinLcdMode);
  actionState=digitalRead(pinAction);
  if(actionState)
  {
    switch(lcdMode)
    {
    case 0 :
       break;
    case 1 :
      break;
    case 2 :
      for(int i=0;i<SENSOR_NUMBER;++i)
      { 
        flowSensor[i].resetCounterTotal();
      }
      break;
    default : 
      lcd.setCursor(0,1);
      lcd.print("Unknown mode ");
      delay(200);
      break;
    
    }
  }
  if (modeState) 
  {
      lcdMode++;
      if(lcdMode>2) lcdMode=0;
  }
  if(modeState && actionState)
  {
    T=0;
    lcdMode=0;
  }

  /*
  for(int i=0;i<SENSOR_NUMBER;++i)
  { 
    resetState=digitalRead(pinReset[i]);
    timeReset &= resetState;
    if(resetState) flowSensor[i].resetCounterTotal();
  }
  if (timeReset) T=0;
  */
}

