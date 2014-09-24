/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.uboot.rwtest.c
 * Date         : 2014.08.30
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.08.30 TW.KIM)
 *
 * Description  :
 *
 ******************************************************************************/
#define __MIO_UBOOT_RWTEST_GLOBAL__

#include "mio.uboot.rwtest.h"

#include <malloc.h>

#include <mio.uboot.h>

#define __SUPPORT_DEBUG_PRINT_MIO_UBOOT_RWTEST__
#if defined (__SUPPORT_DEBUG_PRINT_MIO_UBOOT_RWTEST__)
    #define DBG_UBOOT_RWTEST(fmt, args...) printf(fmt, ##args)
#else
    #define DBG_UBOOT_RWTEST(fmt, args...)
#endif

/*******************************************************************************
 * local variables
 *******************************************************************************/
static struct
{
    ulong uiCapacity;

    ulong uiWriteRatio; // 0 ~ 100
    ulong uiSeqRatio;   // 0 ~ 100

    ulong uiMinTransferSectors;
    ulong uiMaxTransferSectors;

    ulong uiFlushCmdCycle;
    ulong uiStandbyCycle;
    ulong uiPowerdownCycle;
    ulong uiTrimCycle;

    ulong uiSectorsToTest;

    ulong uiBuffSize;
    unsigned char *pucBuff;
    unsigned int *puiIsWrittenLba;

} gstMioRwtestParam;

static struct 
{
    unsigned int uiCmdNo;

    unsigned int uiWrittenSectors;
    unsigned int uiReadSectors;
    unsigned int uiComparedSectors;
    unsigned int uiWriteCmdCnt[2];   // [0]: sequent, [1]: random
    unsigned int uiReadCmdCnt[2];    // [0]: sequent, [1]: random
    unsigned int uiFlushCmdCnt;
    unsigned int uiStandbyCmdCnt;
    unsigned int uiPowerdownCmdCnt;

    unsigned int uiError;

    struct 
    {
        unsigned int uiAddress;
        unsigned int uiSectors;
        unsigned int uiWrittenData;
        unsigned int uiReadData;
        unsigned int uiCmdNo;
        unsigned int uiCmdStartAddr;
        unsigned int uiCmdSectors;
        
    } stErrInfo;
    
} gstMioRwtestResult;

/*******************************************************************************
 * local functions
 *******************************************************************************/
static int mio_rwtest_init(ulong ulTestSectors, ulong ulCapacity, unsigned char ucWriteRatio, unsigned char ucSequentRatio);
static void mio_rwtest_deinit(void);
static ulong mio_rwtest_write(ulong blknr, lbaint_t blkcnt);
static ulong mio_rwtest_read_verify(ulong blknr, lbaint_t blkcnt);
static void print_view_rwtest_result(void);

#ifndef rand
    /*
     * Simple xorshift PRNG
     *   see http://www.jstatsoft.org/v08/i14/paper
     *
     * Copyright (c) 2012 Michael Walle
     * Michael Walle <michael@walle.cc>
     *
     * See file CREDITS for list of people who contributed to this
     * project.
     *
     * This program is free software; you can redistribute it and/or
     * modify it under the terms of the GNU General Public License as
     * published by the Free Software Foundation; either version 2 of
     * the License, or (at your option) any later version.
     *
     * This program is distributed in the hope that it will be useful,
     * but WITHOUT ANY WARRANTY; without even the implied warranty of
     * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     * GNU General Public License for more details.
     *
     * You should have received a copy of the GNU General Public License
     * along with this program; if not, write to the Free Software
     * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
     * MA 02111-1307 USA
     */

    static unsigned int y = 1U;

    unsigned int rand_r(unsigned int *seedp)
    {
        *seedp ^= (*seedp << 13);
        *seedp ^= (*seedp >> 17);
        *seedp ^= (*seedp << 5);

        return *seedp;
    }

    unsigned int rand(void)
    {
        return rand_r(&y);
    }

    void srand(unsigned int seed)
    {
        y = seed;
    }
#endif

/*******************************************************************************
 *
 *******************************************************************************/
int mio_rwtest_run(ulong ulTestSectors, ulong ulCapacity, unsigned char ucWriteRatio, unsigned char ucSequentRatio)
{
    int MaxTestLoop=300000, CurrTestLoop=0;

    int siResp=0;
    //ulong ulStartSeconds=0;
    unsigned int uiAddr=0, uiNextSeqWriteAddr=0, uiNextSeqReadAddr=0;
    unsigned int uiSectors=0, uiTestSectors=0;
    unsigned int uiWriteSectors=0, uiReadSectors=0;
    unsigned char ucIsWrite=0, ucIsSequent=0;
    
    siResp = mio_rwtest_init(ulTestSectors, ulCapacity, ucWriteRatio, ucSequentRatio);
    if (siResp < 0)
    {
        DBG_UBOOT_RWTEST("mio_rwtest_run: mio_rwtest_init() error\n");
    }
    else
    {
        memset(&gstMioRwtestResult, 0, sizeof(gstMioRwtestResult));

        srand(1234);

        DBG_UBOOT_RWTEST("mio_rwtest_run: gstMioRwtestParam.uiCapacity:%lu, uiMaxTransferSectors:%lu, uiMinTransferSectors:%lu\n",
            gstMioRwtestParam.uiCapacity, gstMioRwtestParam.uiMaxTransferSectors, gstMioRwtestParam.uiMinTransferSectors);
        do
        {
            if (gstMioRwtestResult.uiCmdNo && !(gstMioRwtestResult.uiCmdNo % gstMioRwtestParam.uiFlushCmdCycle))
            {
                DBG_UBOOT_RWTEST("0x%08X mio_rwtest_run: Flush %u\n", CurrTestLoop, gstMioRwtestResult.uiCmdNo);
                mio_flush();
                gstMioRwtestResult.uiFlushCmdCnt += 1;
            }
            else if (gstMioRwtestResult.uiCmdNo && !(gstMioRwtestResult.uiCmdNo % gstMioRwtestParam.uiStandbyCycle))
            {
                DBG_UBOOT_RWTEST("0x%08X mio_rwtest_run: Standby %u)\n", CurrTestLoop, gstMioRwtestResult.uiCmdNo);
                mio_standby();
                
                siResp = mio_init();
                if (siResp < 0)
                {
                    DBG_UBOOT_RWTEST("mio_rwtest_run: mio_rwtest_init() failed !!\n");
                    break;
                }

                gstMioRwtestResult.uiStandbyCmdCnt += 1;
            }
            else if (gstMioRwtestResult.uiCmdNo && !(gstMioRwtestResult.uiCmdNo % gstMioRwtestParam.uiPowerdownCycle))
            { 
                DBG_UBOOT_RWTEST("0x%08X mio_rwtest_run: Powerdown %u\n", CurrTestLoop, gstMioRwtestResult.uiCmdNo);
              //mio_powerdown();
                gstMioRwtestResult.uiPowerdownCmdCnt += 1;
                // ...
                // mio_init();
                // ...
            }
            else
            {
                ucIsWrite = ((((unsigned int)rand())%100) >= (100 - gstMioRwtestParam.uiWriteRatio))? 1: 0;
                ucIsSequent = ((((unsigned int)rand())%100) >= (100 - gstMioRwtestParam.uiWriteRatio))? 1: 0;

                if (ucIsWrite)
                {
                    uiAddr = (ucIsSequent)? uiNextSeqWriteAddr: (((unsigned int)rand()) % gstMioRwtestParam.uiCapacity);
                }
                else
                {
                    uiAddr = (ucIsSequent)? uiNextSeqReadAddr: (((unsigned int)rand()) % gstMioRwtestParam.uiCapacity);
                }

                uiSectors = ((unsigned int)rand()) % (gstMioRwtestParam.uiMaxTransferSectors - gstMioRwtestParam.uiMinTransferSectors);
                uiSectors += gstMioRwtestParam.uiMinTransferSectors;
                if (uiAddr + uiSectors > gstMioRwtestParam.uiCapacity)
                {
                    uiSectors = gstMioRwtestParam.uiCapacity - uiAddr;
                }

                if (ucIsWrite)
                {
                    DBG_UBOOT_RWTEST("0x%08X mio_rwtest_write(0x%08x, 0x%8x), \tnextSeqAddr:0x%08x\n", CurrTestLoop, uiAddr, uiSectors, uiAddr + uiSectors);
                    uiWriteSectors = mio_rwtest_write(uiAddr, uiSectors);
                    if (uiWriteSectors != uiSectors)
                    {
                        DBG_UBOOT_RWTEST("mio_rwtest_write: error %d, %d\n", uiWriteSectors, uiSectors);
                        break;
                    }
                    uiNextSeqWriteAddr = ((uiAddr + uiSectors) == gstMioRwtestParam.uiCapacity)? 0: uiAddr + uiSectors;
                    

                    if (ucIsSequent)    gstMioRwtestResult.uiWriteCmdCnt[0] += 1;
                    else                gstMioRwtestResult.uiWriteCmdCnt[1] += 1;
                }
                else
                {
                    DBG_UBOOT_RWTEST("0x%08X mio_rwtest_read(0x%08x, 0x%8x), \tnextSeqAddr:0x%08x\n", CurrTestLoop, uiAddr, uiSectors, uiAddr + uiSectors);
                    uiReadSectors = mio_rwtest_read_verify(uiAddr, uiSectors);
                    if (uiReadSectors != uiSectors)
                    {
                        DBG_UBOOT_RWTEST("mio_rwtest_read_verify: error %d, %d\n", uiReadSectors, uiSectors);
                        break;
                    }
                    uiNextSeqReadAddr = ((uiAddr + uiSectors) == gstMioRwtestParam.uiCapacity)? 0: uiAddr + uiSectors;

                    if (ucIsSequent)    gstMioRwtestResult.uiReadCmdCnt[0] += 1;
                    else                gstMioRwtestResult.uiReadCmdCnt[1] += 1;
                }
                uiTestSectors += uiSectors;
            }
            
            gstMioRwtestResult.uiCmdNo += 1;

            if (gstMioRwtestResult.uiError)
                break;

#if 1
            CurrTestLoop++;
            if (CurrTestLoop >= MaxTestLoop)
            {
                break;
            }
#endif
        } while(uiTestSectors < gstMioRwtestParam.uiSectorsToTest);

        DBG_UBOOT_RWTEST("mio_rwtest_run: Test end ! \n");
        
    }

    print_view_rwtest_result();
    mio_rwtest_deinit();

    return 0;
}

/*******************************************************************************
 * local functions
 *******************************************************************************/
static int mio_rwtest_init(ulong ulSectorsToTest, ulong ulCapacity, unsigned char ucWriteRatio, unsigned char ucSequentRatio)
{
    int siResp=-1;
    unsigned int uiBuffSize=0;

    memset(&gstMioRwtestParam, 0, sizeof(gstMioRwtestParam));
    
    // set parameter for Read/Write Test 
    gstMioRwtestParam.uiCapacity = ulCapacity;

    gstMioRwtestParam.uiWriteRatio = ucWriteRatio; // 0 ~ 100 %
    gstMioRwtestParam.uiSeqRatio = ucSequentRatio; // 0 ~ 100 %

    gstMioRwtestParam.uiFlushCmdCycle =  38;
    gstMioRwtestParam.uiStandbyCycle =   200;
    gstMioRwtestParam.uiPowerdownCycle = 100000;
    gstMioRwtestParam.uiTrimCycle = 1000;

    gstMioRwtestParam.uiSectorsToTest = (unsigned int)ulSectorsToTest;

    // allocate buffer
    gstMioRwtestParam.uiBuffSize = 10*1024*1024;
    gstMioRwtestParam.pucBuff = (unsigned char *)malloc(gstMioRwtestParam.uiBuffSize);
    if (!gstMioRwtestParam.pucBuff)
        siResp = -1;

  //gstMioRwtestParam.uiMinTransferSectors = (4*1024)/512;       //  4 KB
  //gstMioRwtestParam.uiMaxTransferSectors = (32*1024*1024)/512; // 32 MB

    gstMioRwtestParam.uiMinTransferSectors = (4*1024)/512;       //  4 KB
    gstMioRwtestParam.uiMaxTransferSectors = (1*1024*1024)/512;  //  1 MB

    if (gstMioRwtestParam.uiMaxTransferSectors > (gstMioRwtestParam.uiBuffSize / 512))
    {
        gstMioRwtestParam.uiMaxTransferSectors = gstMioRwtestParam.uiBuffSize / 512;
    }

    uiBuffSize = (gstMioRwtestParam.uiCapacity + 7) / 8;
    gstMioRwtestParam.puiIsWrittenLba = (unsigned int *)malloc(uiBuffSize);
    if (gstMioRwtestParam.puiIsWrittenLba)
        memset(gstMioRwtestParam.puiIsWrittenLba, 0, uiBuffSize);
    else
        siResp = -1;

    memset(&gstMioRwtestResult, 0, sizeof(gstMioRwtestResult));

    siResp = 0;

    if (siResp < 0)
    {
        printf("mio_rwtest_init: error\n");
        mio_rwtest_deinit();
    }

    return siResp;
}

static void mio_rwtest_deinit(void)
{
    if (gstMioRwtestParam.pucBuff)
        free(gstMioRwtestParam.pucBuff);

    if (gstMioRwtestParam.puiIsWrittenLba)
        free(gstMioRwtestParam.puiIsWrittenLba);
}

static ulong mio_rwtest_write(ulong blknr, lbaint_t blkcnt)
{
    ulong ulSize=0;

    unsigned int uiSectorsLeft = blkcnt;
    unsigned int uiByteIndex=0, uiBitIndex=0;
    unsigned int uiAddr=blknr, uiSectors=0, uiBlkIndex=0, uiBuffOffset=0;
    const unsigned int uiMaxSectorsInCmd = (gstMioRwtestParam.uiBuffSize/512);
    unsigned int *puiData=0;

    while(uiSectorsLeft)
    {
        uiSectors = (uiSectorsLeft > uiMaxSectorsInCmd)? uiMaxSectorsInCmd: uiSectorsLeft;

        for (uiBlkIndex=0; uiBlkIndex < uiSectors; uiBlkIndex++)
        {
            uiBuffOffset = uiBlkIndex % uiMaxSectorsInCmd;

            puiData = (unsigned int *)(gstMioRwtestParam.pucBuff + (uiBuffOffset * 512));

            memset(puiData, 0, 512);
            puiData[0] = (unsigned int)(uiAddr + uiBlkIndex);      // 1st 4B: blknr
            puiData[1] = (unsigned int)gstMioRwtestResult.uiCmdNo; // 2nd 4B: write command number
            puiData[2] = (unsigned int)uiAddr;                     // 3rd 4B: start blknr of write command
            puiData[3] = (unsigned int)uiSectors;                  // 4th 4B: blkcnt of write command

            uiByteIndex = puiData[0] / 32;
            uiBitIndex = puiData[0] % 32;

            gstMioRwtestParam.puiIsWrittenLba[uiByteIndex] |= (1 << uiBitIndex);
        }
        
        ulSize = mio_write(uiAddr, uiSectors, gstMioRwtestParam.pucBuff);
        if (ulSize != uiSectors)
        {
            gstMioRwtestResult.uiError = 1;
            gstMioRwtestResult.stErrInfo.uiAddress = uiAddr;
            gstMioRwtestResult.stErrInfo.uiSectors = uiSectors;
          //gstMioRwtestResult.stErrInfo.uiWrittenData = 0;
          //gstMioRwtestResult.stErrInfo.uiReadData = 0;
            gstMioRwtestResult.stErrInfo.uiCmdNo = gstMioRwtestResult.uiCmdNo;
            gstMioRwtestResult.stErrInfo.uiCmdStartAddr = uiAddr;
            gstMioRwtestResult.stErrInfo.uiCmdSectors = uiSectors;
        }

        uiAddr += ulSize;
        uiSectorsLeft -= ulSize;

        if (gstMioRwtestResult.uiError)
            break;

    }

    gstMioRwtestResult.uiWrittenSectors += (blkcnt - uiSectorsLeft);

    if (uiSectorsLeft)
    {
        printf("mio_rwtest_write() failed!\n");
    }

    return (blkcnt - uiSectorsLeft);
}

static ulong mio_rwtest_read_verify(ulong blknr, lbaint_t blkcnt)
{
    ulong ulSize=0, ulComparedSectors=0;

    unsigned int uiSectorsLeft = blkcnt;
    unsigned int uiByteIndex=0, uiBitIndex=0;
    unsigned int uiAddr=blknr, uiSectors=0, uiBlkIndex=0;
    const unsigned int uiMaxSectorsInCmd = (gstMioRwtestParam.uiBuffSize/512);
    unsigned int *puiData = (unsigned int *)gstMioRwtestParam.pucBuff;

    while(uiSectorsLeft)
    {
        uiSectors = (uiSectorsLeft > uiMaxSectorsInCmd)? uiMaxSectorsInCmd: uiSectorsLeft;

        ulSize = mio_read(uiAddr, uiSectors, puiData);
        if (ulSize == uiSectors)
        {
            for (uiBlkIndex=0; uiBlkIndex < ulSize; uiBlkIndex++)
            {
                puiData = (unsigned int *)(gstMioRwtestParam.pucBuff + (uiBlkIndex * 512));

                uiByteIndex = (uiAddr + uiBlkIndex) / 32;
                uiBitIndex = (uiAddr + uiBlkIndex) % 32;

              //printf("uiSectorsLeft %d, uiBlkIndex %d, ulSize %d, Byte %d Bit %d [0x%0x]\n",
              //    uiSectorsLeft, uiBlkIndex, ulSize, uiByteIndex, uiBitIndex, gstMioRwtestParam.puiIsWrittenLba[uiByteIndex]);

                if (gstMioRwtestParam.puiIsWrittenLba[uiByteIndex] & (1 << uiBitIndex))
                {
                    if ((puiData[0] == (unsigned int)(uiAddr + uiBlkIndex)))
                    {
                        ulComparedSectors++;
                        gstMioRwtestResult.uiComparedSectors++;
                    }
                    else
                    {   // Verify failed!
                        gstMioRwtestResult.uiError = (1<<2);

                        gstMioRwtestResult.stErrInfo.uiAddress = uiAddr + uiBlkIndex;
                      //gstMioRwtestReslut.stErrInfo.uiSectors = 0;
                        gstMioRwtestResult.stErrInfo.uiWrittenData = uiAddr;
                        gstMioRwtestResult.stErrInfo.uiReadData = puiData[0];
                        gstMioRwtestResult.stErrInfo.uiCmdNo = puiData[1];
                        gstMioRwtestResult.stErrInfo.uiCmdStartAddr = puiData[2];
                        gstMioRwtestResult.stErrInfo.uiCmdSectors = puiData[3];
                    }
                }
                else // not written yet.
                {
                }
            }
        }
        else
        {
            gstMioRwtestResult.uiError = (1<<1);
            gstMioRwtestResult.stErrInfo.uiAddress = uiAddr;
            gstMioRwtestResult.stErrInfo.uiSectors = uiSectors;
          //gstMioRwtestResult.stErrInfo.uiWrittenData = uiAddr;
          //gstMioRwtestResult.stErrInfo.uiReadData = puiData[0];
          //gstMioRwtestResult.stErrInfo.uiCmdNo = gstMioRwtestResult.uiCmdNo;
          //gstMioRwtestResult.stErrInfo.uiCmdStartAddr = puiData[2];
          //gstMioRwtestResult.stErrInfo.uiCmdSectors = puiData[3];

        }

        uiAddr += ulSize;
        uiSectorsLeft -= ulSize;

        if (gstMioRwtestResult.uiError)
            break;

    }

    gstMioRwtestResult.uiReadSectors += (blkcnt - uiSectorsLeft);

    return (blkcnt - uiSectorsLeft);
}

static void print_view_rwtest_result(void)
{
    printf("##########################\n");
    printf("READ/WRITE TEST SUMMARY!!!\n");
    printf(" Total written  sectors: %u (%u MB)\n", gstMioRwtestResult.uiWrittenSectors, gstMioRwtestResult.uiWrittenSectors/(2*1024));
    printf(" Total read     sectors: %u (%u MB)\n", gstMioRwtestResult.uiReadSectors, gstMioRwtestResult.uiReadSectors/(2*1024));
    printf(" Total compared sectors: %u (%u MB)\n", gstMioRwtestResult.uiComparedSectors, gstMioRwtestResult.uiComparedSectors/(2*1024));
    printf("\n");
    printf(" write     command count: %u (Seq:%u, Rand:%u)\n", gstMioRwtestResult.uiWriteCmdCnt[0]+gstMioRwtestResult.uiWriteCmdCnt[1], gstMioRwtestResult.uiWriteCmdCnt[0], gstMioRwtestResult.uiWriteCmdCnt[1]);
    printf(" read      command count: %u (Seq:%u, Rand:%u)\n", gstMioRwtestResult.uiReadCmdCnt[0]+gstMioRwtestResult.uiReadCmdCnt[1], gstMioRwtestResult.uiReadCmdCnt[0], gstMioRwtestResult.uiReadCmdCnt[1]);
    printf(" flush     command count: %u\n", gstMioRwtestResult.uiFlushCmdCnt);
    printf(" standby   command count: %u\n", gstMioRwtestResult.uiStandbyCmdCnt);
    printf(" powerdown command count: %u\n", gstMioRwtestResult.uiPowerdownCmdCnt);
  //printf(" Total test time: %u secs\n", gstMioRwtestResult....);
  //printf("\n");

    if (gstMioRwtestResult.uiError & (1<<0))
    {
        printf("\n");
        printf(" write command failed!\n");
        printf("  Address: %X, Sectors: %X\n", gstMioRwtestResult.stErrInfo.uiAddress, gstMioRwtestResult.stErrInfo.uiSectors);
        printf("  write command number       : %d\n", gstMioRwtestResult.stErrInfo.uiCmdNo);
        printf("  write command start address: %d\n", gstMioRwtestResult.stErrInfo.uiCmdStartAddr);
        printf("  write command sectors      : %d\n", gstMioRwtestResult.stErrInfo.uiCmdSectors);
    }

    if (gstMioRwtestResult.uiError & (1<<1))
    {
        printf("\n");
        printf(" read command failed!\n");
        printf("  address: %X, sectors: %X\n", gstMioRwtestResult.stErrInfo.uiAddress, gstMioRwtestResult.stErrInfo.uiSectors);
    }

    if (gstMioRwtestResult.uiError & (1<<2))
    {
        printf("\n");
        printf(" Verify failed!\n");
        printf("  address:%d",  gstMioRwtestResult.stErrInfo.uiAddress);
        printf("  written data[0]: %08X\n", gstMioRwtestResult.stErrInfo.uiWrittenData);
        printf("  read    data[0]: %08X\n", gstMioRwtestResult.stErrInfo.uiReadData);
        printf("  This sector is written by the followings write command\n");
        printf("   write command number                : %d\n", gstMioRwtestResult.stErrInfo.uiCmdNo);
        printf("   previous write command start address: %d\n", gstMioRwtestResult.stErrInfo.uiCmdStartAddr);
        printf("   previous write command sectors      : %d\n", gstMioRwtestResult.stErrInfo.uiCmdSectors);
    }
    printf("##########################\n");
}
