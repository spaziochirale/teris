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
  Lettura e invio su TTN dell'Inverter Sofar Solar 1-40 kW g1

  Testato su scheda Arduino MKR WAN 1310 con Shield 485 MKR
  
  Configurazione degli switch dello shield:
  1 -> Ininfluente, non usato poiché lo shield in modalità HALF-DUPLEX lutilizza solo i terminali Y e Z
  2 -> OFF - Shield impostato per la modalità HALF-DUPLEX
  3 -> Raccomandato su ON per cablaggi lunghi e non schermati. ON = Resistore di terminazione sul bus RS485. 
       Su cablaggi corti e cavi di qualità funziona anche se OFF  

  Cablaggio del BUS RS485:
   Terminale Y dello Shield collegato ai terminali A degli inverter
   Terminale Z dello Shield collegato ai terminali B degli inverter
  
  */

/* Questo sketch interroga un singolo inverter che si presuppone di ID pari ad 1. Deve essere modificato per adattarsi alla configurazione effettiva
   */

#include <MKRWAN.h>
#include <ArduinoRS485.h> 
#include <ArduinoModbus.h>

#define RS485_BAUDRATE 9600 // Verificare che i dispositivi siano configurati con lo stesso baudrate

// Definizione degli indirizzi iniziali degli HOLDING REGISTERS dell'inverter Sofar Solar
// riferimento documento: Manuale ModBus Sofar Solar

#define PV1_VOLTAGE 0x0006
#define PV1_CURRENT 0x0007
#define PV2_VOLTAGE 0x0008
#define PV2_CURRENT 0x0009
#define PV1_POWER 0x000A
#define PV2_POWER 0x000B
#define ACTIVE_POWER_OUTPUT 0x000C
#define TOTAL_PRODUCTION_HB 0x0015
#define TOTAL_PRODUCTION_LB 0x0016
#define DAILY_ENERGY 0x0019
#define INVERTER_INTERNAL_TEMPERATURE 0x001C
#define INVERTER_BUS_VOLTAGE 0x001D
#define INVERTER_ALARM 0x0021
#define PV_INSULATION_RESISTANCE_TO_GROUND 0x0026

#define MODBUS_DELAY 5000

// Impostazione delle credenziali per l'accesso all'Applicazione The things Network (TTN)
// Verificare i valori con quelli riportati nella console TTN
String appEui = "0000000000000000" ;                       
String appKey = "33065E1BE7B1821ACC696DA9E23225D1"; 

LoRaModem modem; // Dichiarazione dell'oggetto per la gestione della trasmissione con protocollo LoRa

void setup() {

  Serial.begin(9600); // Inizializzo la comunicazione seriale con il PC-IDE Arduino
  delay(5000);  // attendo per essere sicuro che la connessione seriale sia disponibile
  
  Serial.println("");
  Serial.println("Lettura Inverter Sofar Solar 1-40 KW g1- v.3.0 by Chirale");
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
    Serial.println("Inizializzazione non riuscita!");
    while (1); // blocco l'esecuzione del programma con un loop infinito
  } 
  else Serial.println("Inizializzazione completata.");

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

int errore =0; // flag usato per segnalare che è avvenuto un errore nella lettura ModBus
int erroriTot; // contatore degli errori totali riscontrati durante un ciclo della funzione loop

///-----------------------------------------------------------
void loop() {

  union {
    uint16_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Sofar a 16 bit
    byte buf[2]; // Rappresentazione della stessa variabile come array di 2 byte utile per preparare il pacchetto dati LoRa
  } u;

union {
    uint32_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Sofar a 32 bit
    byte buf[4]; // Rappresentazione della stessa variabile come array di 4 byte utile per preparare il pacchetto dati LoRa
  } u32;

  Serial.println("Ciclo di lettura e invio..."); 

  erroriTot = 0; // Inizia un nuovo ciclo per cui resetto il numero totale degli errori

  int deviceAddr = 1; // indirizzo del dispositivo sul bus ModBus

  modem.beginPacket(); // Preparo il pacchetto di dati


  u.bitRegister = SofarSolarReadHoldingRegister(deviceAddr,PV1_VOLTAGE);
  if (!errore){
    // Se la funzione di lettura ModBus non ha settato il flag errore
    // posso procedere a visualizzare sull'eventuale serial monitor i valori 
    // sia in chiaro, applicando le formule riportate sul manuale, che come valore esadecimale
    // utile per confrontarlo con il valore ricevuto sul server LoRaWan
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" pv1_voltage register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Volt =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Volt
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    // a questo punto, accodo i 2 byte del registro nel pacchetto dati
    // utilizzando la rappresentazione del registro sotto forma di array (campo buf della union)
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura pv1_voltage non riuscita");
  }
  delay(MODBUS_DELAY);
  

  u.bitRegister = SofarSolarReadHoldingRegister(deviceAddr,PV1_CURRENT);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" pv1_current register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Ampere =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Ampere
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura pv1_current non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = SofarSolarReadHoldingRegister(deviceAddr,PV2_VOLTAGE);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" pv2_voltage register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Volt =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Volt
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura pv2_voltage non riuscita");
  }
  delay(MODBUS_DELAY);
  
u.bitRegister = SofarSolarReadHoldingRegister(deviceAddr,PV2_CURRENT);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" pv2_current register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Ampere =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Ampere
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura pv2_current non riuscita");
  }
  delay(MODBUS_DELAY);

 u.bitRegister = SofarSolarReadHoldingRegister(deviceAddr,PV1_POWER);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" pv1_power register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" KWatt =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 KWatt
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura pv1_power non riuscita");
  }
  delay(MODBUS_DELAY);
   
 u.bitRegister = SofarSolarReadHoldingRegister(deviceAddr,PV2_POWER);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" pv2_power register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" KWatt =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 KWatt
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura pv2_power non riuscita");
  }
  delay(MODBUS_DELAY);

 u.bitRegister = SofarSolarReadHoldingRegister(deviceAddr,ACTIVE_POWER_OUTPUT);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" active_power_output register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" KWatt =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 KWatt
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura active_power_output non riuscita");
  }
  delay(MODBUS_DELAY);
 
 u32.bitRegister = SofarSolarReadHoldingRegister32(deviceAddr,TOTAL_PRODUCTION_HB); // Specifico l'indirizzo del primo rsgistro a 16 bit
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" total_production register = ");
    Serial.print(u32.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u32.bitRegister); // il valore del registro è in unità da 0,01 KWatt
    modem.write(u32.buf[3]); 
    modem.write(u32.buf[2]);
    modem.write(u32.buf[1]); 
    modem.write(u32.buf[0]);
  }
  else {
    Serial.println("Lettura total_production non riuscita");
  }
  delay(MODBUS_DELAY);
  
 u.bitRegister = SofarSolarReadHoldingRegister(deviceAddr,DAILY_ENERGY);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" daily_energy register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Kwh
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura active_power_output non riuscita");
  }
  delay(MODBUS_DELAY);

 
 u.bitRegister = SofarSolarReadHoldingRegister(deviceAddr,INVERTER_INTERNAL_TEMPERATURE);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" inverter_internal_temperature = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" C =  ");
    Serial.println(u.bitRegister); // il valore del registro è in unità da 1 grado C
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura inverter_internal_temperature non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = SofarSolarReadHoldingRegister(deviceAddr,INVERTER_BUS_VOLTAGE);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" inverter_bus_voltage register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Volt =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Volt
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura inverter_bus_voltage non riuscita");
  }
  delay(MODBUS_DELAY);
 
  u.bitRegister = SofarSolarReadHoldingRegister(deviceAddr,INVERTER_ALARM);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" inverter_alarm register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Valore decimale =  ");
    Serial.println(u.bitRegister); // il manuale non specifica il significato
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura inverter_alarm non riuscita");
  }
  delay(MODBUS_DELAY);
 
  u.bitRegister = SofarSolarReadHoldingRegister(deviceAddr,PV_INSULATION_RESISTANCE_TO_GROUND);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" pv_insulation_resistance_to_ground register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Valore decimale =  ");
    Serial.println(u.bitRegister); // il manuale non specifica il significato
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura pv_insulation_resistance_to_ground non riuscita");
  }
  delay(MODBUS_DELAY);

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
  for (int i=120; i>0; i--) {
    Serial.print("-");
    Serial.print(i);
    Serial.print(" ");    
    delay(10000);
  }
  Serial.println("");


}

// Lettura registro singolo da 16 bit
uint16_t SofarSolarReadHoldingRegister(int id,int address){
  
  if (!ModbusRTUClient.requestFrom(id, HOLDING_REGISTERS, address, 1)) { 
    // in caso di errore imposto il flag errore a 1 e incremento il conteggio relativo al ciclo
    errore = 1;
    erroriTot++;
    Serial.print("Errore lettura address: ");
    Serial.print(address);
    Serial.print(" ");
    Serial.println(ModbusRTUClient.lastError());
    return 0xFFFF; //Restituisco il valore massimo per segnalare l'errore di lettura
  }else{
    errore = 0; // resetto o confermo a 0 il flag errore
    return ModbusRTUClient.read();
  }
  
}

// Lettura registro doppio da 32 bit
uint32_t SofarSolarReadHoldingRegister32(int id,int address){
  
  if (!ModbusRTUClient.requestFrom(id, HOLDING_REGISTERS, address, 2)) { 
    // in caso di errore imposto il flag errore a 1 e incremento il conteggio relativo al ciclo
    errore = 1;
    erroriTot++;
    Serial.print("Errore lettura address: ");
    Serial.print(address);
    Serial.print(" ");
    Serial.println(ModbusRTUClient.lastError());
    return 0xFFFF; //Restituisco il valore massimo per segnalare l'errore di lettura
  }else{
    errore = 0; // resetto o confermo a 0 il flag errore
    uint16_t hb = ModbusRTUClient.read();
    uint16_t lb = ModbusRTUClient.read();
    uint32_t doubleReg = hb << 16 | lb;
    return doubleReg;
  }
  
}

