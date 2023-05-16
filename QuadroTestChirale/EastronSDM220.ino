/*
  PROGETTO TERIS CONTROL

  Author: Chirale S.r.l.
  
  Funzioni di lettura e trasmissione dati per Energy Meater EASTRON SDM220MODBUS
  
  Testato su scheda Arduino MKR WAN 1310 con Shield 485 MKR
  
  Configurazione degli switch dello shield:
  1 -> Ininfluente, non usato poiché lo shield in modalità HALF-DUPLEX lutilizza solo i terminali Y e Z
  2 -> OFF - Shield impostato per la modalità HALF-DUPLEX
  3 -> Raccomandato su ON per cablaggi lunghi e non schermati. ON = Resistore di terminazione sul bus RS485. 
       Su cablaggi corti e cavi di qualità funziona anche se OFF  

  Cablaggio del BUS RS485:
   Terminale Y dello Shield collegato ai terminali A dei meter SDM220 (pin 9)
   Terminale Z dello Shield collegato ai terminali B dei meter SDM220 (pin 8)     
  
  */




// Definizione degli indirizzi iniziali degli INPUT REGISTERS del meter SDM220
// per i soli dati di interesse
// riferimento documento: SDM220 ModBus Protocol v 1.1

// Dati da acquisire con frequenza elevata (ogni 15 min.)
#define VOLTAGE  0x0000
#define CURRENT  0x0006
#define POWER_FACTOR 0x001E
#define FREQUENCY  0x0046

// Dati da acquisire giornalmente (ogni 24 ore)
#define IMPORT_ACTIVE_ENERGY  0x0048   
#define EXPORT_ACTIVE_ENERGY  0x004A  
#define IMPORT_REACTIVE_ENERGY  0x004C
#define EXPORT_REACTIVE_ENERGY  0x004E


/*
                 * EastronSDM220FrequentDataCollection *

  Questa funzione effettua la lettura dei registri del dispositivo SDM220
  che devono essere acquisiti con frequenza elevata, compone il pacchetto
  di dati di tipo 10 (v. protocollo applicativo TERIS), imposta fport a 10
  e trasmette il pacchetto sulla rete LoRa

  Vengono utilizzate le seguenti variabili globali:
   - modem
   - errore
   - erroriTot
*/
void EastronSDM220FrequentDataCollection(int deviceAddr) {
  
  union {
    uint32_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Eanstron SDM220 a 32 bit
    byte buf[4]; // Rappresentazione della stessa variabile come array di 4 byte utile per preparare il pacchetto dati LoRa
  } u;

  Serial.print("EastronSDM220 lettura e invio dati frequenti del meter ModBus Addr: "); 
  Serial.println(deviceAddr);

  erroriTot = 0; // Inizia un nuovo ciclo di lettura per cui resetto il numero totale degli errori

  modem.beginPacket(); // Preparo il pacchetto di dati
  modem.write((uint8_t)deviceAddr); // Valorizzo il primo byte del pacchetto con l'id del dispositivo

  u.bitRegister = SDM220readInputRegister_raw(deviceAddr,VOLTAGE);
  if (!errore) {
    // Se la funzione di lettura ModBus non ha settato il flag errore
    // posso procedere a visualizzare sull'eventuale serial monitor i valori 
    // sia in chiaro, come numero con virgola, che come valore esadecimale
    // utile per confrontarlo con il valore ricevuto sul server LoRaWan
    float voltage = rawBitToFloat(u.bitRegister); // converto i 4 byte del regsitro nel tipo float
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Voltage = ");
    Serial.print(voltage);
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
    Serial.println("Lettura Voltage non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = SDM220readInputRegister_raw(deviceAddr,CURRENT);
  if (!errore) {
    float current = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Current = ");
    Serial.print(current);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura Current non riuscita");
  }
  delay(MODBUS_DELAY);
  

  u.bitRegister = SDM220readInputRegister_raw(deviceAddr,POWER_FACTOR);
  if (!errore) {
    float power_factor = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Power Factor = ");
    Serial.print(power_factor);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura Power Factor non riuscita");
  }
  delay(MODBUS_DELAY);
  
  
  u.bitRegister = SDM220readInputRegister_raw(deviceAddr,FREQUENCY);
  if (!errore) {
    float frequency = rawBitToFloat(u.bitRegister); 
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" Frequency = ");
    Serial.print(frequency);
    Serial.print(" Register HEX: ");
    Serial.println(u.bitRegister,HEX);
    for(int i=3;i>=0;i--) {
      modem.write(u.buf[i]); 
    };
  }
  else {
    Serial.println("Lettura Frequency non riuscita");
  }
  delay(MODBUS_DELAY);
  
  Serial.print("Terminata la lettura dei registri. In totale ci sono stati ");
  Serial.print(erroriTot);
  Serial.println(" errori.");

  if (!erroriTot) {
    Serial.println("Trasmetto messaggio su rete LoRaWan");
    modem.setPort(10); // imposto fport a 10 per identificare il tipo pacchetto
    if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
    } 
    else {
      Serial.println("Errore invio Messaggio LoRa!");
    }
  }
}

/*
                 * EastronSDM220DailyDataCollection *
                 
  Questa funzione effettua la lettura dei registri del dispositivo SDM220
  che devono essere acquisiti con frequenza giornaliera, compone il pacchetto
  di dati di tipo 11 (v. protocollo applicativo TERIS), imposta fport a 11
  e trasmette il pacchetto sulla rete LoRa

  Vengono utilizzate le seguenti variabili globali:
   - modem
   - errore
   - erroriTot
*/
void EastronSDM220DailyDataCollection(int deviceAddr) {
  
  union {
    uint32_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Eanstron SDM220 a 32 bit
    byte buf[4]; // Rappresentazione della stessa variabile come array di 4 byte utile per preparare il pacchetto dati LoRa
  } u;

  Serial.print("EastronSDM220 lettura e invio dati giornalieri del meter ModBus Addr: "); 
  Serial.println(deviceAddr);

  erroriTot = 0; // Inizia un nuovo ciclo di lettura per cui resetto il numero totale degli errori

  modem.beginPacket(); // Preparo il pacchetto di dati
  modem.write((uint8_t)deviceAddr); // Valorizzo il primo byte del pacchetto con l'id del dispositivo

  u.bitRegister = SDM220readInputRegister_raw(deviceAddr,IMPORT_ACTIVE_ENERGY);
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

  u.bitRegister = SDM220readInputRegister_raw(deviceAddr,EXPORT_ACTIVE_ENERGY);
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
  

  u.bitRegister = SDM220readInputRegister_raw(deviceAddr,IMPORT_REACTIVE_ENERGY);
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
  
  
  u.bitRegister = SDM220readInputRegister_raw(deviceAddr,EXPORT_REACTIVE_ENERGY);
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
    modem.setPort(11); // imposto fport a 11 per identificare il tipo pacchetto
    if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
    } 
    else {
      Serial.println("Errore invio Messaggio LoRa!");
    }
  }
}

uint32_t SDM220readInputRegister_raw(int id,int address){
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

