/*
  PROGETTO TERIS CONTROL

  Author: Chirale S.r.l.
  
  Questo file contiene le definizioni comuni per il device
  - inclusione di librerie
  - macro globali
  - variabili globali
  
  */
#include <MKRWAN.h>
#include <ArduinoRS485.h> 
#include <ArduinoModbus.h>



LoRaModem modem; // Dichiarazione dell'oggetto per la gestione della trasmissione con protocollo LoRa

// Timestamp delle ultime letture di dati
unsigned long timeLastFrequentCollection, timeLastDailyCollection;

int errore =0; // flag usato per segnalare che è avvenuto un errore nella lettura ModBus
int erroriTot; // contatore degli errori totali riscontrati durante un ciclo della funzione loop

