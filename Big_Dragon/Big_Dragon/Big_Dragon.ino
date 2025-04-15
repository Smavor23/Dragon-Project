#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "LowPower.h"

#define ONE_WIRE_BUS A2  // Pin du capteur DS18B20
#define VANNE_PIN PB7     // Pin de contrôle de la vanne
#define CANDLE_PIN PD5    // Pin de contrôle de la bougie
#define BUTTON_1 PD3      // Pin du premier bouton (PCINT19)
#define BUTTON_2 PD4      // Pin du deuxième bouton (PCINT20)
#define EXT_LED  17
#define LED_R PD6
#define LED_G 15

int count_button_1=0;
int countMemo_button_1=0;
int count_button_2=0;
int countMemo_button_2=0;
int Count_StartTmp = 0;
int eepromAddress = 0;   // Adresse de l'EEPROM pour stocker la valeur
int countAuto = 0;
int countAutoMemo = 0;
float eepromTable [] = {5.0, 10.0, 15.0}; 
float tempC;
bool valveOpen = false;
bool candleOn = false;
int wdtInterruptCount = 0; // Compteur pour mesurer le temps avec le WDT

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(EXT_LED, OUTPUT);
  pinMode(CANDLE_PIN, OUTPUT);
  digitalWrite(CANDLE_PIN, HIGH);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  DDRB |= (1 << PB7);   // Set pin PB7 as output

  if (EEPROM.read(eepromAddress) == 255) {
    EEPROM.write(eepromAddress, 0); // Initialize EEPROM with a valid state
  }
  sensors.begin();

  // Activer les interruptions de changement de pin sur BUTTON_1 (PD3) et BUTTON_2 (PD4)
  PCICR |= (1 << PCIE2);  // Activer PCIE2 pour surveiller PORTD
  PCMSK2 |= (1 << PCINT19) | (1 << PCINT20); // Activer PCINT19 pour PD3 et PCINT20 pour PD4

  // Configurer le Watchdog Timer pour générer une interruption toutes les 8 secondes
  WDTCSR = (1 << WDCE) | (1 << WDE);
  WDTCSR = (1 << WDIE) | (1 << WDP3) | (1 << WDP0); // 8 seconds
}

void blink_EXT_LED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(EXT_LED, HIGH);
    delay(700); // Durée de l'allumage
    digitalWrite(EXT_LED, LOW);
    delay(400); // Durée de l'extinction
  }
}

void blink_LED_R(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_R, HIGH);
    delay(700); // Durée de l'allumage
    digitalWrite(LED_R, LOW);
    delay(400); // Durée de l'extinction
  }
}

void blink_LED_G(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_G, HIGH);
    delay(700); // Durée de l'allumage
    digitalWrite(LED_G, LOW);
    delay(400); // Durée de l'extinction
  }
}

int readEEPROM() {
  int value = EEPROM.read(eepromAddress);
  return value;
}

void openValve() {
  PORTB |= (1 << PB7);
  Serial.println("Vanne ouverte.");
}

void closeValve() {
  PORTB &= ~(1 << PB7);
  Serial.println("Vanne fermée.");
}

void turnOnCandle() {
  digitalWrite(CANDLE_PIN, LOW);
  Serial.println("Bougie allumée.");
}

void turnOffCandle() {
  digitalWrite(CANDLE_PIN, HIGH);
  Serial.println("Bougie éteinte.");
}

void measureTemperature() {
  sensors.requestTemperatures(); // Demander la température au capteur
  tempC = sensors.getTempCByIndex(0);
  Serial.print("Température: ");
  Serial.print(tempC);
  Serial.println(" °C");
}
void Light_Up(){
    digitalWrite(EXT_LED, HIGH);
    //Préchauffage 
    openValve();
    delay(1250); //3000 pour le bruleur
    closeValve();
    delay(10000);
    turnOnCandle();
    delay(1000);
    openValve();
    delay(1900); //3000 pour le brulleur 
    closeValve();
    turnOffCandle();
    for(int i = 0 ; i < 6 ; i++)
    { 
      delay(7000);
      openValve();
      delay(1500); //3000 pour le brulleur 
      closeValve();
    }
    //Fin de Préchauffage 
    delay(30000);
    
    //Démarrage
    openValve();
    delay(1250); //3000 pour le bruleur
    closeValve();
    delay(10000);
    turnOnCandle();
    delay(1000);
    openValve();
    delay(1900); //3000 pour le brulleur 
    closeValve();
    turnOffCandle();
    for(int i = 0 ; i < 4 ; i++)
    { 
      delay(7000);
      openValve();
      delay(1500); //3000 pour le brulleur 
      closeValve();
    }
    //Fin de Démarrage
    digitalWrite(EXT_LED, LOW);
  }
  
void loop() {
  //blink_LED_R(2);
  //sensors.requestTemperatures();
  //float temperatureC = sensors.getTempCByIndex(0);// Lisez la température du premier capteur détecté
  // Vérifier et traiter les événements des boutons
  if (count_button_1 > 0) {
    Light_Up();
    count_button_1 = 0;
  }

  if (count_button_2 > 0) {
    int eepromValue = readEEPROM();
    if (eepromValue == 0) {
      blink_EXT_LED(2);
      EEPROM.write(eepromAddress, 1);
    } else if (eepromValue == 1) {
      blink_EXT_LED(3);
      EEPROM.write(eepromAddress, 2);
    } else if (eepromValue == 2) {
      blink_EXT_LED(1);
      EEPROM.write(eepromAddress, 0);
    }
    count_button_2 = 0;
  }
   if (tempC <= eepromTable [readEEPROM()] && Count_StartTmp == 8)
    {
      Light_Up();
      count_button_2 = 0;
      Count_StartTmp = 0;
    }
  // Entrée en mode sommeil profond avec Watchdog Timer activé
  Serial.println("START Sleep mode");
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

// ISR pour les interruptions de changement de pin sur PORTD
ISR(PCINT2_vect) {
  if (!(PIND & (1 << PD3))) {  // Si BUTTON_1 (PD3) est pressé
    count_button_1++;
  }
  if (!(PIND & (1 << PD4))) {  // Si BUTTON_2 (PD4) est pressé
    count_button_2++;
  }
}

// ISR pour le Watchdog Timer
ISR(WDT_vect) {
  wdtInterruptCount++;
  Count_StartTmp++;
  if (wdtInterruptCount >= 3) { // 8 * 8 secondes = environ 1 minute
    Serial.println("Wake up");
    wdtInterruptCount = 0;
    measureTemperature(); // Mesurer la température toutes les minutes
  }
  if(Count_StartTmp > 8){
    Count_StartTmp = 0;
    }
}
