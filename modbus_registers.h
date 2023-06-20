/*
 * modbus_driver.h
 *
 *  Created on: 12 giu 2023
 *      Author: Matteo
 */

#ifndef MODBUS_REGISTERS_H_
#define MODBUS_REGISTERS_H_

#include <stdint.h>


#define TOTAL_MODBUS_REGISTERS    100


#define BOARD_STATUS      0x0000    //
#define CONFIGURED_FLAG   0x0001    // 0: ha il mapping di default
#define BLE_STATUS        0x0010    // 0: disconnesso, 1: connesso, 2: pairing

#define KEY_STATUS_WORD   0x1000    // status word pulsanti, bitfield. 0: pulsante non premuto
#define KEY_STATUS_01     0x1001
#define KEY_STATUS_02     0x1002
#define KEY_STATUS_03     0x1003
#define KEY_STATUS_04     0x1004
#define KEY_STATUS_05     0x1005
#define KEY_STATUS_06     0x1006
#define KEY_STATUS_07     0x1007
#define KEY_STATUS_08     0x1008
#define KEY_STATUS_09     0x1009
#define KEY_STATUS_0A     0x100A
#define KEY_STATUS_0B     0x100B
#define KEY_STATUS_0C     0x100C
#define KEY_STATUS_0D     0x100D
#define KEY_STATUS_0E     0x100E

// indirizzi configurazione funzioni: word
#define CFG_KEY_NUMBER    0x4000
#define CFG_KEY_01        0x4001
#define CFG_KEY_02        0x4002
#define CFG_KEY_03        0x4003
#define CFG_KEY_04        0x4004
#define CFG_KEY_05        0x4005
#define CFG_KEY_06        0x4006
#define CFG_KEY_07        0x4007
#define CFG_KEY_08        0x4008
#define CFG_KEY_09        0x4009
#define CFG_KEY_0A        0x400A
#define CFG_KEY_0B        0x400B
#define CFG_KEY_0C        0x400C
#define CFG_KEY_0D        0x400D
#define CFG_KEY_0E        0x400E

// indirizzi configurazione colori: long
#define CFG_COLOR_NUMBER  0x4100
#define CFG_COLOR_01      0x4101
#define CFG_COLOR_02      0x4102
#define CFG_COLOR_03      0x4103
#define CFG_COLOR_04      0x4104
#define CFG_COLOR_05      0x4105
#define CFG_COLOR_06      0x4106
#define CFG_COLOR_07      0x4107
#define CFG_COLOR_08      0x4108
#define CFG_COLOR_09      0x4109
#define CFG_COLOR_0A      0x410A
#define CFG_COLOR_0B      0x410B
#define CFG_COLOR_0C      0x410C
#define CFG_COLOR_0D      0x410D
#define CFG_COLOR_0E      0x410E

#define CFG_KEY_FUNC_START_ADDRESS    CFG_KEY_01
#define CFG_KEY_COLOR_START_ADDRESS   CFG_COLOR_01




enum register_type
{
  READ_ONLY   = 1,
  WRITE_ONLY  = 2,
  READ_WRITE  = 3,
};

// definizione struttura registri opentherm
typedef struct
{
  uint16_t id;
  uint16_t* data_ptr;
  uint8_t  type;
} MB_mapping;


/** REGISTRI OPENTHERM **/
extern MB_mapping modbusMap[TOTAL_MODBUS_REGISTERS];


/** FUNZIONI REGISTRI MODBUS **/
uint16_t ModbusReadRegister(uint16_t indirizzo);                 //! read holding registers
uint16_t ModbusWriteRegister(uint16_t indirizzo, uint16_t val);  //! write registers

/** Funzioni gestione opentherm **/
void OpenthermLoop(void);
//! aggiornamento mappa opentherm
void ModbusGestMap(uint8_t cmd_type, uint8_t ot_reg_id, uint16_t ot_value);



#endif /* MODBUS_REGISTERS_H_ */
