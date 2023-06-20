/*
 * Gestisce le comunicazioni attraverso la porta seriale senza sfruttare nessun protocollo specifico.
 * Gestisce inizializzazione e buffer, di trasmissione e ricezione.
 *
 * Necessaria la virtualizzione di più interfacce seriali.
 *
 * Note: manca della tabella di autoconfigurazione
 */

#ifndef NESER_H
#define NESER_H

#include <stdint.h>


/*******************************************************************************
		DEFINIZIONE PROPRIETA SERIALE
*******************************************************************************/

enum SerialSpeed
{
  UndefinedBaud	= 255,
  Baud9600 		= 9600,
  Baud19200 	= 19200,
  Baud38400 	= 38400,
  Baud57600 	= 57600,
  Baud115200 	= 115200,
  Baud1000000	= 1000000
};

enum SerialProtocol				/* enumeratore per la scelta del protocollo di comunicazione, andrà sostituito una volta passarti a c++ */
{
  UndefinedProtocol = 0,
  NeProtocol		    = 1,
  NeSerCanProtocol	= 2
};


/*******************************************************************************
		DEFINIZIONI BUFFER
*******************************************************************************/

/* lunghezza buffer trasmissione e ricezione */
#define LEN_RXBUF			50     		/* buffer per gestione ricezione	*/
#define LEN_TXBUF			50     		/* buffer per gestione trasmissione	*/


/* Struttura buffer seriale, utilizzato da serial.c e dai file di definizione dei protocolli
 * Non sono presenti le definizioni generali di comunicazione, come velocità e timeout
 */
typedef struct
{
  uint8_t  BuffRx[LEN_RXBUF]; 	/* buffer per interrupt di ricezione (riempito dalle funzioni di interrupt, buffer circolare) */
//  uint8_t  BuffRx   [LENRX]; 			/* buffer di elaborazione messaggio ricevuto (protocollo NE), contiene singolo pacchetto. Parte dall'indice 0	*/
  uint16_t RxBufferIndex;				/* indice del primo elemento nel buffer gestito dall'interrupt (idx_last = idx_first+length-1), relativo a BuffRxInt */
  uint16_t RxBufferLength;				/* numero di byte ricevuti dalla porta seriale (non ancora decodificati), viene usato in combinazione con index, relativo a BuffRxInt */
  uint16_t RxBytes;						/* byte ricevuti (rx size), relativo a BuffRx */
  uint8_t  RxCheck;	    				/* per calcolo checksum */

  uint8_t  BuffTxInt[LEN_TXBUF];		/* buffer per interrupt di trasmissione, contiene tutti i pacchetti da inviare */
//  uint8_t  BuffTx   [LENTX]; 			/* buffer per trasmissione messaggio, contiene un singolo pacchetto che andrà trasmesso al buffer BuffTxInt	*/
  uint16_t TxBufferIndex;				/* indice del primo elemento nel buffer gestito dall'interrupt (idx_last = idx_first+length-1), relativo a BuffTxInt */
  uint16_t TxBufferLength;				/* numero di byte da trasmettere sulla porta seriale, viene usato in combinazione con index, relativo a BuffTxInt */
  uint16_t TxBytes;						/* byte da trasmettere, relativo a BuffTx, start index sempre=0 */
  uint8_t  TxCheck;	    				/* per calcolo checksum */

  uint16_t NumBytesRx;  				/* numero di bytes da ricevere */
  uint16_t StatoRx;	    				/* stato ricezione */
  uint8_t  RxRun;						/* ricezione in corso */

  int16_t  Timeout;						/* timeout di fine ricezione frame seriale in tick (valore impostato) */
  int16_t  TickSer;						/* tick per il timeout (valore da aggiornare) */

  void (*GestSerialFunc_ptr)(void);		/* Puntatore alla funzione per la gestione del protocollo seriale */
} SerType;


/* struttura buffer seriale */
#define OFFBUFFRXINT		0								/* buffer interrupt ricezione seriale */
#define OFFBUFFTXINT		OFFBUFFRXINT+LENBUFFRXINT		/* buffer interrupt trasmissione seriale */
#define OFFBUFFRX			OFFBUFFTXINT+LENBUFFTXINT		/* buffer ricezione */
#define OFFPTRLRXINT		OFFBUFFRX+300					/* puntatore carico  buffer interrupt ricezione seriale */
#define OFFPTRURXINT		OFFPTRLRXINT+4					/* puntatore scarico buffer interrupt ricezione seriale */
#define OFFPTRLTXINT		OFFPTRURXINT+4					/* puntatore carico  buffer interrupt trasmissione seriale */
#define OFFPTRUTXINT		OFFPTRLTXINT+4					/* puntatore scarico  buffer interrupt trasmissione seriale */
#define OFFRXRUN			OFFPTRUTXINT+4					/* indica ricezione in corso */
#define OFFNUMBYTESRX		OFFRXRUN+2						/* numero di bytes da ricevere */
#define OFFSTATORX			OFFNUMBYTESRX+2					/* stato ricezione */
#define OFFCHECK			OFFSTATORX+2					/* per calcolo check sum */
#define OFFRXPTR			OFFCHECK+2						/* puntatore al buffer ricezione */
#define OFFMODBUS			OFFRXPTR+4						/* se 1 attiva la modalit� modbus (ASCON) */
#define OFFTIMEOUT			OFFMODBUS+2						/* numero di tick di timeout ricezione seriale */
#define OFFBUFFTX			OFFTIMEOUT+2					/* buffer per trasmissione messaggio (modbus slave) */
#define OFFINSBUFFRX		OFFBUFFTX+300					/* puntatore per inserimento nel buffer di elaborazione messaggio ricevuto (modbus slave) */
#define OFFGETBUFFRX		OFFINSBUFFRX+4					/* puntatore per estrazione dal buffer di elaborazione messaggio ricevuto (modbus slave) */
#define LENBUFSER			OFFGETBUFFRX+4					/* lunghezza struttura buffer seriale */

//#define TIMEOUT				10							/* time-out seriale = 10ms */
//#define TIMEOUT				50								/* time-out seriale = 50ms */


/** VARIABILI ESTERNE **/
extern volatile SerType BuffSer;
extern void (*GestSerialPtr)(void);							/* puntatore alla funzione per la gestione della seriale in funzione del protocollo specificato */



/** PROTOTIPI FUNZIONI INTERNE **/
unsigned char CalcolaChkSum(unsigned char *, short);
void WrL(unsigned char *, long);


/** PROTOTIPI FUNZIONI ESTERNE **/
//! inizializzazione interfaccia seriale
void InitSerialPort		(enum SerialSpeed baud_rate);
//! apertura porta e abilitazione tx e rx
void OpenSerialPort		(void (*SerialProtocolHandler)(void));

void ResetSerialPort	( void );
void InitSerialBuffers	( void );

//! gestione trasmissione e ricezione secondo la funzione specificata
void GestSerialPort		( void );

void WritePacketInt		( void );
//! scrive l'array nel buffer
void SerialWriteData	(uint8_t* data_array, uint8_t array_length);
//! invia l'array su seriale
void SerialSendData		(uint8_t* data_array, uint8_t array_length);
//! invia una stringa su seriale
void SerialSendCString(const char* c_str);
//! ottiene l'ultimo carattere nel buffer di ricezione
uint8_t SerialGetByte		( void );
//! ritorna il numero di byte non leti nel buffer
uint8_t SerialGetRxFifoSize	( void );
//! copia gli ultimi byte_count byte nell'array di destinazione
void ReadLastRxBytes	(uint8_t* dest_array, uint8_t byte_count);

//! avvia la trasmissione del buffer su seriale
void COMStartSend		( void );

//! abilita la ricezione
void SerialRxEnable		( void );
//! disailita la ricezione
void SerialRxDisable	( void );

// TIMEOUT NON IMPLEMENTATI
void StartTimeout		( void );
void StopTimeout		( void );



/*******************************************************************************
		PROTOTYPE INTERRUPT ROUTINES
*******************************************************************************/
//#pragma interrupt ( COM2RxIsr( vect=VECT(SCI5, RXI5) ) )
//#pragma interrupt ( COM2TxIsr( vect=VECT(SCI5, TXI5) ) )
//
//void COM2RxIsr( void );
//void COM2TxIsr( void );
//
////void COM2ErrIsr( void );


#endif
