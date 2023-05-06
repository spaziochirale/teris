/*
  PROGETTO TERIS CONTROL

  Author: Chirale S.r.l.
  
  Sketch per la definizione del device di controllo
  
   Il device si presume realizzato medianter scheda Arduino MKR WAN 1310 con Shield 485 MKR
  
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

// Impostazione delle credenziali per l'accesso del device all'Applicazione The things Network (TTN)
// Verificare i valori con quelli riportati nella console TTN per lo specifico device
String appEui = "0000000000000000" ;                       
String appKey = "33065E1BE7B1821ACC696DA9E23225D1"; 


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
