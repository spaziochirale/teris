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
  Test del protocollo di comunicazione e definizione del payload
  Trasmissione dati su rete LoRaWan The Things Network  

  Testato su scheda Arduino MKR WAN 1310 
*/

#include <MKRWAN.h>

/*
   Tabella per la decodifica dei parametri di trasmissione

   DataRate  	SF 	BandWidth 	bit/s
       0        12         125           250
       1        11         125           440
       2        10         125           980
       3         9         125         1.760
       4         8         125         3.125
       5         7         125         5.470
       6         7         250        11.000

*/
int tabellaDataRate[][3] = {12,125,250,
                           11,125,440,
                           10,125,980,
                            9,125,1760,
                            8,125,3125,
                            7,125,5470,
                            7,250,11000};


// Impostazione delle credenziali per l'accesso all'Applicazione The things Network (TTN)
// Verificare i valori con quelli riportati nella console TTN
String appEui = "0000000000000000" ;                       
String appKey = "33065E1BE7B1821ACC696DA9E23225D1"; 

LoRaModem modem; // Dichiarazione dell'oggetto per la gestione della trasmissione con protocollo LoRa

void setup() {

  Serial.begin(9600); // Inizializzo la comunicazione seriale con il PC-IDE Arduino
  while(!Serial);  // attendo finché la connessione seriale non è disponibile
  
  Serial.println("");
  Serial.println("===========================================================================");
  Serial.println("");
  Serial.println("Tester di Comunicazione su rete LoRaWan TTN  - v.1.0 by Chirale");
  Serial.println("");
  Serial.println("===========================================================================");
  Serial.println("");
  Serial.println("                          INIZIALIZZAZIONE");
  Serial.println("");
  if (!modem.begin(EU868)) {
    Serial.println("Inizializzazione del modem LoRa non riuscita!");
    Serial.println("Esecuzione del programma terminata.");
    while (1) {} // blocco l'esecuzione del programma con un loop infinito
  };
  Serial.print("Modem LoRa inizializzato.");
  Serial.print("Versione del modem LoRa: ");
  Serial.println(modem.version());
  Serial.print("DeviceEUI: ");  
  Serial.println(modem.deviceEUI()); 
  Serial.println("");   
  Serial.println("Connessione alla rete LoRaWan...");
  if(!modem.joinOTAA(appEui, appKey)) {
    Serial.println("Connessione alla rete LoRaWan non riuscita!");
    Serial.println("Esecuzione del programma terminata.");
    while(1) {}
  }

  Serial.println("Connessione LoRaWan OK.");
  delay(5000); // Attendo un tempo sufficente per consentire al modem di ricevere eventuali messaggi di downlink

}

///-----------------------------------------------------------

void loop() {
  
  Serial.println("");
  Serial.println("===========================================================================");
  Serial.println("Inizio il ciclo di verifica parametri rete, preparazione pacchetti e invio."); 
  Serial.println("---------------------------------------------------------------------------");
  Serial.println("");
  Serial.println("                          PRIMO PACCHETTO DATI");
  Serial.println("");
  
  Serial.println("Recupero dal Modem LoRa i parametri di rete...");
  Serial.println("");
  int adr = modem.getADR();
  int dataRate = modem.getDataRate(); // Recupero il valore dataRate impostato sul modem
  int sf = tabellaDataRate[dataRate][0]; // Leggo dalla tabella il valore Spreading Factor corrispondente
  int bw = tabellaDataRate[dataRate][1]; // Leggo dalla tabella il valore di Ampiezza di Banda corrispondente
  int bs = tabellaDataRate[dataRate][2]; // Leggo dalla tabella il valore del troughput corrispondente
  Serial.print("Adaptive Data Rate ADR = ");
  Serial.println(adr);
  Serial.print("dataRate = ");
  Serial.println(dataRate);
  Serial.print("Spreading Factor (SF) = ");
  Serial.println(sf);
  Serial.print("Band Widht = ");
  Serial.println(bw);
  Serial.print("Velocità (bit/s) = ");
  Serial.println(bs);
  Serial.println("");
  Serial.println("");
  
  uint8_t fport = 1; // Imposto a 1 la variabile fport
  modem.setPort(fport); // Imposto il parametro fport ad 1

  modem.beginPacket(); // Preparo il pacchetto di dati

  uint8_t buf[] = {0X0A, 0X0B}; // Inizializzo un array di byte contenente i due valori esadecimali 0A e 0B (10 e 11 in decimale)
  modem.write(buf,2); // Aggiungo l'array di lunghezza due byte al pacchetto
 
  Serial.print("Trasmetto su rete LoRaWan il messaggio con payload= ");
  Serial.print(buf[0],HEX);
  Serial.print(buf[1],HEX);
  Serial.print(" e indicatore fport= ");
  Serial.println(fport);
  Serial.println("");
  if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
  } 
  else {
      Serial.println("Errore invio Messaggio LoRa!");
  }
  Serial.println("");

  delay(10000); // attendo 10 secondi per rispettare la fair use policy
  Serial.println("---------------------------------------------------------------------------");
  Serial.println("");
  Serial.println("                          SECONDO PACCHETTO DATI");
  Serial.println("");
  Serial.println("Recupero dal Modem LoRa i parametri di rete...");
  Serial.println("");
  adr = modem.getADR(); // Recupero dal modem il valore del flag ADR
  dataRate = modem.getDataRate(); // Recupero il valore dataRate impostato sul modem
  sf = tabellaDataRate[dataRate][0]; // Leggo dalla tabella il valore Spreading Factor corrispondente
  bw = tabellaDataRate[dataRate][1]; // Leggo dalla tabella il valore di Ampiezza di Banda corrispondente
  bs = tabellaDataRate[dataRate][2]; // Leggo dalla tabella il valore del troughput corrispondente
  Serial.print("Adaptive Data Rate ADR = ");
  Serial.println(adr);
  Serial.print("dataRate = ");
  Serial.println(dataRate);
  Serial.print("Spreading Factor (SF) = ");
  Serial.println(sf);
  Serial.print("Band Widht = ");
  Serial.println(bw);
  Serial.print("Velocità (bit/s) = ");
  Serial.println(bs);
  Serial.println("");
  Serial.println("");
  
  fport = 2; // Imposto a 2 la variabile fport in modo da inviare un secondo pacchetto con diverso fport
  modem.setPort(fport); // Imposto il parametro fport ad 1

  modem.beginPacket(); // Preparo il pacchetto di dati

  buf[0] = 0X0C;
  buf[1] = 0X0D; // modifico l'array di byte inserendo i due valori esadecimali 0C e 0D (12 e 13 in decimale)
  modem.write(buf,2); // Aggiungo l'array di lunghezza due byte al pacchetto
 
  Serial.print("Trasmetto su rete LoRaWan il messaggio con payload= ");
  Serial.print(buf[0],HEX);
  Serial.print(buf[1],HEX);
  Serial.print(" e indicatore fport= ");
  Serial.println(fport);
  Serial.println("");
  if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
  } 
  else {
      Serial.println("Errore invio Messaggio LoRa!");
  }

  Serial.println("");
  Serial.println("---------------------------------------------------------------------------");
  Serial.println("");
  Serial.println("In attesa del prossimo ciclo di lettura e invio...");

  // Count down per lasciar trascorrere un tempo sufficiente a rispettare la Fair Use Policy di TTN
  for (int i=10; i>0; i--) {
    Serial.print("-");
    Serial.print(i);
    Serial.print(" ");    
    delay(10000);
  }
  Serial.println("");

}

