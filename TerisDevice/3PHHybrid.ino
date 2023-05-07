/*
  PROGETTO TERIS CONTROL

  Author: Chirale S.r.l.
  
  Funzioni di lettura e trasmissione dati per Inverter ZCS 3PH Hybrid
  
  Testato su scheda Arduino MKR WAN 1310 con Shield 485 MKR
  
  Configurazione degli switch dello shield:
  1 -> Ininfluente, non usato poiché lo shield in modalità HALF-DUPLEX lutilizza solo i terminali Y e Z
  2 -> OFF - Shield impostato per la modalità HALF-DUPLEX
  3 -> Raccomandato su ON per cablaggi lunghi e non schermati. ON = Resistore di terminazione sul bus RS485. 
       Su cablaggi corti e cavi di qualità funziona anche se OFF  

  Cablaggio del BUS RS485:
   Terminale Y dello Shield collegato ai terminali A dei dispositivi 
   Terminale Z dello Shield collegato ai terminali B dei dispositivi     
  
  */
// Definizione degli indirizzi iniziali degli HOLDING REGISTERS degli inverter 
// riferimento documento: Manuale ModBus ZCS Inverter Ibridi

// Dati da acquisire con frequenza elevata (ogni 15 min) in orario diurno
#define HY_PV1_VOLTAGE 0x0584
#define HY_PV1_CURRENT 0x0585
#define HY_PV2_VOLTAGE 0x0587
#define HY_PV2_CURRENT 0x0588
#define HY_BAT1_VOLTAGE 0x0604
#define HY_BAT1_CURRENT 0x0605
#define HY_BAT1_ENV_TEMPERATURE 0x0607
#define HY_BAT1_SOC 0x0608
#define HY_ACTIVE_POWER_OUTPUT_TOTAL 0x0485
#define HY_REACTIVE_POWER_OUTPUT_TOTAL 0x0486
#define HY_ACTIVE_POWER_PCC_TOTAL 0x0488

// Dati da acquisire con frequenza elevata (ogni 15 min) H24
#define HY_FREQUENCY_GRID 0x0484
#define HY_ACTIVE_POWER_LOAD_TOTAL 0x0504
#define HY_REACTIVE_POWER_LOAD_TOTAL 0x0505

// Dati da acquisire con frequenza giornaliera alle 20:30
// Sono tutte registri a 32 bit!
#define HY_PV_GENERATION_TODAY 0x0684 
#define HY_PV_GENERATION_TOTAL 0x0686
#define HY_LOAD_CONSUMPTION_TODAY 0x0688 
#define HY_LOAD_CONSUMPTION_TOTAL 0x068A
#define HY_ENERGY_PURCHASE_TODAY 0x068C 
#define HY_ENERGY_PURCHASE_TOTAL 0x068E
#define HY_ENERGY_SELLING_TODAY 0x0690 
#define HY_ENERGY_SELLING_TOTAL 0x0692
#define HY_BAT_CHARGE_TODAY 0x0694 
#define HY_BAT_CHARGE_TOTAL 0x0696   
#define HY_BAT_DISCHARGE_TODAY 0x0698 
#define HY_BAT_DISCHARGE_TOTAL 0x069A

// DA COMPLETARE - WORK IN PROGRESS
