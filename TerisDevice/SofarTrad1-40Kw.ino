/*
  PROGETTO TERIS CONTROL

  Author: Chirale S.r.l.
  
  Funzioni di lettura e trasmissione dati per Inverter ZCS Sofar Solar Tradizionale 1-40 Kw
  
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

// Definizione degli indirizzi iniziali degli HOLDING REGISTERS dell'inverter Sofar Solar
// riferimento documento: Manuale ModBus Sofar Solar

// Dati da acquisire con frequenza elevata (ogni 15 min)
#define PV1_VOLTAGE 0x0006
#define PV1_CURRENT 0x0007
#define PV2_VOLTAGE 0x0008
#define PV2_CURRENT 0x0009
#define ACTIVE_POWER_OUTPUT 0x000C

// Dati da acquisire con frequenza giornaliera
#define TOTAL_PRODUCTION_HB 0x0015
#define TOTAL_PRODUCTION_LB 0x0016
#define DAILY_ENERGY 0x0019
#define PV_INSULATION_RESISTANCE_TO_GROUND 0x0026

// Dati per valutazione allarmi e alert
#define INVERTER_INTERNAL_TEMPERATURE 0x001C
#define INVERTER_BUS_VOLTAGE 0x001D
#define INVERTER_ALARM 0x0021
#define GFCI_RMS 0x002A


/*
                 * SofarTrad40KwFrequentDataCollection *

  Questa funzione effettua la lettura dei registri del dispositivo Sofar Solar Trad 1-40Kw
  che devono essere acquisiti con frequenza elevata, compone il pacchetto
  di dati di tipo 30 (v. protocollo applicativo TERIS), imposta fport a 30
  e trasmette il pacchetto sulla rete LoRa

  Vengono utilizzate le seguenti variabili globali:
   - modem
   - errore
   - erroriTot
*/

void SofarTrad40KwFrequentDataCollection(int deviceAddr) {

  union {
    uint16_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Sofar a 16 bit
    byte buf[2]; // Rappresentazione della stessa variabile come array di 2 byte utile per preparare il pacchetto dati LoRa
  } u;

  union {
    uint32_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Sofar a 32 bit
    byte buf[4]; // Rappresentazione della stessa variabile come array di 4 byte utile per preparare il pacchetto dati LoRa
  } u32;

  Serial.print("Sofar Solar Trad 1-40Kw lettura e invio dati frequenti dell'inverter ModBus Addr: "); 
  Serial.println(deviceAddr);

  erroriTot = 0; // Inizia un nuovo ciclo per cui resetto il numero totale degli errori

  modem.beginPacket(); // Preparo il pacchetto di dati


  u.bitRegister = SofarTrad40KwReadHoldingRegister(deviceAddr,PV1_VOLTAGE);
  if (!errore){
    // Se la funzione di lettura ModBus non ha settato il flag errore
    // posso procedere a visualizzare sull'eventuale serial monitor i valori 
    // sia in chiaro, applicando le formule riportate sul manuale, che come valore esadecimale
    // utile per confrontarlo con il valore ricevuto sul server LoRaWan
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" pv1_voltage register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Volt =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Volt
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    // a questo punto, accodo i 2 byte del registro nel pacchetto dati
    // utilizzando la rappresentazione del registro sotto forma di array (campo buf della union)
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura pv1_voltage non riuscita");
  }
  delay(MODBUS_DELAY);
  

  u.bitRegister = SofarTrad40KwReadHoldingRegister(deviceAddr,PV1_CURRENT);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" pv1_current register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Ampere =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Ampere
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura pv1_current non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = SofarTrad40KwReadHoldingRegister(deviceAddr,PV2_VOLTAGE);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" pv2_voltage register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Volt =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Volt
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura pv2_voltage non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = SofarTrad40KwReadHoldingRegister(deviceAddr,PV2_CURRENT);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" pv2_current register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Ampere =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Ampere
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura pv2_current non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = SofarTrad40KwReadHoldingRegister(deviceAddr,ACTIVE_POWER_OUTPUT);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" active_power_output register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" KWatt =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 KWatt
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura active_power_output non riuscita");
  }
  delay(MODBUS_DELAY);
 
  Serial.print("Terminata la lettura dei registri. In totale ci sono stati ");
  Serial.print(erroriTot);
  Serial.println(" errori.");

  if (!erroriTot) {
    Serial.println("Trasmetto messaggio su rete LoRaWan");
    modem.setPort(30); // Imposto fprort a 30 per identificare il tipo pacchetto
    if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
    } 
    else {
      Serial.println("Errore invio Messaggio LoRa!");
    }
  }
  
}


/*
                 * SofarTrad40KwDailyDataCollection *
                 
  Questa funzione effettua la lettura dei registri del dispositivo Sofar Solar Trad 1-40Kw
  che devono essere acquisiti con frequenza giornaliera, compone il pacchetto
  di dati di tipo 31 (v. protocollo applicativo TERIS), imposta fport a 31
  e trasmette il pacchetto sulla rete LoRa

  Vengono utilizzate le seguenti variabili globali:
   - modem
   - errore
   - erroriTot
*/
void SofarTrad40KwDailyDataCollection(int deviceAddr) {

  union {
    uint16_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Sofar a 16 bit
    byte buf[2]; // Rappresentazione della stessa variabile come array di 2 byte utile per preparare il pacchetto dati LoRa
  } u;

  union {
    uint32_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Sofar a 32 bit
    byte buf[4]; // Rappresentazione della stessa variabile come array di 4 byte utile per preparare il pacchetto dati LoRa
  } u32;

  Serial.print("Sofar Solar Trad 1-40Kw lettura e invio dati giornalieri dell'inverter ModBus Addr: "); 
  Serial.println(deviceAddr);

  erroriTot = 0; // Inizia un nuovo ciclo per cui resetto il numero totale degli errori

  modem.beginPacket(); // Preparo il pacchetto di dati

 u32.bitRegister = SofarTrad40KwReadHoldingRegister32(deviceAddr,TOTAL_PRODUCTION_HB); // Specifico l'indirizzo del primo rsgistro a 16 bit
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" total_production register = ");
    Serial.print(u32.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u32.bitRegister); // il valore del registro è in unità da 0,01 KWatt
    modem.write(u32.buf[3]); 
    modem.write(u32.buf[2]);
    modem.write(u32.buf[1]); 
    modem.write(u32.buf[0]);
  }
  else {
    Serial.println("Lettura total_production non riuscita");
  }
  delay(MODBUS_DELAY);
  
 u.bitRegister = SofarTrad40KwReadHoldingRegister(deviceAddr,DAILY_ENERGY);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" daily_energy register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Kwh
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura active_power_output non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = SofarTrad40KwReadHoldingRegister(deviceAddr,PV_INSULATION_RESISTANCE_TO_GROUND);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" pv_insulation_resistance_to_ground register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Valore decimale =  ");
    Serial.println(u.bitRegister); // il manuale non specifica il significato
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura pv_insulation_resistance_to_ground non riuscita");
  }
  delay(MODBUS_DELAY);

  Serial.print("Terminata la lettura dei registri. In totale ci sono stati ");
  Serial.print(erroriTot);
  Serial.println(" errori.");

  if (!erroriTot) {
    Serial.println("Trasmetto messaggio su rete LoRaWan");
    modem.setPort(31); // Imposto fport a 31 per identificare il tipo pacchetto
    if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
    } 
    else {
      Serial.println("Errore invio Messaggio LoRa!");
    }
  }
}
  

/*
                 * SofarTrad40KwAlert *
                 
  Questa funzione effettua la lettura dei registri del dispositivo Sofar Solar Trad 1-40Kw
  che devono essere valutati per eventuali alert, in caso di allarme compone il pacchetto
  di dati di tipo 32 (v. protocollo applicativo TERIS), imposta fport a 32
  e trasmette il pacchetto sulla rete LoRa

  Vengono utilizzate le seguenti variabili globali:
   - modem
   - errore
   - erroriTot
*/
void SofarTrad40KwAlert(int deviceAddr) {
  union {
    uint16_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del device Sofar a 16 bit
    byte buf[2]; // Rappresentazione della stessa variabile come array di 2 byte utile per preparare il pacchetto dati LoRa
  } u;

  uint16_t alarm;
  uint16_t temperature;

  u.bitRegister = SofarTrad40KwReadHoldingRegister(deviceAddr,INVERTER_ALARM);
  alarm = u.bitRegister; // memorizzo il valore del registro di allarme
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" inverter_alarm register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Valore decimale =  ");
    Serial.println(u.bitRegister); // vedere decodifica sul manuale 
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura inverter_alarm non riuscita");
  }
  delay(MODBUS_DELAY);
 
 u.bitRegister = SofarTrad40KwReadHoldingRegister(deviceAddr,INVERTER_INTERNAL_TEMPERATURE);
 temperature = u.bitRegister; // memorizzo il valore della temperatura
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" inverter_internal_temperature = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" C =  ");
    Serial.println(u.bitRegister); // il valore del registro è in unità da 1 grado C
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura inverter_internal_temperature non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = SofarTrad40KwReadHoldingRegister(deviceAddr,INVERTER_BUS_VOLTAGE);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" inverter_bus_voltage register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Volt =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Volt
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura inverter_bus_voltage non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = SofarTrad40KwReadHoldingRegister(deviceAddr,GFCI_RMS);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" gfci_rms register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" GFCI RMS mA =  ");
    Serial.println(u.bitRegister); // il valore del registro è in unità da 1 mA
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura inverter_alarm non riuscita");
  }
  delay(MODBUS_DELAY);

// Se le letture sono andate a buon fine e si verifica una delle seguenti condizioni
// temperatura interna > 50 oppure registro alarm valorizzato (non zero)
// trasmetto il messaggio di alert
  if ((!erroriTot) and ((alarm) or (temperature > 50))) {
    Serial.println("C'è un alert: Trasmetto messaggio su rete LoRaWan");
    modem.setPort(32); // imposto fport a 32 per identificare il tipo pacchetto
    if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
    }
    else {
      Serial.println("Errore invio Messaggio LoRa!");
    }
  }
  
}



// Lettura registro singolo da 16 bit
uint16_t SofarTrad40KwReadHoldingRegister(int id,int address){
  
  if (!ModbusRTUClient.requestFrom(id, HOLDING_REGISTERS, address, 1)) { 
    // in caso di errore imposto il flag errore a 1 e incremento il conteggio relativo al ciclo
    errore = 1;
    erroriTot++;
    Serial.print("Errore lettura address: ");
    Serial.print(address);
    Serial.print(" ");
    Serial.println(ModbusRTUClient.lastError());
    return 0xFFFF; //Restituisco il valore massimo per segnalare l'errore di lettura
  }else{
    errore = 0; // resetto o confermo a 0 il flag errore
    return ModbusRTUClient.read();
  }
  
}



// Lettura registro doppio da 32 bit
uint32_t SofarTrad40KwReadHoldingRegister32(int id,int address){
  
  if (!ModbusRTUClient.requestFrom(id, HOLDING_REGISTERS, address, 2)) { 
    // in caso di errore imposto il flag errore a 1 e incremento il conteggio relativo al ciclo
    errore = 1;
    erroriTot++;
    Serial.print("Errore lettura address: ");
    Serial.print(address);
    Serial.print(" ");
    Serial.println(ModbusRTUClient.lastError());
    return 0xFFFF; //Restituisco il valore massimo per segnalare l'errore di lettura
  }else{
    errore = 0; // resetto o confermo a 0 il flag errore
    uint16_t hb = ModbusRTUClient.read();
    uint16_t lb = ModbusRTUClient.read();
    uint32_t doubleReg = hb << 16 | lb;
    return doubleReg;
  }
  
}


