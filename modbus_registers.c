/*
 * modbus_opentherm.c
 *
 *  Created on: 08 mag 2020
 *      Author: Matteo
 */

#include <modbus_registers.h>
#include <globals.h>

#include <stdlib.h>

// MAPPA GLOBALE REGISTRI OPENTHERM
// I PRIMI OPENTHERM_MAIN_REGISTERS REGISTRI SONO QUELLI CHE CONTINUNANO A GIRARE
// SERVIRA UN INDICE E UN OLD_INDX PER I REGISTRI INSERITI DI VOLTA IN VOLTA NELLA TABELLA

// QUANDO RICEVO QUALCOSA CICLO FOR SUGLI ID. QUANDO CORRISPONDE ID == MODBUS REG TROVATO

MB_mapping modbusMap[TOTAL_MODBUS_REGISTERS] =
  {/*ID,  VALREAD,  VALWRITE    TYPE READ_ONLY    BLACKL    BLACL_VAL
                      READ_WRITE)           */
  { KEY_STATUS_WORD ,             NULL,      READ_ONLY },   
  { KEY_STATUS_01   ,             NULL,      READ_ONLY },   
  { KEY_STATUS_02   ,             NULL,      READ_ONLY },   
  { KEY_STATUS_03   ,             NULL,      READ_ONLY },   
  { KEY_STATUS_04   ,             NULL,      READ_ONLY },    
  { KEY_STATUS_05   ,             NULL,      READ_ONLY },    
  { KEY_STATUS_06   ,             NULL,      READ_ONLY },   
  { KEY_STATUS_07   ,             NULL,      READ_ONLY },    
  { KEY_STATUS_08   ,             NULL,      READ_ONLY },    
  { KEY_STATUS_09   ,             NULL,      READ_ONLY },   
  { KEY_STATUS_0A   ,             NULL,      READ_ONLY },   
  { KEY_STATUS_0B   ,             NULL,      READ_ONLY },    
  { KEY_STATUS_0C   ,             NULL,      READ_ONLY },    
  { KEY_STATUS_0D   ,             NULL,      READ_ONLY },   
  { KEY_STATUS_0E   ,             NULL,      READ_ONLY },    
  { CFG_KEY_NUMBER  ,             NULL,      READ_ONLY },   
  { CFG_KEY_01      ,             NULL,     READ_WRITE },
  { CFG_KEY_02      ,             NULL,     READ_WRITE },   
  { CFG_KEY_03      ,             NULL,     READ_WRITE },   
  { CFG_KEY_04      ,             NULL,     READ_WRITE },    
  { CFG_KEY_05      ,             NULL,     READ_WRITE },    
  { CFG_KEY_06      ,             NULL,     READ_WRITE },   
  { CFG_KEY_07      ,             NULL,     READ_WRITE },    
  { CFG_KEY_08      ,             NULL,     READ_WRITE },    
  { CFG_KEY_09      ,             NULL,     READ_WRITE },   
  { CFG_KEY_0A      ,             NULL,     READ_WRITE },   
  { CFG_KEY_0B      ,             NULL,     READ_WRITE },    
  { CFG_KEY_0C      ,             NULL,     READ_WRITE },    
  { CFG_KEY_0D      ,             NULL,     READ_WRITE },   
  { CFG_KEY_0E      ,             NULL,     READ_WRITE },    
  { CFG_COLOR_NUMBER,             NULL,      READ_ONLY },   
  { CFG_COLOR_01    ,             NULL,     READ_WRITE },   
  { CFG_COLOR_02    ,             NULL,     READ_WRITE },   
  { CFG_COLOR_03    ,             NULL,     READ_WRITE },   
  { CFG_COLOR_04    ,             NULL,     READ_WRITE },    
  { CFG_COLOR_05    ,             NULL,     READ_WRITE },    
  { CFG_COLOR_06    ,             NULL,     READ_WRITE },   
  { CFG_COLOR_07    ,             NULL,     READ_WRITE },    
  { CFG_COLOR_08    ,             NULL,     READ_WRITE },    
  { CFG_COLOR_09    ,             NULL,     READ_WRITE },   
  { CFG_COLOR_0A    ,             NULL,     READ_WRITE },   
  { CFG_COLOR_0B    ,             NULL,     READ_WRITE },    
  { CFG_COLOR_0C    ,             NULL,     READ_WRITE },    
  { CFG_COLOR_0D    ,             NULL,     READ_WRITE },   
  { CFG_COLOR_0E    ,             NULL,     READ_WRITE },    
};


/*******************************************************************************
    VARIABILI LOCALI
*******************************************************************************/


/** Lettura del registro relativo ai comandi opentherm
 *
 *  Parametri:  indirizzo: indirizzo del registro da leggere
 *
 **/
uint16_t ModbusReadRegister(uint16_t indirizzo)
{
  uint8_t i;
  uint8_t val = 0;

  for (i = 0; i < TOTAL_MODBUS_REGISTERS; i++)
  {
    if (modbusMap[i].id == indirizzo)
    {
      if(modbusMap[i].type & READ_ONLY)
        if(modbusMap[i].data_ptr != NULL)
          val = *modbusMap[i].data_ptr;
      break;
    }
  }

  return val;
}

/** Scrittura del registro relativo ai comandi opentherm
 *
 *  Parametri:  indirizzo: indirizzo del registro da leggere
 *
 **/
uint16_t ModbusWriteRegister(uint16_t indirizzo, uint16_t val)
{
  uint8_t i;
  uint8_t temp_type = 0;
  uint8_t temp_otIndx = 0xff;

  for (i = 0; i < TOTAL_MODBUS_REGISTERS; i++)
  {
    if (modbusMap[i].id == indirizzo)
    {
      if (modbusMap[i].type & WRITE_ONLY)
        if(modbusMap[i].data_ptr != NULL)
          *modbusMap[i].data_ptr = val;

      temp_otIndx = i;
      break;
    }
  }

  return 1;
}










































