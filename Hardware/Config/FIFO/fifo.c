
#include "Fifo.h"
#include "global.h"
//******************************************************************************************
//!                     ASSERT MACRO
//******************************************************************************************
#ifndef ASSERT

#ifdef  NDEBUG
#define ASSERT(x)
#else
#define ASSERT(x) do {while(!(x));} while(0)
#endif

#endif  // ASSERT


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
int FIFO_S_Init(FIFO_S_t* pFIFO, void* pBaseAddr, uint32_t UnitCnt)
{
    //! Check input parameters.
    memset(pFIFO, 0, sizeof(FIFO_S_t));
    pFIFO->pStartAddr = pBaseAddr;
    pFIFO->buffSize = UnitCnt;
	pFIFO->usedLen = 0;
	
    return TRUE;
}


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
int FIFO_S_Put(FIFO_S_t* pFIFO, uint8_t Element)
{
    uint32_t datalen = 0;

    if (pFIFO->writeIndex >= pFIFO->readIndex) 
	{
        datalen = pFIFO->writeIndex - pFIFO->readIndex + 1;
    }
	else
	{
        datalen = pFIFO->writeIndex + pFIFO->buffSize - pFIFO->readIndex + 1;
    }
    if (pFIFO->buffSize > datalen) 
	{
        pFIFO->pStartAddr[pFIFO->writeIndex++] = Element;
        pFIFO->usedLen++;
        if (pFIFO->buffSize <= pFIFO->writeIndex) 
		{
            pFIFO->writeIndex = 0;
        }
        return 0;
    }
	
    return FALSE;
}


int FIFO_S_PutN(FIFO_S_t* pFIFO, uint8_t *pBuff, uint16_t cnt)
{
    int i;
    int ret = TRUE;

    for (i=0; i<cnt; i++) {
        ret |= FIFO_S_Put(pFIFO, pBuff[i]);
    }
    return ret;
}


//******************************************************************************************
//
//! \brief  Get an element from FIFO(in single mode).
//!
//! \param  [in]  pFIFO is the pointer of valid FIFO.
//!
//!
//
//******************************************************************************************
int FIFO_S_Get(FIFO_S_t* pFIFO, uint8_t *pData)
{
    if (pFIFO->writeIndex != pFIFO->readIndex) 
    {
        *pData = pFIFO->pStartAddr[pFIFO->readIndex++];
        if (pFIFO->buffSize <= pFIFO->readIndex) 
        {
            pFIFO->readIndex = 0;
        }
        if (pFIFO->usedLen) 
        {
            pFIFO->usedLen--;
        }
        return TRUE;
    }
    return FALSE;
}

uint16_t FIFO_S_GetAllData(FIFO_S_t* pFIFO, uint8_t *pData)
{
	//printf("writeIndex= [%d], readIndex= [%d] \r\n", pFIFO->writeIndex, pFIFO->readIndex);
	uint8_t *pdt = pData;
	uint16_t usedLen = pFIFO->usedLen;
	
	for(int i=0;pFIFO->usedLen>0;i++)
	{
		if (pFIFO->writeIndex != pFIFO->readIndex) 
		{		
			*pdt++ = pFIFO->pStartAddr[pFIFO->readIndex++];
			if (pFIFO->buffSize <= pFIFO->readIndex) 
			{
				pFIFO->readIndex = 0;
			}
			if (pFIFO->usedLen) 
			{
				pFIFO->usedLen--;
			}
		}
	}

	return usedLen;
}


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
int FIFO_S_IsEmpty(FIFO_S_t* pFIFO)
{
    //! Check input parameter.
    ASSERT(NULL != pFIFO);
    return (pFIFO->writeIndex == pFIFO->readIndex) ? TRUE : FALSE;
}


//******************************************************************************************
//
//! \brief  Get FIFO the number of elements(in single mode)?
//!
//! \param  [in] pFIFO is the pointer of valid FIFO.
//!
//! \retval The number of elements in FIFO.
//
//******************************************************************************************
int FIFO_S_CountUsed(FIFO_S_t* pFIFO)
{
    //! Check input parameter.
    ASSERT(NULL != pFIFO);
    return (pFIFO->usedLen);
}


//******************************************************************************************
//
//! \brief  Flush the content of FIFO.
//!
//! \param  [in] pFIFO is the pointer of valid FIFO.
//!
//! \retval 0 if success, -1 if failure.
//
//******************************************************************************************
int FIFO_S_Flush(FIFO_S_t* pFIFO)
{
    //! Check input parameters.
    ASSERT(NULL != pFIFO);
    //! Initialize FIFO Control Block.
    pFIFO->usedLen = 0;
    pFIFO->readIndex = 0;
    pFIFO->writeIndex = 0;
	
    return (0);
}




