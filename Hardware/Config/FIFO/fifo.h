#ifndef __FIFO_H__
#define __FIFO_H__


#ifdef __cplusplus
extern "C"
{
#endif


#define USE_SYS_INTXX                      //!< Use stdint standard datatype

#define MCU_ARM

#include <stdio.h>
#include <string.h>
#ifdef USE_SYS_INTXX
#include <stdint.h>
#else
//typedef unsigned char uint8_t;
//typedef unsigned short uint16_t;
//typedef unsigned int  uint32_t;
#endif

#ifdef USE_DYNAMIC_MEMORY
#include <stdlib.h>
#endif
#include "gd32f30x.h"

#define MASTER_INT_EN()  __enable_irq()
#define MASTER_INT_DIS() __disable_irq()


//******************************************************************************************
//!                           PUBLIC TYPE
//******************************************************************************************

//! FIFO Memory Model
typedef struct
{
    uint8_t* pStartAddr;                   //!< FIFO Memory Pool Start Address
    uint8_t* pEndAddr;                     //!< FIFO Memory Pool End Address
    uint32_t Free;                         //!< The capacity of FIFO
    uint32_t Used;                         //!< The number of elements in FIFO
    uint8_t  UnitSize;                     //!< FIFO Element Size(Unit: Byte)
    uint8_t* pReadIndex;                   //!< FIFO Data Read Index Pointer
    uint8_t* pWriteIndex;                  //!< FIFO Data Write Index Pointer
}FIFO_t;

//! FIFO Memory Model (Single Byte Mode)
typedef struct
{
    volatile uint8_t* pStartAddr;                    //!< FIFO Memory Pool Start Address
    volatile uint16_t buffSize;                      //!< The capacity of FIFO
    volatile uint16_t usedLen;                       //!< The number of elements in FIFO
    volatile uint16_t readIndex;                     //!< FIFO Data Read Index Pointer
    volatile uint16_t writeIndex;                    //!< FIFO Data Write Index Pointer
}FIFO_S_t;

//******************************************************************************************
//!                           PUBLIC API
//******************************************************************************************

//******************************************************************************************
//
//! \brief  Initialize an static FIFO struct.
//!
//! \param  [in] pFIFO is the pointer of valid FIFO instance.
//! \param  [in] pBaseAddr is the base address of pre-allocate memory, such as array.
//! \param  [in] UnitSize is fifo element size.
//! \param  [in] UnitCnt is count of fifo elements.
//! \retval 0 if initialize successfully, otherwise return -1.
//
//******************************************************************************************
extern int FIFO_Init(FIFO_t* pFIFO, void* pBaseAddr, uint8_t UnitSize, uint32_t UnitCnt);

//******************************************************************************************
//
//! \brief  Initialize an static FIFO struct(in single mode).
//!
//! \param  [in] pFIFO is the pointer of valid FIFO instance.
//! \param  [in] pBaseAddr is the base address of pre-allocate memory, such as array.
//! \param  [in] UnitCnt is count of fifo elements.
//! \retval 0 if initialize successfully, otherwise return -1.
//
//******************************************************************************************
extern int FIFO_S_Init(FIFO_S_t* pFIFO, void* pBaseAddr, uint32_t UnitCnt);

//******************************************************************************************
//
//! \brief  Put an element into FIFO.
//!
//! \param  [in]  pFIFO is the pointer of valid FIFO.
//! \param  [in]  pElement is the address of element you want to put
//!
//! \retval 0 if operate successfully, otherwise return -1.
//
//******************************************************************************************
extern int FIFO_Put(FIFO_t* pFIFO, void* pElement);

extern int FIFO_S_PutN(FIFO_S_t* pFIFO, uint8_t *pBuff, uint16_t cnt);

//******************************************************************************************
//
//! \brief  Put an element into FIFO(in single mode).
//!
//! \param  [in]  pFIFO is the pointer of valid FIFO.
//! \param  [in]  Element is the data element you want to put
//!
//! \retval 0 if operate successfully, otherwise return -1.
//
//******************************************************************************************
extern int FIFO_S_Put(FIFO_S_t* pFIFO, uint8_t Element);

//******************************************************************************************
//
//! \brief  Get an element from FIFO.
//!
//! \param  [in]  pFIFO is the pointer of valid FIFO.
//! \param  [out] pElement is the address of element you want to get
//!
//! \retval 0 if operate successfully, otherwise return -1.
//
//******************************************************************************************
extern int FIFO_Get(FIFO_t* pFIFO, void* pElement);

//******************************************************************************************
//
//! \brief  Get an element from FIFO(in single mode).
//!
//! \param  [in]  pFIFO is the pointer of valid FIFO.
//!
//! \retval the data element of FIFO.
//
//******************************************************************************************
int FIFO_S_Get(FIFO_S_t* pFIFO, uint8_t *pData);

extern uint16_t FIFO_S_GetAllData(FIFO_S_t* pFIFO, uint8_t *pData);

//******************************************************************************************
//
//! \brief  Pre-Read an element from FIFO.
//!
//! \param  [in]  pFIFO is the pointer of valid FIFO.
//! \param  [in]  Offset is the offset from current pointer.
//! \param  [out] pElement is the address of element you want to get
//!
//! \retval 0 if operate successfully, otherwise return -1.
//
//******************************************************************************************
extern int FIFO_PreRead(FIFO_t* pFIFO, uint8_t Offset, void* pElement);

//******************************************************************************************
//
//! \brief  Pre-Read an element from FIFO(in single mode).
//!
//! \param  [in]  pFIFO is the pointer of valid FIFO.
//! \param  [in]  Offset is the offset from current pointer.
//!
//! \retval the data element of FIFO.
//
//******************************************************************************************
extern uint8_t FIFO_S_PreRead(FIFO_S_t* pFIFO, uint8_t Offset);

//******************************************************************************************
//
//! \brief  FIFO is empty ?
//!
//! \param  [in] pFIFO is the pointer of valid FIFO.
//!
//! \retval - None-zero(true) if empty.
//!         - Zero(false) if not empty.
//
//******************************************************************************************
extern int FIFO_IsEmpty(FIFO_t* pFIFO);

//******************************************************************************************
//
//! \brief  FIFO is empty (in single mode)?
//!
//! \param  [in] pFIFO is the pointer of valid FIFO.
//!
//! \retval - None-zero(true) if empty.
//!         - Zero(false) if not empty.
//
//******************************************************************************************
extern int FIFO_S_IsEmpty(FIFO_S_t* pFIFO);

//******************************************************************************************
//
//! \brief  Get FIFO the number of elements?
//!
//! \param  [in] pFIFO is the pointer of valid FIFO.
//!
//! \retval The number of elements in FIFO.
//
//******************************************************************************************
extern int FIFO_CountUsed(FIFO_t* pFIFO);

//******************************************************************************************
//
//! \brief  Get FIFO the number of elements(in single mode)?
//!
//! \param  [in] pFIFO is the pointer of valid FIFO.
//!
//! \retval The number of elements in FIFO.
//
//******************************************************************************************
extern int FIFO_S_CountUsed(FIFO_S_t* pFIFO);

//******************************************************************************************
//
//! \brief  Get FIFO the number of elements?
//!
//! \param  [in] pFIFO is the pointer of valid FIFO.
//!
//! \retval The number of elements in FIFO.
//
//******************************************************************************************
extern int FIFO_CountFree(FIFO_t* pFIFO);

//******************************************************************************************
//
//! \brief  Get FIFO the number of elements(in single mode)?
//!
//! \param  [in] pFIFO is the pointer of valid FIFO.
//!
//! \retval The number of elements in FIFO.
//
//******************************************************************************************
extern int FIFO_S_CountFree(FIFO_S_t* pFIFO);

//******************************************************************************************
//
//! \brief  Flush the content of FIFO.
//!
//! \param  [in] pFIFO is the pointer of valid FIFO.
//!
//! \retval 0 if success, -1 if failure.
//
//******************************************************************************************
extern int FIFO_Flush(FIFO_t* pFIFO);

//******************************************************************************************
//
//! \brief  Flush the content of FIFO.
//!
//! \param  [in] pFIFO is the pointer of valid FIFO.
//!
//! \retval 0 if success, -1 if failure.
//
//******************************************************************************************
extern int FIFO_S_Flush(FIFO_S_t* pFIFO);

extern int FIFO_S_GetOverFlow(FIFO_S_t* pFIFO);

#ifdef __cplusplus
}
#endif

#endif // __FIFO_H__
