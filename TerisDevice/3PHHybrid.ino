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
// Sono registri a 16 bit signed o unsigned come indicato nei commenti di dettaglio
#define HY_PV1_VOLTAGE 0x0584 // uint16_t
#define HY_PV1_CURRENT 0x0585 // uint16_t
#define HY_PV2_VOLTAGE 0x0587 // uint16_t
#define HY_PV2_CURRENT 0x0588 // uint16_t
#define HY_BAT1_VOLTAGE 0x0604 // uint16_t
#define HY_BAT1_CURRENT 0x0605 // int16_t
#define HY_BAT1_ENV_TEMPERATURE 0x0607 // int16_t
#define HY_BAT1_SOC 0x0608 // uint16_t
#define HY_ACTIVE_POWER_OUTPUT_TOTAL 0x0485 // int16_t
#define HY_REACTIVE_POWER_OUTPUT_TOTAL 0x0486 // int16_t
#define HY_ACTIVE_POWER_PCC_TOTAL 0x0488 // int16_t

// Dati da acquisire con frequenza elevata H24
// Registri a 16 bit signed o unsigned come indicato nei commenti di dettaglio
#define HY_FREQUENCY_GRID 0x0484 // uint16_t
#define HY_ACTIVE_POWER_LOAD_TOTAL 0x0504 // int16_t
#define HY_REACTIVE_POWER_LOAD_TOTAL 0x0505 // int16_t

// Dati da acquisire con frequenza giornaliera alle 20:30
// Sono tutte registri a 32 bit unsigned (uint32_t)
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


/*
                 * ZCSHybridDayLightFrequentDataCollection *

  Questa funzione effettua la lettura dei registri del dispositivo ZCS Hybrid 3PH
  che devono essere acquisiti con frequenza elevata nelle ore diurne, compone il pacchetto
  di dati di tipo 40 (v. protocollo applicativo TERIS), imposta fport a 40
  e trasmette il pacchetto sulla rete LoRa

  Vengono utilizzate le seguenti variabili globali:
   - modem
   - errore
   - erroriTot
*/
void ZCSHybridDayLightFrequentDataCollection(int deviceAddr) {
  // Struttura dati "union" utilizzata per gestire le interpretazioni dei tipi di dato dai valori raw dei registri
  union {
    uint16_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del dispositivo a 16 bit
    int16_t  int16; // Rappresentazione del registro come int16_t (intero con segno)
    byte buf[2]; // Rappresentazione della stessa variabile come array di 2 byte utile per preparare il pacchetto dati LoRa
  } u;


  Serial.print("ZCS 3PH Hybrid lettura e invio dati frequenti (diurni) dell'inverter ModBus Addr: "); 
  Serial.println(deviceAddr);

  erroriTot = 0; // Inizia un nuovo ciclo per cui resetto il numero totale degli errori

  modem.beginPacket(); // Preparo il pacchetto di dati
  modem.write((uint8_t)deviceAddr); // Valorizzo il primo byte del pacchetto con l'id del dispositivo


  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_PV1_VOLTAGE);
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
  

  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_PV1_CURRENT);
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
  
  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_PV2_VOLTAGE);
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
  
  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_PV2_CURRENT);
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

  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_BAT1_VOLTAGE);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" bat1_voltage register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Volt =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Volt
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura bat1_voltage non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_BAT1_CURRENT);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" bat1_current register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Ampere =  ");
    Serial.println(u.int16*0.01); // il valore del registro è in unità da 0,01 Ampere (il tipo è int16_t)
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura bat1_current non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_BAT1_ENV_TEMPERATURE);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" bat1_env_temperature register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Temp. Gradi centigradi =  ");
    Serial.println(u.int16); // il valore del registro è in unità da 1 grado C
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura bat1_env_temperature non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_BAT1_SOC);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" bat1_soc register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" SOC % =  ");
    Serial.println(u.bitRegister); // il valore del registro è in unità percentuali
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura bat1_soc non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_ACTIVE_POWER_OUTPUT_TOTAL);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" active_power_output_total register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kw =  ");
    Serial.println(u.int16*0.01); // il valore del registro è in unità da 0,01 Kw
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura active_power_output_total non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_REACTIVE_POWER_OUTPUT_TOTAL);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" reactive_power_output_total register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kw =  ");
    Serial.println(u.int16*0.01); // il valore del registro è in unità da 0,01 Kw
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura reactive_power_output_total non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_ACTIVE_POWER_PCC_TOTAL);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" active_power_pcc_total register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kw =  ");
    Serial.println(u.int16*0.01); // il valore del registro è in unità da 0,01 Kw
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura active_power_pcc_total non riuscita");
  }
  delay(MODBUS_DELAY);

  Serial.print("Terminata la lettura dei registri. In totale ci sono stati ");
  Serial.print(erroriTot);
  Serial.println(" errori.");

  if (!erroriTot) {
    Serial.println("Trasmetto messaggio su rete LoRaWan");
    modem.setPort(40); // Imposto fprort a 40 per identificare il tipo pacchetto
    if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
    } 
    else {
      Serial.println("Errore invio Messaggio LoRa!");
    }
  }
  
}

/*
                 * ZCSHybridFrequentDataCollection *

  Questa funzione effettua la lettura dei registri del dispositivo ZCS Hybrid 3PH
  che devono essere acquisiti con frequenza elevata nelle ore diurne e notturne, compone il pacchetto
  di dati di tipo 41 (v. protocollo applicativo TERIS), imposta fport a 41
  e trasmette il pacchetto sulla rete LoRa

  Vengono utilizzate le seguenti variabili globali:
   - modem
   - errore
   - erroriTot
*/
void ZCSHybridFrequentDataCollection(int deviceAddr) {

  union {
    uint16_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del dispositivo a 16 bit
    int16_t  int16; // Rappresentazione del registro come int16_t (intero con segno)
    byte buf[2]; // Rappresentazione della stessa variabile come array di 2 byte utile per preparare il pacchetto dati LoRa
  } u;

  Serial.print("ZCS 3PH Hybrid lettura e invio dati frequenti dell'inverter ModBus Addr: "); 
  Serial.println(deviceAddr);

  erroriTot = 0; // Inizia un nuovo ciclo per cui resetto il numero totale degli errori

  modem.beginPacket(); // Preparo il pacchetto di dati
  modem.write((uint8_t)deviceAddr); // Valorizzo il primo byte del pacchetto con l'id del dispositivo


  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_FREQUENCY_GRID);
  if (!errore){
    // Se la funzione di lettura ModBus non ha settato il flag errore
    // posso procedere a visualizzare sull'eventuale serial monitor i valori 
    // sia in chiaro, applicando le formule riportate sul manuale, che come valore esadecimale
    // utile per confrontarlo con il valore ricevuto sul server LoRaWan
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" frequency_grid register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Hz =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Hz
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    // a questo punto, accodo i 2 byte del registro nel pacchetto dati
    // utilizzando la rappresentazione del registro sotto forma di array (campo buf della union)
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura frequency_grid non riuscita");
  }
  delay(MODBUS_DELAY);
  

  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_ACTIVE_POWER_LOAD_TOTAL);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" active_power_load_total register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kw =  ");
    Serial.println(u.int16*0.01); // il valore del registro è in unità da 0,01 Kw
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura active_power_load_total non riuscita");
  }
  delay(MODBUS_DELAY);
  u.bitRegister = ZCSHybridReadHoldingRegister(deviceAddr,HY_REACTIVE_POWER_LOAD_TOTAL);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" reactive_power_load_total register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kw =  ");
    Serial.println(u.int16*0.01); // il valore del registro è in unità da 0,01 Kw
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    Serial.println("Lettura reactive_power_load_total non riuscita");
  }
  delay(MODBUS_DELAY);
  
  Serial.print("Terminata la lettura dei registri. In totale ci sono stati ");
  Serial.print(erroriTot);
  Serial.println(" errori.");

  if (!erroriTot) {
    Serial.println("Trasmetto messaggio su rete LoRaWan");
    modem.setPort(41); // Imposto fprort a 41 per identificare il tipo pacchetto
    if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
    } 
    else {
      Serial.println("Errore invio Messaggio LoRa!");
    }
  }
  
}

/*
                 * ZCSHybridDailyDataCollection *

  Questa funzione effettua la lettura dei registri del dispositivo ZCS Hybrid 3PH
  che devono essere acquisiti con frequenza giornaliera, compone il pacchetto
  di dati di tipo 42 (v. protocollo applicativo TERIS), imposta fport a 42
  e trasmette il pacchetto sulla rete LoRa

  Vengono utilizzate le seguenti variabili globali:
   - modem
   - errore
   - erroriTot
*/
void ZCSHybridDailyDataCollection(int deviceAddr) {

  union {
    uint32_t bitRegister; // Variabile di appoggio per ricevere la lettura di un registro del dispositivo a 32 bit
    byte buf[4]; // Rappresentazione della stessa variabile come array di 4 byte utile per preparare il pacchetto dati LoRa
  } u;

  Serial.print("ZCS 3PH Hybrid lettura e invio dati giornalieri dell'inverter ModBus Addr: "); 
  Serial.println(deviceAddr);

  erroriTot = 0; // Inizia un nuovo ciclo per cui resetto il numero totale degli errori

  modem.beginPacket(); // Preparo il pacchetto di dati
  modem.write((uint8_t)deviceAddr); // Valorizzo il primo byte del pacchetto con l'id del dispositivo


  u.bitRegister = ZCSHybridReadHoldingRegister32(deviceAddr,HY_PV_GENERATION_TODAY);
  if (!errore){
    // Se la funzione di lettura ModBus non ha settato il flag errore
    // posso procedere a visualizzare sull'eventuale serial monitor i valori 
    // sia in chiaro, applicando le formule riportate sul manuale, che come valore esadecimale
    // utile per confrontarlo con il valore ricevuto sul server LoRaWan
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" HY_PV_GENERATION_TODAY register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kw =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Kw
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    // a questo punto, accodo i 4 byte del registro nel pacchetto dati
    // utilizzando la rappresentazione del registro sotto forma di array (campo buf della union)
    modem.write(u.buf[3]); 
    modem.write(u.buf[2]);
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura HY_PV_GENERATION_TODAY non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = ZCSHybridReadHoldingRegister32(deviceAddr,HY_PV_GENERATION_TOTAL);
  if (!errore){
    // Se la funzione di lettura ModBus non ha settato il flag errore
    // posso procedere a visualizzare sull'eventuale serial monitor i valori 
    // sia in chiaro, applicando le formule riportate sul manuale, che come valore esadecimale
    // utile per confrontarlo con il valore ricevuto sul server LoRaWan
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" HY_PV_GENERATION_TOTAL register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kw =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Kw
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    // a questo punto, accodo i 4 byte del registro nel pacchetto dati
    // utilizzando la rappresentazione del registro sotto forma di array (campo buf della union)
    modem.write(u.buf[3]); 
    modem.write(u.buf[2]);
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura HY_PV_GENERATION_TOTAL non riuscita");
  }
  delay(MODBUS_DELAY);

  u.bitRegister = ZCSHybridReadHoldingRegister32(deviceAddr,HY_LOAD_CONSUMPTION_TODAY);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" HY_LOAD_CONSUMPTION_TODAY register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Kwh
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    modem.write(u.buf[3]); 
    modem.write(u.buf[2]);
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura HY_LOAD_CONSUMPTION_TODAY non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = ZCSHybridReadHoldingRegister32(deviceAddr,HY_LOAD_CONSUMPTION_TOTAL);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" HY_LOAD_CONSUMPTION_TOTAL register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Kwh
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    modem.write(u.buf[3]); 
    modem.write(u.buf[2]);
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura HY_LOAD_CONSUMPTION_TOTAL non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = ZCSHybridReadHoldingRegister32(deviceAddr,HY_ENERGY_PURCHASE_TODAY);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" HY_ENERGY_PURCHASE_TODAY register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Kwh
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    modem.write(u.buf[3]); 
    modem.write(u.buf[2]);
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura HY_ENERGY_PURCHASE_TODAY non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = ZCSHybridReadHoldingRegister32(deviceAddr,HY_ENERGY_PURCHASE_TOTAL);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" HY_ENERGY_PURCHASE_TOTAL register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Kwh
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    modem.write(u.buf[3]); 
    modem.write(u.buf[2]);
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura HY_ENERGY_PURCHASE_TOTAL non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = ZCSHybridReadHoldingRegister32(deviceAddr,HY_ENERGY_SELLING_TODAY);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" HY_ENERGY_SELLING_TODAY register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Kwh
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    modem.write(u.buf[3]); 
    modem.write(u.buf[2]);
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura HY_ENERGY_SELLING_TODAY non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = ZCSHybridReadHoldingRegister32(deviceAddr,HY_ENERGY_SELLING_TOTAL);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" HY_ENERGY_SELLING_TOTAL register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Kwh
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    modem.write(u.buf[3]); 
    modem.write(u.buf[2]);
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura HY_ENERGY_SELLING_TOTAL non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = ZCSHybridReadHoldingRegister32(deviceAddr,HY_BAT_CHARGE_TODAY);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" HY_BAT_CHARGE_TODAY register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Kwh
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    modem.write(u.buf[3]); 
    modem.write(u.buf[2]);
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura HY_BAT_CHARGE_TODAY non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = ZCSHybridReadHoldingRegister32(deviceAddr,HY_BAT_CHARGE_TOTAL);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" HY_BAT_CHARGE_TOTAL register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Kwh
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    modem.write(u.buf[3]); 
    modem.write(u.buf[2]);
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura HY_BAT_CHARGE_TOTAL non riuscita");
  }
  delay(MODBUS_DELAY);
  

  u.bitRegister = ZCSHybridReadHoldingRegister32(deviceAddr,HY_BAT_DISCHARGE_TODAY);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" HY_BAT_DISCHARGE_TODAY register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u.bitRegister*0.01); // il valore del registro è in unità da 0,01 Kwh
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    modem.write(u.buf[3]); 
    modem.write(u.buf[2]);
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura HY_BAT_DISCHARGE_TODAY non riuscita");
  }
  delay(MODBUS_DELAY);
  
  u.bitRegister = ZCSHybridReadHoldingRegister32(deviceAddr,HY_BAT_DISCHARGE_TOTAL);
  if (!errore){
    Serial.print("Device ");
    Serial.print(deviceAddr);
    Serial.print(" HY_BAT_DISCHARGE_TOTAL register = ");
    Serial.print(u.bitRegister,HEX);
    Serial.print(" Kwh =  ");
    Serial.println(u.bitRegister*0.1); // il valore del registro è in unità da 0,1 Kwh
    delay(MODBUS_DELAY); // è bene lasciare un tempo di attesa di qualche secondo prima di fare una nuova interrogazione al bus
    modem.write(u.buf[3]); 
    modem.write(u.buf[2]);
    modem.write(u.buf[1]); 
    modem.write(u.buf[0]);
  }
  else {
    // inutile visualizzare e inviare i dati se la lettura non è riuscita
    Serial.println("Lettura HY_BAT_DISCHARGE_TOTAL non riuscita");
  }
  delay(MODBUS_DELAY);
  
  Serial.print("Terminata la lettura dei registri. In totale ci sono stati ");
  Serial.print(erroriTot);
  Serial.println(" errori.");

  if (!erroriTot) {
    Serial.println("Trasmetto messaggio su rete LoRaWan");
    modem.setPort(42); // Imposto fprort a 42 per identificare il tipo pacchetto
    if(modem.endPacket()){
      Serial.println("Messaggio LoRa inviato!");
    } 
    else {
      Serial.println("Errore invio Messaggio LoRa!");
    }
  }
  
}


// Lettura registro singolo da 16 bit
uint16_t ZCSHybridReadHoldingRegister(int id,int address){
  
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
uint32_t ZCSHybridReadHoldingRegister32(int id,int address){
  
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


