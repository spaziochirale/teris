/*
  PROGETTO TERIS CONTROL

  Author: Chirale S.r.l.
  
  Funzioni di lettura e trasmissione dati per Energy Meater EASTRON SDM630
  
  Testato su scheda Arduino MKR WAN 1310 con Shield 485 MKR
  
  Configurazione degli switch dello shield:
  1 -> Ininfluente, non usato poiché lo shield in modalità HALF-DUPLEX lutilizza solo i terminali Y e Z
  2 -> OFF - Shield impostato per la modalità HALF-DUPLEX
  3 -> Raccomandato su ON per cablaggi lunghi e non schermati. ON = Resistore di terminazione sul bus RS485. 
       Su cablaggi corti e cavi di qualità funziona anche se OFF  

  Cablaggio del BUS RS485:
   Terminale Y dello Shield collegato ai terminali A dei meter 
   Terminale Z dello Shield collegato ai terminali B dei meter     
  
  */




// Definizione degli indirizzi iniziali degli INPUT REGISTERS del meter SDM630
// per i soli dati di interesse
// riferimento documento: SDM630 ModBus Protocol 

// Dati da acquisire con frequenza elevata (ogni 15 min.)
// Primo blocco
#define PH1_VOLTAGE  0x0000
#define PH2_VOLTAGE  0x0002
#define PH3_VOLTAGE  0x0004
#define PH1_CURRENT  0x0006
#define PH2_CURRENT  0x0008
#define PH3_CURRENT  0x000A
#define PH1_POWER_FACTOR 0x001E
#define PH2_POWER_FACTOR 0x0020
#define PH3_POWER_FACTOR 0x0022
// Secondo blocco
#define FREQUENCY  0x0046
#define NEUTRAL_CURRENT 0x00E0
#define AVERAGE_LINE_TO_NEUTRAL_VOLTS_THD 0x00F8
#define AVERAGE_LINE_CURRENT_THD 0x00FA
#define AVERAGE_LINE_TO_LINE_VOLTS_THD 0x0154


// Dati da acquisire giornalmente (ogni 24 ore)
#define IMPORT_ACTIVE_ENERGY  0x0048   
#define EXPORT_ACTIVE_ENERGY  0x004A  
#define IMPORT_REACTIVE_ENERGY  0x004C
#define EXPORT_REACTIVE_ENERGY  0x004E


/*
                 * EastronSDM630FrequentDataCollection1 *

  Questa funzione effettua la lettura del primo gruppo di registri del dispositivo SDM630
  che devono essere acquisiti con frequenza elevata, compone il pacchetto
  di dati di tipo 20 (v. protocollo applicativo TERIS), imposta fport a 20
  e trasmette il pacchetto sulla rete LoRa

  Vengono utilizzate le seguenti variabili globali:
   - modem
   - errore
   - erroriTot
*/
void EastronSDM630FrequentDataCollection1(int deviceAddr) {
  
  union {
    uint32_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Eanstron SDM220 a 32 bit
    byte buf[4]; // Rappresentazione della stessa variabile come array di 4 byte utile per preparare il pacchetto dati LoRa
  } u;

  Serial.print("EastronSDM630 lettura e invio primo blocco dati frequenti del meter ModBus Addr: "); 
  Serial.println(deviceAddr);

  erroriTot = 0; // Inizia un nuovo ciclo di lettura per cui resetto il numero totale degli errori

  modem.beginPacket(); // Preparo il pacchetto di dati

  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,PH1_VOLTAGE);
  if (!errore) {
    // Se la funzione di lettura ModBus non ha settato il flag errore
    // posso procedere a visualizzare sull'eventuale serial monitor i valori 
    // sia in chiaro, come numero con virgola, che come valore esadecimale
    // utile per confrontarlo con il valore ricevuto sul server LoRaWan
    float ph1_voltage = rawBitToFloat(u.bitRegister); // converto i 4 byte del regsitro nel tipo float
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" PH1 Voltage = ");
    Serial.print(ph1_voltage);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    // a questo punto, accodo uno per volta i 4 byte del registro nel pacchetto dati
    // utilizzando la rappresentazione del registro sotto forma di array (campo buf della union)
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    // inutile visualizzare e trasmettere i dati se la lettura non è riuscita
    Serial.println("Lettura PH1 Voltage non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,PH2_VOLTAGE);
  if (!errore) {
    float ph2_voltage = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" PH2 Voltage = ");
    Serial.print(ph2_voltage);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura PH2 Voltage non riuscita");
  }
  delay(MODBUS_DELAY);


  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,PH3_VOLTAGE);
  if (!errore) {
    float ph3_voltage = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" PH3 Voltage = ");
    Serial.print(ph3_voltage);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura PH3 Voltage non riuscita");
  }
  delay(MODBUS_DELAY);


  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,PH1_CURRENT);
  if (!errore) {
    float ph1_current = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" PH1 Current = ");
    Serial.print(ph1_current);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura PH1 Current non riuscita");
  }
  delay(MODBUS_DELAY);
 
  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,PH2_CURRENT);
  if (!errore) {
    float ph2_current = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" PH2 Current = ");
    Serial.print(ph2_current);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura PH2 Current non riuscita");
  }
  delay(MODBUS_DELAY);
  

  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,PH3_CURRENT);
  if (!errore) {
    float ph3_current = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" PH3 Current = ");
    Serial.print(ph3_current);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura PH3 Current non riuscita");
  }
  delay(MODBUS_DELAY);
  
 
  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,PH1_POWER_FACTOR);
  if (!errore) {
    float ph1_power_factor = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" PH1 Power Factor = ");
    Serial.print(ph1_power_factor);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura PH1 Power Factor non riuscita");
  }
  delay(MODBUS_DELAY);
  
 
  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,PH2_POWER_FACTOR);
  if (!errore) {
    float ph2_power_factor = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" PH2 Power Factor = ");
    Serial.print(ph2_power_factor);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura PH2 Power Factor non riuscita");
  }
  delay(MODBUS_DELAY);
 
  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,PH3_POWER_FACTOR);
  if (!errore) {
    float ph3_power_factor = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" PH3 Power Factor = ");
    Serial.print(ph3_power_factor);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura PH3 Power Factor non riuscita");
  }
  delay(MODBUS_DELAY);
  
  Serial.print("Terminata la lettura dei registri. In totale ci sono stati ");
  Serial.print(erroriTot);
  Serial.println(" errori.");

  if (!erroriTot) {
    Serial.println("Trasmetto messaggio su rete LoRaWan");
    modem.setPort(20); // imposto fport a 20 per identificare il tipo pacchetto
    if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
    } 
    else {
      Serial.println("Errore invio Messaggio LoRa!");
    }
  }
}


/*
                 * EastronSDM630FrequentDataCollection2 *

  Questa funzione effettua la lettura del secondo gruppo di registri del dispositivo SDM630
  che devono essere acquisiti con frequenza elevata, compone il pacchetto
  di dati di tipo 21 (v. protocollo applicativo TERIS), imposta fport a 21
  e trasmette il pacchetto sulla rete LoRa

  Vengono utilizzate le seguenti variabili globali:
   - modem
   - errore
   - erroriTot
*/
void EastronSDM630FrequentDataCollection2(int deviceAddr) {
  
  union {
    uint32_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Eanstron SDM220 a 32 bit
    byte buf[4]; // Rappresentazione della stessa variabile come array di 4 byte utile per preparare il pacchetto dati LoRa
  } u;

  Serial.print("EastronSDM630 lettura e invio secondo blocco dati frequenti del meter ModBus Addr: "); 
  Serial.println(deviceAddr);

  erroriTot = 0; // Inizia un nuovo ciclo di lettura per cui resetto il numero totale degli errori

  modem.beginPacket(); // Preparo il pacchetto di dati

  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,FREQUENCY);
  if (!errore) {
    // Se la funzione di lettura ModBus non ha settato il flag errore
    // posso procedere a visualizzare sull'eventuale serial monitor i valori 
    // sia in chiaro, come numero con virgola, che come valore esadecimale
    // utile per confrontarlo con il valore ricevuto sul server LoRaWan
    float frequency = rawBitToFloat(u.bitRegister); // converto i 4 byte del regsitro nel tipo float
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" frequency = ");
    Serial.print(frequency);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    // a questo punto, accodo uno per volta i 4 byte del registro nel pacchetto dati
    // utilizzando la rappresentazione del registro sotto forma di array (campo buf della union)
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    // inutile visualizzare e trasmettere i dati se la lettura non è riuscita
    Serial.println("Lettura Frequency non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,NEUTRAL_CURRENT);
  if (!errore) {
    float neutral_current = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Neutral current = ");
    Serial.print(neutral_current);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura Neutral current non riuscita");
  }
  delay(MODBUS_DELAY);


  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,AVERAGE_LINE_TO_NEUTRAL_VOLTS_THD);
  if (!errore) {
    float average_line_to_neutral_volts_thd = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Average line to neutral volts THD = ");
    Serial.print(average_line_to_neutral_volts_thd);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura Average line to neutral volts THD non riuscita");
  }
  delay(MODBUS_DELAY);


  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,AVERAGE_LINE_CURRENT_THD);
  if (!errore) {
    float average_line_current_thd = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Average line current THD = ");
    Serial.print(average_line_current_thd);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura Average line current THD non riuscita");
  }
  delay(MODBUS_DELAY);
 
  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,AVERAGE_LINE_TO_LINE_VOLTS_THD);
  if (!errore) {
    float average_line_to_neutral_volts_thd = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Average line to line volts THD = ");
    Serial.print(average_line_to_neutral_volts_thd);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura Average line to line volts THD non riuscita");
  }
  delay(MODBUS_DELAY);
  
  Serial.print("Terminata la lettura dei registri. In totale ci sono stati ");
  Serial.print(erroriTot);
  Serial.println(" errori.");

  if (!erroriTot) {
    Serial.println("Trasmetto messaggio su rete LoRaWan");
    modem.setPort(21); // imposto fport a 21 per identificare il tipo pacchetto
    if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
    } 
    else {
      Serial.println("Errore invio Messaggio LoRa!");
    }
  }
}


/*
                 * EastronSDM630DailyDataCollection *
                 
  Questa funzione effettua la lettura dei registri del dispositivo SDM630
  che devono essere acquisiti con frequenza giornaliera, compone il pacchetto
  di dati di tipo 22 (v. protocollo applicativo TERIS), imposta fport a 22
  e trasmette il pacchetto sulla rete LoRa

  Vengono utilizzate le seguenti variabili globali:
   - modem
   - errore
   - erroriTot
*/
void EastronSDM630DailyDataCollection(int deviceAddr) {
  
  union {
    uint32_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Eanstron SDM220 a 32 bit
    byte buf[4]; // Rappresentazione della stessa variabile come array di 4 byte utile per preparare il pacchetto dati LoRa
  } u;

  Serial.print("EastronSDM630 lettura e invio dati giornalieri del meter ModBus Addr: "); 
  Serial.println(deviceAddr);

  erroriTot = 0; // Inizia un nuovo ciclo di lettura per cui resetto il numero totale degli errori

  modem.beginPacket(); // Preparo il pacchetto di dati

  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,IMPORT_ACTIVE_ENERGY);
  if (!errore) {
    // Se la funzione di lettura ModBus non ha settato il flag errore
    // posso procedere a visualizzare sull'eventuale serial monitor i valori 
    // sia in chiaro, come numero con virgola, che come valore esadecimale
    // utile per confrontarlo con il valore ricevuto sul server LoRaWan
    float import_active_energy = rawBitToFloat(u.bitRegister); // converto i 4 byte del regsitro nel tipo float
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Import Active Energy = ");
    Serial.print(import_active_energy);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    // a questo punto, accodo uno per volta i 4 byte del registro nel pacchetto dati
    // utilizzando la rappresentazione del registro sotto forma di array (campo buf della union)
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    // inutile visualizzare e trasmettere i dati se la lettura non è riuscita
    Serial.println("Lettura Import Active Energy non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,EXPORT_ACTIVE_ENERGY);
  if (!errore) {
    float export_active_energy = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Export Active Energy = ");
    Serial.print(export_active_energy);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura Export Active Energy non riuscita");
  }
  delay(MODBUS_DELAY);
  

  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,IMPORT_REACTIVE_ENERGY);
  if (!errore) {
    float import_reactive_energy = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Import Reactive Energy = ");
    Serial.print(import_reactive_energy);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura Import Reactive Energy non riuscita");
  }
  delay(MODBUS_DELAY);
  
  
  u.bitRegister = SDM630readInputRegister_raw(deviceAddr,EXPORT_REACTIVE_ENERGY);
  if (!errore) {
    float export_reactive_energy = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Export Reactive Energy = ");
    Serial.print(export_reactive_energy);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura Export Reactive Energy non riuscita");
  }
  delay(MODBUS_DELAY);
  
  Serial.print("Terminata la lettura dei registri. In totale ci sono stati ");
  Serial.print(erroriTot);
  Serial.println(" errori.");

  if (!erroriTot) {
    Serial.println("Trasmetto messaggio su rete LoRaWan");
    modem.setPort(22); // imposto fport a 22 per identificare il tipo pacchetto
    if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
    } 
    else {
      Serial.println("Errore invio Messaggio LoRa!");
    }
  }
}

uint32_t SDM630readInputRegister_raw(int id,int address){
  uint32_t val = 0XFFFF;
  if (!ModbusRTUClient.requestFrom(id, INPUT_REGISTERS, address, 2)) { 
    // in caso di errore imposto il flag errore a 1 e incremento il conteggio relativo al ciclo
    errore = 1;
    erroriTot ++;
    Serial.print("Errore lettura address: ");
    Serial.print(address);
    Serial.print(" ");
    Serial.println(ModbusRTUClient.lastError());
  }else{
    errore = 0; // resetto o confermo a 0 il flag errore
    uint16_t word1 = ModbusRTUClient.read();
    uint16_t word2 = ModbusRTUClient.read();
    val = word1 << 16 | word2;
    
  }
  return val;
}

