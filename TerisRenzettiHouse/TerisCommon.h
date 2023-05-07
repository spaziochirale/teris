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

#define RS485_BAUDRATE 9600 // Verificare che i dispositivi siano configurati con lo stesso baudrate

// Definizione del tempo di attesa in ms tra un'interrogazione del ModBus e la successiva
// è necessario per assicurarsi che il dispositivo abbia terminato la risposta prima
// di inviare una nuova interrogazione. Il parametro può essere sottoposto a tuning testando
// il comportamento del bus e dei device. Se riducendo il tempo le letture iniziano ad
// andare in timeout, il valore deve essere incrementato
#define MODBUS_DELAY 3000


LoRaModem modem; // Dichiarazione dell'oggetto per la gestione della trasmissione con protocollo LoRa

int errore =0; // flag usato per segnalare che è avvenuto un errore nella lettura ModBus
int erroriTot; // contatore degli errori totali riscontrati durante un ciclo della funzione loop

