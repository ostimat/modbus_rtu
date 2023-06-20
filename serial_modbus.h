/*
 * Descrizione: Header per la gestione del protocollo di comunicazione modbus
 *
 * File:   serial_modbus.h
 * Author: Powers
 *
 * Data creazione:  20/11/2018
 * Ultima modifica: 20/11/2018
 *
 */

#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>


/* SPECIFICHE PACCHETTO MODBUS */
//#define MODBUS_SLAVE_ADDRESS    0x01

/** MODBUS FUNCTION CODES **/
#define MODBUS_FUNC_READ_COILS        0x01
#define MODBUS_FUNC_READ_COILS_IN     0x02
#define MODBUS_FUNC_READ              0x03
#define MODBUS_FUNC_READ_IN           0x04
#define MODBUS_FUNC_WRITE_SINGLE_COIL 0x05
#define MODBUS_FUNC_WRITE_SINGLE      0x06
#define MODBUS_FUNC_WRITE_COILS       0x0f
#define MODBUS_FUNC_WRITE             0x10

#define MODBUS_FUNC_ERR_MASK          0x80
#define MODBUS_FUNC_FORCE_ERROR

/** MODBUS ERROR CODES **/
#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION         0x01
#define MODBUS_EXCEPTION_ILLEGAL_ADDRESS          0x02
#define MODBUS_EXCEPTION_ILLEGAL_DATAVAL          0x03
#define MODBUS_EXCEPTION_DEVICE_FAILURE           0x04
#define MODBUS_EXCEPTION_DEVICE_ACK               0x05
#define MODBUS_EXCEPTION_DEVICE_BUSY              0x06
#define MODBUS_EXCEPTION_DEVICE_NACK              0x07
#define MODBUS_EXCEPTION_CRC_ERR                  0x08
#define MODBUS_EXCEPTION_GATEWAY_PATH             0x0A
#define MODBUS_EXCEPTION_GATEWAY_DEVICE_TIMEOUT   0x0B


/** OFFSET PACCHETTO MODBUS **/
#define MODBUS_OFF_CLIENTADDR     0x00
#define MODBUS_OFF_FUNC           MODBUS_OFF_CLIENTADDR + 0x01

#define MODBUS_OFF_ADDRH          MODBUS_OFF_FUNC + 0x01
#define MODBUS_OFF_ADDRL          MODBUS_OFF_ADDRH + 0x01
#define MODBUS_OFF_ADDR           MODBUS_OFF_ADDRH

#define MODBUS_OFF_NREGH          MODBUS_OFF_ADDR + 0x02
#define MODBUS_OFF_NREGL          MODBUS_OFF_NREGH + 0x01
#define MODBUS_OFF_NREG           MODBUS_OFF_NREGH

// per write single coil (0x05)
#define MODBUS_OFF_STATUSH        MODBUS_OFF_ADDR + 0x02
#define MODBUS_OFF_STATUSL        MODBUS_OFF_STATUSH + 0x01
#define MODBUS_OFF_STATUS         MODBUS_OFF_STATUSH

// per write single (0x06)
#define MODBUS_OFF_VALUEH         MODBUS_OFF_ADDR + 0x02
#define MODBUS_OFF_VALUEL         MODBUS_OFF_VALUEH + 0x01
#define MODBUS_OFF_VALUE          MODBUS_OFF_VALUEH

// per tutti i casi tranne write coils (0x0f) e write multiple registers (0x10)
#define MODBUS_OFF_CRCH           MODBUS_OFF_NREGH + 0x02
#define MODBUS_OFF_CRCL           MODBUS_OFF_CRCH + 0x01
#define MODBUS_OFF_CRC            MODBUS_OFF_CRCH

// per write coils (0x0f) e write multiple registers (0x10)
#define MODBUS_OFF_LEN            MODBUS_OFF_NREGH + 0x02
#define MODBUS_OFF_WRDATA         MODBUS_OFF_LEN + 0x01


#define MODBUS_SD_OFF_BYTES        MODBUS_OFF_FUNC + 0x01
#define MODBUS_SD_OFF_DATA         MODBUS_SD_OFF_BYTES + 0x01
#define MODBUS_SD_OFF_ADDR         MODBUS_OFF_FUNC + 0x01
#define MODBUS_SD_OFF_NREG         MODBUS_SD_OFF_ADDR + 0x02
#define MODBUS_SD_OFF_CRC          MODBUS_SD_OFF_NREG + 0x02

// offset per risposte d'errore
#define MODBUS_SD_OFF_EXC_CODE     MODBUS_OFF_FUNC+0x01
#define MODBUS_SD_OFF_EXC_CRC      MODBUS_SD_OFF_EXC_CODE+0x01

#define MODBUS_RD_OFF_ADDR         MODBUS_OFF_FUNC + 0x01
#define MODBUS_RD_OFF_NREG         MODBUS_RD_OFF_ADDR + 0x02
#define MODBUS_RD_OFF_LEN          MODBUS_RD_OFF_NREG + 0x02
#define MODBUS_RD_OFF_DATA         MODBUS_RD_OFF_LEN  + 0x01
#define MODBUS_RD_OFF_CRC          MODBUS_RD_OFF_DATA + 0x02

#define MODBUS_SD_CRC_CALC_LEN     MODBUS_SD_OFF_CRC
#define MODBUS_SD_PACKET_LENGTH    MODBUS_SD_CRC_CALC_LEN+2
#define MODBUS_RD_CRC_CALC_LEN     MODBUS_RD_OFF_CRC
#define MODBUS_RD_PACKET_LENGTH    MODBUS_RD_CRC_CALC_LEN+2

//! Modbus master Read Request
#define MODBUS_M_RRQ_OFF_ADDR       MODBUS_OFF_FUNC+0x01
#define MODBUS_M_RRQ_OFF_NREG       MODBUS_M_RRQ_OFF_ADDR+0x02
#define MODBUS_M_RRQ_OFF_CRC        MODBUS_M_RRQ_OFF_NREG+0x02

//! Modbus master Write Request
#define MODBUS_M_WRQ_OFF_ADDR       MODBUS_OFF_FUNC+0x01
#define MODBUS_M_WRQ_OFF_NREG       MODBUS_M_WRQ_OFF_ADDR+0x02
#define MODBUS_M_WRQ_OFF_LEN        MODBUS_M_WRQ_OFF_NREG+0x02
#define MODBUS_M_WRQ_OFF_DATA       MODBUS_M_WRQ_OFF_LEN+0x01

//! Modbus master Read Response
#define MODBUS_M_RRS_OFF_LEN        MODBUS_OFF_FUNC+0x01
#define MODBUS_M_RRS_OFF_DATA       MODBUS_M_RRS_OFF_LEN+0x01

//! Modbus master Write Response
#define MODBUS_M_WRS_OFF_ADDR       MODBUS_OFF_FUNC+0x01
#define MODBUS_M_WRS_OFF_NREG       MODBUS_M_WRS_OFF_ADDR+0x02
#define MODBUS_M_WRS_OFF_CRC        MODBUS_M_WRS_OFF_NREG+0x02


/** Specifiche buffer **/
#define RX_BUFFER_LEN     70
#define TX_BUFFER_LEN     85    // fino a 40 registri

/* MODBUS MAX MULTIPLE RW */
#define MODBUS_MAX_RW_REGISTERS   ((TX_BUFFER_LEN-5)/2)



typedef enum  {
  MODBUS_MASTER   = 1,
  MODBUS_SLAVE    = 2,
} mb_type;



/** Inizializza la gestione modbus */
void ModbusInit(mb_type modbus_type, uint8_t slave_address);
/** Gestisce le code e controlla i comandi */
void ModbusHandler(void);

/** Binding funzione read coil (0x01) */
void ModbusBind_ReadCoil( uint16_t (*readCoil)(uint16_t) );
/** Binding funzione read input status (0x02) */
void ModbusBind_ReadInput( uint16_t (*readInput)(uint16_t) );
/** Binding funzione read holding registers (0x03) */
void ModbusBind_ReadHolding( uint16_t (*readHoldingRegisters)(uint16_t) );
/** Binding funzione read input registers (0x04) */
void ModbusBind_ReadInputRegisters( uint16_t (*readInputRegisters)(uint16_t) );
/** Binding funzione write coil (0x05-0x0f) */
void ModbusBind_WriteCoil( uint16_t (*writeCoil)(uint16_t, uint16_t) );
/** Binding funzione write register (0x06-0x10) */
void ModbusBind_WriteRegister( uint16_t (*writeRegisters)(uint16_t, uint16_t) );


/** Invia il codice errore specificato **/
void ModbusSendError(uint8_t c_address, uint8_t mb_function, uint8_t err_code);


/* Buffer comandi modbus */
extern volatile uint8_t ModbusRxCommand[];           //! Comando ricevuto via modbus
extern volatile uint8_t ModbusTxCommand[];           //! Comando da inviare tramite modbus



/** LOCAL FUNCTIONS **/
uint8_t ModbusGetChar(void);
uint8_t ModbusNewRxBytes(void);
void ModbusRead(uint8_t rxData);

void GestModbusCommandMaster(void);
void GestModbusCommandSlave(void);

uint8_t ModbusMasterReadRegister(uint8_t slave_address, uint16_t register_address, uint16_t address_number, uint16_t* dest_array);
uint8_t ModbusMasterWriteRegister(uint8_t slave_address, uint16_t register_address, uint16_t address_number, uint16_t* array_data);




#endif  /* XC_HEADER_TEMPLATE_H */

