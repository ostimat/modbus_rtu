#include <serial.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "sl_uartdrv_instances.h"
#include <globals.h>

uint8_t rxByte;


/** VARIABILI ESTERNE **/
// Specifiche delle porte seriali
volatile SerType BuffSer;


/** VARIABILI INTERNE **/

bool tx_InProgress;


/* Inizializzazione porte seriali */
void InitSerialPort(enum SerialSpeed baud_rate)
{
	// inizializzazione buffers
	InitSerialBuffers();

	return;
}

/* Apertura comunicazioni */
void OpenSerialPort(void (*SerialProtocolHandler)(void))
{
	/* Inizializzo il puntatore al protocollo utilizzato e inizializziamo il protocollo utilizzato */
	BuffSer.GestSerialFunc_ptr = SerialProtocolHandler;

//  UARTDRV_Receive(sl_uartdrv_usart_usart0_handle, &rxByte, 1, UART_rx_callback);

	InitSerialBuffers();

  return;
}

void ResetSerialPort(void)
{
  BuffSer.NumBytesRx = 0;
  BuffSer.RxBytes = 0;
  BuffSer.StatoRx = 0;
  BuffSer.RxRun   = 0;

  BuffSer.TickSer = -1;

  return;
}

/* Inizializzazione dei buffer */
void InitSerialBuffers(void)
{
  BuffSer.RxBytes = 0;
  BuffSer.TxBytes = 0;
  BuffSer.RxRun   = 0;				// ricezione non in corso
  BuffSer.StatoRx = 0;				// stato ricezione
  BuffSer.Timeout = 0;				// Timeout seriale (tick rtc)

  return;
}

/* Sfrutta il puntatore alla funzione della gestione degli stati del protocollo specificato */
void GestSerialPort(void)
{
  if(BuffSer.GestSerialFunc_ptr != NULL)
    BuffSer.GestSerialFunc_ptr();

  return;
}

void WritePacketInt(void)
{
  // trasferisce il pacchetto di BuffTx in BuffTxInt
  // Scrivo il contenuto del registro data nel buffer di scrittura.
  uint16_t i;
  uint16_t write_idx;

  // copio i dati nel buffer, parto dall'indirizzo index, man mano aumento la lunghezza dei dati nel buffer
  for(i = 0; i < BuffSer.TxBytes; i++)
  {
	write_idx = BuffSer.TxBufferIndex + BuffSer.TxBufferLength;
	if(write_idx >= LEN_TXBUF)
	  write_idx -= LEN_TXBUF;

	// circular buffer index
	if(BuffSer.TxBufferLength >= LEN_TXBUF)
	  BuffSer.TxBufferLength = LEN_TXBUF;
	else
	  BuffSer.TxBufferLength++;

  }
  return;
}

void SerialWriteData(uint8_t* data_array, uint8_t array_length)
{
  // trasferisce il pacchetto di data_array in BuffTxInt
  // Scrivo il contenuto del registro data nel buffer di scrittura.
  uint16_t i;
  uint16_t write_idx;

  if(array_length == 0)
    return;

  // copio i dati nel buffer, parto dall'indirizzo index, man mano aumento la lunghezza dei dati nel buffer
  for(i = 0; i < array_length; i++)
  {
	write_idx = BuffSer.TxBufferIndex + BuffSer.TxBufferLength;
	if(write_idx >= LEN_TXBUF)
	  write_idx -= LEN_TXBUF;

	BuffSer.BuffTxInt[write_idx] = data_array[i];

	// circular buffer index
	if(BuffSer.TxBufferLength >= LEN_TXBUF)
	  BuffSer.TxBufferLength = LEN_TXBUF;
	else
	  BuffSer.TxBufferLength++;

  }
  return;
}

void SerialSendData(uint8_t* data_array, uint8_t array_length)
{
  SerialWriteData(data_array, array_length);
  COMStartSend();
  return;
}

void SerialSendCString(const char* c_str)
{
  SerialWriteData(c_str, strlen(c_str)+1);
  COMStartSend();
  return;
}


void SerTimeout(void)
{
  // Controllo time-out seriali, inibizione timeout, impostazione a -1
  if( BuffSer.Timeout == -1 )
	return;
  // se il tick vale -1 il conteggio del timeout va fermato (non richiesto e non impostato)
  if( BuffSer.TickSer == -1)
	return;

  // se il timeout non è scaduto decremento il contatore
  if( BuffSer.TickSer > 0 )
	BuffSer.TickSer--;
  else
  {
	if(BuffSer.RxRun == 1)		// ricezione in corso
	{
	  ResetSerialPort();
	}
//	if(expected a response)

  }

  return;
}

void StartTimeout(void)
{
  BuffSer.TickSer = BuffSer.Timeout;
  return;
}
void StopTimeout(void)
{
  BuffSer.TickSer = -1;
  return;
}

// Estrae il primo byte dalla fifo
uint8_t SerialGetByte(void)
{
  uint8_t tmp = 0;
  if(BuffSer.RxBufferLength > 0)
  {
	tmp = BuffSer.BuffRx[ BuffSer.RxBufferIndex ];

	BuffSer.RxBufferIndex++;
	if( BuffSer.RxBufferIndex >= LEN_RXBUF)
	  BuffSer.RxBufferIndex -= LEN_RXBUF;

	BuffSer.RxBufferLength--;
  }

  return tmp;
}

uint8_t SerialGetRxFifoSize(void)
{
  return BuffSer.RxBufferLength;
}

void ReadLastRxBytes(uint8_t* dest_array, uint8_t byte_count)
{
  uint8_t i;
  for(i = 0; i < byte_count; i++)
  {
	uint16_t idx = BuffSer.RxBufferIndex + BuffSer.RxBufferLength - 1;
	idx -= (byte_count-i);
	if(idx >= LEN_RXBUF)
	  idx -= LEN_RXBUF;

	dest_array[i] = BuffSer.BuffRx[idx];
  }
  return;
}

void COMStartSend(void)
{
  if(tx_InProgress)
  	return;

	tx_InProgress = true;
  UARTDRV_Transmit(sl_uartdrv_usart_usart0_handle, BuffSer.BuffTxInt, BuffSer.TxBufferLength, NULL);

  return;
}

void SerialRxEnable(void)
{
//	SCI5.SCR.BIT.RE = 1;

  return;
}

void SerialRxDisable(void)
{
//	SCI5.SCR.BIT.RE = 0;

  return;
}

/* ------- INTERRUPT FUNCTIONS ------- */
static void UART_rx_callback(UARTDRV_Handle_t handle, Ecode_t transferStatus, uint8_t *data,
                             UARTDRV_Count_t transferCount)
{
  *(uint32_t*)(key_status) += 1;

	uint16_t idx;

  if(BuffSer.RxBufferLength >= LEN_RXBUF)
  {
		BuffSer.RxBufferLength = LEN_RXBUF;
		// aumento di uno il puntatore al più vecchio (l'elemento precedente viene sovrascritto)
		BuffSer.RxBufferIndex++;
		if( BuffSer.RxBufferIndex >= LEN_RXBUF )
			BuffSer.RxBufferIndex -= LEN_RXBUF;
  }
  else
  	BuffSer.RxBufferLength++;

  idx = BuffSer.RxBufferIndex + BuffSer.RxBufferLength - 1;
  if(idx >= LEN_RXBUF)
  	idx -= LEN_RXBUF;

  BuffSer.BuffRx[idx] = *data;

  /* RX the next byte */
  UARTDRV_Receive(sl_uartdrv_usart_usart0_handle, &rxByte, 1, UART_rx_callback);

  return;
}



