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
  Lettura dell'Inverter Sofar Solar

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

#include <ArduinoRS485.h> 
#include <ArduinoModbus.h>
#include <String>
#include <iostream>

#define RS485_BAUDRATE 9600 // Verificare che i dispositivi siano configurati con lo stesso baudrate

// Definizione degli indirizzi iniziali degli HOLDING REGISTERS dell'inverter Sofar Solar
// riferimento documento: Manuale ModBus Sofar Solar

#define C_REGISTERS_VOLTAGE_PV1 0x0006
#define C_REGISTERS_CURRENT_PV1 0x0007
#define C_REGISTERS_VOLTAGE_PV2 0x0008
#define C_REGISTERS_CURRENT_PV2 0x0009
#define C_REGISTERS_POWER_PV1 0x000A
#define C_REGISTERS_POWER_PV2 0x000B
#define C_REGISTERS_OUTPUT_ACTIVE_POWER 0x000C
#define C_REGISTERS_TOTAL_PRODUCTION_HB 0x0015
#define C_REGISTERS_TOTAL_PRODUCTION_LB 0x0016
#define C_REGISTERS_TODAY_GENERATION 0x0019

#define C_REGISTERS_GRID_FREQUENCY 0x000E
#define C_REGISTERS_INVERTER_MODULE_TEMPERATURE 0x001B
#define C_REGISTERS_INVERTER_INNER_TEMPERATURE 0x001C
#define C_REGISTERS_INVERTER_BUS_VOLTAGE 0x001D
#define C_REGISTERS_INVERTER_ALERT_MESSAGE 0x0021
#define C_REGISTERS_INVERTER_COMMUNICATION_BOARD_INNER_MESSAGE 0x0023


void setup() {

  Serial.begin(9600); // Inizializzo la comunicazione seriale con il PC-IDE Arduino
  while(!Serial);  // attendo finché la connessione seriale non è disponibile
  
  Serial.println("");
  Serial.println("Lettura Inverter Sofar Solar - v.2.1 by Chirale");
  Serial.println("");
  Serial.print("Inizializzo il Client ModBusRTU BaudRate: ");
  Serial.println(RS485_BAUDRATE);
  
  if (!ModbusRTUClient.begin(RS485_BAUDRATE)) {
    Serial.println("Inizializzazione non riuscita!");
    while (1); // blocco l'esecuzione del programma con un loop infinito
  } 
  else Serial.println("Inizializzazione completata.");
  
}

///-----------------------------------------------------------
void loop() {
  
  // definisco una variabile esattamente di 16 bit di tipo intero senza segno
  // dalla documentazione dell'inverter nel registro omonimo il valore presente
  // è un intero di 16 bit, senza segno che rappresenta PV1 in unità da 0.1 volt
  uint16_t voltage_pv1; 

  voltage_pv1 = SofarSolarReadHoldingRegister(1,C_REGISTERS_VOLTAGE_PV1);
  Serial.print("Device 1 Voltage_pv1 = ");
  Serial.println(voltage_pv1);
  delay(5000); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus

  // aggiungere da questo punto in poi eventuali altre operazioni di lettura
  
  
  
}

uint16_t SofarSolarReadHoldingRegister(int id,int address){
  
  if (!ModbusRTUClient.requestFrom(id, HOLDING_REGISTERS, address, 1)) { 
    Serial.print("Errore lettura address: ");
    Serial.print(address);
    Serial.print(" ");
    Serial.println(ModbusRTUClient.lastError());
    return 0xFFFF; //Restituisco il valore massimo per segnalare l'errore di lettura
  }else{
    return ModbusRTUClient.read();
  }
  
}

