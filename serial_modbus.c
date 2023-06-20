/*
 * Descrizione: Definizioni per la gestione del protocollo di comunicazione modbus
 *
 * File:   serial_modbus.c
 * Author: Powers
 *
 * Data creazione:  20/11/2018
 * Ultima modifica: 21/11/2018
 *
 */

#include <stdlib.h>

#include <serial.h>
#include <crc16.h>
#include <serial_modbus.h>

#include <app.h>


/** Puntatori a funzioni per registri e coils **/
uint16_t (*readCoil_ptr)            (uint16_t) = NULL;            //! Binding funzione read coil (0x01)
uint16_t (*readInput_ptr)           (uint16_t) = NULL;            //! Binding funzione read input status (0x02)
uint16_t (*readHoldingRegisters_ptr)(uint16_t) = NULL;            //! Binding funzione read holding registers (0x03)
uint16_t (*readInputRegisters_ptr)  (uint16_t) = NULL;            //! Binding funzione read input registers (0x04)
uint16_t (*writeCoil_ptr)           (uint16_t, uint16_t) = NULL;  //! Binding funzione write coil (0x05-0x0f)
uint16_t (*writeRegisters_ptr)      (uint16_t, uint16_t) = NULL;  //! Binding funzione write register (0x06-0x10)



/** LOCAL BUFFERS **/
volatile uint8_t ModbusRxCommand[RX_BUFFER_LEN];           //! Comando ricevuto via modbus
volatile uint8_t ModbusTxCommand[TX_BUFFER_LEN];           //! Comando da inviare tramite modbus
/** VARIABILI INTERNE **/
volatile uint8_t statoRx;
volatile uint8_t contTimeout;
volatile uint8_t timeoutFlag;
volatile uint8_t ModbusDecodeCommand;

volatile uint8_t _mb_slave_address;
volatile mb_type _modbus_type;

volatile uint8_t   _mb_master_response_flag;                     //! 0-> nessuna richiesta, 1->richiesta inviata, 2->risposta ricevuta, 4-> errore
volatile uint16_t  _mb_master_rd_rsp_len;
volatile uint16_t* _mb_master_rd_rsp_array;

/** FUNZIONI INTERNE **/
void ModbusDecode(uint8_t rxData);    //! Passa il carattere ricevuto alla macchina a stati
void ModbusDecoderReset(void);        //! Reset della macchina a stati


void ModbusInit(mb_type modbus_type, uint8_t slave_address)
{
  uint8_t i;

  statoRx             = 0;
  ModbusDecodeCommand = 0;
  contTimeout         = 0;
  timeoutFlag         = 0;

  for(i = 0; i < RX_BUFFER_LEN; i++)
    ModbusRxCommand[i] = 0;
  for(i = 0; i < TX_BUFFER_LEN; i++)
    ModbusTxCommand[i] = 0;

  _modbus_type = modbus_type;
  if(_modbus_type == MODBUS_SLAVE)
  {
    if(slave_address > 0)
      _mb_slave_address = slave_address;
    else
      _mb_slave_address = 1;
  }
  else
    _mb_slave_address = 0;

  return;
}


void ModbusHandler(void)
{
  uint8_t tmp;

  // Finche ci sono caratteri non decodificati li passo alla macchina a stati
  while( SerialGetRxFifoSize() != 0 )
  {
    // Passo il dato alla macchina a stati, se identifica il pacchetto
    // corretto lo decodifica ed esegue le azioni previste
    tmp = SerialGetByte();
    ModbusDecode(tmp);
  }
  if(ModbusDecodeCommand == 1)  // comando ricevuto, decodifico
  {
    if(_modbus_type == MODBUS_SLAVE)
      GestModbusCommandSlave();
    else if(_modbus_type == MODBUS_MASTER)
      GestModbusCommandMaster();

    ModbusDecodeCommand = 0;
  }

  // Gestione timeout ricezione (tick)
  if(timeoutFlag == 1)
  {
  if(contTimeout == 200)   // Timeout ricezione modbus
    ModbusDecoderReset();
  else
    contTimeout++;
  }
  return;
}


// Macchina a stati per la decodifica del comando ricevuto
void ModbusDecode(uint8_t rxData)
{
  static uint8_t bytesReceived;
  uint16_t crc16_value;
  uint16_t rxCrc;

  // Ricevo il primo comando
  switch(statoRx)
  {
    case 0:     // sono in attesa, byte 0, verifico il destinatario
      bytesReceived = 0;
      ModbusDecodeCommand = 0;

      ModbusRxCommand[MODBUS_OFF_CLIENTADDR] = rxData;
      statoRx++;
      bytesReceived = 0;
      break;

    case 1:   // leggo byte da 1 a 5
      ModbusRxCommand[MODBUS_OFF_FUNC+bytesReceived] = rxData;
      bytesReceived++;
      if(bytesReceived >= 5)
      {
        bytesReceived = 0;

        // per alcuni comandi ho finito, verifico crc
        if(ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_READ_COILS
            || ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_READ_COILS_IN
            || ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_READ
            || ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_READ_IN
            || ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_WRITE_SINGLE_COIL
            || ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_WRITE_SINGLE)
          statoRx = 4;
        else
          statoRx++;
      }
      break;

    case 2:   // leggo numero byte
      ModbusRxCommand[MODBUS_RD_OFF_LEN] = rxData;
      statoRx++;
      break;

    case 3:   // leggo dati, vale per write e write coils
      ModbusRxCommand[MODBUS_RD_OFF_DATA+bytesReceived] = rxData;
      bytesReceived++;
      if(bytesReceived >= ModbusRxCommand[MODBUS_RD_OFF_LEN])
      {
        statoRx++;
        bytesReceived = 0;
      }
      break;
    case 4:   // leggo crc
      if(ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_READ_COILS
          || ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_READ_COILS_IN
          || ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_READ
          || ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_READ_IN
          || ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_WRITE_SINGLE_COIL
          || ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_WRITE_SINGLE)
      {
        ModbusRxCommand[MODBUS_OFF_CRC+bytesReceived] = rxData;
        bytesReceived++;
        if(bytesReceived >= 2)
        {
          bytesReceived = MODBUS_OFF_CRC;  // escluso crc
          rxCrc = ModbusRxCommand[MODBUS_OFF_CRC] << 8
              | ModbusRxCommand[MODBUS_OFF_CRC+1];
          statoRx = 5;
        }
      }
      else if(ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_WRITE
          || ModbusRxCommand[MODBUS_OFF_FUNC] == MODBUS_FUNC_WRITE_COILS)
      {
        ModbusRxCommand[ 6+1+ModbusRxCommand[MODBUS_OFF_LEN]+bytesReceived ] = rxData;
        bytesReceived++;
        if(bytesReceived >= 2)
        {
          bytesReceived = MODBUS_OFF_WRDATA + ModbusRxCommand[MODBUS_OFF_LEN]; // escluso crc
          rxCrc = ModbusRxCommand[bytesReceived] << 8
              | ModbusRxCommand[bytesReceived+1];

          statoRx = 5;
        }
      }
      break;

    case 5:   // controllo crc
      crc16_value = CRC16(ModbusRxCommand, bytesReceived);

      if(rxCrc == crc16_value)
      {
        ModbusDecodeCommand = 1;
        statoRx = 0;
      }
      else if(rxCrc != crc16_value)
      {
        // se non è corretto invio errore
        ModbusSendError(ModbusRxCommand[MODBUS_OFF_CLIENTADDR],
                        ModbusRxCommand[MODBUS_OFF_FUNC],
                        MODBUS_EXCEPTION_CRC_ERR);
      }

      break;

    default:
      break;
  }
  return;
}

// Prende il pacchetto presente nel buffer e lo elabora
void GestModbusCommandSlave(void)
{
  uint16_t mb_address;
  uint16_t mb_registers;

  uint16_t mb_reg_count;
  uint16_t mb_reg_value;
  uint16_t crc16_value;

  uint8_t mb_tx_command_length;

  if(ModbusRxCommand[MODBUS_OFF_CLIENTADDR] != _mb_slave_address)
    return;

  if(ModbusRxCommand[MODBUS_RD_OFF_ADDR] != 0)
    return;

  mb_address = (uint16_t)ModbusRxCommand[MODBUS_RD_OFF_ADDR] << 8
      | (uint16_t)ModbusRxCommand[MODBUS_RD_OFF_ADDR+1];
  mb_registers = (uint16_t)ModbusRxCommand[MODBUS_RD_OFF_NREG] << 8
      | (uint16_t)ModbusRxCommand[MODBUS_RD_OFF_NREG+1];

  if (mb_registers >= MODBUS_MAX_RW_REGISTERS)
    mb_registers = MODBUS_MAX_RW_REGISTERS;

  // elaboro le risposte in base al comando ricevuto
  switch (ModbusRxCommand[MODBUS_OFF_FUNC])
  {
    case MODBUS_FUNC_READ:
      if(readHoldingRegisters_ptr == NULL)
      {
        mb_tx_command_length = 0;
        ModbusSendError(ModbusRxCommand[MODBUS_OFF_CLIENTADDR],
                        ModbusRxCommand[MODBUS_OFF_FUNC],
                        MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
        break;
      }

      ModbusTxCommand[MODBUS_OFF_CLIENTADDR] = ModbusRxCommand[MODBUS_OFF_CLIENTADDR];
      ModbusTxCommand[MODBUS_OFF_FUNC]       = ModbusRxCommand[MODBUS_OFF_FUNC];

      for(mb_reg_count = 0; mb_reg_count < mb_registers; mb_reg_count++)
      {
        mb_reg_value = readHoldingRegisters_ptr(mb_address+mb_reg_count);
        ModbusTxCommand[ MODBUS_SD_OFF_DATA+2*mb_reg_count   ] = (uint8_t)(mb_reg_value >> 8);
        ModbusTxCommand[ MODBUS_SD_OFF_DATA+2*mb_reg_count+1 ] = (uint8_t)mb_reg_value;
      }
      ModbusTxCommand[MODBUS_SD_OFF_BYTES]   = 5 + mb_registers*2;
      mb_tx_command_length = 5 + mb_registers*2;

      crc16_value = CRC16(ModbusTxCommand, mb_tx_command_length-2);
      ModbusTxCommand[ MODBUS_SD_OFF_DATA+2*mb_registers   ] = (uint8_t)(crc16_value >> 8);
      ModbusTxCommand[ MODBUS_SD_OFF_DATA+2*mb_registers+1 ] = (uint8_t)crc16_value;

      break;

    case MODBUS_FUNC_WRITE:
      if(writeRegisters_ptr == NULL)
      {
        mb_tx_command_length = 0;
        ModbusSendError(ModbusRxCommand[MODBUS_OFF_CLIENTADDR],
                        ModbusRxCommand[MODBUS_OFF_FUNC],
                        MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
        break;
      }

      ModbusTxCommand[MODBUS_OFF_CLIENTADDR] = ModbusRxCommand[MODBUS_OFF_CLIENTADDR];
      ModbusTxCommand[MODBUS_OFF_FUNC]       = ModbusRxCommand[MODBUS_OFF_FUNC];
      ModbusTxCommand[MODBUS_SD_OFF_ADDR]    = ModbusRxCommand[MODBUS_RD_OFF_ADDR];
      ModbusTxCommand[MODBUS_SD_OFF_ADDR+1]  = ModbusRxCommand[MODBUS_RD_OFF_ADDR+1];
      ModbusTxCommand[MODBUS_SD_OFF_NREG]    = ModbusRxCommand[MODBUS_RD_OFF_NREG];
      ModbusTxCommand[MODBUS_SD_OFF_NREG+1]  = ModbusRxCommand[MODBUS_RD_OFF_NREG+1];

      for(mb_reg_count = 0; mb_reg_count < mb_registers; mb_reg_count++)
      {
        mb_reg_value = (uint16_t)ModbusRxCommand[MODBUS_RD_OFF_DATA+2*mb_reg_count] << 8
            | (uint16_t)ModbusRxCommand[MODBUS_RD_OFF_DATA+2*mb_reg_count+1];
        writeRegisters_ptr(mb_address+mb_reg_count, mb_reg_value);
      }
      mb_tx_command_length = 6;

      crc16_value = CRC16(ModbusTxCommand, mb_tx_command_length-2);
      ModbusTxCommand[ MODBUS_SD_OFF_CRC   ] = (uint8_t)(crc16_value >> 8);
      ModbusTxCommand[ MODBUS_SD_OFF_CRC+1 ] = (uint8_t)crc16_value;

      break;

    // casi non implementati
    default:
    case MODBUS_FUNC_READ_COILS:
    case MODBUS_FUNC_READ_COILS_IN:
    case MODBUS_FUNC_READ_IN:
    case MODBUS_FUNC_WRITE_SINGLE_COIL:
    case MODBUS_FUNC_WRITE_SINGLE:
    case MODBUS_FUNC_WRITE_COILS:
      mb_tx_command_length = 0;
      ModbusSendError(ModbusRxCommand[MODBUS_OFF_CLIENTADDR],
                      ModbusRxCommand[MODBUS_OFF_FUNC],
                      MODBUS_EXCEPTION_ILLEGAL_FUNCTION);

      break;
  }

  if(mb_tx_command_length != 0)
    SerialSendData(ModbusTxCommand, mb_tx_command_length);
  return;
}

// Prende il pacchetto presente nel buffer e lo elabora
void GestModbusCommandMaster(void)
{
  uint16_t mb_address;
  uint16_t mb_registers;

  uint16_t mb_reg_count;
  uint16_t mb_reg_value;
  uint16_t crc16_value;

  uint8_t mb_tx_command_length;

//  if(ModbusRxCommand[MODBUS_OFF_CLIENTADDR] != _slave_address)
//    return;

  if(ModbusRxCommand[MODBUS_RD_OFF_ADDR] != 0)
    return;

  mb_address = (uint16_t)ModbusRxCommand[MODBUS_RD_OFF_ADDR] << 8
      | (uint16_t)ModbusRxCommand[MODBUS_RD_OFF_ADDR+1];
  mb_registers = (uint16_t)ModbusRxCommand[MODBUS_RD_OFF_NREG] << 8
      | (uint16_t)ModbusRxCommand[MODBUS_RD_OFF_NREG+1];

  if (mb_registers >= MODBUS_MAX_RW_REGISTERS)
    mb_registers = MODBUS_MAX_RW_REGISTERS;

  // elaboro le risposte in base alla risposta ricevuta
  switch (ModbusRxCommand[MODBUS_OFF_FUNC])
  {
    case MODBUS_FUNC_READ:

      _mb_master_response_flag = 1;
      for(mb_reg_count = 0; mb_reg_count < _mb_master_rd_rsp_len; mb_reg_count++)
      {
        _mb_master_rd_rsp_array[mb_reg_count] = (uint16_t)(ModbusRxCommand[ MODBUS_M_RRS_OFF_DATA + 2*mb_reg_count ] << 8 ) |
                                                  ModbusRxCommand[ MODBUS_M_RRS_OFF_DATA + 2*mb_reg_count+1 ];
      }

      _mb_master_response_flag = 2;
      // TODO: controllo crc

      break;

    case MODBUS_FUNC_WRITE:

//      ModbusTxCommand[MODBUS_OFF_CLIENTADDR] = ModbusRxCommand[MODBUS_OFF_CLIENTADDR];
//      ModbusTxCommand[MODBUS_OFF_FUNC]       = ModbusRxCommand[MODBUS_OFF_FUNC];
//      ModbusTxCommand[MODBUS_SD_OFF_ADDR]    = ModbusRxCommand[MODBUS_RD_OFF_ADDR];
//      ModbusTxCommand[MODBUS_SD_OFF_ADDR+1]  = ModbusRxCommand[MODBUS_RD_OFF_ADDR+1];
//      ModbusTxCommand[MODBUS_SD_OFF_NREG]    = ModbusRxCommand[MODBUS_RD_OFF_NREG];
//      ModbusTxCommand[MODBUS_SD_OFF_NREG+1]  = ModbusRxCommand[MODBUS_RD_OFF_NREG+1];
//
//      for(mb_reg_count = 0; mb_reg_count < mb_registers; mb_reg_count++)
//      {
//        mb_reg_value = (uint16_t)ModbusRxCommand[MODBUS_RD_OFF_DATA+2*mb_reg_count] << 8
//            | (uint16_t)ModbusRxCommand[MODBUS_RD_OFF_DATA+2*mb_reg_count+1];
//        writeRegisters_ptr(mb_address+mb_reg_count, mb_reg_value);
//      }
//      mb_tx_command_length = 6;
//
//      crc16_value = CRC16(ModbusTxCommand, mb_tx_command_length-2);
//      ModbusTxCommand[ MODBUS_SD_OFF_CRC   ] = (uint8_t)(crc16_value >> 8);
//      ModbusTxCommand[ MODBUS_SD_OFF_CRC+1 ] = (uint8_t)crc16_value;

      _mb_master_response_flag = 2;
      // TODO: controllo crc e pacchetto

      break;

    // casi non implementati
    default:
    case MODBUS_FUNC_READ_COILS:
    case MODBUS_FUNC_READ_COILS_IN:
    case MODBUS_FUNC_READ_IN:
    case MODBUS_FUNC_WRITE_SINGLE_COIL:
    case MODBUS_FUNC_WRITE_SINGLE:
    case MODBUS_FUNC_WRITE_COILS:
//      mb_tx_command_length = 0;
//      ModbusSendError(ModbusRxCommand[MODBUS_OFF_CLIENTADDR],
//                      ModbusRxCommand[MODBUS_OFF_FUNC],
//                      MODBUS_EXCEPTION_ILLEGAL_FUNCTION);

      break;
  }

//  if(mb_tx_command_length != 0)
//    SerialSendData(ModbusTxCommand, mb_tx_command_length);
  return;
}



/** Binding funzione read coil (0x01) */
void ModbusBind_ReadCoil( uint16_t (*readCoil)(uint16_t) )
{ readCoil_ptr = readCoil; }
/** Binding funzione read input status (0x02) */
void ModbusBind_ReadInput( uint16_t (*readInput)(uint16_t) )
{ readInput_ptr = readInput; }
/** Binding funzione read holding registers (0x03) */
void ModbusBind_ReadHolding( uint16_t (*readHoldingRegisters)(uint16_t) )
{ readHoldingRegisters_ptr = readHoldingRegisters; }
/** Binding funzione read input registers (0x04) */
void ModbusBind_ReadInputRegisters( uint16_t (*readInputRegisters)(uint16_t) )
{ readInputRegisters_ptr = readInputRegisters; }
/** Binding funzione write coil (0x05-0x0f) */
void ModbusBind_WriteCoil( uint16_t (*writeCoil)(uint16_t, uint16_t) )
{ writeCoil_ptr = writeCoil; }
/** Binding funzione write register (0x06-0x10) */
void ModbusBind_WriteRegister( uint16_t (*writeRegisters)(uint16_t, uint16_t) )
{ writeRegisters_ptr = writeRegisters; }


void ModbusSendError(uint8_t c_address, uint8_t mb_function, uint8_t err_code)
{
  uint16_t crc16_value;

  ModbusTxCommand[MODBUS_OFF_CLIENTADDR]  = c_address;
  ModbusTxCommand[MODBUS_OFF_FUNC]        = mb_function;
  ModbusTxCommand[MODBUS_OFF_FUNC]        |= MODBUS_FUNC_ERR_MASK;

  ModbusTxCommand[MODBUS_SD_OFF_EXC_CODE] = err_code;

  crc16_value = CRC16(ModbusTxCommand, 3);
  ModbusTxCommand[ MODBUS_SD_OFF_EXC_CRC   ] = (uint8_t)(crc16_value >> 8);
  ModbusTxCommand[ MODBUS_SD_OFF_EXC_CRC+1 ] = (uint8_t)crc16_value;

  SerialSendData(ModbusTxCommand, 5);
  return;
}

void ModbusDecoderReset(void)
{
  contTimeout = 0;
  timeoutFlag = 0;

  statoRx = 0;
  return;
}

/**
 * @param slave_address
 * @param register_address
 * @param length
 * @param dest_data
 * @return 0xff: error
 */
uint8_t ModbusMasterReadRegister(uint8_t slave_address, uint16_t register_address, uint16_t address_number, uint16_t* dest_array)
{
  uint16_t crc16_value;

  if(_modbus_type == MODBUS_SLAVE)
    return 0xff;

  _mb_master_response_flag = 1;
  _mb_master_rd_rsp_array = dest_array;
  _mb_master_rd_rsp_len   = address_number;

  ModbusTxCommand[MODBUS_OFF_CLIENTADDR]  = slave_address;
  ModbusTxCommand[MODBUS_OFF_FUNC]        = MODBUS_FUNC_READ;
  ModbusTxCommand[MODBUS_M_RRQ_OFF_ADDR]     = register_address >> 8;
  ModbusTxCommand[MODBUS_M_RRQ_OFF_ADDR+1]   = register_address & 0xff;
  ModbusTxCommand[MODBUS_M_RRQ_OFF_NREG]     = address_number >> 8;
  ModbusTxCommand[MODBUS_M_RRQ_OFF_NREG+1]   = address_number & 0xff;

  crc16_value = CRC16(ModbusTxCommand, 8-2);
  ModbusTxCommand[ MODBUS_M_RRQ_OFF_CRC ]    = (uint8_t)(crc16_value >> 8);
  ModbusTxCommand[ MODBUS_M_RRQ_OFF_CRC+1 ] = (uint8_t)crc16_value;

  SerialSendData(ModbusTxCommand, 8);

  return 0x00;
}

/**
 * @param slave_address
 * @param register_address
 * @param length
 * @param dest_data
 * @return 0xff: error
 */
uint8_t ModbusMasterWriteRegister(uint8_t slave_address, uint16_t register_address, uint16_t address_number, uint16_t* array_data)
{
  uint16_t crc16_value;
  uint16_t mb_tx_command_length;

  if(_modbus_type == MODBUS_SLAVE)
    return 0xff;

  _mb_master_response_flag = 1;

  ModbusTxCommand[MODBUS_OFF_CLIENTADDR]  = slave_address;
  ModbusTxCommand[MODBUS_OFF_FUNC]        = MODBUS_FUNC_WRITE;
  ModbusTxCommand[MODBUS_M_WRQ_OFF_ADDR]     = register_address >> 8;
  ModbusTxCommand[MODBUS_M_WRQ_OFF_ADDR+1]   = register_address & 0xff;
  ModbusTxCommand[MODBUS_M_WRQ_OFF_NREG]     = address_number >> 8;
  ModbusTxCommand[MODBUS_M_WRQ_OFF_NREG+1]   = address_number & 0xff;

  for(uint16_t mb_reg_count = 0; mb_reg_count < address_number; mb_reg_count++)
  {
    ModbusTxCommand[ MODBUS_M_WRQ_OFF_DATA+2*mb_reg_count   ] = (uint8_t)(array_data[mb_reg_count] >> 8);
    ModbusTxCommand[ MODBUS_M_WRQ_OFF_DATA+2*mb_reg_count+1 ] = (uint8_t)array_data[mb_reg_count];
  }
  ModbusTxCommand[MODBUS_M_WRQ_OFF_LEN]   = address_number*2;
  mb_tx_command_length = 7 + address_number*2;

  crc16_value = CRC16(ModbusTxCommand, mb_tx_command_length-2);
  ModbusTxCommand[ MODBUS_SD_OFF_DATA+2*address_number+0x01   ] = (uint8_t)(crc16_value >> 8);
  ModbusTxCommand[ MODBUS_SD_OFF_DATA+2*address_number+0x02 ] = (uint8_t)crc16_value;

  SerialSendData(ModbusTxCommand, mb_tx_command_length+2);

  return 0x00;
}

uint8_t ModbusResponseReady(void)
{
  if(_mb_master_response_flag == 2)
    return 1;
  else
    return 0;
}
