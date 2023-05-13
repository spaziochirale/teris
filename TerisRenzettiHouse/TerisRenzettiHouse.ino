/*
  PROGETTO TERIS CONTROL

  Author: Chirale S.r.l.
  
  Sketch per la definizione del device di controllo RenzettiHouse
  
   Il device è costituito da scheda Arduino MKR WAN 1310 con Shield 485 MKR
   e gestisce la seguente configurazione:

   - 1 Inverter Sofar Solar Trad 1-40 Kw
   - 1 Meter Eastron SDM220
  
  Configurazione degli switch dello shield:
  1 -> Ininfluente, non usato poiché lo shield in modalità HALF-DUPLEX lutilizza solo i terminali Y e Z
  2 -> OFF - Shield impostato per la modalità HALF-DUPLEX
  3 -> Raccomandato su ON per cablaggi lunghi e non schermati. ON = Resistore di terminazione sul bus RS485. 
       Su cablaggi corti e cavi di qualità funziona anche se OFF  

  Cablaggio del BUS RS485:
   Terminale Y dello Shield collegato ai terminali A dei dispositivi
   Terminale Z dello Shield collegato ai terminali B dei dispositivi 
  
  */

#include "TerisCommon.h"

// Configurazione dell'impianto su ModBus
// Modificare con i corretti indirizzi ModBus
// e con il valore del BAUDRATE
#define RS485_BAUDRATE 9600
#define EASTRON220 1
#define SOFARTRAD 2

// Tempo definito in millisecondi
#define HALF_HOUR 1800000L
#define SIX_HOURS 21600000L

#define JOIN_RETRY_INTERVAL 5000

// Timestamp delle ultime letture di dati
unsigned long timeLastFrequentCollection, timeLastDailyCollection;

// Impostazione delle credenziali per l'accesso del device all'Applicazione The things Network (TTN)
// Verificare i valori con quelli riportati nella console TTN per lo specifico device
String appEui = "0000000000000000" ;                       
String appKey = "xxxxxxxxxxxxxxxx"; 


void setup() {
  Serial.begin(9600); // Inizializzo la comunicazione seriale con il PC-IDE Arduino
  delay(5000);  // attendo per assicurare che la connessione seriale sia disponibile
  
  Serial.println("");
  Serial.println("===========================================================================");
  Serial.println("");
  Serial.println("              Teris Control Device  - v.2.0 by Chirale");
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
  
  while(!modem.joinOTAA(appEui, appKey)) {
    Serial.println("Connessione alla rete LoRaWan non riuscita!");
    Serial.println("Attendo....");
    delay(JOIN_RETRY_INTERVAL);
    Serial.println("Riprovo la connessione alla rete LoRaWan");
  }

  Serial.println("Connessione LoRaWan OK.");
  delay(5000); // Attendo un tempo sufficente per consentire al modem di ricevere eventuali messaggi di downlink

// Effettuo i primi cicli di lettura

EastronSDM220FrequentDataCollection(EASTRON220);
SofarTrad40KwFrequentDataCollection(SOFARTRAD);
timeLastFrequentCollection = millis();

EastronSDM220DailyDataCollection(EASTRON220);
SofarTrad40KwDailyDataCollection(SOFARTRAD);
timeLastDailyCollection = millis();

}

void loop() {
  
  // verifico se è trascorsa mezz'ora oppure se millis() ha avuto l'overflow 
  // e in caso affermativo effettuo la lettura dei dati da raccogliere con frequenza elevata
  if ((((millis()- timeLastFrequentCollection)>HALF_HOUR)) or ((millis()- timeLastFrequentCollection) <0)) {
    EastronSDM220FrequentDataCollection(EASTRON220);
    SofarTrad40KwFrequentDataCollection(SOFARTRAD);
    timeLastFrequentCollection = millis();
  }
   // verifico se sono trascorse sei ore oppure se millis() ha avuto l'overflow 
  // e in caso affermativo effettuo la lettura dei dati da raccogliere con frequenza elevata
  if ((((millis()- timeLastDailyCollection)>SIX_HOURS)) or ((millis()- timeLastDailyCollection) <0)) {
    EastronSDM220DailyDataCollection(EASTRON220);
    SofarTrad40KwDailyDataCollection(SOFARTRAD);
    timeLastDaylyCollection = millis();
  }


}
