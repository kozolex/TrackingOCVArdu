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
}

void loop() 
{ 
  if(Serial.available() > 0) { //Czy Arduino odebrano dane
    odebraneDane = Serial.readStringUntil('\n'); //Jeśli tak, to odczytaj je do znaku końca linii i zapisz w zmiennej odebraneDane
    //Serial.println("Send -> " + odebraneDane ); //Wyświetl komunikat
    lcd.setCursor(0,0);
    lcd.print("                     ");
    lcd.setCursor(0,0);
    lcd.print(odebraneDane);
    
    val=odebraneDane.toInt();
  if (val==170) val=10;  // scale it to use it with the servo (value between 0 and 180) 
  myservo.write(val);   // sets the servo position according to the scaled value 
  }

  // reads the value of the potentiometer (value between 0 and 1023) 

  
  delay(30);                           // waits for the servo to get there 
} 

