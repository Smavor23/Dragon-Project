#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "LowPower.h"

#define ONE_WIRE_BUS A2  // Pin du capteur DS18B20
#define VANNE_PIN PB7     // Pin de contrôle de la vanne
#define CANDLE_PIN PD5    // Pin de contrôle de la bougie
#define BUTTON_1 PD3      // Pin du premier bouton
#define BUTTON_2 PD4      // Pin du deuxième bouton
#define EXT_LED  17
#define LED_R PD6
#define LED_L 15

int count_button_1=0;
int countMemo_button_1=0;
int count_button_2=0;
int countMemo_button_2=0;
int eepromAddress = 0;   // Adresse de l'EEPROM pour stocker la valeur
int countAuto = 0;
int countAutoMemo = 0;
float eepromTable [] = {5.0, 10.0, 15.0}; 
bool valveOpen = false;
bool candleOn = false;


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(EXT_LED, OUTPUT);
  pinMode(CANDLE_PIN,OUTPUT);
  digitalWrite(CANDLE_PIN, HIGH);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(LED_R,OUTPUT);
  pinMode(LED_L,OUTPUT);
  DDRB |= (1 << PB7);   //Set pin PB7 as output

  // Enable PCIE2 Bit3 = 1 (Port D)
  PCICR |= B00000100;
  // Select PCINT20 Bit4 = 1 (Pin D4)
  PCMSK2 |= B00011000;
  if (EEPROM.read(eepromAddress) == 255) {
    EEPROM.write(eepromAddress, 0); // Initialize EEPROM with a valid state
  }
  sensors.begin();
}

void blink_EXT_LED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(EXT_LED, HIGH);
    delay(350); // Durée de l'allumage
    digitalWrite(EXT_LED, LOW);
    delay(200); // Durée de l'extinction
  }
}

int readEEPROM() {
  int value = EEPROM.read(eepromAddress);
  return value;
}
void openValve() {
  PORTB |= (1 << PB7);
  valveOpen = false;
  //Serial.println("Vanne fermée.");
  //delay(1000);
}

void closeValve() {
  PORTB &= ~(1 << PB7);
  valveOpen = true;
  //Serial.println("Vanne ouverte.");
}

void turnOnCandle() {
  digitalWrite(CANDLE_PIN, LOW);
  candleOn = true;
  //Serial.println("Bougie allumée.");
}

void turnOffCandle() {
  digitalWrite(CANDLE_PIN, HIGH);
  candleOn = false;
  //Serial.println("Bougie éteinte.");
}

void loop() {
    if (count_button_1 != 0) { // Mode manuel activé
 
      openValve();
      delay(300);
      closeValve();
      delay(5000);
      turnOnCandle();
      delay(200);
      openValve();
      delay(500);
      closeValve();
      turnOffCandle();
    
    count_button_1 = 0;
    } 
     if(count_button_2 != 0 && readEEPROM() == 0) 
    {   
      blink_EXT_LED(2);
      EEPROM.write(eepromAddress, 1);
      count_button_2 = 0;
    } else     
    if(count_button_2 != 0 && readEEPROM() == 1) 
    {   
      blink_EXT_LED(3);
      EEPROM.write(eepromAddress, 2);
      count_button_2 = 0;
    }else
    if(count_button_2 != 0 && readEEPROM() == 2) 
    {   
      blink_EXT_LED(1);
      EEPROM.write(eepromAddress, 0);
      count_button_2 = 0;
    }
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);// Lisez la température du premier capteur détecté

   if (temperatureC <= eepromTable [readEEPROM()])
    {
      openValve();
      delay(300);
      closeValve();
      delay(5000);
      turnOnCandle();
      delay(200);
      openValve();
      delay(500);
      closeValve();
      turnOffCandle();
      
  for(int i = 0 ; i < 8 ; i++){
      LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
                SPI_OFF, USART0_OFF, TWI_OFF);
  }
  } else { // Mode manuel désactivé
    turnOffCandle();
    closeValve();
    }

}

ISR (PCINT2_vect)
{
  
  if (!(PIND & (1 << PD3))) {  // Si BUTTON_1 (PD3) est pressé
  count_button_1++;
  }
  if (!(PIND & (1 << PD4))) {  // Si BUTTON_2 (PD4) est pressé
  count_button_2++;
}
}
