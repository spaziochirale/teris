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
  Lettura dell'Energy Meater EASTRON SDM220MODBUS
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
#define C_REGISTERS_VOLTAGE  0x0000
#define C_REGISTERS_CURRENT  0x0006
#define C_REGISTERS_IMPORT_ACTIVE_ENERGY  0x0048   
#define C_REGISTERS_EXPORT_ACTIVE_ENERGY  0x004A  
#define C_REGISTERS_TOTAL_ACTIVE_ENERGY   0x0156     

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
void loop() {
  
  float current;
  float voltage;
  
  String bufferStr ="";

  Serial.println("Ciclo di lettura e invio..."); 

  voltage = SDM220readInputRegister(1,C_REGISTERS_VOLTAGE);
  Serial.print("Device 1 Voltage = ");
  Serial.println(voltage);
  delay(5000);
  current = SDM220readInputRegister(1,C_REGISTERS_CURRENT);
  Serial.print("Device 1 Current = ");
  Serial.println(current);
  delay(5000);

  bufferStr.concat(voltage);
  bufferStr.concat("|");
  bufferStr.concat(current);
  bufferStr.concat("|");
    
  voltage = SDM220readInputRegister(2,C_REGISTERS_VOLTAGE);
  Serial.print("Device 2 Voltage = ");
  Serial.println(voltage);
  delay(5000);
  current = SDM220readInputRegister(2,C_REGISTERS_CURRENT);
  Serial.print("Device 2 current = ");
  Serial.println(current);
  delay(5000);

  bufferStr.concat(voltage);
  bufferStr.concat("|");
  bufferStr.concat(current);

  Serial.print("Trasmetto messaggio: ");
  Serial.println(bufferStr);

  modem.beginPacket();
  modem.print(bufferStr);
  if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
  } 
  else {
      Serial.println("Errore invio Messaggio LoRa!");
  }
  Serial.println("In attesa del prossimo ciclo di lettura e invio...");
  for (int i=10; i>0; i--) {
    Serial.print("-");
    Serial.print(i);
    Serial.print(" ");    
    delay(30000);
  }
  Serial.println("");
  
}

float SDM220readInputRegister(int id,int address){
  union
  {   
      unsigned long a ;
      float b ;
  } u ;
  if (!ModbusRTUClient.requestFrom(id, INPUT_REGISTERS, address, 2)) { 
    Serial.print("Errore lettura address: ");
    Serial.print(address);
    Serial.print(" ");
    Serial.println(ModbusRTUClient.lastError());
  }else{
    uint16_t word1 = ModbusRTUClient.read();
    uint16_t word2 = ModbusRTUClient.read();
    u.a = word1 << 16 | word2;
    
  }
  return u.b;
}

