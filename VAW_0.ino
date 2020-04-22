/*
 * indicator de panou
 * program scris de Nicu FLORICA (niq_ro) pentru Georgel GHITA
 * 
 * ver.0 - indicare tensiune, curent, putere, temperatura, decuplare releu la supratempratura si supracurent
 * ver.0a1 - corectat afisare temperaura si facut mai multe masuratori de temperatura ca si la tensiune si curent
 */

#define pinAN0 A0  // pinul de masura tensiune
#define pinAN1 A1  // pinul de masura curent
#define pinAN2 A2  // pinul de masura temperatura

#define pinRST 12  // pinul de armare/dezarmare releu (buton fara retinere)
#define pinREL 11  // pinul de comanda releu
#define pinFAN 10  // pin de comanda ventilator
#define pinBUZ 9  // pin de comanda avertizor acustic

#include <LiquidCrystal.h>  // se foloseste libraria pentru controlul afisajului LCD
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;  // definire conectare pini
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  // creere "obiect" pentru afisare

// http://arduino.cc/en/Reference/LiquidCrystalCreateChar  // definim un simbol pentru grad Celsius
byte grad[8] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
};

float R2 = 51.; // rezistenta conectata de la + la A0
float R1 = 1. ; // rezistenta conectata de la A0 la GND
float rsunt = 0.01 ; // valoare rezistenta masura (sunt)
float Amp = 10. ; // valoare amplificare operational (reglata din semireglabil)
float vref = 1.1 ;  // tensiune de referinta interna (1,1V)

int trcurent = 0;  // treapta masurare curent (0...1023)
int trtensiune = 0; // treapta masurare tensiune (0...1023)

float sumatensiune = 0.;   // valoare insumare tensiune pentru calcul medie
float sumacurent = 0.; // valoare insumare curent pentru calcul medie
float sumatemp = 0. ; // valoare insumare temperaturi pentru calcul medie

float curent = 0.;   // valoare curent
float tensiune = 0.; // valoare tensiune
float putere = 0.;   // valoare putere
float curentmax = 11.; // curent maxim

int te;  // variabila pentru temperatura
int trtemp = 0; // treapta masurare temperatura (0...1023)
int temin = 40;  // prag temperatura pentru pornire ventilator
int rpmmin = 105; // turatie minima pentru pornire ventilator
int temax = 60;  // prag temperatura pentru pornire ventilator la turatie maxima
int rpmmax = 255; // turatie maxima ventilator
int telim = 100; //temperatura pentru care decupleaza releul
int rpm;  // turatie controlata ventilator

//boolean senzorLM35 = false; 
boolean senzorLM35 = true; // senzor LM35 montat

void setup() {  // ce este pus aici ruleaza doar o data
 lcd.begin(16, 2);  // selectie afisaj 1602 (16 colane si 2 randuri)
 lcd.createChar(0, grad);  // crearea simbolului pentru grad Celsius
 lcd.clear();   // stergere ecran

pinMode(pinAN0, INPUT);  // pin definit ca intrare
pinMode(pinAN1, INPUT);  // pin definit ca intrare
pinMode(pinAN2, INPUT);  // pin definit ca intrare
pinMode(pinRST, INPUT);  // pin definit ca intrare
pinMode(pinREL, OUTPUT);  // pin definit ca iesire
pinMode(pinFAN, OUTPUT);  // pin definit ca iesire
pinMode(pinBUZ, OUTPUT);  // pin definit ca iesire
digitalWrite(pinRST, HIGH); // activare rezistenta de pull-up 
digitalWrite(pinREL, LOW); // releu necomandat
analogWrite(pinFAN, 0); // ventilator oprit
digitalWrite(pinBUZ, LOW);  // avertizor acustic oprit

  lcd.print("indicator panou");  
  lcd.setCursor(0, 1);
  lcd.print("tensiune-curent");
  delay (1000);
  lcd.clear();
  
  lcd.setCursor(3, 0);
  lcd.print("Umax = 55V");  
  lcd.setCursor(3, 1);
  lcd.print("Imax = 10A");
  delay (10000);
  lcd.clear();
}

void loop() {
  // pune in zero variabilele de insumare pentru a calcula ulterior tensiunea medie
  sumatensiune = 0;
  sumacurent = 0;
     
  for (int i=1; i <= 20; i++)
  {
  // citeste treptele de tensiune (0-1023) si adauga la o variabila de cumulare
  trcurent = analogRead(pinAN1);    //  citire valoare pe intrarea analogica 
  sumacurent = sumacurent + trcurent;  // cumuleaza valoarea

  trtensiune = analogRead(pinAN0); 
  sumatensiune = sumatensiune + trtensiune;
  
  delay (20);  // pauza de 20ms intre masuratori
    }

// calculam valorile medii
sumacurent = sumacurent/20.;
sumatensiune = sumatensiune/20.;

// calculam valorile tensiunii si curentului de la sursa
curent = (float)(Amp* rsunt * sumacurent * vref/ 1024.0) ;
tensiune = (float)((R1+R2)/R1 * sumatensiune * vref / 1024.0) ;
tensiune = (float)(tensiune - curent * rsunt);  // facem media rezultatelor si scadem caderea de tensiune de pe rezistenta sunt
putere = (float)(tensiune * curent);   // putere consumata

// afisare valori
 lcd.setCursor(0, 0);
 if (tensiune < 10.0) lcd.print(" ");  // daca tensiunea e mai mica de 10V 
 lcd.print(tensiune);
 lcd.print("V ");
 
 lcd.setCursor(0, 1);
 if (curent < 10.0) lcd.print(" ");  // daca curentul e mai mic de 10A 
 lcd.print(curent);
 lcd.print("A");

 lcd.setCursor(10, 0);
 if (putere < 100.0) lcd.print(" ");  // daca putrea e mai mica de 100W  
 if (putere < 10.0) lcd.print(" ");  // daca putrea e mai mica de 10W 
 lcd.print(putere,1);  // afisare putere cu o singura cifra dupa virgula
 lcd.print("W"); 

if (senzorLM35)  // daca este senzor LM35
{  
 sumatemp = 0;    
  for (int i=1; i <= 20; i++)
  {
  // citeste treptele de tensiune (0-1023) si adauga la o variabila de cumulare
  trtemp = analogRead(pinAN2);    //  citire valoare pe intrarea analogica 
  sumatemp = sumatemp + trtemp;  // cumuleaza valoarea
  delay (20);  // pauza de 20ms intre masuratori
    }
// calculam valorile medii
sumatemp = sumatemp/20.;
   te = (float)(1000 * vref * sumatemp / 1024.0) ; // conversie in grade Celsius
   
if ((digitalRead(pinRST) == LOW) and (te < telim))
{
  digitalWrite(pinREL, HIGH);  // cuplez bornele
  delay(250); // o mica pauza
}

lcd.setCursor(11, 1);
if (te < 100.0) lcd.print(" ");  // daca tempratura e mai mica de 100 grade Celsius 
if (te < 10.0) lcd.print(" ");  // daca tempratura e mai mica de 10 grade Celsius 
lcd.print(te);  // afisam doar valoarea intreaga
//   lcd.write(0b11011111);  // caracter asemanatpor cu gradul Celsius
lcd.write(byte(0));  // simbolul de grad Celsius creat de mine
lcd.print("C"); 

if (te < temin)
{
  analogWrite(pinFAN, 0);  // ventilator oprit
}
else
if ((te>temin) and (te<temin))
{
  rpm = map (te, temin, temax, rpmmin, rpmmax);  // calculeaza turatia variabila
  analogWrite(pinFAN, rpm);
}
else
if ((te>temax) and (te<telim))
{
  analogWrite(pinFAN, rpmmax);  // ventilator laturatie maxima
}
else
if (te > telim)
{
  analogWrite(pinFAN, rpmmax);  // ventilator laturatie maxima
  digitalWrite(pinREL, LOW);    // decuplare releu
  digitalWrite(pinBUZ, HIGH);   // avertizare ca este o problema
  delay(100);   // pauza mica
}  

if (curent > curentmax)  // depasire curent maxim
{
  digitalWrite(pinREL, LOW);    // decuplare releu
  digitalWrite(pinBUZ, HIGH);   // avertizare ca este o problema
  delay(100);   // pauza mica
}   
} // sfarsit conditie de existenta senzor temperatura

digitalWrite(pinBUZ, LOW);   // aoprire avertizare
delay(50);   // pauza mica
}  // sfarsit de program principal
