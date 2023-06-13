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
  Scansione dei dispositivi Energy Meter Eastron SDM220-ModBus attivi sul Bus

  Testato su scheda Arduino MKR con Shield 485 MKR
  
  Configurazione degli switch dello shield:
  1 -> Ininfluente, non usato poiché lo shield in modalità HALF-DUPLEX lutilizza solo i terminali Y e Z
  2 -> OFF - Shield impostato per la modalità HALF-DUPLEX
  3 -> Raccomandato su ON per cablaggi lunghi e non schermati. ON = Resistore di terminazione sul bus RS485. 
       Su cablaggi corti e cavi di qualità funziona anche se OFF  

  Cablaggio del BUS RS485:
   Terminale Y dello Shield collegato ai terminali A del Bus 485 (pin 9 SDM220-ModBus)
   Terminale Z dello Shield collegato ai terminali B del Bus 485 (pin 8 SDM220-ModBus)   
  
  */


#include <ArduinoRS485.h> 
#include <ArduinoModbus.h>

#define RS485_BAUDRATE 9600 // Verificare che i dispositivi siano configurati con lo stesso Baudrate 

// Definizione dell'indirizzo di uno degli INPUT REGISTERS del meter SDM220
// riferimento documento: SDM220 ModBus Protocol v 1.1
#define C_REGISTERS_VOLTAGE  0x0000

void setup() {
  Serial.begin(9600); // inizializzo la comunicazione tra Arduino e Terminale seriale sul PC
  while(!Serial);  // attendo che la comunicazione seriale sia attiva
  Serial.println("");
  Serial.println("ModBus Simple Scanner - v.0.1 by Chirale");
  Serial.println("");
  Serial.print("Inizializzo il Client ModBusRTU BaudRate: ");
  Serial.println(RS485_BAUDRATE);
  
  if (!ModbusRTUClient.begin(RS485_BAUDRATE)) {
    Serial.println("Inizializzazione non riuscita!");
    while (1); // blocco l'esecuzione del programma con un loop infinito
  } 
  else Serial.println("Inizializzazione completata.");
}

void loop() {
 Serial.println("Inizio ciclo di scansione...");
 for(int id = 1; id <= 247; id++){  // Gli indirizzi dei dispositivi ModBus vanno da 1 a 247. 0 è l'indicatore broadcast, gli altri fino a 255 sono riservati
      Serial.print("Interrogo id=");
      Serial.print(id);
      if(ModbusRTUClient.requestFrom(id, INPUT_REGISTERS, C_REGISTERS_VOLTAGE, 2)){
        Serial.println(" RISPOSTA OK!");
        delay(5000); // Ha risposto, quindi attendo per far completare la trasmissione, evitando contese sul bus
      }
      else {
           Serial.print(" ERRORE: ");
           Serial.println(ModbusRTUClient.lastError());
      }
      //delay(500);         
      
  }
  Serial.println("Ciclo di scansione terminato.");
  

}
