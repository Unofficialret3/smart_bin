// C++ code

//Bibliotheken einbinden
#include <NewPing.h>
#include <LiquidCrystal.h>


// pins defenieren

//ultraschall
#define trig 8
#define echo 7

//RGB Lampe
#define blue 2
#define green 12
#define red 13

// LCD Pins definieren
#define RS 11  
#define E 10   
#define D4 9   // Datenleitungen
#define D5 6   
#define D6 5   
#define D7 4   

//buzzer
#define buzzer 3

// potentiometer
#define potential A0

// variablen setzen
#define maxDistance 255

int stableCount = 0;   
int lastMaxDepth = -1;  // Zuletzt gespeicherte Tiefe
int maxDepth = 0;       // Die eingestellte maximale Tiefe


//später für fehlerbehebungen
int errorCountDistance = 0;
int errorCountDepth = 0;


// newping/ultraschall objekt
NewPing sonar(trig, echo, maxDistance);

//lcd objekt
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);



void setup() {
 
 Serial.begin(9600);

 //pins als ausgänge setzten
 pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(buzzer, OUTPUT);



  //test
  lcd.begin(16, 2);  // LCD initialisieren (16 Zeichen, 2 Zeilen)
   
   
   // Begrüßung
  lcd.setCursor(0, 0);
  lcd.print("Hallo Welt!");  
  lcd.setCursor(0, 1);
  lcd.print("Einst. fuer Muelleimer");

  delay(2000);  // delay zum lesen

  // Anweisung für user
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stelle Tiefe ein");
  lcd.setCursor(0, 1);
  lcd.print("am Potentiometer");
  delay(2000);  // Kurze Verzögerung



  // Tiefe des Mülleimers einstellen
  while (true) { // true ist immer true also läuft es unendlich bis break, was böse sein kann aber i gues muhahahahahahahahahaha

    int potentiometerValue = analogRead(potential);  // Potentiometer auslesen
    maxDepth = map(potentiometerValue, 0, 1023, 5, 255);  // maximalen Tiefe  5-255 cm

    //fehlervermeidung falls Maxdepth null ergibt, würde mathematischen fehler verursachen später
    if(maxDepth<=0){
      maxDepth = 5; // standart
    }

    //aktuellen Wert der Tiefe anzeigen
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Eingest. Tiefe: ");
    lcd.setCursor(0, 1);
    lcd.print(maxDepth);
    lcd.print(" cm");



    // falls zufrieden dann bestätigen durch warten
    if (maxDepth == lastMaxDepth) {
      stableCount++;
    } else {
      stableCount = 0;  // Zähler zurücksetzen, wenn der Wert sich ändert
    }

    // wenn der wert 3 mal hintereinander gleich ist, schleife beenden
    if (stableCount >= 6) {
      break;  // Beendet die Schleife
    }

    lastMaxDepth = maxDepth;  // Speichere den aktuellen Wert als letzten Wert

    delay(400);  // Verzögerung für eine kurze Zeit,um den user die Chance zu geben weiter zu drehen
  }

  // Nach der Eingabe
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tiefe eingestellt!");
  delay(2000);  // Kurze Verzögerung für user


}
void loop(){
  int errorCountDistance = 0;
  //Messung
  int distance = sonar.ping_cm();
  //falls messung fehlschlägt
  if(distance == 0 ){

    //nachmessen ob der fheler weiterhin besteht
    distance = sonar.ping_cm();
    while(distance== 0){
      // zählen wie oft der fehler infolge besteht
      errorCountDistance ++;


      // wenn er häufiger besteht error ausgeben 
      if(errorCountDistance >= 15){

        fatalError(1);
        break;

      }

      distance = sonar.ping_cm();

      delay(100);
    }
    errorCountDistance = 0;
  }

  //ausgabe in Console
  Serial.print("Entfernung: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (maxDepth <0 && distance >= maxDepth){
   // fehler ausgeben
    fatalError(3);

  }
  else {

  // Berechnung des Füllstandes
    float fillPercentage = (float)distance / maxDepth * 100;
    
    if (fillPercentage < 0 || fillPercentage > 100) {

      //nachmessen 
      fillPercentage = (float)distance / maxDepth * 100;

      // mehrfach überprüfen
      while(fillPercentage < 0 || fillPercentage > 100){
        
        // zählen wie oft der fehler infolge besteht
        errorCountDepth ++;


        // wenn er häufiger besteht error ausgeben 
        if(errorCountDepth >= 15){

          fatalError(2);
          break;

        }
        delay(400);
        //erneut messen
        distance = sonar.ping_cm();
        fillPercentage = (float)distance / maxDepth * 100;
        
      }
    errorCountDepth = 0;
    }

  
  
  // Füllstand anzeigen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fuellstand: ");
  lcd.setCursor(0, 1);
  // fill percentage zu int weil int notwendig
  lcd.print(100 - (int)fillPercentage);
  lcd.print("%");

    
    if (fillPercentage <= 20 && fillPercentage >=1) {
      // freiheit unter oder gleich 50% (ROt)
      digitalWrite(green, LOW);  
      digitalWrite(red, HIGH);     // Rot an
      digitalWrite(blue, LOW);    
      
      // in zweite zeile schreiben das der müll raus muss
      lcd.setCursor(5, 1);
      lcd.print("AUSLEHREN!");


      // Buzzer Ton 
      playBuzzer(); // 1000 Hz Ton spielen
    } 
    
    else if (fillPercentage > 20 && fillPercentage <= 55) {
      // Füllstand zwischen 51% und 85% (gelb)
      digitalWrite(green, HIGH);  // Grün leuchten lassen
      digitalWrite(red, HIGH);    // Rot leuchten lassen (Gelb)
      digitalWrite(blue, LOW);    

      // Kein Ton
      stopBuzzer();
    } 
  
    else if (fillPercentage > 55) {
      // Füllstand über 85% (grün)
      digitalWrite(green, HIGH);   // grün an
      digitalWrite(red, LOW);    
      digitalWrite(blue, LOW);    

      
      // Kein Ton
      stopBuzzer();
    }
  
 
  else {
      // keine korrekte 
      fatalError(3);
    }
    delay(200);
  
  }

}
    void playBuzzer() {
      digitalWrite(buzzer, HIGH);  // Buzzer einschalten 
      delay(500);                  
      digitalWrite(buzzer, LOW);   // Buzzer ausschalten 
      delay(500);                 
    }

  void stopBuzzer() {
    digitalWrite(buzzer, LOW);   
  }
      // fatal error, brauch einen error code int
  void fatalError(int errorCode) {
    //Lampe und buzzer austellen
    digitalWrite(red, LOW);
    digitalWrite(green, LOW);
    digitalWrite(blue, LOW);
    stopBuzzer();


    // user erklären was los ist
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("FATAL ERROR ");
    // Error Code anzeigen
    lcd.print( errorCode);
    lcd.setCursor(0, 1);
    

    switch(errorCode){
      
      case 1:
        lcd.print("Sensorfehler");
        break;

      case 2:
        lcd.print("ungueltige Tiefe");
        break;

      case 3:
        lcd.print("Tiefe > max");
        break;
      default:
      lcd.print("unbek. fehler");

    }
    // fehleranzeigen für 4 sek
    delay(4000);
    
    //user erklären wie zu fixen
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Fuer Fix");

    lcd.setCursor(0, 1);
    lcd.print("Bitte Neustarten");
    


    // schleife damit wir hier in dem state bleiben bis der zum neustart
    while (true) {//true bleibt immer war 
      digitalWrite(red, HIGH);//aufmerksamkeit kriegen durch rot blinken
      stopBuzzer();
      delay(500);
      digitalWrite(red, LOW);
      stopBuzzer();
      delay(500);
    }
  
  }





