/* Nerf OS
 *  Version 0.001
 *  Datum 2018-01-06
 */


// Einbindung von zusätzlichen Bibliotheken
#include <LCD5110_Graph.h>


//Definition Pins, Variablen und Parameter

//Arduino Pins
const int LEDGELBAUS=13; //Gelbe LED
const int LEDROTAUS=12; //Rote LED
const int LAUF=A0; //Lichtschranke Lauf
const int BATTERIE=A5; //Spannungsmesser Batterie
const int MAGAZIN=8; //Magazinerkennung 
const int SCHALTER1=9; // Schalter 1
const int SCHALTER2=10; // Schalter 2

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
LCD5110 myGLCD(7,6,5,3,4);



//Variablen
int HELLIGKEIT = 1023; //Definition Helligkeitsmessewert
float VOLT; // Ausgabewert der gemessenen Spannung in Volt
float Version =0.001; // Version der Software
int AMMO = 99; // Variable für Anzahl Restmunition
unsigned long currentTime=millis(); // Zeitstempel des Durchlaufs - Aktuelle Zeit
unsigned long lasttimeVolt=0; //Zeitstempel des letzten Durchlaufs der Voltmessung
int UpdateDisplay = 1; //Muss der Bildschirm aktuallisiert werden

//Parameter
const float referenceVolts = 5.0; // MAX Spannung Eingang
const float RA = 1000; //Widerstand 1 in Ohm
const float RB = 510; //Widerstand 2 in Ohm
const float resistorFactor = 1023.0 * (RB/(RA + RB)); // Widerstandsfaktor Spannungsteiler
const int HELLIGKEITREF = 200; // Wert unter den das Licht fallen muss, damit die Lichtschranke auslöst
long MessungVolt = 10000; // Millisekunden bis zur nächsten Voltmessung

extern uint8_t SmallFont[]; // Einbinden der kleinen Schrift
extern unsigned char TinyFont[]; // Einbinden der sehr kleinen Schrift
extern unsigned char MediumNumbers[]; // Einbinden der mittlere Zahlen
extern unsigned char BigNumbers[]; // Einbinden der große Zahlen
extern uint8_t NerfLogo[];
extern uint8_t Test[];

void setup() {

  
//PinMode
pinMode(LEDGELBAUS, OUTPUT);
pinMode(LEDROTAUS, OUTPUT);
pinMode(LAUF, INPUT);
pinMode(BATTERIE, INPUT);
pinMode(MAGAZIN, INPUT);
pinMode(SCHALTER1, INPUT);
pinMode(SCHALTER2, INPUT);


Serial.begin(9600);  // Starte Serial Monitor
myGLCD.setContrast(70); //Displaykontrast auf 70 (0-127)
myGLCD.InitLCD(); // Löschen und Rücksetzen des Displays


// Startbildschirm für 3 Sekunden mit Logo und Version
myGLCD.clrScr();
myGLCD.drawBitmap(0, 0, NerfLogo, 84, 48);
myGLCD.setFont(TinyFont);
myGLCD.print("Vers.", 42, 43);
myGLCD.printNumF(Version, 3, RIGHT, 43);
myGLCD.update();
delay(3000);
myGLCD.clrScr();
myGLCD.update();

// Male den Umriss der Batterie
myGLCD.drawRect(59,0,81,9);
myGLCD.drawRect(82,2,83,7);
myGLCD.update();

////// Demo für Magazinscroller  

// Male Counter für Test

myGLCD.setFont(TinyFont);
myGLCD.print("1", 3, 0);
myGLCD.print("0", 10, 0);

myGLCD.setFont(SmallFont);
myGLCD.print("1", 1, 8);
myGLCD.print("2", 8, 8);

// Auffüllen der Linien um die invertierte Zahl
myGLCD.drawLine(1, 18, 14, 18);
myGLCD.drawLine(14, 18, 14, 27);
myGLCD.drawLine(7, 18, 7, 27);

myGLCD.setFont(SmallFont);
myGLCD.invertText(true); 
myGLCD.print("1", 1, 19);
myGLCD.print("8", 8, 19);
myGLCD.invertText(false); 

myGLCD.setFont(SmallFont);
myGLCD.print("2", 1, 31);
myGLCD.print("5", 8, 31);

myGLCD.setFont(TinyFont);
myGLCD.print("3", 3, 42);
myGLCD.print("5", 10, 42);

myGLCD.update();
delay(1000);

// Male Counter für Test

myGLCD.setFont(TinyFont);
myGLCD.print(" ", 3, 0);
myGLCD.print("6", 10, 0);

myGLCD.setFont(SmallFont);
myGLCD.print("1", 1, 8);
myGLCD.print("0", 8, 8);

// Auffüllen der Linien um die invertierte Zahl
myGLCD.drawLine(1, 18, 14, 18);
myGLCD.drawLine(14, 18, 14, 27);
myGLCD.drawLine(7, 18, 7, 27);

myGLCD.setFont(SmallFont);
myGLCD.invertText(true); 
myGLCD.print("1", 1, 19);
myGLCD.print("2", 8, 19);
myGLCD.invertText(false); 

myGLCD.setFont(SmallFont);
myGLCD.print("1", 1, 31);
myGLCD.print("8", 8, 31);

myGLCD.setFont(TinyFont);
myGLCD.print("2", 3, 42);
myGLCD.print("5", 10, 42);

myGLCD.update();
delay(1000);


// Male Counter für Test

myGLCD.setFont(TinyFont);
myGLCD.print("U", 3, 0);
myGLCD.print("p", 10, 0);

myGLCD.setFont(SmallFont);
myGLCD.print(" ", 1, 8);
myGLCD.print("6", 8, 8);

// Auffüllen der Linien um die invertierte Zahl
myGLCD.drawLine(1, 18, 14, 18);
myGLCD.drawLine(14, 18, 14, 27);
myGLCD.drawLine(7, 18, 7, 27);

myGLCD.setFont(SmallFont);
myGLCD.invertText(true); 
myGLCD.print("1", 1, 19);
myGLCD.print("0", 8, 19);
myGLCD.invertText(false); 

myGLCD.setFont(SmallFont);
myGLCD.print("1", 1, 31);
myGLCD.print("2", 8, 31);

myGLCD.setFont(TinyFont);
myGLCD.print("1", 3, 42);
myGLCD.print("8", 10, 42);

myGLCD.update();
delay(1000);


// Male Counter für Test

myGLCD.setFont(TinyFont);
myGLCD.print("3", 3, 0);
myGLCD.print("5", 10, 0);

myGLCD.setFont(SmallFont);
myGLCD.print("U", 1, 8);
myGLCD.print("p", 8, 8);

// Auffüllen der Linien um die invertierte Zahl
myGLCD.drawLine(1, 18, 14, 18);
myGLCD.drawLine(14, 18, 14, 27);
myGLCD.drawLine(7, 18, 7, 27);

myGLCD.setFont(SmallFont);
myGLCD.invertText(true); 
myGLCD.print(" ", 1, 19);
myGLCD.print("6", 8, 19);
myGLCD.invertText(false); 

myGLCD.setFont(SmallFont);
myGLCD.print("1", 1, 31);
myGLCD.print("0", 8, 31);

myGLCD.setFont(TinyFont);
myGLCD.print("1", 3, 42);
myGLCD.print("2", 10, 42);

myGLCD.update();
delay(1000);


// Male Counter für Test

myGLCD.setFont(TinyFont);
myGLCD.print("2", 3, 0);
myGLCD.print("5", 10, 0);

myGLCD.setFont(SmallFont);
myGLCD.print("3", 1, 8);
myGLCD.print("5", 8, 8);

// Auffüllen der Linien um die invertierte Zahl
myGLCD.drawLine(1, 18, 14, 18);
myGLCD.drawLine(14, 18, 14, 27);
myGLCD.drawLine(7, 18, 7, 27);

myGLCD.setFont(SmallFont);
myGLCD.invertText(true); 
myGLCD.print("U", 1, 19);
myGLCD.print("p", 8, 19);
myGLCD.invertText(false); 

myGLCD.setFont(SmallFont);
myGLCD.print(" ", 1, 31);
myGLCD.print("6", 8, 31);

myGLCD.setFont(TinyFont);
myGLCD.print("1", 3, 42);
myGLCD.print("0", 10, 42);

myGLCD.update();
delay(1000);


// Male Counter für Test

myGLCD.setFont(TinyFont);
myGLCD.print("1", 3, 0);
myGLCD.print("8", 10, 0);

myGLCD.setFont(SmallFont);
myGLCD.print("2", 1, 8);
myGLCD.print("5", 8, 8);

// Auffüllen der Linien um die invertierte Zahl
myGLCD.drawLine(1, 18, 14, 18);
myGLCD.drawLine(14, 18, 14, 27);
myGLCD.drawLine(7, 18, 7, 27);

myGLCD.setFont(SmallFont);
myGLCD.invertText(true); 
myGLCD.print("3", 1, 19);
myGLCD.print("5", 8, 19);
myGLCD.invertText(false); 

myGLCD.setFont(SmallFont);
myGLCD.print("U", 1, 31);
myGLCD.print("p", 8, 31);

myGLCD.setFont(TinyFont);
myGLCD.print(" ", 3, 42);
myGLCD.print("6", 10, 42);

myGLCD.update();
delay(1000);


// Male Counter für Test

myGLCD.setFont(TinyFont);
myGLCD.print("1", 3, 0);
myGLCD.print("2", 10, 0);

myGLCD.setFont(SmallFont);
myGLCD.print("1", 1, 8);
myGLCD.print("8", 8, 8);

// Auffüllen der Linien um die invertierte Zahl
myGLCD.drawLine(1, 18, 14, 18);
myGLCD.drawLine(14, 18, 14, 27);
myGLCD.drawLine(7, 18, 7, 27);

myGLCD.setFont(SmallFont);
myGLCD.invertText(true); 
myGLCD.print("2", 1, 19);
myGLCD.print("5", 8, 19);
myGLCD.invertText(false); 

myGLCD.setFont(SmallFont);
myGLCD.print("3", 1, 31);
myGLCD.print("5", 8, 31);

myGLCD.setFont(TinyFont);
myGLCD.print("U", 3, 42);
myGLCD.print("p", 10, 42);

myGLCD.update();
delay(1000);


////// Ende Demo Magazinauswahl



}

void loop() {

currentTime=millis(); //Zeitstempel für den Durchlauf


HELLIGKEIT =analogRead(LAUF);

if (HELLIGKEIT < HELLIGKEITREF ) //Wenn der Sensorwert über Parameter beträgt….
{
digitalWrite(LEDGELBAUS, HIGH); //…soll die LED leuchten…
digitalWrite(LEDROTAUS, HIGH); //…soll die LED leuchten…
AMMO = AMMO - 1; //Counter einen runter Zählen
UpdateDisplay = 1;
}

else //andernfalls…
{
digitalWrite(LEDGELBAUS, LOW); //….soll sie nicht leuchten.
digitalWrite(LEDROTAUS, LOW); //….soll sie nicht leuchten.
}



if(currentTime - lasttimeVolt > MessungVolt) // Prüfung ob Zeit für die nächste Voltmessung gekommen ist 
          {
           lasttimeVolt = currentTime; // Setzte Zeitstemple der aktuellen Messung
           ReadVoltage(); // Messe Volt
           UpdateDisplay = 1; // Display Update nötig
          }



Debug(); ///Serielle Ausgabe für Infos

if(UpdateDisplay = 1) // Update Display wenn nötig
{
myGLCD.setFont(TinyFont);
myGLCD.print("    ", 63, 2); 
myGLCD.update();
myGLCD.printNumF(VOLT, 1, 63, 2);
myGLCD.update();
myGLCD.setFont(BigNumbers);
myGLCD.printNumI(AMMO, CENTER, 13);
UpdateDisplay = 0;
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
}
