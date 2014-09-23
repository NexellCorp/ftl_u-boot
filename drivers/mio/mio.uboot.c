/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.uboot.c
 * Date         : 2014.08.30
 * Author       : TW.KIM (taewon@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.08.30 TW.KIM)
 *
 * Description  :
 *
 ******************************************************************************/
#define __MIO_UBOOT_GLOBAL__

/*******************************************************************************
 *
 *******************************************************************************/
#include "mio.uboot.h"

#include "media/exchange.h"
#include "mio.definition.h"

#include <common.h>
#include <malloc.h>

#include "media/nfc/phy/nfc.phy.lowapi.h"
#include "media/nfc/phy/nfc.phy.h"
#include "media/nfc/phy/nfc.phy.readretry.h"

#if 1
#include "mio.uboot.rwtest.h"
#endif

/*******************************************************************************
 *
 *******************************************************************************/
#if defined (__BUILD_MODE_X86_LINUX_DEVICE_DRIVER__)
#define __MEDIA_ON_RAM__
#elif defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
#define __MEDIA_ON_NAND__
#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
#define __MEDIA_ON_NAND__
#endif

/*******************************************************************************
 *
 *******************************************************************************/
#define MEDIA_READ_WRITE_TEST

#if defined (MEDIA_READ_WRITE_TEST)
    static struct
    {
        U32 uiDataSize;

        U8 *pucWData;
        U8 *pucRData;

} gstRW;
#endif

/*******************************************************************************
 *
 *******************************************************************************/
#define __SUPPORT_DEBUG_MIO_UBOOT_ERROR_STOP__

/*******************************************************************************
 * local functions
 *******************************************************************************/
static unsigned char is_mio_init = 0;

static S32 mio_cmd_to_ftl(U16 usCommand, U8 ucFeature, U32 uiAddress, U32 uiLength);
static void mio_fill_read_buffer(void *pvBuff, U32 uiSectors);
static void mio_fill_write_cache(const void *pvBuff, U32 uiSectors);

/*******************************************************************************
 * functions
 *******************************************************************************/

/*******************************************************************************
 *
 *******************************************************************************/
int mio_format(int _format_type)
{
#if defined (__MEDIA_ON_NAND__)

    int resp = -1;
    int capacity = -1;

    if (is_mio_init)
    {
        mio_deinit();
    }

    /**************************************************************************
     * FTL Need Leaner Buffer
     **************************************************************************/
    DBG_MEDIA("mio_format: Memory Pool Pre Alloc:\n");
    Exchange.buffer.mpool_size  = 0;
    Exchange.buffer.mpool_size += 1 * 4 * (4<<20); // 1CH x 4WAY x 4MB (Page Map Table per Lun)
    Exchange.buffer.mpool_size += 1 * 4 * (1<<20); // 1CH x 4WAY x 1MB (Update Map Table per Lun)
    Exchange.buffer.mpool_size += (1<<20);         // 1MB (Misc)
    Exchange.buffer.mpool = (unsigned char *)malloc(Exchange.buffer.mpool_size);

    if (!Exchange.buffer.mpool)
    {
        DBG_MEDIA("mio_format: Memory Pool Pre Alloc: fail\n");
        return -1;
    }

    DBG_MEDIA("mio_format: EXCHANGE_init:\n");
    EXCHANGE_init();

    /**************************************************************************
     * MIO Debug Options
     **************************************************************************/
    Exchange.debug.ftl.format = 1;
  //Exchange.debug.ftl.format_progress = 1;
    Exchange.debug.ftl.configurations = 1;
    Exchange.debug.ftl.open = 1;
    Exchange.debug.ftl.memory_usage = 1;
    Exchange.debug.ftl.boot = 1;
    Exchange.debug.ftl.error = 1;

  //Exchange.debug.nfc.sche.operation = 1;

  //Exchange.debug.nfc.phy.operation = 1;
  //Exchange.debug.nfc.phy.info_feature = 1;
  //Exchange.debug.nfc.phy.info_ecc = 1;
  //Exchange.debug.nfc.phy.info_ecc_correction = 1;
  //Exchange.debug.nfc.phy.info_ecc_corrected = 1;
    Exchange.debug.nfc.phy.warn_prohibited_block_access = 1;
    Exchange.debug.nfc.phy.warn_ecc_uncorrectable = 1;

    /**************************************************************************
     * FTL Format
     **************************************************************************/
    DBG_MEDIA("mio_format: Exchange.ftl.fnFormat()\n");
    if ((resp = Exchange.ftl.fnFormat((unsigned char *)"NXP4330", 0xC0067000, (unsigned char)_format_type)) < 0)
    {
        DBG_MEDIA("mio_format: Exchange.ftl.fnFormat: fail\n");
    }

    capacity = *Exchange.ftl.Capacity;
    DBG_MEDIA("mio_init: Capacity %xh(%d) Sectors = %d MB: \n", capacity, capacity, ((capacity>>10)<<9)>>10);

    is_mio_init = 1;

    /**************************************************************************
     *
     **************************************************************************/
    mio_deinit();

    if (resp < 0)
    {
        return -1;
    }
#endif
    return 1;
}

int mio_init(void)
{
#if defined (__MEDIA_ON_NAND__)
    int resp = -1;
    int capacity = -1;

    if (is_mio_init)
    {
        mio_deinit();
    }

    /**************************************************************************
     * FTL Need Leaner Buffer
     **************************************************************************/
    Exchange.buffer.mpool_size  = 0;
    Exchange.buffer.mpool_size += 1 * 4 * (4<<20); // 1CH x 4WAY x 4MB (Page Map Table per Lun)
    Exchange.buffer.mpool_size += 1 * 4 * (1<<20); // 1CH x 4WAY x 1MB (Update Map Table per Lun)
    Exchange.buffer.mpool_size += (1<<20);         // 1MB (Misc)
    Exchange.buffer.mpool = (unsigned char *)malloc(Exchange.buffer.mpool_size);

    if (!Exchange.buffer.mpool)
    {
        DBG_MEDIA("mio_init: Memory Pool Pre Alloc: fail\n");
        return -1;
    }

    DBG_MEDIA("mio_init: EXCHANGE_init:\n");
    EXCHANGE_init();

    /**************************************************************************
     * MIO Debug Options
     **************************************************************************/
    Exchange.debug.ftl.format = 1;
    Exchange.debug.ftl.format_progress = 1;
    Exchange.debug.ftl.configurations = 1;
    Exchange.debug.ftl.open = 1;
    Exchange.debug.ftl.memory_usage = 1;
    Exchange.debug.ftl.boot = 1;
    Exchange.debug.ftl.error = 1;

  //Exchange.debug.nfc.sche.operation = 1;

  //Exchange.debug.nfc.phy.operation = 1;
  //Exchange.debug.nfc.phy.info_feature = 1;
  //Exchange.debug.nfc.phy.info_ecc = 1;
  //Exchange.debug.nfc.phy.info_ecc_correction = 1;
  //Exchange.debug.nfc.phy.info_ecc_corrected = 1;
    Exchange.debug.nfc.phy.warn_prohibited_block_access = 1;
    Exchange.debug.nfc.phy.warn_ecc_uncorrectable = 1;

    /**************************************************************************
     * FTL Open & Boot
     **************************************************************************/
    do 
    {
        DBG_MEDIA("mio_init: Exchange.ftl.fnOpen()\n");
        if ((resp = Exchange.ftl.fnOpen((unsigned char *)"NXP4330", 0xC0067000, 0)) < 0)
        {
            DBG_MEDIA("mio_init: Exchange.ftl.fnOpen(): fail\n");
            break;
        }

        DBG_MEDIA("mio_init: Exchange.ftl.fnBoot(0)\n");
        if ((resp = Exchange.ftl.fnBoot(0)) < 0)
        {
            DBG_MEDIA("mio_init: Exchange.ftl.fnBoot(0): fail\n");
            break;
        }

    } while (0);

    if (resp < 0)
    {
        mio_deinit();
        return -1;
    }

    capacity = *Exchange.ftl.Capacity;
    DBG_MEDIA("WriteCacheBase: 0x%0X, Sectors: 0x%0X\n", (U32)(*Exchange.buffer.BaseOfWriteCache), (U32)(*Exchange.buffer.SectorsOfWriteCache));
    DBG_MEDIA("ReadBufferBase: 0x%0X, Sectors: 0x%0X\n", (U32)(*Exchange.buffer.BaseOfReadBuffer), (U32)(*Exchange.buffer.SectorsOfReadBuffer));
    DBG_MEDIA("mio_init: Capacity %xh(%d) Sectors = %d MB: \n", capacity, capacity, ((capacity>>10)<<9)>>10);

    is_mio_init = 1;

    /**************************************************************************
     * Memory Allocations : Read/Write Test
     **************************************************************************/
#if defined (MEDIA_READ_WRITE_TEST)
    {
        U32 i = 0;

        gstRW.uiDataSize = 10 * 1024 * 1024;

        gstRW.pucWData = (U8 *)malloc(gstRW.uiDataSize);
        gstRW.pucRData = (U8 *)malloc(gstRW.uiDataSize);

        if (!gstRW.pucWData || !gstRW.pucRData)
        {
            DBG_MEDIA("RW data buffer alloc failed!\n");

            if (gstRW.pucWData)
                free(gstRW.pucWData);

            if (gstRW.pucRData)
                free(gstRW.pucRData);
        }

        for (i = 0; i < gstRW.uiDataSize / 4; i++)
        {
            ((U32 *)gstRW.pucWData)[i] = i;
        }

        printf("WriteBuff: 0x%0X, ReadBuff: 0x%0X\n", (U32)gstRW.pucWData, (U32)gstRW.pucRData);
    }
#endif

#endif
    return 1;
}

/*******************************************************************************
 *
 *******************************************************************************/
int mio_deinit(void)
{
#if defined (__MEDIA_ON_NAND__)
    if (is_mio_init)
    {
        Exchange.ftl.fnClose();
        free(Exchange.buffer.mpool);

        is_mio_init = 0;

#if defined (MEDIA_READ_WRITE_TEST)
        if (gstRW.pucWData)
            free(gstRW.pucWData);

        if (gstRW.pucRData)
            free(gstRW.pucRData);
#endif
    }
#endif

    return 1;
}

/*******************************************************************************
 *
 *******************************************************************************/
int mio_info(void)
{
    int resp = 0;
    NAND nand;

    if (!is_mio_init)
    {
        DBG_MEDIA("mio_info(): mio is not initialized!!\n");
        return 0;
    }

    resp = Exchange.ftl.fnGetNandInfo(&nand);

    if (resp < 0)
    {
        DBG_MEDIA("mio_info(): failed to get NAND information.\n");
        return 0;
    }

    printf("\n NAND INFORMATION");

    printf("\n");
    printf("*******************************************************************************\n");
    printf("* NAND Configuration Summary\n");
    printf("*\n");
    printf("* - Manufacturer : %s\n", nand._f.manufacturer);
    printf("* - Model Name : %s\n", nand._f.modelname);
    printf("* - Generation : %s\n", nand._f.generation);

    printf("*\n");
    printf("* - Interfacetype : %d\n", nand._f.interfacetype);
    printf("* - ONFI Detected : %d\n", nand._f.onfi_detected);
    printf("* - ONFI Timing Mode : %d\n", nand._f.onfi_timing_mode);

    printf("*\n");
    printf("* - tClk : %d\n", nand._f.timing.async.tClk);
    printf("* - tRWC : %d\n", nand._f.timing.async.tRWC);
    printf("* - tR : %d\n", nand._f.timing.async.tR);
    printf("* - tWB : %d\n", nand._f.timing.async.tWB);
    printf("* - tCCS : %d\n", nand._f.timing.async.tCCS);
    printf("* - tADL : %d\n", nand._f.timing.async.tADL);
    printf("* - tRHW : %d\n", nand._f.timing.async.tRHW);
    printf("* - tWHR : %d\n", nand._f.timing.async.tWHR);
    printf("* - tWW : %d\n", nand._f.timing.async.tWW);

    printf("*\n");
    printf("* - tCS : %d\n", nand._f.timing.async.tCS);
    printf("* - tCH : %d\n", nand._f.timing.async.tCH);
    printf("* - tCLS : %d\n", nand._f.timing.async.tCLS);
    printf("* - tALS : %d\n", nand._f.timing.async.tALS);
    printf("* - tCLH : %d\n", nand._f.timing.async.tCLH);
    printf("* - tALH : %d\n", nand._f.timing.async.tALH);
    printf("* - tWP : %d\n", nand._f.timing.async.tWP);
    printf("* - tWH : %d\n", nand._f.timing.async.tWH);
    printf("* - tWC : %d\n", nand._f.timing.async.tWC);
    printf("* - tDS : %d\n", nand._f.timing.async.tDS);
    printf("* - tDH : %d\n", nand._f.timing.async.tDH);
    printf("* - tCEA : %d\n", nand._f.timing.async.tCEA);
    printf("* - tREA : %d\n", nand._f.timing.async.tREA);
    printf("* - tRP : %d\n", nand._f.timing.async.tRP);
    printf("* - tREH : %d\n", nand._f.timing.async.tREH);
    printf("* - tRC : %d\n", nand._f.timing.async.tRC);
    printf("* - tCOH : %d\n", nand._f.timing.async.tCOH);

    printf("*\n");
    printf("* - Luns Per Ce : %d\n", nand._f.luns_per_ce);
    printf("* - Databytes Per Page : %d\n", nand._f.databytes_per_page);
    printf("* - Sparebytes Per Page : %d\n", nand._f.sparebytes_per_page);
    printf("* - Number Of Planes : %d\n", nand._f.number_of_planes);
    printf("* - Pages Per Block : %d\n", nand._f.pages_per_block);
    printf("* - Mainblocks Per Lun : %d\n", nand._f.mainblocks_per_lun);
    printf("* - Extendedblocks Per Lun : %d\n", nand._f.extendedblocks_per_lun);
    printf("* - Next Lun Address : %d\n", nand._f.next_lun_address);
    printf("* - Over Provisioning : %d\n", nand._f.over_provisioning);
    printf("* - Bits Per Cell : %d\n", nand._f.bits_per_cell);
    printf("* - Number Of Bits Ecc Correctability : %d\n", nand._f.number_of_bits_ecc_correctability);
    printf("* - Maindatabytes Per Eccunit : %d\n", nand._f.maindatabytes_per_eccunit);
    printf("* - Eccbits Per Maindata : %d\n", nand._f.eccbits_per_maindata);
    printf("* - Eccbits Per Blockinformation : %d\n", nand._f.eccbits_per_blockinformation);
    printf("* - Block Endurance : %d\n", nand._f.block_endurance);
    printf("* - Factorybadblocks Per Nand : %d\n", nand._f.factorybadblocks_per_nand);

    printf("*\n");
    printf("* - Multiplane Read %d\n", nand._f.support_type.multiplane_read);
    printf("* - Multiplane Write %d\n", nand._f.support_type.multiplane_write);
    printf("* - Cache Read %d\n", nand._f.support_type.cache_read);
    printf("* - Cache Write %d\n", nand._f.support_type.cache_write);
    printf("* - Interleave %d\n", nand._f.support_type.interleave);
    printf("* - Paired Page Mapping %d\n", nand._f.support_type.paired_page_mapping);
    printf("* - Block Indicator %d\n", nand._f.support_type.block_indicator);
    printf("* - Paired Plane %d\n", nand._f.support_type.paired_plane);
    printf("* - Multiplane Erase %d\n", nand._f.support_type.multiplane_erase);
    printf("* - Read Retry %d\n", nand._f.support_type.read_retry);
    printf("*\n");

    printf("*******************************************************************************\n");
    printf("\n");

    return 1;
}

/*******************************************************************************
 * mio_read()
 *******************************************************************************/
ulong mio_read(ulong blknr, lbaint_t blkcnt, void *pvBuffer)
{
    S32 siStartExtIndex = -1;
    U32 uiAddress = blknr;
    U8 *pucBuffer = (U8 *)pvBuffer;

    struct 
    {
        U32 uiSectorsLeft;
        U32 uiPartialSectors;
        U32 uiPartialAddr;
    } stFtl;

    struct
    {
        U32 uiSectorsLeft;
        U32 uiPartialSectors;
        U32 uiReadSectors;
    } stData;

    if (!is_mio_init)
    {
        DBG_MEDIA("mio_read(): mio is not initialized!!\n");
        return 0;
    }

    stData.uiSectorsLeft = blkcnt;
    stFtl.uiSectorsLeft = blkcnt;
    stFtl.uiPartialAddr = uiAddress;

  //DBG_MEDIA("mio_read(): 0x%0X, 0x%0X\n", (U32)blknr, (U32)blkcnt);

    while (stFtl.uiSectorsLeft || stData.uiSectorsLeft)
    {
        Exchange.ftl.fnMain();

        // put command to FTL 
        while(stFtl.uiSectorsLeft)
        {
            if (stFtl.uiSectorsLeft > IO_CMD_MAX_READ_SECTORS)
                stFtl.uiPartialSectors = IO_CMD_MAX_READ_SECTORS;
            else
                stFtl.uiPartialSectors = stFtl.uiSectorsLeft;

            siStartExtIndex = mio_cmd_to_ftl(IO_CMD_READ, 0, stFtl.uiPartialAddr, stFtl.uiPartialSectors);
            if (siStartExtIndex >= 0)
            {
              //DBG_MEDIA("mio_cmd_to_ftl(READ): 0x%0X, 0x%0X\n", stFtl.uiPartialAddr, stFtl.uiPartialSectors);
            
                if (uiAddress == stFtl.uiPartialAddr)
                {
                    // set external index
                    *Exchange.buffer.ReadBlkIdx = (U32)siStartExtIndex;
                }

                stFtl.uiPartialAddr += stFtl.uiPartialSectors;
                stFtl.uiSectorsLeft -= stFtl.uiPartialSectors;
            }
            else
            {
                break;
            }
        }

        // copy data from read buffer
        if (uiAddress != stFtl.uiPartialAddr)
        {
            stData.uiReadSectors = Exchange.buffer.fnGetRequestReadSeccnt();
            if (stData.uiReadSectors && stData.uiSectorsLeft)
            {
                if (stData.uiSectorsLeft > stData.uiReadSectors)
                    stData.uiPartialSectors = stData.uiReadSectors;
                else
                    stData.uiPartialSectors = stData.uiSectorsLeft;

                mio_fill_read_buffer(pucBuffer, stData.uiPartialSectors);
              //DBG_MEDIA("mio_fill_read_buffer: 0x%0X, 0x%0X\n", (U32)pucBuffer, stData.uiPartialSectors);

                pucBuffer += (stData.uiPartialSectors << 9);
                stData.uiSectorsLeft -= stData.uiPartialSectors;
                
                // increment external index
                *Exchange.buffer.ReadBlkIdx += stData.uiPartialSectors;
            }
        }
    }

    return (blkcnt - stData.uiSectorsLeft);
}

/*******************************************************************************
 * mio_write()
 *******************************************************************************/
ulong mio_write(ulong blknr, lbaint_t blkcnt, const void *pvBuffer)
{
    S32 siStartExtIndex = -1;
    U32 uiAddress = blknr;
    U8 *pucBuffer = (U8 *)pvBuffer;

    struct 
    {
        U32 uiSectorsLeft;
        U32 uiPartialSectors;
        U32 uiPartialAddr;
        U32 uiFeature;
    } stFtl;

    struct
    {
        U32 uiSectorsLeft;
        U32 uiPartialSectors;
        U32 uiAvaliableSectors;
    } stData;

    if (!is_mio_init)
    {
        DBG_MEDIA("mio_write(): mio is not initialized!!\n");
        return 0;
    }

  //DBG_MEDIA("mio_write(): 0x%lX, 0x%lX\n", blknr, blkcnt);

    stData.uiSectorsLeft = blkcnt;
    stFtl.uiSectorsLeft = blkcnt;
    stFtl.uiPartialAddr = uiAddress;

    while (stFtl.uiSectorsLeft || stData.uiSectorsLeft)
    {
        Exchange.ftl.fnMain();

        // put command to FTL
        while(stFtl.uiSectorsLeft)
        {
            if (stFtl.uiSectorsLeft > IO_CMD_MAX_WRITE_SECTORS)
            {
                stFtl.uiPartialSectors = IO_CMD_MAX_WRITE_SECTORS;
                stFtl.uiFeature = IO_CMD_FEATURE_WRITE_CONTINUE;
            }            
            else
            {
                stFtl.uiPartialSectors = stFtl.uiSectorsLeft;
                stFtl.uiFeature = 0;
            }

            siStartExtIndex = mio_cmd_to_ftl(IO_CMD_WRITE, stFtl.uiFeature, stFtl.uiPartialAddr, stFtl.uiPartialSectors);
            if (siStartExtIndex >= 0)
            {
              //DBG_MEDIA("mio_cmd_to_ftl(WRITE): 0x%0X, 0x%0X\n", stFtl.uiPartialAddr, stFtl.uiPartialSectors);
            
                if (uiAddress == stFtl.uiPartialAddr)
                {
                    // set external index
                    *Exchange.buffer.WriteBlkIdx = (U32)siStartExtIndex;
                }

                stFtl.uiPartialAddr += stFtl.uiPartialSectors;
                stFtl.uiSectorsLeft -= stFtl.uiPartialSectors;
            }
            else
            {
                break;
            }
        }

        // copy data to write cache
        if ((uiAddress != stFtl.uiPartialAddr) && stData.uiSectorsLeft)
        {
            stData.uiAvaliableSectors = (*Exchange.buffer.WriteBlkIdx - *Exchange.buffer.WriteNfcIdx) & (*Exchange.buffer.SectorsOfWriteCache - 1);
            stData.uiAvaliableSectors += 1;
            stData.uiAvaliableSectors = *Exchange.buffer.SectorsOfWriteCache - stData.uiAvaliableSectors;

          //DBG_MEDIA("Index: EXT(0x%0X), NAND(0x%0X), Aval:0x%0X\n", *Exchange.buffer.WriteBlkIdx, *Exchange.buffer.WriteNfcIdx, stData.uiAvaliableSectors);
            if (stData.uiAvaliableSectors)
            {
                if (stData.uiSectorsLeft > stData.uiAvaliableSectors)
                    stData.uiPartialSectors = stData.uiAvaliableSectors;
                else
                    stData.uiPartialSectors = stData.uiSectorsLeft;

                mio_fill_write_cache(pucBuffer, stData.uiPartialSectors);
              //DBG_MEDIA("mio_fill_write_cache: 0x%0X, 0x%0X\n", (U32)pucBuffer, stData.uiPartialSectors);

                pucBuffer += (stData.uiPartialSectors << 9);
                stData.uiSectorsLeft -= stData.uiPartialSectors;

                // increment external index
                *Exchange.buffer.WriteBlkIdx += stData.uiPartialSectors;
            }
        }
    }
    
    mio_cmd_to_ftl(IO_CMD_FLUSH, 0, 0, 0);

    return (stFtl.uiPartialAddr - uiAddress);
}

/*******************************************************************************
 * flush, standby, powerdown
 *******************************************************************************/
int mio_flush(void)
{
    if (mio_cmd_to_ftl(IO_CMD_FLUSH, 0, 0, 0) < 0)
    {
        return -1;
    }
    return 0;
}

int mio_standby(void)
{
    if (mio_cmd_to_ftl(IO_CMD_STANDBY, 0, 0, 0) < 0)
    {
        return -1;
    }
    return 0;
}

int mio_powerdown(void)
{
    if (mio_cmd_to_ftl(IO_CMD_POWER_DOWN, 0, 0, 0) < 0)
    {
        return -1;
    }
    return 0;
}

/*******************************************************************************
 * low level api.
 *******************************************************************************/
int mio_nand_write(loff_t ofs, size_t *len, u_char *buf)
{
    int ret = 0;
    NAND nand;
    MIO_NAND_INFO info;

    if (!is_mio_init)
    {
        DBG_MEDIA("mio_nand_write(): mio is not initialized!!\n");
        return 0;
    }

    Exchange.ftl.fnGetNandInfo(&nand);

	info.channel = 0;
	info.phyway = 0;
	info.pages_per_block = nand._f.pages_per_block;
	info.bytes_per_page = nand._f.databytes_per_page;
	info.blocks_per_lun = nand._f.mainblocks_per_lun;

	info.ecc_bits = nand._f.number_of_bits_ecc_correctability;
	info.bytes_per_ecc = nand._f.maindatabytes_per_eccunit;
	info.bytes_per_parity = (14 * info.ecc_bits + 7) / 8;
	info.bytes_per_parity = (info.bytes_per_parity + (4-1)) & ~(4-1); // 4Byte Align

	info.readretry_type = nand._f.support_type.read_retry;

    ret = NFC_PHY_LOWAPI_nand_write(&info, ofs, len, buf);

    return ret;
}


#if 0
int mio_nand_read(loff_t ofs, size_t *len, u_char *buf)
{
    int ret = 0;
    NAND nand;
    MIO_NAND_INFO info;
    unsigned int channels = 1, ways = 1; 

    // H27UCG8T2ATR
	info.channel = 0;
	info.phyway = 0;
	info.pages_per_block = 256;
	info.bytes_per_page = 8192;
	info.blocks_per_lun = 4096;

	info.ecc_bits = 40;
	info.bytes_per_ecc = 1024;
	info.bytes_per_parity = (14 * info.ecc_bits + 7) / 8;
	info.bytes_per_parity = (info.bytes_per_parity + (4-1)) & ~(4-1); // 4Byte Align

	info.readretry_type = 10;

	NFC_PHY_Init();
	NFC_PHY_EccInfoInit(1, 1, 0);
	// based on ONFI timing mode 0
    NFC_PHY_SetFeatures(channels, // _max_channel,
                        ways, // _max_way,
                        NAND_INTERFACE_ONFI_ASYNC, // _interface_type,
                        0, // _onfi_timing_mode,
                        10*1000*1000, // _tClk,  // Hz
                        200, // _tWB,
                        400, // _tCCS,
                        400, // _tADL,
                        200, // _tRHW,
                        120, // _tWHR,
                        100, // _tWW,
                        70,  // _tCS,
                        20,  // _tCH,
                        50,  // _tCLS,
                        50,  // _tALS,  // == _tCLS
                        20,  // _tCLH,
                        20,  // _tALH,  // == _tCLH
                        50,  // _tWP,
                        30,  // _tWH,
                        100, // _tWC,
                        40,  // _tDS,
                        20,  // _tDH,
                        100, // _tCEA,
                        40,  // _tREA,
                        50,  // _tRP,
                        30,  // _tREH,
                        100, // _tRC,
                        0);  // _tCOH)

    ret = 0;
    switch(info.readretry_type)
    {
    	case NAND_PHY_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE:
    	case NAND_PHY_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE:
    	case NAND_PHY_READRETRY_TYPE_HYNIX_1xNM_MLC:
    	{
    		ret = NFC_PHY_HYNIX_READRETRY_Init(channels, ways, 0, info.readretry_type);
    		if (ret >= 0)
    		{
    			ret = NFC_PHY_HYNIX_READRETRY_MakeRegAll();
    		}
    	} break;
    }
    if (ret < 0)
    {
    	printf("HynixReadRetry : Error!\n");
    }

    NFC_PHY_RAND_Init(info.bytes_per_ecc);
    NFC_PHY_RAND_Enable(1);


    ret = NFC_PHY_LOWAPI_nand_read(&info, ofs, len, buf);


    NFC_PHY_RAND_DeInit();
    
    switch(info.readretry_type)
    {
    	case NAND_PHY_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE:
    	case NAND_PHY_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE:
    	case NAND_PHY_READRETRY_TYPE_HYNIX_1xNM_MLC:
        {
            NFC_PHY_HYNIX_READRETRY_DeInit();
        } break;
    }

	NFC_PHY_DeInit();
	NFC_PHY_EccInfoDeInit();

    return ret;
}
#else
int mio_nand_read(loff_t ofs, size_t *len, u_char *buf)
{
    int ret = 0;
    NAND nand;
    MIO_NAND_INFO info;

    if (!is_mio_init)
    {
        DBG_MEDIA("mio_nand_read(): mio is not initialized!!\n");
        return 0;
    }

    Exchange.ftl.fnGetNandInfo(&nand);

	info.channel = 0;
	info.phyway = 0;
	info.pages_per_block = nand._f.pages_per_block;
	info.bytes_per_page = nand._f.databytes_per_page;
	info.blocks_per_lun = nand._f.mainblocks_per_lun;

	info.ecc_bits = nand._f.number_of_bits_ecc_correctability;
	info.bytes_per_ecc = nand._f.maindatabytes_per_eccunit;
	info.bytes_per_parity = (14 * info.ecc_bits + 7) / 8;
	info.bytes_per_parity = (info.bytes_per_parity + (4-1)) & ~(4-1); // 4Byte Align

	info.readretry_type = nand._f.support_type.read_retry;

    ret = NFC_PHY_LOWAPI_nand_read(&info, ofs, len, buf);

    return ret;
}
#endif

int mio_nand_erase(loff_t ofs, size_t size)
{
    int ret = 0;
    NAND nand;
    MIO_NAND_INFO info;

    if (!is_mio_init)
    {
        DBG_MEDIA("mio_nand_erase(): mio is not initialized!!\n");
        return 0;
    }

    Exchange.ftl.fnGetNandInfo(&nand);

	info.channel = 0;
	info.phyway = 0;
	info.pages_per_block = nand._f.pages_per_block;
	info.bytes_per_page = nand._f.databytes_per_page;
	info.blocks_per_lun = nand._f.mainblocks_per_lun;

    ret = NFC_PHY_LOWAPI_nand_erase(&info, ofs, size);

    return ret;
}

int mio_nand_raw_write(loff_t ofs, size_t *len, u_char *buf)
{
    int ret = 0;
    NAND nand;
    MIO_NAND_INFO info;

    if (!is_mio_init)
    {
        DBG_MEDIA("mio_nand_raw_write(): mio is not initialized!!\n");
        return 0;
    }

    Exchange.ftl.fnGetNandInfo(&nand);

	info.channel = 0;
	info.phyway = 0;
	info.pages_per_block = nand._f.pages_per_block;
	info.bytes_per_page = nand._f.databytes_per_page;
	info.blocks_per_lun = nand._f.mainblocks_per_lun;

    ret = NFC_PHY_LOWAPI_nand_raw_write(&info, ofs, len, buf);

    return ret;
}

int mio_nand_raw_read(loff_t ofs, size_t *len, u_char *buf)
{
    int ret = 0;
    MIO_NAND_INFO info;

#if 1
	info.channel = 0;
	info.phyway = 0;
	info.pages_per_block = 256;
	info.bytes_per_page = 8192;
	info.blocks_per_lun = 4096;
#else
    NAND nand;

    if (!is_mio_init)
    {
        DBG_MEDIA("mio_nand_raw_read(): mio is not initialized!!\n");
        return 0;
    }

    Exchange.ftl.fnGetNandInfo(&nand);

	info.channel = 0;
	info.phyway = 0;
	info.pages_per_block = nand._f.pages_per_block;
	info.bytes_per_page = nand._f.databytes_per_page;
	info.blocks_per_lun = nand._f.mainblocks_per_lun;
#endif

    ret = NFC_PHY_LOWAPI_nand_raw_read(&info, ofs, len, buf);

    return ret;
}

/*******************************************************************************
 * local functions
 *******************************************************************************/
static S32 mio_cmd_to_ftl(U16 usCommand, U8 ucFeature, U32 uiAddress, U32 uiLength)
{
    S32 siResp = -1;
    S32 siExtIndex = -1;
    U8 ucIsNeedRetry = 0;
    U8 ucIsWaitForDone = 0;

	switch (usCommand)
	{
        case IO_CMD_READ_DIRECT:
        case IO_CMD_WRITE_DIRECT:
        case IO_CMD_DATA_SET_MANAGEMENT:
        case IO_CMD_FLUSH:
        case IO_CMD_STANDBY:
		case IO_CMD_SWITCH_PARTITION:
        case IO_CMD_POWER_DOWN:
		{
			ucIsNeedRetry = 1;
			ucIsWaitForDone = 1;
		} break;
    }

    do
    {
        siResp = Exchange.ftl.fnPrePutCommand(usCommand, ucFeature, uiAddress, uiLength);
        if (siResp >= 0)
        {   siExtIndex = siResp;
            siResp = Exchange.ftl.fnPutCommand(usCommand, ucFeature, uiAddress, uiLength);
            if (siResp >= 0)
            {
                ucIsNeedRetry = 0;

                if (usCommand == IO_CMD_READ)
                {
                    Exchange.statistics.ios.cur.read += (uiLength << 9);
                    Exchange.statistics.ios.cur.read_seccnt += uiLength;
                    Exchange.statistics.ios.accumulate.read += (uiLength << 9);
                    Exchange.statistics.ios.accumulate.read_seccnt += uiLength;
                }
                else if (usCommand == IO_CMD_WRITE)
                {
                    Exchange.statistics.ios.cur.write += (uiLength << 9);
                    Exchange.statistics.ios.cur.write_seccnt += uiLength;
                    Exchange.statistics.ios.accumulate.write += (uiLength << 9);
                    Exchange.statistics.ios.accumulate.write_seccnt += uiLength;
                }
            }
        }

        if (ucIsNeedRetry)
        {
            Exchange.ftl.fnMain();
        }
    } while (ucIsNeedRetry);

    // wait for done
    if ((siResp >= 0) && ucIsWaitForDone)
    {
        unsigned int (*fnIsDone)(void);

        switch (usCommand)
        {
            case IO_CMD_READ_DIRECT:         { fnIsDone = Exchange.ftl.fnIsIdle; }          break;
            case IO_CMD_WRITE_DIRECT:        { fnIsDone = Exchange.ftl.fnIsIdle; }          break;
            case IO_CMD_DATA_SET_MANAGEMENT: { fnIsDone = Exchange.ftl.fnIsTrimDone; }      break;
            case IO_CMD_FLUSH:               { fnIsDone = Exchange.ftl.fnIsFlushDone; }     break;
            case IO_CMD_STANDBY:             { fnIsDone = Exchange.ftl.fnIsStandbyDone; }   break;
    		case IO_CMD_SWITCH_PARTITION:    { fnIsDone = Exchange.ftl.fnIsIdle; }          break;
            case IO_CMD_POWER_DOWN:          { fnIsDone = Exchange.ftl.fnIsPowerDonwDone; } break;
            default:                         { fnIsDone = Exchange.ftl.fnIsIdle; }          break; 
        }

        do 
        {
            Exchange.ftl.fnMain();
            
        } while(fnIsDone());
    }

    return siExtIndex;
}

static void mio_fill_read_buffer(void *pvBuff, U32 uiSectors)
{
    U8 *pucSrcBuff = 0;
    U8 *pucDestBuff = (U8 *)pvBuff;
    U32 uiCpyBytes = 0;
    U32 uiCurrExtIndex = *Exchange.buffer.ReadBlkIdx & (*Exchange.buffer.SectorsOfReadBuffer - 1);

    if ((uiCurrExtIndex + uiSectors) > *Exchange.buffer.SectorsOfReadBuffer)
    { // is rollover
        pucSrcBuff = (U8 *)(*Exchange.buffer.BaseOfReadBuffer + (uiCurrExtIndex << 9));
        uiCpyBytes = (*Exchange.buffer.SectorsOfReadBuffer - uiCurrExtIndex) << 9;
        memcpy((void *)pucDestBuff, (const void *)pucSrcBuff, uiCpyBytes);
#if defined (__SUPPORT_DEBUG_MIO_UBOOT_ERROR_STOP__)
        if (!uiCpyBytes) { DBG_MEDIA("mio_fill_read_buffer: memcpy error (copybytes 0)\n"); while(1); }
#endif

        pucDestBuff += uiCpyBytes;
        pucSrcBuff = (U8 *)(*Exchange.buffer.BaseOfReadBuffer);
        uiCpyBytes = (uiSectors << 9) - uiCpyBytes;
        memcpy((void *)pucDestBuff, (const void *)pucSrcBuff, uiCpyBytes);
#if defined (__SUPPORT_DEBUG_MIO_UBOOT_ERROR_STOP__)
        if (!uiCpyBytes) { DBG_MEDIA("mio_fill_read_buffer: memcpy error (copybytes 0)\n"); while(1); }
#endif
    }
    else
    {
        pucSrcBuff = (U8*)(*Exchange.buffer.BaseOfReadBuffer + (uiCurrExtIndex << 9));
        uiCpyBytes = uiSectors << 9;
        memcpy((void *)pucDestBuff, (const void *)pucSrcBuff, uiCpyBytes);
#if defined (__SUPPORT_DEBUG_MIO_UBOOT_ERROR_STOP__)
        if (!uiCpyBytes) { DBG_MEDIA("mio_fill_read_buffer: memcpy error (copybytes 0)\n"); while(1); }
#endif
    }
}

static void mio_fill_write_cache(const void *pvBuff, U32 uiSectors)
{
    U8 *pucSrcBuff = (U8 *)pvBuff;
    U8 *pucDestBuff = 0;
    U32 uiCpyBytes = 0;
    U32 uiCurrExtIndex = *Exchange.buffer.WriteBlkIdx & (*Exchange.buffer.SectorsOfWriteCache - 1);

    if ((uiCurrExtIndex + uiSectors) > *Exchange.buffer.SectorsOfWriteCache)
    { // is rollover
        pucDestBuff = (U8 *)(*Exchange.buffer.BaseOfWriteCache + (uiCurrExtIndex << 9));
        uiCpyBytes = (*Exchange.buffer.SectorsOfWriteCache - uiCurrExtIndex) << 9;
        memcpy((void *)pucDestBuff, (const void *)pucSrcBuff, uiCpyBytes);
#if defined (__SUPPORT_DEBUG_MIO_UBOOT_ERROR_STOP__)
        if (!uiCpyBytes) { DBG_MEDIA("mio_fill_write_cache: memcpy error (copybytes 0)\n"); while(1); }
#endif

        pucDestBuff = (U8 *)(*Exchange.buffer.BaseOfWriteCache);
        pucSrcBuff += uiCpyBytes;
        uiCpyBytes = (uiSectors << 9) - uiCpyBytes;
        memcpy((void *)pucDestBuff, (const void *)pucSrcBuff, uiCpyBytes);
#if defined (__SUPPORT_DEBUG_MIO_UBOOT_ERROR_STOP__)
        if (!uiCpyBytes) { DBG_MEDIA("mio_fill_write_cache: memcpy error (copybytes 0)\n"); while(1); }
#endif
    }
    else
    {
        pucDestBuff = (U8 *)(*Exchange.buffer.BaseOfWriteCache + (uiCurrExtIndex << 9));
        uiCpyBytes = uiSectors << 9;
        memcpy((void *)pucDestBuff, (const void *)pucSrcBuff, uiCpyBytes);
#if defined (__SUPPORT_DEBUG_MIO_UBOOT_ERROR_STOP__)
        if (!uiCpyBytes) { DBG_MEDIA("mio_fill_write_cache: memcpy error (copybytes 0)\n"); while(1); }
#endif
    }
}
        
/*******************************************************************************
 *
 *******************************************************************************/
int mio_rwtest(ulong ulTestSectors, ulong ulCapacity, unsigned char ucWriteRatio, unsigned char ucSequentRatio)
{
    if (!is_mio_init)
    {
        DBG_MEDIA("mio_nand_write(): mio is not initialized!!\n");
        return -1;
    }

    if (ulCapacity < 4*1024)
    {
        DBG_MEDIA("mio_rwtest(): wrong capacity: %lu\n", ulCapacity);
        return -1;
    }

    //ulTestSectors = 1000*1024*1024 / 512;
    //ulCapacity = 100*1024*1024 / 512;
    //ucWriteRatio = 50;
    //ucSequentRatio = 50;

    if (ulCapacity > *Exchange.ftl.Capacity)
        ulCapacity = *Exchange.ftl.Capacity;

    if (ucWriteRatio > 100)
        ucWriteRatio = 100;
    if (ucSequentRatio > 100)
        ucSequentRatio = 100;

    DBG_MEDIA("mio_rwtest(): %lu sectors(%lu MB), Capacity: %lu sectors(%lu MB), writeRatio:%d%%, sequentRatio:%d%%\n",
        ulTestSectors, ulTestSectors/(2*1024), ulCapacity, ulCapacity/(2*1024), ucWriteRatio, ucSequentRatio);
    mio_rwtest_run(ulTestSectors, ulCapacity, ucWriteRatio, ucSequentRatio);

    return 0;
}
