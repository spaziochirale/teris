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
  Lettura dell'Inverter Sofar Solar 1-40 kW g1

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


void setup() {

  Serial.begin(9600); // Inizializzo la comunicazione seriale con il PC-IDE Arduino
  while(!Serial);  // attendo finché la connessione seriale non è disponibile
  
  Serial.println("");
  Serial.println("Lettura Inverter Sofar Solar 1-40 KW g1- v.3.0 by Chirale");
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

  uint16_t pv1_voltage = SofarSolarReadHoldingRegister(1,PV1_VOLTAGE);
  Serial.print("Device 1 pv1_voltage register = ");
  Serial.print(pv1_voltage);
  Serial.print(" Volt =  ");
  Serial.println(pv1_voltage*0.1); // il valore del registro è in unità da 0,1 Volt
  delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus

  uint16_t pv1_current = SofarSolarReadHoldingRegister(1,PV1_CURRENT);
  Serial.print("Device 1 pv1_current register = ");
  Serial.print(pv1_current);
  Serial.print(" Ampere =  ");
  Serial.println(pv1_current*0.01); // il valore del registro è in unità da 0,01 Ampere
  delay(MODBUS_DELAY); 
  
  uint16_t pv2_voltage = SofarSolarReadHoldingRegister(1,PV2_VOLTAGE);
  Serial.print("Device 1 pv2_voltage register = ");
  Serial.print(pv2_voltage);
  Serial.print(" Volt =  ");
  Serial.println(pv2_voltage*0.1); // il valore del registro è in unità da 0,1 Volt
  delay(MODBUS_DELAY); 

  uint16_t pv2_current = SofarSolarReadHoldingRegister(1,PV2_CURRENT);
  Serial.print("Device 1 pv2_current register = ");
  Serial.print(pv2_current);
  Serial.print(" Ampere =  ");
  Serial.println(pv2_current*0.01); // il valore del registro è in unità da 0,01 Ampere
  delay(MODBUS_DELAY); 
   
  uint16_t pv1_power = SofarSolarReadHoldingRegister(1,PV1_POWER);
  Serial.print("Device 1 pv1_power register = ");
  Serial.print(pv1_power);
  Serial.print(" KWatt =  ");
  Serial.println(pv1_power*0.01); // il valore del registro è in unità da 0,01 KWatt
  delay(MODBUS_DELAY); 
     
  uint16_t pv2_power = SofarSolarReadHoldingRegister(1,PV2_POWER);
  Serial.print("Device 1 pv2_power register = ");
  Serial.print(pv2_power);
  Serial.print(" KWatt =  ");
  Serial.println(pv2_power*0.01); // il valore del registro è in unità da 0,01 KWatt
  delay(MODBUS_DELAY); 
     
  uint16_t active_power_output = SofarSolarReadHoldingRegister(1,ACTIVE_POWER_OUTPUT);
  Serial.print("Device 1 active_power_output register = ");
  Serial.print(active_power_output);
  Serial.print(" KWatt =  ");
  Serial.println(active_power_output*0.01); // il valore del registro è in unità da 0,01 KWatt
  delay(MODBUS_DELAY); 
      
  uint16_t total_production_hb = SofarSolarReadHoldingRegister(1,TOTAL_PRODUCTION_HB); // leggo i primi 16 bit
  uint16_t total_production_lb = SofarSolarReadHoldingRegister(1,TOTAL_PRODUCTION_LB); // leggo i secondi 16 bit
  uint32_t total_production = total_production_hb << 16 | total_production_lb; // Compongo il dato a 32 bit
  Serial.print("Device 1 total_production kWh = ");
  Serial.println(total_production); // il registro è in unità da 1 kWh
  delay(MODBUS_DELAY); 
   
  uint16_t daily_energy = SofarSolarReadHoldingRegister(1,DAILY_ENERGY);
  Serial.print("Device 1 daily_energy register = ");
  Serial.print(daily_energy);
  Serial.print(" kWh =  ");
  Serial.println(daily_energy*0.01); // il valore del registro è in unità da 0,01 kWh
  delay(MODBUS_DELAY); 
   
  uint16_t inverter_internal_temperature = SofarSolarReadHoldingRegister(1,INVERTER_INTERNAL_TEMPERATURE);
  Serial.print("Device 1 inverter_internal_temperature C = ");
  Serial.println(inverter_internal_temperature); // il valore del registro è in unità da 1 grado C
  delay(MODBUS_DELAY); 
      
  uint16_t inverter_bus_voltage = SofarSolarReadHoldingRegister(1,INVERTER_BUS_VOLTAGE);
  Serial.print("Device 1 inverter_bus_voltage register = ");
  Serial.print(inverter_bus_voltage);
  Serial.print(" Volt =  ");
  Serial.println(inverter_bus_voltage*0.1); // il valore del registro è in unità da 0,1 Volt
  delay(MODBUS_DELAY); 
 
  uint16_t inverter_alarm = SofarSolarReadHoldingRegister(1,INVERTER_ALARM);
  Serial.print("Device 1 inverter_alarm = ");
  Serial.println(inverter_alarm); // il manuale non specifica come interpretare il valore
  delay(MODBUS_DELAY); 
      
  uint16_t pv_insulation_resistance_to_ground = SofarSolarReadHoldingRegister(1,PV_INSULATION_RESISTANCE_TO_GROUND);
  Serial.print("Device 1 pv_insulation_resistance_to_ground = ");
  Serial.println(pv_insulation_resistance_to_ground); // il manuale non specifica come interpretare il valore
  delay(MODBUS_DELAY); 
 

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

