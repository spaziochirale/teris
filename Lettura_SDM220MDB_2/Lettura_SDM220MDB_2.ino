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
  Lettura dei registri di interesse dell'Energy Meater EASTRON SDM220MODBUS Versione 2.0
  senza trasmissione su rete LoRa  

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

#include <ArduinoRS485.h> 
#include <ArduinoModbus.h>


#define RS485_BAUDRATE 9600 // Verificare che i dispositivi siano configurati con lo stesso baudrate

// Definizione degli indirizzi iniziali degli INPUT REGISTERS di interesse del meter SDM220
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

void setup() {

  Serial.begin(9600); // Inizializzo la comunicazione seriale con il PC-IDE Arduino
  while(!Serial);  // attendo finché la connessione seriale non è disponibile
  
  Serial.println("");
  Serial.println("Lettura Energy Meter Eastron SCM220-MODBUS - v.2.2 by Chirale");
  Serial.println("");

  Serial.print("Inizializzo il Client ModBusRTU BaudRate: ");
  Serial.println(RS485_BAUDRATE);
  
  if (!ModbusRTUClient.begin(RS485_BAUDRATE)) {
    Serial.println("Inizializzazione ModBus non riuscita!");
    while (1); 
  } 
  else Serial.println("Inizializzazione ModBus completata.");

  Serial.println("");
  

}


///-----------------------------------------------------------
int errore =0; // flag usato per segnalare che è avvenuto un errore nella lettura ModBus
int erroriTot; // contatore degli errori totali riscontrati durante un ciclo della funzione loop

void loop() {
  
  Serial.println("Ciclo di lettura e invio..."); 

  erroriTot = 0; // Inizia un nuovo ciclo per cui resetto il numero totale degli errori

  int deviceAddr = 1; // indirizzo del dispositivo sul bus ModBus

  float voltage = SDM220readInputRegister(deviceAddr,VOLTAGE);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Voltage = ");
    Serial.println(voltage);
  }
  else {
    Serial.println("Lettura Voltage non riuscita");
  }
  delay(MODBUS_DELAY);
  
  float current = SDM220readInputRegister(deviceAddr,CURRENT);
  if (!errore) {
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Current = ");
    Serial.println(current);
  }
  else {
    Serial.println("Lettura Current non riuscita");
  }
  delay(MODBUS_DELAY);

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


  Serial.print("Terminata la lettura dei registri. In totale ci sono stati ");
  Serial.print(erroriTot);
  Serial.println(" errori.");

}

float SDM220readInputRegister(int id,int address){
  union
  {   
      unsigned long a ;
      float b ;
  } u ;
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
    u.a = word1 << 16 | word2;
    
  }
  return u.b;
}

