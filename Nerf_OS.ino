/* Nerf OS
    Version 0.003
    Datum 2018-01-22

*/


// Einbindung von zusätzlichen Bibliotheken
#include <LCD5110_Graph.h> // LCD
#include "OneButton.h" //Buttons

//Definition Pins, Variablen und Parameter

//Arduino Pins
const int LEDGELBAUS = 13; //Gelbe LED
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
float Version = 0.003; // Version der Software
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


// Einbindung von Bildern und Schriften für das Display
extern uint8_t SmallFont[]; // Einbinden der kleinen Schrift
extern unsigned char TinyFont[]; // Einbinden der sehr kleinen Schrift
extern unsigned char MediumNumbers[]; // Einbinden der mittlere Zahlen
extern unsigned char BigNumbers[]; // Einbinden der große Zahlen
extern uint8_t NerfLogo[]; //Nerflogobild


void setup() {


  //PinMode
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

  // Male den Umriss der Batterie
  myGLCD.drawRect(59, 0, 81, 9);
  myGLCD.drawRect(82, 2, 83, 7);
  myGLCD.update();

  // Auffüllen der Linien um die invertierte Zahl
  myGLCD.drawLine(1, 18, 14, 18);
  myGLCD.drawLine(14, 18, 14, 27);
  myGLCD.drawLine(7, 18, 7, 27);

  // Initialisierung der Schalters 1 mit der Funktion Click
  SCHALTER1.attachClick(click1);

  // Initialisierung der Schalters 2 mit allen Funktion
  SCHALTER2.attachClick(click2);
  SCHALTER2.attachDoubleClick(doubleclick2);
  SCHALTER2.attachLongPressStart(longPressStart2);
  SCHALTER2.attachLongPressStop(longPressStop2);
  SCHALTER2.attachDuringLongPress(longPress2);


}

void loop() {

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


  //Debug(); ///Serielle Ausgabe für Infos

}

void CheckLauf() //Prüfe ob Geschossen wird
{
  HELLIGKEIT = analogRead(LAUF);

  if (HELLIGKEIT < HELLIGKEITREF ) //Wenn der Sensorwert über Parameter beträgt….
  {
    digitalWrite(LEDGELBAUS, HIGH); //…soll die LED leuchten…
    digitalWrite(LEDROTAUS, HIGH); //…soll die LED leuchten…
    CalcAmmo();
  }
  else //andernfalls…
  {
    digitalWrite(LEDGELBAUS, LOW); //….soll sie nicht leuchten.
    digitalWrite(LEDROTAUS, LOW); //….soll sie nicht leuchten.
  }
}


void CalcAmmo()
{
  if ( (Display[4] * 10) + Display[5] == 0)
  {
    if(AMMO >=999)
    {
    AMMO = 999;
    Leer = 1;  
    }
    else
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
    }
  }
  UpdateDisplay = 1;
}


void CheckMag()
{
  Drin = digitalRead(MAGAZIN);
  if ( Drin != DrinAlt)
  {
    UpdateDisplay = 1;
    Clearcounter = 1;
    AMMO = (Display[4] * 10) + Display[5];
    DrinAlt = Drin;
  }
}


void AnzeigeNeu() //Aktuallisierung des Displays
{
  if (Clearcounter == 1)
  {
    myGLCD.setFont(TinyFont);
    myGLCD.print("    ", 63, 2);
    myGLCD.setFont(BigNumbers);
    myGLCD.print("...", CENTER, 13);
    myGLCD.clrRect(26, 34, 27, 36);
    myGLCD.clrRect(28, 34, 29, 36);
    myGLCD.clrRect(40, 34, 41, 36);
    myGLCD.clrRect(42, 34, 43, 36);
    myGLCD.clrRect(54, 34, 55, 36);
    myGLCD.clrRect(56, 34, 57, 36);
    myGLCD.update();
    Clearcounter = 0;
  }

  if (Drin == 0)
  {
    myGLCD.setFont(BigNumbers);
    myGLCD.print("--", CENTER, 13);
  }
  else
  {
    myGLCD.setFont(BigNumbers);
    myGLCD.printNumI(AMMO, CENTER, 13);
  }
  myGLCD.setFont(TinyFont);
  myGLCD.printNumF(VOLT, 1, 63, 2);
  myGLCD.printNumI(Display[0], 3, 0);
  myGLCD.printNumI(Display[1], 10, 0);
  myGLCD.printNumI(Display[8], 3, 42);
  myGLCD.printNumI(Display[9], 10, 42);

  myGLCD.setFont(SmallFont);
  myGLCD.printNumI(Display[2], 1, 8);
  myGLCD.printNumI(Display[3], 8, 8);
  myGLCD.printNumI(Display[6], 1, 31);
  myGLCD.printNumI(Display[7], 8, 31);


  myGLCD.setFont(SmallFont);
  myGLCD.invertText(true);
  myGLCD.printNumI(Display[4], 1, 19);
  myGLCD.printNumI(Display[5], 8, 19);
  myGLCD.invertText(false);

  myGLCD.update();
  UpdateDisplay = 0;
}

void click1() {
  Serial.println("Button 1 click.");
  Mag = Mag + 1; // Maginzart wechseln
  if (Mag > (sizeof(Magtyp) / sizeof(int) - 1)) // Umbruch des Arrays
  {
    Mag = 0;
  }
  MagDisplayRead(); //Befüllen des Arrays für das Display über die Funktion
  AMMO = (Display[4] * 10) + Display[5];
  Clearcounter = 1;
  UpdateDisplay = 1;
}
void click2() {
  Serial.println("Button 2 click.");
} // click2


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





void MagDisplayRead() // Auslesen des Arrays für die 5 Magazine die auf dem Display angezeigt werden
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



void ReadVoltage() // Auslesen der Batteriespannung und Fehlmessungen auf 0 Volt setzten
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
  delay(1000);

}


