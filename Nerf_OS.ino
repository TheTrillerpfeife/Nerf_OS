/* Nerf OS
    Version 0.005
    Datum 2018-01-27

*/


// Einbindung von zusätzlichen Bibliotheken
#include <LCD5110_Graph.h> // LCD
#include "OneButton.h" //Buttons

//Definition Pins, Variablen und Parameter

//Arduino Pins
const int LEDGELBAUS = 11; //Gelbe LED
const int LEDROTAUS = 12; //Rote LED
const int LAUF = A0; //Lichtschranke Lauf
const int BATTERIE = A5; //Spannungsmesser Batterie
const int MAGAZIN = 8; //Magazinerkennung
OneButton SCHALTER1(9, false); // Schalter 1
OneButton SCHALTER2(10, false); // Schalter 2

// Pinbelegung für das Display
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS/CE)
// pin 3 - LCD reset (RST)
LCD5110 myGLCD(7, 6, 5, 3, 4);

//Allegemien Parameter und Variblen
float Version = 0.005; // Version der Software
unsigned long currentTime = millis(); // Zeitstempel des Durchlaufs - Aktuelle Zeit


//Parameter und Variblen für das Display
int UpdateDisplay = 1; //Muss der Bildschirm aktuallisiert werden
int Clearcounter = 0; // Counter Löschen zur Verhinderung von Artefakten auf dem Display

//Parameter und Variblen für die Lichtschranke im Lauf
int HELLIGKEIT = 1023; //Definition Helligkeitsmessewert
const int HELLIGKEITREF = 200; // Wert unter den das Licht fallen muss, damit die Lichtschranke auslöst


//Parameter und Variblen für die Magazine
int Magtyp[8] = {0, 6, 10, 12, 15, 18, 25, 35}; // Arry für Magazingrössen  (0=0=Hochzählen/1=6/2=10/3=12/4=15/5=18/6=25/7=35)
int Display[10] = {1, 2, 1, 5, 1, 8, 2, 5, 3, 5}; // Liste der Magazinwerte für das Display initalbefüllung - Muss Zu Mag passen
int Mag = 3; // Variable für das Magazin aus dem Array Standard - Muss zu Display Array passen
int AMMO = (Display[4] * 10) + Display[5]; // Variable für den Munitionscounter - Setzte Startwert = Größe des Standardmagazins Werte 3 und 4 aus dem DispalyArray
int Leer = 0; // Magazin leer wenn 1
int Drin = 0; // Magazin vorhanden 0 = Nein / 1 = Ja
int DrinAlt; // Alter Status Magazin

//Parameter und Variblen für die Spannungsmessung
float VOLT; // Ausgabewert der gemessenen Spannung in Volt
const float referenceVolts = 5.0; // MAX Spannung Eingang
const float RA = 1000; //Widerstand 1 in Ohm
const float RB = 510; //Widerstand 2 in Ohm
const float resistorFactor = 1023.0 * (RB / (RA + RB)); // Widerstandsfaktor Spannungsteiler
long MessungVolt = 10000; // Millisekunden bis zur nächsten Voltmessung
unsigned long lasttimeVolt = 0; //Zeitstempel des letzten Durchlaufs der Voltmessung

//Parameter für die LEDsRot
unsigned long lasttimeRot=0; //Zeitstempel des Durchlaufs LED Rot an
unsigned long lasttimeWarteRot=0; //Zeitstempel des Durchlaufs LED Rot aus
unsigned long previousMillisRot = 0; // Zeitstempel für den letzen Durchlauf Rote LEDs
long WarteRot = 500; // Warten zwischen dem Blinken
long IntervalRot = 500; // Dauer des Blinkens
int BlinkRot = LOW; //Status der LED Low oder High

//Parameter für die LEDsGelb
unsigned long lasttimeGelb=0; //Zeitstempel des Durchlaufs LED Rot an
unsigned long lasttimeWarteGelb=0; //Zeitstempel des Durchlaufs LED Rot aus
unsigned long previousMillisGelb = 0; // Zeitstempel für den letzen Durchlauf Rote LEDs
long WarteGelb = 500; // Warten zwischen dem Blinken
long IntervalGelb = 500; // Dauer des Blinkens
int BlinkGelb = LOW; //Status der LED Low oder High
float WarnungGelb = 0.35; // Wert Restmunition in Prozent ab dem die gelbe LED blinkt

// Einbindung von Bildern und Schriften für das Display
extern uint8_t SmallFont[]; // Einbinden der kleinen Schrift
extern unsigned char TinyFont[]; // Einbinden der sehr kleinen Schrift
extern unsigned char MediumNumbers[]; // Einbinden der mittlere Zahlen
extern unsigned char BigNumbers[]; // Einbinden der große Zahlen
extern uint8_t NerfLogo[]; //Nerflogobild

//Setup läuft einmalig vor dem Loop
void setup()
{
  //PinModes des Arduino setzen
  pinMode(LEDGELBAUS, OUTPUT);
  pinMode(LEDROTAUS, OUTPUT);
  pinMode(LAUF, INPUT);
  pinMode(BATTERIE, INPUT);
  pinMode(MAGAZIN, INPUT);
  // Die Pushbuttons werden über OneButton Bibliothek gesetzt


  Serial.begin(9600);  // Starte Serial Monitor nur Nötig für den DebugModus
  myGLCD.setContrast(70); //Displaykontrast auf 70 (0-127)
  myGLCD.InitLCD(); // Löschen und Rücksetzen des Displays


  // Startbildschirm für 3 Sekunden mit Logo und Version dann Löschen des Bildschirms
  myGLCD.drawBitmap(0, 0, NerfLogo, 84, 48);
  myGLCD.setFont(TinyFont);
  myGLCD.print("Vers.", 42, 43);
  myGLCD.printNumF(Version, 3, RIGHT, 43);
  myGLCD.update();
  delay(3000);
  myGLCD.clrScr();
  myGLCD.update();

  // Initialisierung der Schalters 1 mit der Funktion Click
  SCHALTER1.attachClick(click1);

  // Initialisierung der Schalters 2 mit allen Funktion
  SCHALTER2.attachClick(click2);
  SCHALTER2.attachDoubleClick(doubleclick2);
  SCHALTER2.attachLongPressStart(longPressStart2);
  SCHALTER2.attachLongPressStop(longPressStop2);
  SCHALTER2.attachDuringLongPress(longPress2);
}

// Loop Funktiom
void loop() 
{

  currentTime = millis(); //Zeitstempel für den Durchlauf

  SCHALTER1.tick(); // Prüfe Schalter 1
  SCHALTER2.tick(); // Prüfe Schalter 2
  CheckMag(); // Prüfe ob ein Magazin eingelegt oder entfernt wurde
  CheckLauf(); // Prüfe ob geschossen wurde

  if (currentTime - lasttimeVolt > MessungVolt) // Prüfung ob Zeit für die nächste Voltmessung gekommen ist
  {
    lasttimeVolt = currentTime; // Setzte Zeitstemple der aktuellen Messung
    ReadVoltage(); // Messe Volt
  }

  if (UpdateDisplay == 1) // Update Display wenn nötig
  {
    AnzeigeNeu();
  }
  LEDRot(); //Aufruf der Routine für die Rote LED
  LEDGelb(); //Aufruf der Routine für die Gelbe LED
  
  
  //Debug(); ///Serielle Ausgabe für Infos im Standard auskommentiert, da sehr Performance lastig


}


// Routine für die Gelbe LED
void LEDGelb()
 {
  if (AMMO <= ((Display[4] * 10) + Display[5]) * WarnungGelb ) // Löst aus, wenn Restmunition unter dem Schwellwert
      {
      if(BlinkGelb == HIGH && currentTime - previousMillisGelb > IntervalGelb ) // Gelb an und dauer für an erreicht
          {
           previousMillisGelb = currentTime; 
           BlinkGelb = LOW;
           digitalWrite(LEDGELBAUS, BlinkGelb);
          }
         if(BlinkGelb == LOW && currentTime - previousMillisGelb > WarteGelb) // Gelb aus und dauer für Pause erreicht
          {
           previousMillisGelb = currentTime; 
           BlinkGelb = HIGH;
           digitalWrite(LEDGELBAUS, BlinkGelb);
          }
      }
  if (AMMO <= 1)  // LED aus, wenn Funktionsbereich der Roten LED erreicht
      {    
        digitalWrite(LEDGELBAUS, LOW);
        }
          
  if (Leer != 1 && AMMO > ((Display[4] * 10) + Display[5]) * WarnungGelb) //LED aus wenn Magazin nicht leer und Munition > Schwellwert 
    {
       digitalWrite(LEDGELBAUS, LOW); 
    }
  if (((Display[4] * 10) + Display[5]) == 0)  // Hochzählmodus aktiv -> LED aus
    {
       digitalWrite(LEDGELBAUS, LOW); 
    }
 }


//Routine für die Rote LED
void LEDRot()
 {
  if (AMMO == 1 && ((Display[4] * 10) + Display[5]) != 0 )  // Restmunition = 1 und Hochzählmodus nicht aktiv
      { 
        if(BlinkRot == HIGH && currentTime - previousMillisRot > IntervalRot ) // Rot an und dauer für an erreicht
          {
           previousMillisRot = currentTime; 
           BlinkRot = LOW;
           digitalWrite(LEDROTAUS, BlinkRot);
          }
         if(BlinkRot == LOW && currentTime - previousMillisRot > WarteRot) // Rot aus und dauer für Pause erreicht
          {
           previousMillisRot = currentTime; 
           BlinkRot = HIGH;
           digitalWrite(LEDROTAUS, BlinkRot);
          }
        }
  if (Leer == 1) // Rot an, wenn Magazin leer
      {  
        digitalWrite(LEDROTAUS, HIGH);
        }
          
  if (Leer != 1 && AMMO !=1) //Rot aus, wenn Magazin nicht leer und Munition <>1
    {
     digitalWrite(LEDROTAUS, LOW); 
    }
  
 }



//Prüfe ob Geschossen wird
void CheckLauf() 
{
  HELLIGKEIT = analogRead(LAUF);

  if (HELLIGKEIT < HELLIGKEITREF ) //Wenn der Sensorwert über Parameter beträgt….
  {
    CalcAmmo();  //Berechnung Restmunition
  }
}

// Berechnung Restmunition
void CalcAmmo()
{
  if ( (Display[4] * 10) + Display[5] == 0) // Hochzählmodus aktiv
  {
    if(AMMO >=99) //Max 99
    {
    AMMO = 99;
    Leer = 1;  
    }
    else //Zähle um 1 hoch
    {
    AMMO = AMMO + 1;  
    }
  }
  else
  {
    if (AMMO <= 0) // Munition nicht ins Negative und Magazin = Leer setzen
    {
      AMMO = 0;
      Leer = 1;
    }
    else
    {
      AMMO = AMMO - 1; //Counter einen runter Zählen
      if (AMMO == 9) //Beim Wechsel von 2 auf eine Stelle wird einmal die Anzeige geleert
      {
        Clearcounter = 1;
      }
      if (AMMO == 0) //Beim Wechsel von 2 auf eine Stelle wird einmal die Anzeige geleert
      {
        Leer = 1;
      }
    }
  }
  UpdateDisplay = 1; //Display neu schreiben
}

// Prüfe ob Magazin verändert
void CheckMag()
{
  Drin = digitalRead(MAGAZIN);
  if ( Drin != DrinAlt)
  {
    UpdateDisplay = 1;
    Clearcounter = 1;
    AMMO = (Display[4] * 10) + Display[5];
    DrinAlt = Drin;
    Leer = 0;
  }
}

//Aktuallisierung des Displays aktuell eckiges Design
void AnzeigeNeu() 
{
  if (Clearcounter == 1)
  {
    myGLCD.setFont(TinyFont);
    myGLCD.print("    ", 67, 0);

// 2er Löschblock    
    myGLCD.setFont(BigNumbers);
    myGLCD.print("..", CENTER, 4);
    myGLCD.clrRect(33, 25, 34, 27);
    myGLCD.clrRect(35, 25, 36, 27);
    myGLCD.clrRect(47, 25, 48, 27);
    myGLCD.clrRect(49, 25, 50, 27);

// 3er Löschblock
//    myGLCD.setFont(BigNumbers);
//    myGLCD.print("...", CENTER, 13);
//    myGLCD.clrRect(26, 34, 27, 36);
//    myGLCD.clrRect(28, 34, 29, 36);
//    myGLCD.clrRect(40, 34, 41, 36);
//    myGLCD.clrRect(42, 34, 43, 36);
//    myGLCD.clrRect(54, 34, 55, 36);
//    myGLCD.clrRect(56, 34, 57, 36);


    myGLCD.update();
    Clearcounter = 0;
  }

  if (Drin == 0)
  {
    myGLCD.setFont(BigNumbers);
    myGLCD.print("--", CENTER, 4);
  }
  else
  {
    myGLCD.setFont(BigNumbers);
    myGLCD.printNumI(AMMO, CENTER, 4);
  }
  myGLCD.setFont(TinyFont);
  myGLCD.printNumF(VOLT, 1, 67, 0);
  myGLCD.printNumI(Display[0], 0, 42);
  myGLCD.printNumI(Display[1], 5, 42);
  myGLCD.printNumI(Display[8], 75, 42);
  myGLCD.printNumI(Display[9], 80, 42);

  myGLCD.printNumI(Display[2], 20, 37);
  myGLCD.printNumI(Display[3], 25, 37);
  myGLCD.printNumI(Display[6], 55, 37);
  myGLCD.printNumI(Display[7], 60, 37);

  myGLCD.setFont(MediumNumbers);
  //myGLCD.invertText(true);
  myGLCD.printNumI(Display[4], 30, 30);
  myGLCD.printNumI(Display[5], 42, 30);
  // myGLCD.invertText(false);

//Aufbau Bildschirm für eckiges Design
    myGLCD.drawLine(0,6,18,6);
    myGLCD.drawLine(18,6,21,3);
    myGLCD.drawLine(19,9,28,0);
    myGLCD.drawLine(29,0,55,0);
    myGLCD.drawLine(55,0,64,9);
    myGLCD.drawLine(62,3,65,6);
    myGLCD.drawLine(65,6,83,6);
    
    myGLCD.drawLine(0,40,9,40);
    myGLCD.drawLine(9,40,13,44);
    myGLCD.drawLine(83,40,74,40);
    myGLCD.drawLine(74,40,70,44);
    
    myGLCD.drawLine(14,41,19,46);
    myGLCD.drawLine(14,41,20,35);
    myGLCD.drawLine(64,46,69,41);
    myGLCD.drawLine(69,41,63,35);

    myGLCD.drawLine(23,44,28,44);
    myGLCD.drawLine(28,44,31,47);
    myGLCD.drawLine(31,47,52,47);
    myGLCD.drawLine(52,47,55,44);
    myGLCD.drawLine(55,44,60,44);

  myGLCD.update();
  UpdateDisplay = 0;

/// Start Rundes design
//  if (Drin == 0)
//  {
//    myGLCD.setFont(BigNumbers);
//    myGLCD.print("--", CENTER, 12);
//  }
//  else
//  {
//    myGLCD.setFont(BigNumbers);
//    myGLCD.printNumI(AMMO, CENTER, 12);
//  }
// Kreise
//  myGLCD.drawCircle(42,24,41);
//  myGLCD.drawCircle(42,24,28);
//  myGLCD.drawCircle(42,24,15);
//  myGLCD.drawCircle(42,24,22);
// Linie  Invertierte Zahl
//  myGLCD.drawLine(2,20,14,20);
//
// Magazine
// myGLCD.setFont(TinyFont);
// myGLCD.printNumI(Display[0], 12, 0);
// myGLCD.printNumI(Display[1], 17, 0);
// myGLCD.printNumI(Display[8], 12, 43);
// myGLCD.printNumI(Display[9], 17, 43);
//
// myGLCD.printNumI(Display[2], 6, 11);
// myGLCD.printNumI(Display[3], 11, 11);
// myGLCD.printNumI(Display[6], 6, 32);
// myGLCD.printNumI(Display[7], 11, 32);
//
//
//  myGLCD.setFont(SmallFont);
//  myGLCD.invertText(true);
//  myGLCD.printNumI(Display[4], 2, 21);
//  myGLCD.printNumI(Display[5], 8, 21);
//  myGLCD.invertText(false);
//
// Pfeile malen
//  Pfeil(66, 0, 0);
//  Pfeil(70, 6 ,0);
//  Pfeil(72, 12 ,0);
//  Pfeil(73, 18, 1);
//  Pfeil(73, 24, 1); 
//  Pfeil(72, 30 ,1); 
//  Pfeil(70, 36, 1); 
//  Pfeil(66, 42, 1);
//  myGLCD.update();
///// Ende Rundes Design

}


// Funktion zum Malen der Pfeile - Pfeil ist 8 Breit und 6 Hoch - Start in linker oberer Ecke
//
//    0 1 2 3 4 5 6 7 
//  0       x x       0
//  1     x     x     1
//  2   x         x   2
//  3 x     x x     x 3
//  4 x   x     x   x 4
//  5 x x         x x 5
//    0 1 2 3 4 5 6 7
//
void Pfeil(int A, int B, int V) // A = X-Koordinate B = Y-Koordinate C = 1 = Voll / 0 = Leer
{
  if(V == 0) // leerer Pfeil
  {
    myGLCD.drawLine(A+0,B+3,A+3,B+0);
    myGLCD.drawLine(A+4,B+0,A+7,B+3);
    myGLCD.drawLine(A+0,B+3,A+0,B+5);
    myGLCD.drawLine(A+7,B+3,A+7,B+5);
    myGLCD.drawLine(A+1,B+5,A+3,B+3);
    myGLCD.drawLine(A+4,B+3,A+6,B+5);
  }
  if(V== 1) // Voller Pfeil
  {
    myGLCD.drawLine(A+3,B+0,A+0,B+3);
    myGLCD.drawLine(A+4,B+0,A+7,B+3);
    myGLCD.drawLine(A+3,B+1,A+0,B+4);
    myGLCD.drawLine(A+4,B+1,A+7,B+4);
    myGLCD.drawLine(A+3,B+2,A+0,B+5);
    myGLCD.drawLine(A+4,B+2,A+7,B+5);
    myGLCD.drawLine(A+3,B+3,A+1,B+5);
    myGLCD.drawLine(A+4,B+3,A+6,B+5);
  }
}



// Button 1 wird geklickt - > Magazinartwechsel
void click1() {
  Mag = Mag + 1; // Maginzart wechseln
  if (Mag > (sizeof(Magtyp) / sizeof(int) - 1)) // Umbruch des Arrays
  {
    Mag = 0;
  }
  MagDisplayRead(); //Befüllen des Arrays für das Display über die Funktion
  AMMO = (Display[4] * 10) + Display[5]; //Setze Munition = Kapazität Magazin
  Clearcounter = 1; 
  UpdateDisplay = 1;
  Leer = 0;
}

// Button 2 Klick -> Schuss
void click2() 
{
  CalcAmmo();
} 


void doubleclick2() {
  Serial.println("Button 2 doubleclick.");
} // doubleclick2


void longPressStart2() {
  Serial.println("Button 2 longPress start");
} // longPressStart2


void longPress2() {
  Serial.println("Button 2 longPress...");
} // longPress2

void longPressStop2() {
  Serial.println("Button 2 longPress stop");
} // longPressStop2



// Auslesen des Arrays für die 5 Magazine die auf dem Display angezeigt werden
void MagDisplayRead() 
{ int n = 0; //Variable zur Befüllung des Display Arrays
  for (int i = Mag; i < Mag + 5; i = i + 1) // Schleife zur Befüllung des Display Array
  {
    if (i > (sizeof(Magtyp) / sizeof(int)) - 1) // Umbruch, wenn Mag Zähler  > Anzahl der Magazine
    {
      Display[n] = (Magtyp[(i - (sizeof(Magtyp) / sizeof(int) - 1)) - 1]  / 10) % 10; // Schreibe Zehner
      n = n + 1;
      Display[n] = Magtyp[(i - (sizeof(Magtyp) / sizeof(int) - 1)) - 1] % 10; // Schreibe Einer
    }
    else
    {
      Display[n] = (Magtyp[i] / 10) % 10; // Schreibe Zehner
      n = n + 1;
      Display[n] = Magtyp[i] % 10;// Schreibe Einer
    }
    n = n + 1;
  }
}


// Auslesen der Batteriespannung und Fehlmessungen auf 0 Volt setzten
void ReadVoltage() 
{
  int VOLTI = analogRead(BATTERIE);
  float VOLTO = (VOLTI / resistorFactor) * referenceVolts ;
  if (VOLTO < 0.05)
  {
    VOLT = 0.00;
  }
  else
  {
    VOLT = VOLTO;
  }
  Clearcounter = 1; // Display löschen
  UpdateDisplay = 1; // Display Update nötig
  return VOLT;
}


void Debug() // Ausgabe aller Wert im SerialMonitor
{
  Serial.print("Sensorwert Helligkeit = " );
  Serial.println(HELLIGKEIT);
  Serial.print("Sensorwert HelligkeitREF = " );
  Serial.println(HELLIGKEITREF);
  Serial.print("Volt = ");
  Serial.println(VOLT);
  Serial.print("Mag = ");
  Serial.println(Mag);
  Serial.print("Ammo = ");
  Serial.println(AMMO);
  Serial.print("Warnung ab ");
  Serial.println((((Display[4] * 10) + Display[5]) * WarnungGelb ));
  Serial.print("Ammo Kalk = ");
  Serial.println((Display[4] * 10) + Display[5]);
  Serial.print("Dispaly Array = ");
  Serial.print(Display[0]);
  Serial.print(";");
  Serial.print(Display[1]);
  Serial.print(";");
  Serial.print(Display[2]);
  Serial.print(";");
  Serial.print(Display[3]);
  Serial.print(";");
  Serial.print(Display[4]);
  Serial.print(";");
  Serial.print(Display[5]);
  Serial.print(";");
  Serial.print(Display[6]);
  Serial.print(";");
  Serial.print(Display[7]);
  Serial.print(";");
  Serial.print(Display[8]);
  Serial.print(";");
  Serial.println(Display[9]);
  Serial.print("Leer = ");
  Serial.println(Leer);
  Serial.print("BlinkRot = ");
  Serial.println(BlinkRot);
  //delay(100);

}


