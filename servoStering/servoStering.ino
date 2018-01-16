#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


Servo myservo;  // create servo object to control a servo
LiquidCrystal_I2C lcd(0x27, 20, 4);

int val;    // variable to read the value from the analog pin
String odebraneDane = ""; //Pusty ciąg odebranych danych
void setup()
{
  myservo.attach(3);  // attaches the servo on pin 9 to the servo object
    Serial.begin(9600); //Uruchomienie komunikacji
        lcd.begin();
    lcd.backlight();
    //Serial.setTimeout(30);
}
int i=0;
void loop() 
{ 
  if(Serial.available() > 0) 
  { 
    odebraneDane = Serial.readStringUntil('\n');
    lcd.setCursor(0,0);
    lcd.print("                    ");
    lcd.setCursor(0,0);
    lcd.print(odebraneDane);
    /*lcd.setCursor(0,i);
    lcd.print("                    ");
    lcd.setCursor(0,i);
    lcd.print(odebraneDane);
    if (i==4) i=0;
    else i++;*/
    val=odebraneDane.toInt();
    myservo.write(val*1.8);   // sets the servo position according to the scaled value 
  }
} 

