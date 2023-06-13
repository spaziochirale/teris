/* Copyright Notice

Copyright (c) 2023 Chirale S.r.l.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE. */


/*
  Lettura dell'Energy Meater EASTRON SDM220MODBUS Versione 2.0
  Trasmissione dati su rete LoRaWan The Things Network  

  Testato su scheda Arduino MKR WAN 1310 con Shield 485 MKR
  
  Configurazione degli switch dello shield:
  1 -> Ininfluente, non usato poiché lo shield in modalità HALF-DUPLEX lutilizza solo i terminali Y e Z
  2 -> OFF - Shield impostato per la modalità HALF-DUPLEX
  3 -> Raccomandato su ON per cablaggi lunghi e non schermati. ON = Resistore di terminazione sul bus RS485. 
       Su cablaggi corti e cavi di qualità funziona anche se OFF  

  Cablaggio del BUS RS485:
   Terminale Y dello Shield collegato ai terminali A dei meter SDM220 (pin 9)
   Terminale Z dello Shield collegato ai terminali B dei meter SDM220 (pin 8)     
  
  */

/* Questo sketch presuppone che sul Bus ModBus siano collegati due apparati SDM220
   configurati rispettivamente con Id: 1 e 2.
   Modificare di conseguenza per adattarlo ad altre configurazioni
   */

#include <MKRWAN.h>
#include <ArduinoRS485.h> 
#include <ArduinoModbus.h>
#include <String>
#include <iostream>

#define RS485_BAUDRATE 9600 // Verificare che i dispositivi siano configurati con lo stesso baudrate

// Definizione degli indirizzi iniziali degli INPUT REGISTERS del meter SDM220
// riferimento documento: SDM220 ModBus Protocol v 1.1
#define VOLTAGE  0x0000
#define CURRENT  0x0006
#define ACTIVE_POWER  0x000C
#define APPARENT_POWER  0x0012
#define REACTIVE_POWER  0x0018
#define POWER_FACTOR  0x001E
#define FREQUENCY  0x0046
#define IMPORT_ACTIVE_ENERGY  0x0048   
#define EXPORT_ACTIVE_ENERGY  0x004A  
#define IMPORT_REACTIVE_ENERGY  0x004C
#define EXPORT_REACTIVE_ENERGY  0x004E
#define MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND  0x0056
#define TOTAL_ACTIVE_ENERGY   0x0156  
#define TOTAL_REACTIVE_ENERGY  0x0158

// Definizione del tempo di attesa in ms tra un'interrogazione del ModBus e la successiva
// è necessario per assicurarsi che il dispositivo abbia terminato la risposta prima
// di inviare una nuova interrogazione. Il parametro può essere sottoposto a tuning testando
// il comportamento del bus e dei device. Se riducendo il tempo le letture iniziano ad
// andare in timeout, il valore deve essere incrementato
#define MODBUS_DELAY 3000

// Impostazione delle credenziali per l'accesso all'Applicazione The things Network (TTN)
// Verificare i valori con quelli riportati nella console TTN
String appEui = "0000000000000000" ;                       
String appKey = "33065E1BE7B1821ACC696DA9E23225D1"; 

LoRaModem modem; // Dichiarazione dell'oggetto per la gestione della trasmissione con protocollo LoRa

void setup() {

  Serial.begin(9600); // Inizializzo la comunicazione seriale con il PC-IDE Arduino
  while(!Serial);  // attendo finché la connessione seriale non è disponibile
  
  Serial.println("");
  Serial.println("Lettura e Trasmissione LoRaWan Energy Meter Eastron SCM220-MODBUS - v.2.1 by Chirale");
  Serial.println("");
  if (!modem.begin(EU868)) {
    Serial.println("Inizializzazione del modem LoRa non riuscita!");
    while (1) {} // blocco l'esecuzione del programma con un loop infinito
  };
  Serial.print("Modem LoRa inizializzato.");
  Serial.print("Versione del modem LoRa: ");
  Serial.println(modem.version());
  Serial.print("DeviceEUI: ");  
  Serial.println(modem.deviceEUI()); 
  Serial.println("");   

  Serial.print("Inizializzo il Client ModBusRTU BaudRate: ");
  Serial.println(RS485_BAUDRATE);
  
  if (!ModbusRTUClient.begin(RS485_BAUDRATE)) {
    Serial.println("Inizializzazione ModBus non riuscita!");
    while (1); 
  } 
  else Serial.println("Inizializzazione ModBus completata.");

  Serial.println("");
  Serial.println("Connessione alla rete LoRaWan...");
  if(!modem.joinOTAA(appEui, appKey)) {
    Serial.println("Connessione alla rete LoRaWan non riuscita!");
    while(1) {}
  }

  Serial.println("Connessione LoRaWan OK.");
// Lampeggio il LED per 5 volte -   informazione connessione riuscita
  pinMode(LED_BUILTIN, OUTPUT);
  for(int i=0;i<5;i++){
   digitalWrite(LED_BUILTIN,HIGH);
   delay(500);
   digitalWrite(LED_BUILTIN,LOW);
  delay(500);  
  }  
  
}

///-----------------------------------------------------------
int errore =0; // flag usato per segnalare che è avvenuto un errore nella lettura ModBus
int erroriTot; // contatore degli errori totali riscontrati durante un ciclo della funzione loop

void loop() {
  
  union {
    uint32_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Eanstron SDM220 a 32 bit
    byte buf[4]; // Rappresentazione della stessa variabile come array di 4 byte utile per preparare il pacchetto dati LoRa
  } u;

  Serial.println("Ciclo di lettura e invio..."); 

  erroriTot = 0; // Inizia un nuovo ciclo per cui resetto il numero totale degli errori

  int deviceAddr = 1; // indirizzo del dispositivo sul bus ModBus

  modem.beginPacket(); // Preparo il pacchetto di dati

  u.bitRegister = SDM220readInputRegister_raw(deviceAddr,VOLTAGE);
  if (!errore) {
    // Se la funzione di lettura ModBus non ha settato il flag errore
    // posso procedere a visualizzare sull'eventuale serial monitor i valori 
    // sia in chiaro, come numero con virgola, che come valore esadecimale
    // utile per confrontarlo con il valore ricevuto sul server LoRaWan
    float voltage = rawbit_to_float(u.bitRegister); // converto i 4 byte del regsitro nel tipo float
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Voltage = ");
    Serial.print(voltage);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    // a questo punto, accodo uno per volta i 4 byte del registro nel pacchetto dati
    // utilizzando la rappresentazione del registro sotto forma di array (campo buf della union)
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura Voltage non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = SDM220readInputRegister_raw(deviceAddr,CURRENT);
  if (!errore) {
    float current = rawbit_to_float(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Current = ");
    Serial.print(current);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura Voltage non riuscita");
  }
  delay(MODBUS_DELAY);
  
  
  /*

  float active_power = SDM220readInputRegister(deviceAddr,ACTIVE_POWER);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Active power = ");
    Serial.println(active_power);
  }
  else {
    Serial.println("Lettura Active power non riuscita");
  }
  delay(MODBUS_DELAY);

  float apparent_power = SDM220readInputRegister(deviceAddr,APPARENT_POWER);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Apparent power = ");
    Serial.println(apparent_power);
  }
  else {
    Serial.println("Lettura Apparent power non riuscita");
  }
  delay(MODBUS_DELAY);

  float reactive_power = SDM220readInputRegister(deviceAddr,REACTIVE_POWER);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Reactive power = ");
    Serial.println(reactive_power);
  }
  else {
    Serial.println("Lettura Reactive power non riuscita");
  }
  delay(MODBUS_DELAY);

  float power_factor = SDM220readInputRegister(deviceAddr,POWER_FACTOR);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Power factor = ");
    Serial.println(power_factor);
  }
  else {
    Serial.println("Lettura Power factor non riuscita");
  }
  delay(MODBUS_DELAY);

  float frequency = SDM220readInputRegister(deviceAddr,FREQUENCY);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Frequency = ");
    Serial.println(frequency);
  }
  else {
    Serial.println("Lettura Frequency non riuscita");
  }
  delay(MODBUS_DELAY);

  float import_active_energy = SDM220readInputRegister(deviceAddr,IMPORT_ACTIVE_ENERGY);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Import active energy = ");
    Serial.println(import_active_energy);
  }
  else {
    Serial.println("Lettura Import active energy non riuscita");
  }
  delay(MODBUS_DELAY);

  float export_active_energy = SDM220readInputRegister(deviceAddr,EXPORT_ACTIVE_ENERGY);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Export active energy = ");
    Serial.println(export_active_energy);
  }
  else {
    Serial.println("Lettura Export active energy non riuscita");
  }
  delay(MODBUS_DELAY);

  float import_reactive_energy = SDM220readInputRegister(deviceAddr,IMPORT_REACTIVE_ENERGY);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Import reactive energy = ");
    Serial.println(import_reactive_energy);
  }
  else {
    Serial.println("Lettura Import reactive energy non riuscita");
  }
  delay(MODBUS_DELAY);

  float export_reactive_energy = SDM220readInputRegister(deviceAddr,EXPORT_REACTIVE_ENERGY);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Export reactive energy = ");
    Serial.println(export_reactive_energy);
  }
  else {
    Serial.println("Lettura Export reactive energy non riuscita");
  }
  delay(MODBUS_DELAY);

  float maximum_total_system_power_demand = SDM220readInputRegister(deviceAddr,MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Maximum total system power demand = ");
    Serial.println(maximum_total_system_power_demand);
  }
  else {
    Serial.println("Lettura Maximum total system power demand non riuscita");
  }
  delay(MODBUS_DELAY);

  float total_active_energy = SDM220readInputRegister(deviceAddr,TOTAL_ACTIVE_ENERGY);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Total active energy = ");
    Serial.println(total_active_energy);
  }
  else {
    Serial.println("Lettura Total active energy non riuscita");
  }
  delay(MODBUS_DELAY);

  float total_reactive_energy = SDM220readInputRegister(deviceAddr,TOTAL_REACTIVE_ENERGY);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Total reactive energy = ");
    Serial.println(total_reactive_energy);
  }
  else {
    Serial.println("Lettura Total reactive energy non riuscita");
  }
  delay(MODBUS_DELAY);

*/

  Serial.print("Terminata la lettura dei registri. In totale ci sono stati ");
  Serial.print(erroriTot);
  Serial.println(" errori.");

if (!erroriTot) {
  Serial.println("Trasmetto messaggio su rete LoRaWan");
  if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
  } 
  else {
      Serial.println("Errore invio Messaggio LoRa!");
  }
}
  Serial.println("In attesa del prossimo ciclo di lettura e invio...");
  for (int i=10; i>0; i--) {
    Serial.print("-");
    Serial.print(i);
    Serial.print(" ");    
    delay(10000);
  }
  Serial.println("");

}

float rawbit_to_float(uint32_t val){
  // Riceve una variabile di 32 bit e restituisce il valore di tipo float corrispondente
  union
  {   
      uint32_t a ;
      float b ;
  } u ;
  u.a = val;
  return u.b;
}

uint32_t SDM220readInputRegister_raw(int id,int address){
  uint32_t val = 0XFFFF;
  if (!ModbusRTUClient.requestFrom(id, INPUT_REGISTERS, address, 2)) { 
    // in caso di errore imposto il flag errore a 1 e incremento il conteggio relativo al ciclo
    errore = 1;
    erroriTot ++;
    Serial.print("Errore lettura address: ");
    Serial.print(address);
    Serial.print(" ");
    Serial.println(ModbusRTUClient.lastError());
  }else{
    errore = 0; // resetto o confermo a 0 il flag errore
    uint16_t word1 = ModbusRTUClient.read();
    uint16_t word2 = ModbusRTUClient.read();
    val = word1 << 16 | word2;
    
  }
  return val;
}

