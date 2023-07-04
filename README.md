# teris
Progetto sperimentale per la realizzazione di un sistema di monitoraggio di impianti solari, attraverso la lettura degli apparati su protocollo ModBus e trasmissione dati su rete LoRaWan.
Questo repository contiene sketch Arduino che esemplificano le operazioni di base del sistema.

La cartella TerisDevice contiene il template di sketch per generare il firmware di un Teris Device, come spiegato nel documento TERISDeviceFirmwareBuild.docx

La cartella UplinkDecoder contiene esempi di payload decoder utilizzati sulla piattaforma DataCake oppure sulla piattaforma  The Things Network.

La cartella Installazioni archivia gli sketch utilizzati per generare il firmware dei Teris Device che sono in esercizio nei diversi impianti.

La cartella Utilities contiene sketch di utilità generale per effettuare il test della lettura ModBus o di comunicazioni LoRaWan.

La cartella Prove archivia sketch utilizzati per effettuare POC o altri esperimenti

La cartella OLD archivia il software della prima e obsoleta versione sperimentale del sistema
