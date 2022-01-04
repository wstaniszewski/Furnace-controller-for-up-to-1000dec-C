/*
* 
* 
* 
* 

2022-01-04   

LCD


  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

 // include the library code:
#include <LiquidCrystal.h>

#include <SPI.h>
#include <Wire.h>
#include "Adafruit_MAX31855.h"

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int potpin = 0;  // analog pin used to connect the potentiometer  ????
int val;    // variable to read the value from the analog pin  ??????

//  relay controll
int relay_pin_1 = 22; // pin do przekaŸnika  
int relay_pin_2 = 14;  // pin do przekaŸnika zapas


// Zmienne

int set_temp = 1;      //  zmienna aktualna nastwawa temp
long int count = 1;   // zmienna do wyswielaania serialem  co zadany czas

long int czas_poczatku_SEG_HOLD = 0;       //czas milis muszi mieæ zmienne jako long bo to wielkie licznby
// sprogram startuje kilka sekund po inicjalizacji mikrokontrolera - ta zmienna to koryguje - normalnie to nie powinno mieæ znaczenia

// tu zadajesz czasy trwania segmentów HOLD w   MINUTACH!!!!!!!!!! -dlatego jest * 60 0000 :)
long int duration_time_HOLD0 = 40 * 60000;   //20 min na styk ledw starcza na zimnym piecu  30 min bezpiecznie
long int duration_time_HOLD1 = 25 * 60000;    // 15 minut  na dojscie z 800 stopni do 1000 + czas wlaœciwy 
long int duration_time_HOLD2 = 30 * 60000;    // 
long int duration_time_HOLD3 = 60 * 60000;         //   2x d³uzej
long int duration_time_HOLD4 = 60 * 60000;            // 2x d³uzej


//  tu zadajesz temperatury segmentów HOLD
int temp_HOLD0 = 700;     // preheating
int temp_HOLD1 = 900;     // Fusing    850 full fuse 700 tack fuse
int temp_HOLD2 = 600;      // Annealing a 
int temp_HOLD3 = 550;      // Annealing b
int temp_HOLD4 = 450;      // Annealing c

// zmienne techniczne HOLD I RAMP
long int endtime_ramp1 = 1;    // zmienna techniczna
long int endtime_ramp2 = 1;    // zmienna techniczna
long int endtime_ramp3 = 1;    // zmienna techniczna

long int end_time_hold0 = 1;    // zmienna techniczna
long int end_time_hold1 = 1;    // zmienna techniczna
long int endtime_hold2 = 1;          // zmienna techniczna
long int endtime_hold3 = 1;          // zmienna techniczna
long int endtime_hold4 = 1;         // zmienna techniczna


int hold_0_finish_flag = 0;     // flaga zakoñczenia segmentu
int hold_1_finish_flag = 0;     // flaga zakoñczenia segmentu
int hold_2_finish_flag = 0;     // flaga zakoñczenia segmentu
int hold_3_finish_flag = 0;     // flaga zakoñczenia segmentu
int hold_4_finish_flag = 0;     // flaga zakoñczenia segmentu


int ramp_1_finish_flag = 0;     // flaga zakoñczenia segmentu
int ramp_2_finish_flag = 0;     // flaga zakoñczenia segmentu
int ramp_3_finish_flag = 0;     // flaga zakoñczenia segmentu
int ramp_4_finish_flag = 0;     // flaga zakoñczenia segmentu

int ramp_temp_diff1 = 1;
float ramp_time1 = 1;
int ramp1_speed = 10;  // stopni na minutê (600 stopni na godzinê)

//  creating a thermocouple instance with software SPI on any three
// digital IO pins.
#define MAXDO   41
#define MAXCS   43
#define MAXCLK  45

// Initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// nie wiem co to todo
#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
#define Serial SerialUSB
#endif



void setup() {
    Serial.begin(9600);

    //termopara   
#ifndef ESP8266
    while (!Serial);     // will pause Zero, Leonardo, etc until serial console opens
#endif

// serwo


// Prekonfiguracja wyœwietlania na LCD

  // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    // Print a message to the LCD.
    lcd.print("Wojciech ");
    lcd.setCursor(0, 1);
    lcd.print("Staniszewski ");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SEG:");
    lcd.setCursor(7, 0);
    lcd.print("TSet:");
    lcd.setCursor(7, 1);
    // print the number of seconds since reset:
    lcd.print("TNow:");

    // przekaŸnik  sterowanie
    pinMode(relay_pin_1, OUTPUT);
    pinMode(relay_pin_2, OUTPUT);
    digitalWrite(relay_pin_1, LOW);   // ¿eby nie odpali³ grzania przypadkowo przy starcie, LOW bo sterowanie stanem wysokim
    czas_poczatku_SEG_HOLD = millis();
}

void loop() {

    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(10, 1);
    // print the number of seconds since reset:
     //lcd.print(millis() / 1000);    - testowe wyswietlanie milisekund

    long int czas = (millis() / 1000);   // czas w sekundach

 // val = analogRead(potpin);            // reads the value of the potentiometer (value between 0 and 1023)

 //  val=czas;


                                                                 // Termopara wyswietlanie na LCD

 // basic readout test, just print the current temp   - zostaw do ewentulanego trouble shootingu
   // lcd.clear();
    //lcd.setCursor(0, 0);
   // lcd.print("Int. Temp = ");
   // lcd.println(thermocouple.readInternal());
   // Serial.print("Int. Temp = ");
   // Serial.println(thermocouple.readInternal());

  //zakladka:   
    int  c = thermocouple.readCelsius();   // orginalnie by³o zamiast int - double i tak jest bardziej prawid³owo, ale mi te 10te stopnia tylko przeszkadzaj¹

   //  if (c==0) 
   //{goto zakladka;}
   //else {}

    lcd.setCursor(12, 1);     // wyswietkanie tepmeratury
    if (isnan(c))
    {
        lcd.print("T/C Problem");
    }
    else
    {
        //   lcd.print("C = "); 
        lcd.print(c);
        lcd.print("  ");

    }

    delay(30);    // mo¿e miec znaczenie dla odczytu termopary

    //  wyœwietlanie na serialu do wykresu

    count++;

    if (count == 100) {
        count = 0;
        Serial.print(czas);


        //  Serial.print(czas/60);
        Serial.print(',');
        Serial.println(c);

    }
    else {}
    // SEGMENT   HOLD  0  pre lanunch tests

    if ((millis() < duration_time_HOLD0)) //  istotny jest tylko czas   
    {
        // wyswietlanie na LCD
        lcd.setCursor(4, 0);  // pozycja wpisania numeru segmentu
        lcd.print("H0");   // wyswietla numer segementu 
        lcd.setCursor(12, 0);  // pozycja wpisania temp now
        lcd.print(temp_HOLD0);   // wyswietla temp seg1 
        delay(10);

        // kontrola temperatury
        if (temp_HOLD0 > (c))        // jak temp ustawiona jest wieksza niz zmierzona odpal grzanie
        {
            digitalWrite(relay_pin_1, HIGH);  // on/off relay   !!!!!!activated by HIGH state!!!!
            lcd.setCursor(15, 1);  // pozycja wpisania temp now
            lcd.print('H');   // wyswietla info ¿e jest grzanie
        }
        else {
            digitalWrite(relay_pin_1, LOW);  // on/off relay
        }
        end_time_hold0 = millis();   // zapisuje czas konca segmentu H1
        hold_0_finish_flag = 1;
    }


    // SEGMENT   HOLD  1

    if ((millis() > (duration_time_HOLD0)) && (millis() < (duration_time_HOLD0 + duration_time_HOLD1))) //  istotny jest tylko czas
  //if ((millis() <= (duration_time_HOLD0 + duration_time_HOLD1))) //  istotny jest tylko czas - wersja uproszczona (bo pierwszy warunek (milis > dur t H0) jest i tak sprawdzony skoro program przeszed³ do sprawdzania drugiego warunku if. opiero sie to na za³o¿eniu ¿e  if przestaje sprawdzaæ warunki po znalezieniu pierwszego pasuj¹cego


    {
        // wyswietlanie na LCD
        lcd.setCursor(4, 0);  // pozycja wpisania numeru segmentu
        lcd.print("H1");   // wyswietla numer segementu 
        lcd.setCursor(12, 0);  // pozycja wpisania temp now
        lcd.print(temp_HOLD1);   // wyswietla temp seg1 
        delay(10);

        // kontrola temperatury
        if (temp_HOLD1 > (c))        // jak temp ustawiona jest wieksza niz zmierzona odpal grzanie
        {
            digitalWrite(relay_pin_1, HIGH);  // on/off relay   !!!!!!activated by HIGH state!!!!
            lcd.setCursor(15, 1);  // pozycja wpisania temp now
            lcd.print('H');   // wyswietla info ¿e jest grzanie
        }



        else {
            digitalWrite(relay_pin_1, LOW);  // on/off relay
        }
        end_time_hold1 = millis();   // zapisuje czas konca segmentu H1
        hold_1_finish_flag = 1;
    }






    // SEGMENT   HOLD  2

    if ((millis() > (duration_time_HOLD0 + duration_time_HOLD1)) && (millis() < (duration_time_HOLD0 + duration_time_HOLD1 + duration_time_HOLD2))) //  istotny jest tylko czas
  //if ((millis() <= (duration_time_HOLD0 + duration_time_HOLD1))) //  istotny jest tylko czas - wersja uproszczona (bo pierwszy warunek (milis > dur t H0) jest i tak sprawdzony skoro program przeszed³ do sprawdzania drugiego warunku if. opiero sie to na za³o¿eniu ¿e  if przestaje sprawdzaæ warunki po znalezieniu pierwszego pasuj¹cego


    {
        // wyswietlanie na LCD
        lcd.setCursor(4, 0);  // pozycja wpisania numeru segmentu
        lcd.print("H2");   // wyswietla numer segementu 
        lcd.setCursor(12, 0);  // pozycja wpisania temp now
        lcd.print(temp_HOLD2);   // wyswietla temp seg1 
        delay(10);

        // kontrola temperatury
        if (temp_HOLD2 > (c))        // jak temp ustawiona jest wieksza niz zmierzona odpal grzanie
        {
            digitalWrite(relay_pin_1, HIGH);  // on/off relay   !!!!!!activated by HIGH state!!!!
            lcd.setCursor(15, 1);  // pozycja wpisania temp now
            lcd.print('H');   // wyswietla info ¿e jest grzanie
        }



        else {
            digitalWrite(relay_pin_1, LOW);  // on/off relay
        }
        end_time_hold1 = millis();   // zapisuje czas konca segmentu H1

    }


    // SEGMENT   HOLD  3

    if ((millis() > (duration_time_HOLD0 + duration_time_HOLD1 + duration_time_HOLD2)) && (millis() < (duration_time_HOLD0 + duration_time_HOLD1 + duration_time_HOLD2 + duration_time_HOLD3))) //  istotny jest tylko czas
  //if ((millis() <= (duration_time_HOLD0 + duration_time_HOLD1))) //  istotny jest tylko czas - wersja uproszczona (bo pierwszy warunek (milis > dur t H0) jest i tak sprawdzony skoro program przeszed³ do sprawdzania drugiego warunku if. opiero sie to na za³o¿eniu ¿e  if przestaje sprawdzaæ warunki po znalezieniu pierwszego pasuj¹cego


    {
        // wyswietlanie na LCD
        lcd.setCursor(4, 0);  // pozycja wpisania numeru segmentu
        lcd.print("H3");   // wyswietla numer segementu 
        lcd.setCursor(12, 0);  // pozycja wpisania temp now
        lcd.print(temp_HOLD3);   // wyswietla temp seg1 
        delay(10);

        // kontrola temperatury
        if (temp_HOLD3 > (c))        // jak temp ustawiona jest wieksza niz zmierzona odpal grzanie
        {
            digitalWrite(relay_pin_1, HIGH);  // on/off relay   !!!!!!activated by HIGH state!!!!
            lcd.setCursor(15, 1);  // pozycja wpisania temp now
            lcd.print('H');   // wyswietla info ¿e jest grzanie
        }



        else {
            digitalWrite(relay_pin_1, LOW);  // on/off relay
        }
        end_time_hold1 = millis();   // zapisuje czas konca segmentu H1
        hold_3_finish_flag = 1;
    }




    // SEGMENT   HOLD  4

    if ((millis() > (duration_time_HOLD0 + duration_time_HOLD1 + duration_time_HOLD2 + duration_time_HOLD3)) && (millis() < (duration_time_HOLD0 + duration_time_HOLD1 + duration_time_HOLD2 + duration_time_HOLD3 + duration_time_HOLD4))) //  istotny jest tylko czas
  //if ((millis() <= (duration_time_HOLD0 + duration_time_HOLD1))) //  istotny jest tylko czas - wersja uproszczona (bo pierwszy warunek (milis > dur t H0) jest i tak sprawdzony skoro program przeszed³ do sprawdzania drugiego warunku if. opiero sie to na za³o¿eniu ¿e  if przestaje sprawdzaæ warunki po znalezieniu pierwszego pasuj¹cego


    {
        // wyswietlanie na LCD
        lcd.setCursor(4, 0);  // pozycja wpisania numeru segmentu
        lcd.print("H4");   // wyswietla numer segementu 
        lcd.setCursor(12, 0);  // pozycja wpisania temp now
        lcd.print(temp_HOLD4);   // wyswietla temp seg1 
        delay(10);

        // kontrola temperatury
        if (temp_HOLD4 > (c))        // jak temp ustawiona jest wieksza niz zmierzona odpal grzanie
        {
            digitalWrite(relay_pin_1, HIGH);  // on/off relay   !!!!!!activated by HIGH state!!!!
            lcd.setCursor(15, 1);  // pozycja wpisania temp now
            lcd.print('H');   // wyswietla info ¿e jest grzanie
        }



        else {
            digitalWrite(relay_pin_1, LOW);  // on/off relay
        }
        end_time_hold1 = millis();   // zapisuje czas konca segmentu H1
        hold_4_finish_flag = 1;
    }


    // Koniec programu (studzenie bierne)
    if (millis() > (duration_time_HOLD0 + duration_time_HOLD1 + duration_time_HOLD2 + duration_time_HOLD3 + duration_time_HOLD4)) //  istotny jest tylko czas

    {
        lcd.setCursor(4, 0);  // pozycja wpisania numeru segmentu

        lcd.print("    END           ");
        digitalWrite(relay_pin_1, LOW);  // switch  off relay heater

    }

}
