/*
  PROGETTO TERIS CONTROL

  Author: Chirale S.r.l.
  
  Sketch per la definizione del device di controllo QUADRO TEST CHIRALE
  Il device effettua il monitoraggio di due dispositivi Eastron SDM220MDB
  
  Il device è realizzato medianter scheda Arduino MKR WAN 1310 con Shield 485 MKR
  
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


//   ----- DEFINIZIONE DELLA CONFIGURAZIONE MODBUS -----

#define RS485_BAUDRATE 9600 // Verificare il corretto valore del baudrate sul display dei dispositivi
#define EASTRON1 1 // il primo meter del quadro ha indirizzo 1
#define EASTRON2 2 // il secondo meter del quadro ha indirizzo 2

// Definizione del tempo di attesa in ms tra un'interrogazione del ModBus e la successiva
// è necessario per assicurarsi che il dispositivo abbia terminato la risposta prima
// di inviare una nuova interrogazione. Il parametro può essere sottoposto a tuning testando
// il comportamento del bus e dei device. Se riducendo il tempo le letture iniziano ad
// andare in timeout, il valore deve essere incrementato
#define MODBUS_DELAY 3000

// Definizione delle costanti di tempo utilizzate per valutare
// la frequenza di acquisizione dei dati - valori in ms
// Trattandosi di uno sketch di test usiamo intervalli molto brevi
#define TEN_MINUTES 600000L
#define THREE_MINUTES 180000L
#define SHORT_INTERVAL THREE_MINUTES
#define LONG_INTERVAL TEN_MINUTES

// Definizione dell'itervallo di attesa prima di ritentare un Join alla rete LoRaWan
#define JOIN_RETRY_INTERVAL 5000

// Impostazione delle credenziali per l'accesso del device all'Applicazione The things Network (TTN)
// Verificare i valori con quelli riportati nella console TTN per lo specifico device
String appEui = "0000000000000000" ;                       
String appKey = "33065E1BE7B1821ACC696DA9E23225D1"; 

void frequentDataCollection(){
/* Questa funzione effettua la lettura dei dati da eseguire
   con frequenza elevata e aggiorna la relativa variabile globale
   che tiene memoria del timestamp dell'ultima chiamata effettuata.
*/

EastronSDM220FrequentDataCollection(EASTRON1);
EastronSDM220FrequentDataCollection(EASTRON2);
timeLastFrequentCollection = millis();
}

void dailyDataCollection() {
/* Questa funzione effettua la lettura dei dati da eseguire
   con frequenza giornaliera o comunque non elevata e aggiorna 
   la relativa variabile globale che tiene memoria del timestamp dell'ultima chiamata effettuata.
*/   

EastronSDM220DailyDataCollection(EASTRON1);
EastronSDM220DailyDataCollection(EASTRON2);
timeLastDailyCollection = millis();
}

void setup() {
  Serial.begin(9600); // Inizializzo la comunicazione seriale con il PC-IDE Arduino
  delay(5000);  // attendo per assicurare che la connessione seriale sia disponibile
  
  Serial.println("");
  Serial.println("===========================================================================");
  Serial.println("");
  Serial.println("              QUADRO TEST CHIRALE Control Device  - v.2.0 by Chirale");
  Serial.println("");
  Serial.println("===========================================================================");
  Serial.println("");
  Serial.println("                          INIZIALIZZAZIONE");
  Serial.println("");
  Serial.println("");
  
  Serial.print("Inizializzazione Client ModBusRTU BaudRate: ");
  Serial.println(RS485_BAUDRATE);
  
  if (!ModbusRTUClient.begin(RS485_BAUDRATE)) {
    Serial.println("Inizializzazione non riuscita. Programma terminato.");
    while (1); // blocco l'esecuzione del programma con un loop infinito
  }
  Serial.println("");
  Serial.println("Inizializzazione modem LoRa...");
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

// Il device è appena avviato e quindi effettuo i primi cicli di lettura
frequentDataCollection();
dailyDataCollection();

}

void loop() {
   // verifico se è trascorso il tempo dell'intervallo breve oppure se millis() ha avuto l'overflow 
   // e in caso affermativo effettuo la lettura dei dati da raccogliere con frequenza più elevata
  if ((((millis()- timeLastFrequentCollection)>SHORT_INTERVAL)) or ((millis()- timeLastFrequentCollection) <0)) {
  frequentDataCollection();
  }
  // verifico se è trascorso il tempo dell'intervallo lungo oppure se millis() ha avuto l'overflow 
  // e in caso affermativo effettuo la lettura dei dati da raccogliere con frequenza meno elevata
  if ((((millis()- timeLastDailyCollection)>LONG_INTERVAL)) or ((millis()- timeLastDailyCollection) <0)) {
    dailyDataCollection();
  }


}
