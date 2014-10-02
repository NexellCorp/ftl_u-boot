/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : nfc.phy.scan.c
 * Date         : 2014.09.18
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.09.18, SD.LEE)
 *
 * Description  : NFC Physical For NXP4330
 *
 ******************************************************************************/
#define __NFC_PHY_SCAN_GLOBAL__
#include "nfc.phy.scan.h"
#include "nfc.phy.h"
#include "nfc.phy.register.h"
#include "nfc.phy.rand.h"

/******************************************************************************
 *
 ******************************************************************************/
#include "../../exchange.h"
#include "../../../mio.definition.h"

/******************************************************************************
 *
 ******************************************************************************/
#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
#include <linux/vmalloc.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/sizes.h>
#include <asm/mach-types.h>
#include <linux/bitops.h>

#include <linux/sched.h>
#include <asm/stacktrace.h>
#include <asm/traps.h>
#include <asm/unwind.h>

#include <mach/devices.h>
#include <mach/soc.h>

#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
#include <common.h>
#include <malloc.h>

#endif

/******************************************************************************
 * ONFI Board Members (2014.04.02)
 *
 *  - Intel Corporation
 *  - Micron Technology, Inc.
 *  - Phison Electronics Corp.
 *  - SanDisk Corporation
 *  - SK Hynix, Inc.
 *  - Sony Corporation
 *  - Spansion
 *
 ******************************************************************************/
unsigned int NFC_PHY_ConfigOnfi(unsigned char * _id, unsigned int _nand, void * _nand_config, void * _onfi_param, void * _onfi_ext_param)
{
    unsigned char * id = _id;
    unsigned int nand = _nand;
    NAND * nand_config = (NAND *)_nand_config;
    ONFI_PARAMETER * onfi_param = (ONFI_PARAMETER *)_onfi_param;
    ONFI_EXT_PARAMETER * onfi_ext_param = (ONFI_EXT_PARAMETER *)_onfi_ext_param;

    unsigned char paired_page_mapping = 0;
    unsigned char block_indicator = 0;
    unsigned char multiplane_erase_type = 0;
    unsigned char read_retry_type = 0;

    /**************************************************************************
     * ONFI Supporting Filter
     **************************************************************************/
#define NOT_SUPPORT_ONFI_CONFIG         (0)
#define SUPPORT_ONFI_CONFIG             (1)
#define SUPPORT_ONFI_CONFIG_NOT_YET     (2)

    unsigned int support_onfi_config = NOT_SUPPORT_ONFI_CONFIG;

    switch (__NROOT(nand&NAND_MASK_MAKER,NAND_FIELD_MAKER))
    {
        case NAND_MAKER_SEC:     { support_onfi_config = NOT_SUPPORT_ONFI_CONFIG; } break;
        case NAND_MAKER_TOSHIBA: { support_onfi_config = NOT_SUPPORT_ONFI_CONFIG; } break;
        case NAND_MAKER_MICRON:  { support_onfi_config = SUPPORT_ONFI_CONFIG; } break;
        case NAND_MAKER_SKHYNIX: { support_onfi_config = SUPPORT_ONFI_CONFIG_NOT_YET; } break;
        case NAND_MAKER_INTEL:   { support_onfi_config = SUPPORT_ONFI_CONFIG; } break;
    }

    if (SUPPORT_ONFI_CONFIG != support_onfi_config)
    {
        return 0;
    }

    /**************************************************************************
     * manufacturer
     **************************************************************************/
    memcpy((void *)nand_config->_f.manufacturer, (const void *)onfi_param->_f.device_manufacturer, sizeof(unsigned char) * 12);
    memcpy((void *)nand_config->_f.modelname, (const void *)onfi_param->_f.device_model, sizeof(unsigned char) * 20);
    memcpy((void *)nand_config->_f.id, (const void *)id, sizeof(unsigned char) * 8);

    switch (__NROOT(nand&NAND_MASK_MAKER,NAND_FIELD_MAKER))
    {
        case NAND_MAKER_MICRON:
        {
            switch (__NROOT(nand&NAND_MASK_GENERATION,NAND_FIELD_GENERATION))
            {
                case 0x01:
                {
                    memcpy((void *)nand_config->_f.generation, (const void *)"L74A", strlen("L74A"));

                    paired_page_mapping = 0;
                    block_indicator = 2;
                    multiplane_erase_type = 2;
                    read_retry_type = 0;

                } break;

                case 0x02:
                {
                    memcpy((void *)nand_config->_f.generation, (const void *)"L83A", strlen("L83A"));

                    paired_page_mapping = 3;
                    block_indicator = 2;
                    multiplane_erase_type = 2;
                    read_retry_type = NAND_READRETRY_TYPE_MICRON_L83A;

                } break;

                case 0x04:
                {
                    memcpy((void *)nand_config->_f.generation, (const void *)"L84A", strlen("L84A"));

                    paired_page_mapping = 3;
                    block_indicator = 2;
                    multiplane_erase_type = 2;
                    read_retry_type = NAND_READRETRY_TYPE_MICRON_L84A;

                } break;

                case 0x08:
                {
                    memcpy((void *)nand_config->_f.generation, (const void *)"L85A", strlen("L85A"));

                    paired_page_mapping = 3;
                    block_indicator = 2;
                    multiplane_erase_type = 2;
                    read_retry_type = 0;

                } break;
            }

        } break;

        case NAND_MAKER_SKHYNIX: {} break;

        case NAND_MAKER_INTEL:
        {
            switch (__NROOT(nand&NAND_MASK_GENERATION,NAND_FIELD_GENERATION))
            {
                case 0x01:
                {
                    memcpy((void *)nand_config->_f.generation, (const void *)"25NM MLC", strlen("25NM MLC"));

                    paired_page_mapping = 0;
                    block_indicator = 2;
                    multiplane_erase_type = 2;
                    read_retry_type = 0;

                } break;

                case 0x02:
                {
                    memcpy((void *)nand_config->_f.generation, (const void *)"25NM SLC", strlen("25NM SLC"));

                    paired_page_mapping = 0xFF;
                    block_indicator = 2;
                    multiplane_erase_type = 2;
                    read_retry_type = 0;

                } break;
            }

        } break;
    }

    /**************************************************************************
     * timing config, only support onfi async
     **************************************************************************/
    nand_config->_f.interfacetype = NAND_INTERFACE_ONFI_ASYNC;
    nand_config->_f.onfi_detected = 1;
    if (onfi_param->_f.sdr_timing_mode_support & (1<<5))
    {
        nand_config->_f.onfi_timing_mode   = 5;

        nand_config->_f.timing.async.tClk  = __MHZ(50);
        nand_config->_f.timing.async.tRWC  = 20;
        nand_config->_f.timing.async.tR    = onfi_param->_f.tR_maximum_page_read_time_us * 1000;
        nand_config->_f.timing.async.tWB   = 100;
        nand_config->_f.timing.async.tCCS  = onfi_param->_f.tCCS_minimum_change_column_setup_time_ns;
        nand_config->_f.timing.async.tADL  = 400;
        nand_config->_f.timing.async.tRHW  = 100;
        nand_config->_f.timing.async.tWHR  = 80;
        nand_config->_f.timing.async.tWW   = 100;
        nand_config->_f.timing.async.tRR   = 20;
        nand_config->_f.timing.async.tFEAT = 1000;

        nand_config->_f.timing.async.tCS   = 15;
        nand_config->_f.timing.async.tCH   = 5;
        nand_config->_f.timing.async.tCLS  = 10;
        nand_config->_f.timing.async.tALS  = 10;
        nand_config->_f.timing.async.tCLH  = 5;
        nand_config->_f.timing.async.tALH  = 5;
        nand_config->_f.timing.async.tWP   = 10;
        nand_config->_f.timing.async.tWH   = 7;
        nand_config->_f.timing.async.tWC   = 20;
        nand_config->_f.timing.async.tDS   = 7;
        nand_config->_f.timing.async.tDH   = 5;

        nand_config->_f.timing.async.tCEA  = 25;
        nand_config->_f.timing.async.tREA  = 16;
        nand_config->_f.timing.async.tRP   = 10;
        nand_config->_f.timing.async.tREH  = 7;
        nand_config->_f.timing.async.tRC   = 20;
        nand_config->_f.timing.async.tCOH  = 15;
    }
    else if (onfi_param->_f.sdr_timing_mode_support & (1<<4))
    {
        nand_config->_f.onfi_timing_mode   = 4;

        nand_config->_f.timing.async.tClk  = __MHZ(40);
        nand_config->_f.timing.async.tRWC  = 25;
        nand_config->_f.timing.async.tR    = onfi_param->_f.tR_maximum_page_read_time_us * 1000;
        nand_config->_f.timing.async.tWB   = 100;
        nand_config->_f.timing.async.tCCS  = onfi_param->_f.tCCS_minimum_change_column_setup_time_ns;
        nand_config->_f.timing.async.tADL  = 400;
        nand_config->_f.timing.async.tRHW  = 100;
        nand_config->_f.timing.async.tWHR  = 80;
        nand_config->_f.timing.async.tWW   = 100;
        nand_config->_f.timing.async.tRR   = 20;
        nand_config->_f.timing.async.tFEAT = 1000;

        nand_config->_f.timing.async.tCS   = 20;
        nand_config->_f.timing.async.tCH   = 5;
        nand_config->_f.timing.async.tCLS  = 10;
        nand_config->_f.timing.async.tALS  = 10;
        nand_config->_f.timing.async.tCLH  = 5;
        nand_config->_f.timing.async.tALH  = 5;
        nand_config->_f.timing.async.tWP   = 12;
        nand_config->_f.timing.async.tWH   = 10;
        nand_config->_f.timing.async.tWC   = 25;
        nand_config->_f.timing.async.tDS   = 10;
        nand_config->_f.timing.async.tDH   = 5;

        nand_config->_f.timing.async.tCEA  = 25;
        nand_config->_f.timing.async.tREA  = 20;
        nand_config->_f.timing.async.tRP   = 12;
        nand_config->_f.timing.async.tREH  = 10;
        nand_config->_f.timing.async.tRC   = 25;
        nand_config->_f.timing.async.tCOH  = 15;
    }
    else if (onfi_param->_f.sdr_timing_mode_support & (1<<3))
    {
        nand_config->_f.onfi_timing_mode   = 3;

        nand_config->_f.timing.async.tClk  = __MHZ(33);
        nand_config->_f.timing.async.tRWC  = 30;
        nand_config->_f.timing.async.tR    = onfi_param->_f.tR_maximum_page_read_time_us * 1000;
        nand_config->_f.timing.async.tWB   = 100;
        nand_config->_f.timing.async.tCCS  = onfi_param->_f.tCCS_minimum_change_column_setup_time_ns;
        nand_config->_f.timing.async.tADL  = 400;
        nand_config->_f.timing.async.tRHW  = 100;
        nand_config->_f.timing.async.tWHR  = 80;
        nand_config->_f.timing.async.tWW   = 100;
        nand_config->_f.timing.async.tRR   = 20;
        nand_config->_f.timing.async.tFEAT = 1000;

        nand_config->_f.timing.async.tCS   = 25;
        nand_config->_f.timing.async.tCH   = 5;
        nand_config->_f.timing.async.tCLS  = 10;
        nand_config->_f.timing.async.tALS  = 10;
        nand_config->_f.timing.async.tCLH  = 5;
        nand_config->_f.timing.async.tALH  = 5;
        nand_config->_f.timing.async.tWP   = 15;
        nand_config->_f.timing.async.tWH   = 10;
        nand_config->_f.timing.async.tWC   = 30;
        nand_config->_f.timing.async.tDS   = 10;
        nand_config->_f.timing.async.tDH   = 5;

        nand_config->_f.timing.async.tCEA  = 25;
        nand_config->_f.timing.async.tREA  = 20;
        nand_config->_f.timing.async.tRP   = 15;
        nand_config->_f.timing.async.tREH  = 10;
        nand_config->_f.timing.async.tRC   = 30;
        nand_config->_f.timing.async.tCOH  = 15;
    }
    else if (onfi_param->_f.sdr_timing_mode_support & (1<<2))
    {
        nand_config->_f.onfi_timing_mode   = 2;

        nand_config->_f.timing.async.tClk  = __MHZ(28);
        nand_config->_f.timing.async.tRWC  = 35;
        nand_config->_f.timing.async.tR    = onfi_param->_f.tR_maximum_page_read_time_us * 1000;
        nand_config->_f.timing.async.tWB   = 100;
        nand_config->_f.timing.async.tCCS  = onfi_param->_f.tCCS_minimum_change_column_setup_time_ns;
        nand_config->_f.timing.async.tADL  = 400;
        nand_config->_f.timing.async.tRHW  = 100;
        nand_config->_f.timing.async.tWHR  = 80;
        nand_config->_f.timing.async.tWW   = 100;
        nand_config->_f.timing.async.tRR   = 20;
        nand_config->_f.timing.async.tFEAT = 1000;

        nand_config->_f.timing.async.tCS   = 25;
        nand_config->_f.timing.async.tCH   = 10;
        nand_config->_f.timing.async.tCLS  = 15;
        nand_config->_f.timing.async.tALS  = 15;
        nand_config->_f.timing.async.tCLH  = 10;
        nand_config->_f.timing.async.tALH  = 10;
        nand_config->_f.timing.async.tWP   = 17;
        nand_config->_f.timing.async.tWH   = 15;
        nand_config->_f.timing.async.tWC   = 35;
        nand_config->_f.timing.async.tDS   = 15;
        nand_config->_f.timing.async.tDH   = 5;

        nand_config->_f.timing.async.tCEA  = 30;
        nand_config->_f.timing.async.tREA  = 25;
        nand_config->_f.timing.async.tRP   = 17;
        nand_config->_f.timing.async.tREH  = 15;
        nand_config->_f.timing.async.tRC   = 35;
        nand_config->_f.timing.async.tCOH  = 15;
    }
    else if (onfi_param->_f.sdr_timing_mode_support & (1<<1))
    {
        nand_config->_f.onfi_timing_mode   = 1;

        nand_config->_f.timing.async.tClk  = __MHZ(20);
        nand_config->_f.timing.async.tRWC  = 50;
        nand_config->_f.timing.async.tR    = onfi_param->_f.tR_maximum_page_read_time_us * 1000;
        nand_config->_f.timing.async.tWB   = 100;
        nand_config->_f.timing.async.tCCS  = onfi_param->_f.tCCS_minimum_change_column_setup_time_ns;
        nand_config->_f.timing.async.tADL  = 400;
        nand_config->_f.timing.async.tRHW  = 100;
        nand_config->_f.timing.async.tWHR  = 80;
        nand_config->_f.timing.async.tWW   = 100;
        nand_config->_f.timing.async.tRR   = 20;
        nand_config->_f.timing.async.tFEAT = 1000;

        nand_config->_f.timing.async.tCS   = 35;
        nand_config->_f.timing.async.tCH   = 10;
        nand_config->_f.timing.async.tCLS  = 25;
        nand_config->_f.timing.async.tALS  = 25;
        nand_config->_f.timing.async.tCLH  = 10;
        nand_config->_f.timing.async.tALH  = 10;
        nand_config->_f.timing.async.tWP   = 25;
        nand_config->_f.timing.async.tWH   = 15;
        nand_config->_f.timing.async.tWC   = 45;
        nand_config->_f.timing.async.tDS   = 20;
        nand_config->_f.timing.async.tDH   = 10;

        nand_config->_f.timing.async.tCEA  = 45;
        nand_config->_f.timing.async.tREA  = 30;
        nand_config->_f.timing.async.tRP   = 25;
        nand_config->_f.timing.async.tREH  = 15;
        nand_config->_f.timing.async.tRC   = 50;
        nand_config->_f.timing.async.tCOH  = 15;
    }
    else // Mandatory : if (onfi_param->_f.sdr_timing_mode_support & (1<<0))
    {
        nand_config->_f.onfi_timing_mode   = 0;

        nand_config->_f.timing.async.tClk  = __MHZ(10);
        nand_config->_f.timing.async.tRWC  = 100;
        nand_config->_f.timing.async.tR    = onfi_param->_f.tR_maximum_page_read_time_us * 1000;
        nand_config->_f.timing.async.tWB   = 200;
        nand_config->_f.timing.async.tCCS  = onfi_param->_f.tCCS_minimum_change_column_setup_time_ns;
        nand_config->_f.timing.async.tADL  = 400;
        nand_config->_f.timing.async.tRHW  = 200;
        nand_config->_f.timing.async.tWHR  = 120;
        nand_config->_f.timing.async.tWW   = 100;
        nand_config->_f.timing.async.tRR   = 40;
        nand_config->_f.timing.async.tFEAT = 1000;

        nand_config->_f.timing.async.tCS   = 70;
        nand_config->_f.timing.async.tCH   = 20;
        nand_config->_f.timing.async.tCLS  = 50;
        nand_config->_f.timing.async.tALS  = 50;
        nand_config->_f.timing.async.tCLH  = 20;
        nand_config->_f.timing.async.tALH  = 20;
        nand_config->_f.timing.async.tWP   = 50;
        nand_config->_f.timing.async.tWH   = 30;
        nand_config->_f.timing.async.tWC   = 100;
        nand_config->_f.timing.async.tDS   = 40;
        nand_config->_f.timing.async.tDH   = 20;

        nand_config->_f.timing.async.tCEA  = 100;
        nand_config->_f.timing.async.tREA  = 40;
        nand_config->_f.timing.async.tRP   = 50;
        nand_config->_f.timing.async.tREH  = 30;
        nand_config->_f.timing.async.tRC   = 100;
        nand_config->_f.timing.async.tCOH  = 0;
    }

    /**************************************************************************
     * cell config
     **************************************************************************/
    nand_config->_f.luns_per_ce            = onfi_param->_f.number_of_luns_per_chip_enable;
    nand_config->_f.databytes_per_page     = onfi_param->_f.number_of_data_bytes_per_page;
    nand_config->_f.sparebytes_per_page    = onfi_param->_f.number_of_spare_bytes_per_page;
    nand_config->_f.number_of_planes       = __POW(1,onfi_param->_f.number_of_plane_address_bits&0x0F);
    nand_config->_f.pages_per_block        = onfi_param->_f.number_of_pages_per_block;
    nand_config->_f.mainblocks_per_lun     = onfi_param->_f.number_of_blocks_per_lun;
    nand_config->_f.extendedblocks_per_lun = 0;
    nand_config->_f.next_lun_address       = 0;
    nand_config->_f.over_provisioning      = 9313; // 9313 : 93.13%, 8731 : 87.31%
    nand_config->_f.bits_per_cell          = onfi_param->_f.number_of_bits_per_cell;

    if (!onfi_param->_f.features_supported.extended_parameter_page)
    {
        unsigned int  i = 0;
        unsigned int block_endurance_multiple = 1;

        /**********************************************************************
         * Calc block information's ecc bits
         **********************************************************************/
        unsigned char valid_ecc_list[] = {4,6,8,12,24,40,60};
        unsigned int entire_page_size = onfi_param->_f.number_of_data_bytes_per_page + onfi_param->_f.number_of_spare_bytes_per_page;
        unsigned int ecc_codeword_size = 512;
        unsigned int ecc_units = onfi_param->_f.number_of_data_bytes_per_page / ecc_codeword_size;
        unsigned int ecc_parity_size = NFC_PHY_GetEccParitySize(onfi_param->_f.number_of_bits_ecc_correctability);
        unsigned int eccbits_per_blockinformation = 0;

        // FTL Data & Block Information
        entire_page_size -= ((ecc_codeword_size + ecc_parity_size) * ecc_units) + 8;

        for (i = 0; i < 7; i++)
        {
            if (entire_page_size < NFC_PHY_GetEccParitySize(valid_ecc_list[i]))
            {
                // Get Block Information's ECC bit
                eccbits_per_blockinformation = valid_ecc_list[i-1];
                break;
            }
        }

        /**********************************************************************
         *
         **********************************************************************/
        for (i = 0; i < onfi_param->_f.block_endurance[1]; i++)
        {
            block_endurance_multiple *= 10;
        }

        /**********************************************************************
         *
         **********************************************************************/
        nand_config->_f.number_of_bits_ecc_correctability = onfi_param->_f.number_of_bits_ecc_correctability;
        nand_config->_f.maindatabytes_per_eccunit         = ecc_codeword_size;
        nand_config->_f.eccbits_per_maindata              = onfi_param->_f.number_of_bits_ecc_correctability;
        nand_config->_f.eccbits_per_blockinformation      = eccbits_per_blockinformation;
        nand_config->_f.block_endurance                   = onfi_param->_f.block_endurance[0] * block_endurance_multiple;
        nand_config->_f.factorybadblocks_per_nand         = onfi_param->_f.bad_blocks_maximum_per_lun;
    }
    else
    {
        unsigned int  i = 0;
        unsigned int block_endurance_multiple = 1;

        /**********************************************************************
         * Calc block information's ecc bits
         **********************************************************************/
        unsigned char valid_ecc_list[] = {4,6,8,12,24,40,60};
        unsigned int entire_page_size = onfi_param->_f.number_of_data_bytes_per_page + onfi_param->_f.number_of_spare_bytes_per_page;
        unsigned int ecc_codeword_size = __POW(1,onfi_ext_param->_f.ecc_codeword_size);
        unsigned int ecc_units = onfi_param->_f.number_of_data_bytes_per_page / ecc_codeword_size;
        unsigned int ecc_parity_size = NFC_PHY_GetEccParitySize(onfi_ext_param->_f.number_of_bits_ecc_correctability);
        unsigned int eccbits_per_blockinformation = 0;

        // FTL Data & Block Information
        entire_page_size -= ((ecc_codeword_size + ecc_parity_size) * ecc_units) + 8;

        for (i = 0; i < 7; i++)
        {
            if (entire_page_size < NFC_PHY_GetEccParitySize(valid_ecc_list[i]))
            {
                // Get Block Information's ECC bit
                eccbits_per_blockinformation = valid_ecc_list[i-1];
                break;
            }
        }

        /**********************************************************************
         *
         **********************************************************************/
        for (i = 0; i < onfi_ext_param->_f.block_endurance[1]; i++)
        {
            block_endurance_multiple *= 10;
        }

        /**********************************************************************
         *
         **********************************************************************/
        nand_config->_f.number_of_bits_ecc_correctability = onfi_ext_param->_f.number_of_bits_ecc_correctability;
        nand_config->_f.maindatabytes_per_eccunit         = ecc_codeword_size;
        nand_config->_f.eccbits_per_maindata              = onfi_ext_param->_f.number_of_bits_ecc_correctability;
        nand_config->_f.eccbits_per_blockinformation      = eccbits_per_blockinformation;
        nand_config->_f.block_endurance                   = onfi_ext_param->_f.block_endurance[0] * block_endurance_multiple;
        nand_config->_f.factorybadblocks_per_nand         = onfi_ext_param->_f.bad_blocks_maximum_per_lun;
    }

    /**************************************************************************
     * operation config
     **************************************************************************/
    nand_config->_f.support_list.slc_mode            = 0;   // must be '0'
    nand_config->_f.support_list.multiplane_read     = 0;   // must be '0'
    nand_config->_f.support_list.multiplane_write    = onfi_param->_f.features_supported.multi_plane_program_and_erase_operations;
    nand_config->_f.support_list.cache_read          = 0;   // must be '0'
    nand_config->_f.support_list.cache_write         = onfi_param->_f.optional_commands_supported.page_cache_program_command;
    nand_config->_f.support_list.interleave          = 0;   // must be '0'

    nand_config->_f.support_type.multiplane_read     = 0;   // must be '0'
    nand_config->_f.support_type.multiplane_write    = onfi_param->_f.features_supported.multi_plane_program_and_erase_operations ? 2 : 0;
    nand_config->_f.support_type.cache_read          = 0;   // must be '0'
    nand_config->_f.support_type.cache_write         = onfi_param->_f.optional_commands_supported.page_cache_program_command ? 1 : 0;
    nand_config->_f.support_type.interleave          = 0;   // must be '0'
    nand_config->_f.support_type.paired_page_mapping = paired_page_mapping;

    nand_config->_f.support_type.block_indicator     = block_indicator;

    // 0: match block & page address
    // 1: match page address
    nand_config->_f.support_type.paired_plane        = onfi_param->_f.multi_plane_operation_attributes.no_block_address_restrictions;

    // 1: CMD1(0x60)-ADDR-CMD1(0x60)-ADDR-CMD2(0xD0)-BSY
    // 2: CMD1(0x60)-ADDR-CMD2(0xD1)-BSY-CMD1(0x60)-ADDR-CMD2(0xD0)-BSY
    nand_config->_f.support_type.multiplane_erase    = multiplane_erase_type;

    nand_config->_f.support_type.read_retry          = read_retry_type;  

    nand_config->_f.step_of_static_wear_leveling     = (nand_config->_f.block_endurance * 20) / 100; // Endurance's 20%
  //nand_config->_f.max_channel;
  //nand_config->_f.max_way;

    /**************************************************************************
     * operating drive strength config
     **************************************************************************/
  //nand_config->_f.ds_support_list._f.micron_a;
  //nand_config->_f.ds_support_list._f.intel_a;
  //nand_config->_f.ds_support_list._f.samsung_a;
  //nand_config->_f.ds_support_list._f.hynix_a;
  //nand_config->_f.ds_support_list._f.thosiba_a;

  //nand_config->_f.ds_support_type.micron_a;
  //nand_config->_f.ds_support_type.intel_a;
  //nand_config->_f.ds_support_type.samsung_a;
  //nand_config->_f.ds_support_type.hynix_a;
  //nand_config->_f.ds_support_type.thosiba_a;

    /**************************************************************************
     *
     **************************************************************************/
    return nand_config->_f.databytes_per_page;
}

/******************************************************************************
 *
 *****************************************************************************/
unsigned int NFC_PHY_ScanSec(unsigned char * _id, unsigned char * _onfi_id, unsigned int _scan_format)
{
    unsigned char * id = _id;

    __print("\n");
    __print("NAND Maker    : SAMSUNG\n");
    __print("NAND ID       : %02x %02x %02x %02x %02x %02x %02x %02x\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7]);
    __print("NAND Decoding : This NAND Does Not Support, Yet. Make Configuration Manually !!\n");
    __print("\n");

    return 0;
}

/******************************************************************************
 * To Get Toshiba NAND DataSheet, Your Company Must Be Large.
 * Otherwise, Toshiba Ignore Your Request !
 ******************************************************************************/
unsigned int NFC_PHY_ScanToshiba(unsigned char * _id, unsigned char * _onfi_id, unsigned int _scan_format)
{
    unsigned char * id = _id;

    __print("\n");
    __print("NAND Maker    : TOSHIBA\n");
    __print("NAND ID       : %02x %02x %02x %02x %02x %02x %02x %02x\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7]);
    __print("NAND Decoding : This NAND Does Not Support, Yet. Make Configuration Manually !!\n");
    __print("\n");

    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
void NFC_PHY_ScanSkhynixHelp(unsigned char * _id)
{
    unsigned char * id = _id;

    __print("\n");
    __print("NAND Maker    : HYNIX\n");
    __print("NAND ID       : %02x %02x %02x %02x %02x %02x %02x %02x\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7]);
    __print("NAND Decoding : This NAND Does Not Support, Yet. Make Configuration Manually !!\n");
    __print("\n");
}

unsigned int NFC_PHY_ScanSkhynix(unsigned char * _id, unsigned char * _onfi_id, unsigned int _scan_format)
{
    unsigned int nand = 0;
    unsigned int bytes_per_page = 0;
    unsigned char * id = _id;
    unsigned char * onfi_id = _onfi_id;
    unsigned int scan_format = _scan_format;

    if (('O' == onfi_id[0]) &&
        ('N' == onfi_id[1]) &&
        ('F' == onfi_id[2]) &&
        ('I' == onfi_id[3]))
    {
        __print("\n");
        __print("NAND Maker    : HYNIX\n");
        __print("NAND ID       : %02x %02x %02x %02x %02x %02x %02x %02x\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7]);
        __print("NAND Decoding : At This Time (2014.09.19), We Have No Information About Process & Die Of SKHynix ONFI NAND !\n");
        __print("                We Have No Plan To Support SK-Hynix ONFI NAND, Until SK-Hynix Let Know About These Information !\n");
        __print("\n");

        /**********************************************************************
         * ONFI Device, Read ONFI Parameter Page
         **********************************************************************/
     ///NFC_PHY_GetOnfiParameter(0, 0, 0x00, (unsigned char *)&phy_features.onfi_param, (unsigned char *)&phy_features.onfi_ext_param);
     ///bytes_per_page = NFC_PHY_ConfigOnfi(id, nand, (void *)&phy_features.nand_config._c, (void *)&phy_features.onfi_param._c, (void *)&phy_features.onfi_ext_param._c);
        bytes_per_page = 0;
    }

    if (!bytes_per_page)
    {
        NAND * nand_config = (NAND *)&phy_features.nand_config;

        /**********************************************************************
         * Check NAND ID
         **********************************************************************/
             if ((0xDE == id[1]) && (0x94 == id[2]) && (0xDA == id[3]) && (0x74 == id[4]) && (0xC4 == id[5])) { nand = NAND_HYNIX_H27UCG8T2ATR; }
        else if ((0xD7 == id[1]) && (0x94 == id[2]) && (0x91 == id[3]) && (0x60 == id[4]) && (0x44 == id[5])) { nand = NAND_HYNIX_H27UCG8T2CTR; }
        else if ((0xD7 == id[1]) && (0x14 == id[2]) && (0x9E == id[3]) && (0x34 == id[4]) && (0x4A == id[5])) { nand = NAND_HYNIX_H27UBG8T2DTR; }
        else if ((0xDE == id[1]) && (0x14 == id[2]) && (0xA7 == id[3]) && (0x42 == id[4]) && (0x4A == id[5])) { nand = NAND_HYNIX_H27UCG8T2ETR; }
        // Add More NAND Here !!
        else
        {
            NFC_PHY_ScanSkhynixHelp(id);
        }

        /**********************************************************************
         * Make Configuration Manually !
         **********************************************************************/
        switch (nand)
        {
            case NAND_HYNIX_H27UCG8T2ATR:
            {
                if (scan_format)
                {
                    /**********************************************************
                     * manufacturer
                     **********************************************************/
                    memcpy((void *)nand_config->_f.manufacturer, (const void *)"HYNIX", strlen("HYNIX"));
                    memcpy((void *)nand_config->_f.modelname, (const void *)"H27UCG8T2ATR", strlen("H27UCG8T2ATR"));
                    memcpy((void *)nand_config->_f.id, (const void *)id, sizeof(unsigned char) * 8);
                    memcpy((void *)nand_config->_f.generation, (const void *)"20NM A-DIE", strlen("20NM A-DIE"));

                    /**********************************************************
                     * timing config
                     **********************************************************/
                    nand_config->_f.interfacetype = NAND_INTERFACE_ASYNC;
                    nand_config->_f.onfi_detected = 0;
                    nand_config->_f.onfi_timing_mode = 0;
#if 0
                    nand_config->_f.timing.async.tClk  = __MHZ(62.5);
                    nand_config->_f.timing.async.tRWC  = 16;    // ns
                    nand_config->_f.timing.async.tR    = 90000; // ns
                    nand_config->_f.timing.async.tWB   = 100;   // ns
                    nand_config->_f.timing.async.tCCS  = 200;   // ns
                    nand_config->_f.timing.async.tADL  = 200;   // ns
                    nand_config->_f.timing.async.tRHW  = 100;   // ns
                    nand_config->_f.timing.async.tWHR  = 80;    // ns
                    nand_config->_f.timing.async.tWW   = 100;   // ns
                    nand_config->_f.timing.async.tRR   = 20;    // ns
                    nand_config->_f.timing.async.tFEAT = 1000;  // ns
                    
                    nand_config->_f.timing.async.tCS  = 20;     // ns
                    nand_config->_f.timing.async.tCH  = 5;      // ns
                    nand_config->_f.timing.async.tCLS = 6;      // ns
                    nand_config->_f.timing.async.tALS = 6;      // ns
                    nand_config->_f.timing.async.tCLH = 3;      // ns
                    nand_config->_f.timing.async.tALH = 3;      // ns
                    nand_config->_f.timing.async.tWP  = 8;      // ns
                    nand_config->_f.timing.async.tWH  = 6;      // ns
                    nand_config->_f.timing.async.tWC  = 16;     // ns
                    nand_config->_f.timing.async.tDS  = 6;      // ns
                    nand_config->_f.timing.async.tDH  = 2;      // ns
                    
                    nand_config->_f.timing.async.tCEA = 10+16;  // ns
                    nand_config->_f.timing.async.tREA = 16;     // ns
                    nand_config->_f.timing.async.tRP  = 8;      // ns
                    nand_config->_f.timing.async.tREH = 6;      // ns
                    nand_config->_f.timing.async.tRC  = 16;     // ns
                    nand_config->_f.timing.async.tCOH = 15;     // ns
#else
                    nand_config->_f.timing.async.tClk  = __MHZ(50); 
                    nand_config->_f.timing.async.tRWC  = 20; 
                    nand_config->_f.timing.async.tR    = 100000; 
                    nand_config->_f.timing.async.tWB   = 100; 
                    nand_config->_f.timing.async.tCCS  = 200;
                    nand_config->_f.timing.async.tADL  = 400; 
                    nand_config->_f.timing.async.tRHW  = 100; 
                    nand_config->_f.timing.async.tWHR  = 80;
                    nand_config->_f.timing.async.tWW   = 100;
                    nand_config->_f.timing.async.tRR   = 20;
                    nand_config->_f.timing.async.tFEAT = 1000;

                    nand_config->_f.timing.async.tCS   = 15;
                    nand_config->_f.timing.async.tCH   = 5;
                    nand_config->_f.timing.async.tCLS  = 10;
                    nand_config->_f.timing.async.tALS  = 10;
                    nand_config->_f.timing.async.tCLH  = 5;
                    nand_config->_f.timing.async.tALH  = 5;
                    nand_config->_f.timing.async.tWP   = 10;
                    nand_config->_f.timing.async.tWH   = 7;
                    nand_config->_f.timing.async.tWC   = 20;
                    nand_config->_f.timing.async.tDS   = 7;
                    nand_config->_f.timing.async.tDH   = 5;
                                                       
                    nand_config->_f.timing.async.tCEA  = 25;
                    nand_config->_f.timing.async.tREA  = 16;
                    nand_config->_f.timing.async.tRP   = 10;
                    nand_config->_f.timing.async.tREH  = 7;
                    nand_config->_f.timing.async.tRC   = 20;
                    nand_config->_f.timing.async.tCOH  = 15;
#endif
                    /**********************************************************
                     * cell config
                     **********************************************************/
                    nand_config->_f.luns_per_ce                       = 1;
                    nand_config->_f.databytes_per_page                = 8192;
                    nand_config->_f.sparebytes_per_page               = 640;
                    nand_config->_f.number_of_planes                  = 2;
                    nand_config->_f.pages_per_block                   = 256;
                    nand_config->_f.mainblocks_per_lun                = 4096;
                    nand_config->_f.extendedblocks_per_lun            = 84;
                    nand_config->_f.next_lun_address                  = 0;
                    nand_config->_f.over_provisioning                 = 9313; // 9313 : 93.13%, 8731 : 87.31%
                    nand_config->_f.bits_per_cell                     = 2;
                    nand_config->_f.number_of_bits_ecc_correctability = 40;
                    nand_config->_f.maindatabytes_per_eccunit         = 1024;
                    nand_config->_f.eccbits_per_maindata              = 40;
                    nand_config->_f.eccbits_per_blockinformation      = 24;
                    nand_config->_f.block_endurance                   = 1000; // Spec Does Not Described !!
                    nand_config->_f.factorybadblocks_per_nand         = 100;

                    /**********************************************************
                     * operation config
                     **********************************************************/
                    nand_config->_f.support_list.slc_mode            = 0;
                    nand_config->_f.support_list.multiplane_read     = 0;
                    nand_config->_f.support_list.multiplane_write    = 1;
                    nand_config->_f.support_list.cache_read          = 0;
                    nand_config->_f.support_list.cache_write         = 1;
                    nand_config->_f.support_list.interleave          = 0;

                    nand_config->_f.support_type.multiplane_read     = 0;
                    nand_config->_f.support_type.multiplane_write    = 1;
                    nand_config->_f.support_type.cache_read          = 0;
                    nand_config->_f.support_type.cache_write         = 1;
                    nand_config->_f.support_type.interleave          = 0;
                    nand_config->_f.support_type.paired_page_mapping = 0;
                    nand_config->_f.support_type.block_indicator     = 1;
                    nand_config->_f.support_type.paired_plane        = 0;   // 0: match block & page address, 1: match page address
                    nand_config->_f.support_type.multiplane_erase    = 1;   // 1: CMD1(0x60)-ADDR-CMD1(0x60)-ADDR-CMD2(0xD0)-BSY, 2: CMD1(0x60)-ADDR-CMD2(0xD1)-BSY-CMD1(0x60)-ADDR-CMD2(0xD0)-BSY
                    nand_config->_f.support_type.read_retry          = NAND_READRETRY_TYPE_HYNIX_20NM_MLC_A_DIE;

                    nand_config->_f.step_of_static_wear_leveling     = (nand_config->_f.block_endurance * 20) / 100; // Endurance's 20%
                  //nand_config->_f.max_channel;
                  //nand_config->_f.max_way;

                    if (Exchange.ftl.fnConfig) { Exchange.ftl.fnConfig((void *)&phy_features.nand_config); }
                }

                bytes_per_page = 8192;

            } break;

            case NAND_HYNIX_H27UCG8T2CTR:
            {
                if (scan_format)
                {
                    /**********************************************************
                     * manufacturer
                     **********************************************************/
                    memcpy((void *)nand_config->_f.manufacturer, (const void *)"HYNIX", strlen("HYNIX"));
                    memcpy((void *)nand_config->_f.modelname, (const void *)"H27UCG8T2CTR", strlen("H27UCG8T2CTR"));
                    memcpy((void *)nand_config->_f.id, (const void *)id, sizeof(unsigned char) * 8);
                    memcpy((void *)nand_config->_f.generation, (const void *)"20NM C-DIE", strlen("20NM C-DIE"));

                    /**********************************************************
                     * timing config
                     **********************************************************/
                    nand_config->_f.interfacetype = NAND_INTERFACE_ASYNC;
                    nand_config->_f.onfi_detected = 0;
                    nand_config->_f.onfi_timing_mode = 0;
#if 0
                    nand_config->_f.timing.async.tClk  = __MHZ(62.5);
                    nand_config->_f.timing.async.tRWC  = 16;    // ns
                    nand_config->_f.timing.async.tR    = 90000; // ns
                    nand_config->_f.timing.async.tWB   = 100;   // ns
                    nand_config->_f.timing.async.tCCS  = 200;   // ns
                    nand_config->_f.timing.async.tADL  = 200;   // ns
                    nand_config->_f.timing.async.tRHW  = 100;   // ns
                    nand_config->_f.timing.async.tWHR  = 80;    // ns
                    nand_config->_f.timing.async.tWW   = 100;   // ns
                    nand_config->_f.timing.async.tRR   = 20;    // ns
                    nand_config->_f.timing.async.tFEAT = 1000;  // ns
                    
                    nand_config->_f.timing.async.tCS  = 20;     // ns
                    nand_config->_f.timing.async.tCH  = 5;      // ns
                    nand_config->_f.timing.async.tCLS = 6;      // ns
                    nand_config->_f.timing.async.tALS = 6;      // ns
                    nand_config->_f.timing.async.tCLH = 2;      // ns
                    nand_config->_f.timing.async.tALH = 2;      // ns
                    nand_config->_f.timing.async.tWP  = 8;      // ns
                    nand_config->_f.timing.async.tWH  = 6;      // ns
                    nand_config->_f.timing.async.tWC  = 16;     // ns
                    nand_config->_f.timing.async.tDS  = 6;      // ns
                    nand_config->_f.timing.async.tDH  = 2;      // ns
                    
                    nand_config->_f.timing.async.tCEA = 10+16;  // ns
                    nand_config->_f.timing.async.tREA = 16;     // ns
                    nand_config->_f.timing.async.tRP  = 8;      // ns
                    nand_config->_f.timing.async.tREH = 6;      // ns
                    nand_config->_f.timing.async.tRC  = 16;     // ns
                    nand_config->_f.timing.async.tCOH = 15;     // ns
#else
                    nand_config->_f.timing.async.tClk  = __MHZ(50); 
                    nand_config->_f.timing.async.tRWC  = 20; 
                    nand_config->_f.timing.async.tR    = 100000; 
                    nand_config->_f.timing.async.tWB   = 100; 
                    nand_config->_f.timing.async.tCCS  = 200;
                    nand_config->_f.timing.async.tADL  = 400; 
                    nand_config->_f.timing.async.tRHW  = 100; 
                    nand_config->_f.timing.async.tWHR  = 80; 
                    nand_config->_f.timing.async.tWW   = 100;
                    nand_config->_f.timing.async.tRR   = 20;
                    nand_config->_f.timing.async.tFEAT = 1000;

                    nand_config->_f.timing.async.tCS   = 15;
                    nand_config->_f.timing.async.tCH   = 5;
                    nand_config->_f.timing.async.tCLS  = 10;
                    nand_config->_f.timing.async.tALS  = 10;
                    nand_config->_f.timing.async.tCLH  = 5;
                    nand_config->_f.timing.async.tALH  = 5;
                    nand_config->_f.timing.async.tWP   = 10;
                    nand_config->_f.timing.async.tWH   = 7;
                    nand_config->_f.timing.async.tWC   = 20;
                    nand_config->_f.timing.async.tDS   = 7;
                    nand_config->_f.timing.async.tDH   = 5;
                                                       
                    nand_config->_f.timing.async.tCEA  = 25;
                    nand_config->_f.timing.async.tREA  = 16;
                    nand_config->_f.timing.async.tRP   = 10;
                    nand_config->_f.timing.async.tREH  = 7;
                    nand_config->_f.timing.async.tRC   = 20;
                    nand_config->_f.timing.async.tCOH  = 15;
#endif
                    /**********************************************************
                     * cell config
                     **********************************************************/
                    nand_config->_f.luns_per_ce                       = 1;
                    nand_config->_f.databytes_per_page                = 8192;
                    nand_config->_f.sparebytes_per_page               = 640;
                    nand_config->_f.number_of_planes                  = 2;
                    nand_config->_f.pages_per_block                   = 256;
                    nand_config->_f.mainblocks_per_lun                = 2048;
                    nand_config->_f.extendedblocks_per_lun            = 44;
                    nand_config->_f.next_lun_address                  = 0;
                    nand_config->_f.over_provisioning                 = 9313; // 9313 : 93.13%, 8731 : 87.31%
                    nand_config->_f.bits_per_cell                     = 2;
                    nand_config->_f.number_of_bits_ecc_correctability = 40;
                    nand_config->_f.maindatabytes_per_eccunit         = 1024;
                    nand_config->_f.eccbits_per_maindata              = 40;
                    nand_config->_f.eccbits_per_blockinformation      = 24;
                    nand_config->_f.block_endurance                   = 1000; // Spec Does Not Described !!
                    nand_config->_f.factorybadblocks_per_nand         = 100;

                    /**********************************************************
                     * operation config
                     **********************************************************/
                    nand_config->_f.support_list.slc_mode            = 0;
                    nand_config->_f.support_list.multiplane_read     = 0;
                    nand_config->_f.support_list.multiplane_write    = 1;
                    nand_config->_f.support_list.cache_read          = 0;
                    nand_config->_f.support_list.cache_write         = 1;
                    nand_config->_f.support_list.interleave          = 0;

                    nand_config->_f.support_type.multiplane_read     = 0;
                    nand_config->_f.support_type.multiplane_write    = 1;
                    nand_config->_f.support_type.cache_read          = 0;
                    nand_config->_f.support_type.cache_write         = 1;
                    nand_config->_f.support_type.interleave          = 0;
                    nand_config->_f.support_type.paired_page_mapping = 0;
                    nand_config->_f.support_type.block_indicator     = 1;
                    nand_config->_f.support_type.paired_plane        = 0;   // 0: match block & page address, 1: match page address
                    nand_config->_f.support_type.multiplane_erase    = 1;   // 1: CMD1(0x60)-ADDR-CMD1(0x60)-ADDR-CMD2(0xD0)-BSY, 2: CMD1(0x60)-ADDR-CMD2(0xD1)-BSY-CMD1(0x60)-ADDR-CMD2(0xD0)-BSY
                    nand_config->_f.support_type.read_retry          = NAND_READRETRY_TYPE_HYNIX_20NM_MLC_BC_DIE;

                    nand_config->_f.step_of_static_wear_leveling     = (nand_config->_f.block_endurance * 20) / 100; // Endurance's 20%
                  //nand_config->_f.max_channel;
                  //nand_config->_f.max_way;

                    if (Exchange.ftl.fnConfig) { Exchange.ftl.fnConfig((void *)&phy_features.nand_config); }
                }

                bytes_per_page = 8192;

            } break;

            case NAND_HYNIX_H27UBG8T2DTR:
            {
                if (scan_format)
                {
                    /**********************************************************
                     * manufacturer
                     **********************************************************/
                    memcpy((void *)nand_config->_f.manufacturer, (const void *)"HYNIX", strlen("HYNIX"));
                    memcpy((void *)nand_config->_f.modelname, (const void *)"H27UBG8T2DTR", strlen("H27UBG8T2DTR"));
                    memcpy((void *)nand_config->_f.id, (const void *)id, sizeof(unsigned char) * 8);
                    memcpy((void *)nand_config->_f.generation, (const void *)"16NM ?-DIE", strlen("16NM ?-DIE"));

                    /**********************************************************
                     * timing config
                     **********************************************************/
                    nand_config->_f.interfacetype = NAND_INTERFACE_ASYNC;
                    nand_config->_f.onfi_detected = 0;
                    nand_config->_f.onfi_timing_mode = 0;
#if 0
                    nand_config->_f.timing.async.tClk  = __MHZ(62.5);
                    nand_config->_f.timing.async.tRWC  = 16;    // ns
                    nand_config->_f.timing.async.tR    = 90000; // ns
                    nand_config->_f.timing.async.tWB   = 100;   // ns
                    nand_config->_f.timing.async.tCCS  = 300;   // ns
                    nand_config->_f.timing.async.tADL  = 300;   // ns
                    nand_config->_f.timing.async.tRHW  = 100;   // ns
                    nand_config->_f.timing.async.tWHR  = 80;    // ns
                    nand_config->_f.timing.async.tWW   = 100;   // ns
                    nand_config->_f.timing.async.tRR   = 20;    // ns
                    nand_config->_f.timing.async.tFEAT = 1000;  // ns
                    
                    nand_config->_f.timing.async.tCS  = 20;     // ns
                    nand_config->_f.timing.async.tCH  = 5;      // ns
                    nand_config->_f.timing.async.tCLS = 6;      // ns
                    nand_config->_f.timing.async.tALS = 6;      // ns
                    nand_config->_f.timing.async.tCLH = 3;      // ns
                    nand_config->_f.timing.async.tALH = 3;      // ns
                    nand_config->_f.timing.async.tWP  = 8;      // ns
                    nand_config->_f.timing.async.tWH  = 6;      // ns
                    nand_config->_f.timing.async.tWC  = 16;     // ns
                    nand_config->_f.timing.async.tDS  = 6;      // ns
                    nand_config->_f.timing.async.tDH  = 2;      // ns
                    
                    nand_config->_f.timing.async.tCEA = 10+16;  // ns
                    nand_config->_f.timing.async.tREA = 16;     // ns
                    nand_config->_f.timing.async.tRP  = 8;      // ns
                    nand_config->_f.timing.async.tREH = 6;      // ns
                    nand_config->_f.timing.async.tRC  = 16;     // ns
                    nand_config->_f.timing.async.tCOH = 15;     // ns
#else
                    nand_config->_f.timing.async.tClk  = __MHZ(50); 
                    nand_config->_f.timing.async.tRWC  = 20; 
                    nand_config->_f.timing.async.tR    = 100000; 
                    nand_config->_f.timing.async.tWB   = 100; 
                    nand_config->_f.timing.async.tCCS  = 200;
                    nand_config->_f.timing.async.tADL  = 400; 
                    nand_config->_f.timing.async.tRHW  = 100; 
                    nand_config->_f.timing.async.tWHR  = 80; 
                    nand_config->_f.timing.async.tWW   = 100;
                    nand_config->_f.timing.async.tRR   = 20;
                    nand_config->_f.timing.async.tFEAT = 1000;

                    nand_config->_f.timing.async.tCS   = 15;
                    nand_config->_f.timing.async.tCH   = 5;
                    nand_config->_f.timing.async.tCLS  = 10;
                    nand_config->_f.timing.async.tALS  = 10;
                    nand_config->_f.timing.async.tCLH  = 5;
                    nand_config->_f.timing.async.tALH  = 5;
                    nand_config->_f.timing.async.tWP   = 10;
                    nand_config->_f.timing.async.tWH   = 7;
                    nand_config->_f.timing.async.tWC   = 20;
                    nand_config->_f.timing.async.tDS   = 7;
                    nand_config->_f.timing.async.tDH   = 5;
                                                       
                    nand_config->_f.timing.async.tCEA  = 25;
                    nand_config->_f.timing.async.tREA  = 16;
                    nand_config->_f.timing.async.tRP   = 10;
                    nand_config->_f.timing.async.tREH  = 7;
                    nand_config->_f.timing.async.tRC   = 20;
                    nand_config->_f.timing.async.tCOH  = 15;
#endif
                    /**********************************************************
                     * cell config
                     **********************************************************/
                    nand_config->_f.luns_per_ce                       = 1;
                    nand_config->_f.databytes_per_page                = 8192;
                    nand_config->_f.sparebytes_per_page               = 832;
                    nand_config->_f.number_of_planes                  = 2;
                    nand_config->_f.pages_per_block                   = 256;
                    nand_config->_f.mainblocks_per_lun                = 2048;
                    nand_config->_f.extendedblocks_per_lun            = 64;
                    nand_config->_f.next_lun_address                  = 0;
                    nand_config->_f.over_provisioning                 = 9313; // 9313 : 93.13%, 8731 : 87.31%
                    nand_config->_f.bits_per_cell                     = 2;
                    nand_config->_f.number_of_bits_ecc_correctability = 40;
                    nand_config->_f.maindatabytes_per_eccunit         = 1024;
                    nand_config->_f.eccbits_per_maindata              = 40;
                    nand_config->_f.eccbits_per_blockinformation      = 24;
                    nand_config->_f.block_endurance                   = 1000; // Spec Does Not Described !!
                    nand_config->_f.factorybadblocks_per_nand         = 115;

                    /**********************************************************
                     * operation config
                     **********************************************************/
                    nand_config->_f.support_list.slc_mode            = 0;
                    nand_config->_f.support_list.multiplane_read     = 0;
                    nand_config->_f.support_list.multiplane_write    = 1;
                    nand_config->_f.support_list.cache_read          = 0;
                    nand_config->_f.support_list.cache_write         = 1;
                    nand_config->_f.support_list.interleave          = 0;

                    nand_config->_f.support_type.multiplane_read     = 0;
                    nand_config->_f.support_type.multiplane_write    = 1;
                    nand_config->_f.support_type.cache_read          = 0;
                    nand_config->_f.support_type.cache_write         = 1;
                    nand_config->_f.support_type.interleave          = 0;
                    nand_config->_f.support_type.paired_page_mapping = 1;
                    nand_config->_f.support_type.block_indicator     = 3;
                    nand_config->_f.support_type.paired_plane        = 0;   // 0: match block & page address, 1: match page address
                    nand_config->_f.support_type.multiplane_erase    = 1;   // 1: CMD1(0x60)-ADDR-CMD1(0x60)-ADDR-CMD2(0xD0)-BSY, 2: CMD1(0x60)-ADDR-CMD2(0xD1)-BSY-CMD1(0x60)-ADDR-CMD2(0xD0)-BSY
                    nand_config->_f.support_type.read_retry          = NAND_READRETRY_TYPE_HYNIX_1xNM_MLC;

                    nand_config->_f.step_of_static_wear_leveling     = (nand_config->_f.block_endurance * 20) / 100; // Endurance's 20%
                  //nand_config->_f.max_channel;
                  //nand_config->_f.max_way;

                    if (Exchange.ftl.fnConfig) { Exchange.ftl.fnConfig((void *)&phy_features.nand_config); }
                }

                bytes_per_page = 8192;

            } break;

            case NAND_HYNIX_H27UCG8T2ETR:
            {
                if (scan_format)
                {
                    /**********************************************************
                     * manufacturer
                     **********************************************************/
                    memcpy((void *)nand_config->_f.manufacturer, (const void *)"HYNIX", strlen("HYNIX"));
                    memcpy((void *)nand_config->_f.modelname, (const void *)"H27UCG8T2ETR", strlen("H27UCG8T2ETR"));
                    memcpy((void *)nand_config->_f.id, (const void *)id, sizeof(unsigned char) * 8);
                    memcpy((void *)nand_config->_f.generation, (const void *)"16NM B-DIE", strlen("16NM B-DIE"));

                    /**********************************************************
                     * timing config
                     **********************************************************/
                    nand_config->_f.interfacetype = NAND_INTERFACE_ASYNC;
                    nand_config->_f.onfi_detected = 0;
                    nand_config->_f.onfi_timing_mode = 0;
#if 0
                    nand_config->_f.timing.async.tClk  = __MHZ(62.5);
                    nand_config->_f.timing.async.tRWC  = 16;    // ns
                    nand_config->_f.timing.async.tR    = 90000; // ns
                    nand_config->_f.timing.async.tWB   = 100;   // ns
                    nand_config->_f.timing.async.tCCS  = 300;   // ns
                    nand_config->_f.timing.async.tADL  = 300;   // ns
                    nand_config->_f.timing.async.tRHW  = 100;   // ns
                    nand_config->_f.timing.async.tWHR  = 80;    // ns
                    nand_config->_f.timing.async.tWW   = 100;   // ns
                    nand_config->_f.timing.async.tRR   = 20;    // ns
                    nand_config->_f.timing.async.tFEAT = 1000;  // ns
                    
                    nand_config->_f.timing.async.tCS  = 20;     // ns
                    nand_config->_f.timing.async.tCH  = 5;      // ns
                    nand_config->_f.timing.async.tCLS = 6;      // ns
                    nand_config->_f.timing.async.tALS = 6;      // ns
                    nand_config->_f.timing.async.tCLH = 3;      // ns
                    nand_config->_f.timing.async.tALH = 3;      // ns
                    nand_config->_f.timing.async.tWP  = 8;      // ns
                    nand_config->_f.timing.async.tWH  = 6;      // ns
                    nand_config->_f.timing.async.tWC  = 16;     // ns
                    nand_config->_f.timing.async.tDS  = 6;      // ns
                    nand_config->_f.timing.async.tDH  = 2;      // ns
                    
                    nand_config->_f.timing.async.tCEA = 10+16;  // ns
                    nand_config->_f.timing.async.tREA = 16;     // ns
                    nand_config->_f.timing.async.tRP  = 8;      // ns
                    nand_config->_f.timing.async.tREH = 6;      // ns
                    nand_config->_f.timing.async.tRC  = 16;     // ns
                    nand_config->_f.timing.async.tCOH = 15;     // ns
#else
                    nand_config->_f.timing.async.tClk  = __MHZ(50); 
                    nand_config->_f.timing.async.tRWC  = 20; 
                    nand_config->_f.timing.async.tR    = 100000; 
                    nand_config->_f.timing.async.tWB   = 100; 
                    nand_config->_f.timing.async.tCCS  = 200;
                    nand_config->_f.timing.async.tADL  = 400; 
                    nand_config->_f.timing.async.tRHW  = 100; 
                    nand_config->_f.timing.async.tWHR  = 80; 
                    nand_config->_f.timing.async.tWW   = 100;
                    nand_config->_f.timing.async.tRR   = 20;
                    nand_config->_f.timing.async.tFEAT = 1000;

                    nand_config->_f.timing.async.tCS   = 15;
                    nand_config->_f.timing.async.tCH   = 5;
                    nand_config->_f.timing.async.tCLS  = 10;
                    nand_config->_f.timing.async.tALS  = 10;
                    nand_config->_f.timing.async.tCLH  = 5;
                    nand_config->_f.timing.async.tALH  = 5;
                    nand_config->_f.timing.async.tWP   = 10;
                    nand_config->_f.timing.async.tWH   = 7;
                    nand_config->_f.timing.async.tWC   = 20;
                    nand_config->_f.timing.async.tDS   = 7;
                    nand_config->_f.timing.async.tDH   = 5;
                                                       
                    nand_config->_f.timing.async.tCEA  = 25;
                    nand_config->_f.timing.async.tREA  = 16;
                    nand_config->_f.timing.async.tRP   = 10;
                    nand_config->_f.timing.async.tREH  = 7;
                    nand_config->_f.timing.async.tRC   = 20;
                    nand_config->_f.timing.async.tCOH  = 15;
#endif
                    /**********************************************************
                     * cell config
                     **********************************************************/
                    nand_config->_f.luns_per_ce                       = 1;
                    nand_config->_f.databytes_per_page                = 16384;
                    nand_config->_f.sparebytes_per_page               = 1664;
                    nand_config->_f.number_of_planes                  = 2;
                    nand_config->_f.pages_per_block                   = 256;
                    nand_config->_f.mainblocks_per_lun                = 2048;
                    nand_config->_f.extendedblocks_per_lun            = 72;
                    nand_config->_f.next_lun_address                  = 0;
                    nand_config->_f.over_provisioning                 = 9313; // 9313 : 93.13%, 8731 : 87.31%
                    nand_config->_f.bits_per_cell                     = 2;
                    nand_config->_f.number_of_bits_ecc_correctability = 40;
                    nand_config->_f.maindatabytes_per_eccunit         = 1024;
                    nand_config->_f.eccbits_per_maindata              = 40;
                    nand_config->_f.eccbits_per_blockinformation      = 24;
                    nand_config->_f.block_endurance                   = 1000; // Spec Does Not Described !!
                    nand_config->_f.factorybadblocks_per_nand         = 115;

                    /**********************************************************
                     * operation config
                     **********************************************************/
                    nand_config->_f.support_list.slc_mode            = 0;
                    nand_config->_f.support_list.multiplane_read     = 0;
                    nand_config->_f.support_list.multiplane_write    = 1;
                    nand_config->_f.support_list.cache_read          = 0;
                    nand_config->_f.support_list.cache_write         = 1;
                    nand_config->_f.support_list.interleave          = 0;

                    nand_config->_f.support_type.multiplane_read     = 0;
                    nand_config->_f.support_type.multiplane_write    = 1;
                    nand_config->_f.support_type.cache_read          = 0;
                    nand_config->_f.support_type.cache_write         = 1;
                    nand_config->_f.support_type.interleave          = 0;
                    nand_config->_f.support_type.paired_page_mapping = 1;
                    nand_config->_f.support_type.block_indicator     = 3;
                    nand_config->_f.support_type.paired_plane        = 0;   // 0: match block & page address, 1: match page address
                    nand_config->_f.support_type.multiplane_erase    = 1;   // 1: CMD1(0x60)-ADDR-CMD1(0x60)-ADDR-CMD2(0xD0)-BSY, 2: CMD1(0x60)-ADDR-CMD2(0xD1)-BSY-CMD1(0x60)-ADDR-CMD2(0xD0)-BSY
                    nand_config->_f.support_type.read_retry          = NAND_READRETRY_TYPE_HYNIX_1xNM_MLC;

                    nand_config->_f.step_of_static_wear_leveling     = (nand_config->_f.block_endurance * 20) / 100; // Endurance's 20%
                  //nand_config->_f.max_channel;
                  //nand_config->_f.max_way;

                    if (Exchange.ftl.fnConfig) { Exchange.ftl.fnConfig((void *)&phy_features.nand_config); }
                }

                bytes_per_page = 16384;

            } break;
            
        }
    }

    return bytes_per_page;
}

/******************************************************************************
 *
 ******************************************************************************/
void NFC_PHY_ScanMicronHelp(unsigned char * _id)
{
    unsigned char * id = _id;

    __print("\n");
    __print("NAND Maker    : MICRON\n");
    __print("NAND ID       : %02x %02x %02x %02x %02x %02x %02x %02x\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7]);
    __print("NAND Decoding : This NAND Does Not Support, Yet. Make Configuration Manually !!\n");
    __print("\n");
}

unsigned int NFC_PHY_ScanMicron(unsigned char * _id, unsigned char * _onfi_id, unsigned int _scan_format)
{
    unsigned int nand = 0;
    unsigned int bytes_per_page = 0;
    unsigned char * id = _id;
    unsigned char * onfi_id = _onfi_id;
    unsigned int scan_format = _scan_format;

    if (('O' == onfi_id[0]) &&
        ('N' == onfi_id[1]) &&
        ('F' == onfi_id[2]) &&
        ('I' == onfi_id[3]))
    {
        /**********************************************************************
         * ONFI Device, Read ONFI Parameter Page
         **********************************************************************/
        NFC_PHY_GetOnfiParameter(0, 0, 0x00, (unsigned char *)&phy_features.onfi_param, (unsigned char *)&phy_features.onfi_ext_param);

        /**********************************************************************
         * Generation : L74A
         **********************************************************************/
             if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F64G08CBAAA",  strlen("MT29F64G08CBAAA")))  { nand = NAND_MICRON_L74A_MT29F64G08CBAAA; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F64G08CBCAB",  strlen("MT29F64G08CBCAB")))  { nand = NAND_MICRON_L74A_MT29F64G08CBCAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F128G08CEAAA", strlen("MT29F128G08CEAAA"))) { nand = NAND_MICRON_L74A_MT29F128G08CEAAA; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F128G08CFAAA", strlen("MT29F128G08CFAAA"))) { nand = NAND_MICRON_L74A_MT29F128G08CFAAA; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F128G08CFAAB", strlen("MT29F128G08CFAAB"))) { nand = NAND_MICRON_L74A_MT29F128G08CFAAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F128G08CECAB", strlen("MT29F128G08CECAB"))) { nand = NAND_MICRON_L74A_MT29F128G08CECAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F256G08CJAAA", strlen("MT29F256G08CJAAA"))) { nand = NAND_MICRON_L74A_MT29F256G08CJAAA; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F256G08CKAAA", strlen("MT29F256G08CKAAA"))) { nand = NAND_MICRON_L74A_MT29F256G08CKAAA; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F256G08CKCAB", strlen("MT29F256G08CKCAB"))) { nand = NAND_MICRON_L74A_MT29F256G08CKCAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F256G08CMAAA", strlen("MT29F256G08CMAAA"))) { nand = NAND_MICRON_L74A_MT29F256G08CMAAA; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F256G08CMCAB", strlen("MT29F256G08CMCAB"))) { nand = NAND_MICRON_L74A_MT29F256G08CMCAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F512G08CUAAA", strlen("MT29F512G08CUAAA"))) { nand = NAND_MICRON_L74A_MT29F512G08CUAAA; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F512G08CUCAB", strlen("MT29F512G08CUCAB"))) { nand = NAND_MICRON_L74A_MT29F512G08CUCAB; }

        /**********************************************************************
         * Generation : L83A
         **********************************************************************/
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F32G08CBADA",  strlen("MT29F32G08CBADA")))  { nand = NAND_MICRON_L83A_MT29F32G08CBADA; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F32G08CBADB",  strlen("MT29F32G08CBADB")))  { nand = NAND_MICRON_L83A_MT29F32G08CBADB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F64G08CFADA",  strlen("MT29F64G08CFADA")))  { nand = NAND_MICRON_L83A_MT29F64G08CFADA; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F64G08CFADB",  strlen("MT29F64G08CFADB")))  { nand = NAND_MICRON_L83A_MT29F64G08CFADB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F64G08CECDB",  strlen("MT29F64G08CECDB")))  { nand = NAND_MICRON_L83A_MT29F64G08CECDB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F128G08CMCDB", strlen("MT29F128G08CMCDB"))) { nand = NAND_MICRON_L83A_MT29F128G08CMCDB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F256G08CUCDB", strlen("MT29F256G08CUCDB"))) { nand = NAND_MICRON_L83A_MT29F256G08CUCDB; }

        /**********************************************************************
         * Generation : L84A
         **********************************************************************/
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F64G08CBABA",  strlen("MT29F64G08CBABA")))  { nand = NAND_MICRON_L84A_MT29F64G08CBABA; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F64G08CBABB",  strlen("MT29F64G08CBABB")))  { nand = NAND_MICRON_L84A_MT29F64G08CBABB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F64G08CBCBB",  strlen("MT29F64G08CBCBB")))  { nand = NAND_MICRON_L84A_MT29F64G08CBCBB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F128G08CECBB", strlen("MT29F128G08CECBB"))) { nand = NAND_MICRON_L84A_MT29F128G08CECBB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F128G08CFABA", strlen("MT29F128G08CFABA"))) { nand = NAND_MICRON_L84A_MT29F128G08CFABA; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F128G08CFABB", strlen("MT29F128G08CFABB"))) { nand = NAND_MICRON_L84A_MT29F128G08CFABB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F256G08CJABA", strlen("MT29F256G08CJABA"))) { nand = NAND_MICRON_L84A_MT29F256G08CJABA; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F256G08CJABB", strlen("MT29F256G08CJABB"))) { nand = NAND_MICRON_L84A_MT29F256G08CJABB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F256G08CKCBB", strlen("MT29F256G08CKCBB"))) { nand = NAND_MICRON_L84A_MT29F256G08CKCBB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F256G08CMCBB", strlen("MT29F256G08CMCBB"))) { nand = NAND_MICRON_L84A_MT29F256G08CMCBB; }

        /**********************************************************************
         * Generation : L85A
         **********************************************************************/
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F128G08CBCAB", strlen("MT29F128G08CBCAB"))) { nand = NAND_MICRON_L85A_MT29F128G08CBCAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F128G08CBEAB", strlen("MT29F128G08CBEAB"))) { nand = NAND_MICRON_L85A_MT29F128G08CBEAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F256G08CECAB", strlen("MT29F256G08CECAB"))) { nand = NAND_MICRON_L85A_MT29F256G08CECAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F256G08CEEAB", strlen("MT29F256G08CEEAB"))) { nand = NAND_MICRON_L85A_MT29F256G08CEEAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F512G08CKCAB", strlen("MT29F512G08CKCAB"))) { nand = NAND_MICRON_L85A_MT29F512G08CKCAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F512G08CKEAB", strlen("MT29F512G08CKEAB"))) { nand = NAND_MICRON_L85A_MT29F512G08CKEAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F512G08CMCAB", strlen("MT29F512G08CMCAB"))) { nand = NAND_MICRON_L85A_MT29F512G08CMCAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F512G08CMEAB", strlen("MT29F512G08CMEAB"))) { nand = NAND_MICRON_L85A_MT29F512G08CMEAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F1T08CUCAB",   strlen("MT29F1T08CUCAB")))   { nand = NAND_MICRON_L85A_MT29F1T08CUCAB; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"MT29F1T08CUEAB",   strlen("MT29F1T08CUEAB")))   { nand = NAND_MICRON_L85A_MT29F1T08CUEAB; }

        /**********************************************************************
         * Generation : Unknown !
         **********************************************************************/
        else
        {
            NFC_PHY_ScanMicronHelp(id);
        }

        /**********************************************************************
         * Start Configuration !
         **********************************************************************/
        if (scan_format)
        {
            bytes_per_page = NFC_PHY_ConfigOnfi(id, nand, (void *)&phy_features.nand_config, (void *)&phy_features.onfi_param, (void *)&phy_features.onfi_ext_param);
            if (Exchange.ftl.fnConfig) { Exchange.ftl.fnConfig((void *)&phy_features.nand_config); }
        }
        else
        {
            bytes_per_page = phy_features.onfi_param._f.number_of_data_bytes_per_page;
        }
    }
    else
    {
        NFC_PHY_ScanMicronHelp(id);
    }

    return bytes_per_page;
}

/******************************************************************************
 *
 ******************************************************************************/
void NFC_PHY_ScanIntelHelp(unsigned char * _id)
{
    unsigned char * id = _id;

    __print("\n");
    __print("NAND Maker    : INTEL\n");
    __print("NAND ID       : %02x %02x %02x %02x %02x %02x %02x %02x\n", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7]);
    __print("NAND Decoding : This NAND Does Not Support, Yet. Make Configuration Manually !!\n");
    __print("\n");
}

unsigned int NFC_PHY_ScanIntel(unsigned char * _id, unsigned char * _onfi_id, unsigned int _scan_format)
{
    unsigned int nand = 0;
    unsigned int bytes_per_page = 0;
    unsigned char * id = _id;
    unsigned char * onfi_id = _onfi_id;
    unsigned int scan_format = _scan_format;

    if (('O' == onfi_id[0]) &&
        ('N' == onfi_id[1]) &&
        ('F' == onfi_id[2]) &&
        ('I' == onfi_id[3]))
    {
        /**********************************************************************
         * ONFI Device, Read ONFI Parameter Page
         **********************************************************************/
        NFC_PHY_GetOnfiParameter(0, 0, 0x00, (unsigned char *)&phy_features.onfi_param, (unsigned char *)&phy_features.onfi_ext_param);

        /**********************************************************************
         * Generation : 25NM
         **********************************************************************/
             if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"JS29F64G08AAME1", strlen("JS29F64G08AAME1"))) { nand = NAND_INTEL_JS29F64G08AAME1; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"JS29F16B08CAME1", strlen("JS29F16B08CAME1"))) { nand = NAND_INTEL_JS29F16B08CAME1; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"JS29F32B08JAME1", strlen("JS29F32B08JAME1"))) { nand = NAND_INTEL_JS29F32B08JAME1; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"JS29F64G08CCNE1", strlen("JS29F64G08CCNE1"))) { nand = NAND_INTEL_JS29F64G08CCNE1; }
        else if (!memcmp((const void *)phy_features.onfi_param._f.device_model, (const void *)"JS29F16B08JCNE1", strlen("JS29F16B08JCNE1"))) { nand = NAND_INTEL_JS29F16B08JCNE1; }

        /**********************************************************************
         * Generation : Unknown !
         **********************************************************************/
        else
        {
            NFC_PHY_ScanIntelHelp(id);
        }

        /**********************************************************************
         * Start Configuration !
         **********************************************************************/
        if (scan_format)
        {
            bytes_per_page = NFC_PHY_ConfigOnfi(id, nand, (void *)&phy_features.nand_config._c, (void *)&phy_features.onfi_param._c, (void *)&phy_features.onfi_ext_param._c);
            if (Exchange.ftl.fnConfig) { Exchange.ftl.fnConfig((void *)&phy_features.nand_config); }
        }
        else
        {
            bytes_per_page = phy_features.onfi_param._f.number_of_data_bytes_per_page;
        }
    }
    else
    {
        NFC_PHY_ScanIntelHelp(id);
    }

    return bytes_per_page;
}

/******************************************************************************
 *
 ******************************************************************************/
unsigned int NFC_PHY_ScanFeature(unsigned int _scan_format)
{
    unsigned int scan_format = _scan_format;
    unsigned int bytes_per_page = 0;
    unsigned char id[8];
    unsigned char onfi_id[4];

    NAND * nand_config = (NAND *)&phy_features.nand_config;

    NFC_PHY_ReadId(0, 0, (char *)id, (char *)onfi_id);

    /**************************************************************************
     * Make Loose Nand Configurations
     **************************************************************************/
    memset((void *)nand_config, 0, sizeof(NAND));

    nand_config->_f.interfacetype = NAND_INTERFACE_ASYNC;
    nand_config->_f.onfi_detected = 0;
    nand_config->_f.onfi_timing_mode = 0;

    nand_config->_f.timing.async.tClk  = __MHZ(10); // Hz
    nand_config->_f.timing.async.tRWC  = 100;       // ns
    nand_config->_f.timing.async.tR    = 200000;    // ns
    nand_config->_f.timing.async.tWB   = 200;       // ns
    nand_config->_f.timing.async.tCCS  = 400;       // ns
    nand_config->_f.timing.async.tADL  = 400;       // ns
    nand_config->_f.timing.async.tRHW  = 200;       // ns
    nand_config->_f.timing.async.tWHR  = 120;       // ns
    nand_config->_f.timing.async.tWW   = 100;       // ns
    nand_config->_f.timing.async.tRR   = 40;        // ns
    nand_config->_f.timing.async.tFEAT = 1000;      // ns

    nand_config->_f.timing.async.tCS   = 70;        // ns
    nand_config->_f.timing.async.tCH   = 20;        // ns
    nand_config->_f.timing.async.tCLS  = 50;        // ns
    nand_config->_f.timing.async.tALS  = 50;        // ns
    nand_config->_f.timing.async.tCLH  = 20;        // ns
    nand_config->_f.timing.async.tALH  = 20;        // ns
    nand_config->_f.timing.async.tWP   = 50;        // ns
    nand_config->_f.timing.async.tWH   = 30;        // ns
    nand_config->_f.timing.async.tWC   = 100;       // ns
    nand_config->_f.timing.async.tDS   = 40;        // ns
    nand_config->_f.timing.async.tDH   = 20;        // ns

    nand_config->_f.timing.async.tCEA  = 100;       // ns
    nand_config->_f.timing.async.tREA  = 40;        // ns
    nand_config->_f.timing.async.tRP   = 50;        // ns
    nand_config->_f.timing.async.tREH  = 30;        // ns
    nand_config->_f.timing.async.tRC   = 100;       // ns
    nand_config->_f.timing.async.tCOH  = 0;         // ns

    /**************************************************************************
     * Make Tight Nand Configurations
     **************************************************************************/
    switch (id[0])
    {
        case NAND_MAKER_SEC:     { bytes_per_page = NFC_PHY_ScanSec(id, onfi_id, scan_format); } break;
        case NAND_MAKER_TOSHIBA: { bytes_per_page = NFC_PHY_ScanToshiba(id, onfi_id, scan_format); } break;
        case NAND_MAKER_SKHYNIX: { bytes_per_page = NFC_PHY_ScanSkhynix(id, onfi_id, scan_format); } break;
        case NAND_MAKER_MICRON:  { bytes_per_page = NFC_PHY_ScanMicron(id, onfi_id, scan_format); } break;
        case NAND_MAKER_INTEL:   { bytes_per_page = NFC_PHY_ScanIntel(id, onfi_id, scan_format); } break;
    }

    if (Exchange.debug.ftl.configurations && bytes_per_page)
    {
        Exchange.std.__print("EWS.NFC: Scan Configurations: Done\n");

        Exchange.std.__print("\n");
        Exchange.std.__print("*******************************************************************************\n");
        Exchange.std.__print("* NAND Configuration Summary\n");
        Exchange.std.__print("*\n");
        Exchange.std.__print("* - Manufacturer : %s\n", nand_config->_f.manufacturer);
        Exchange.std.__print("* - Model Name : %s\n", nand_config->_f.modelname);
        Exchange.std.__print("* - Generation : %s\n", nand_config->_f.generation);

        Exchange.std.__print("*\n");
        Exchange.std.__print("* - Interfacetype : %d\n", nand_config->_f.interfacetype);
        Exchange.std.__print("* - ONFI Detected : %d\n", nand_config->_f.onfi_detected);
        Exchange.std.__print("* - ONFI Timing Mode : %d\n", nand_config->_f.onfi_timing_mode);

        Exchange.std.__print("*\n");
        Exchange.std.__print("* - tClk : %d\n", nand_config->_f.timing.async.tClk);
        Exchange.std.__print("* - tRWC : %d\n", nand_config->_f.timing.async.tRWC);
        Exchange.std.__print("* - tR : %d\n", nand_config->_f.timing.async.tR);
        Exchange.std.__print("* - tWB : %d\n", nand_config->_f.timing.async.tWB);
        Exchange.std.__print("* - tCCS : %d\n", nand_config->_f.timing.async.tCCS);
        Exchange.std.__print("* - tADL : %d\n", nand_config->_f.timing.async.tADL);
        Exchange.std.__print("* - tRHW : %d\n", nand_config->_f.timing.async.tRHW);
        Exchange.std.__print("* - tWHR : %d\n", nand_config->_f.timing.async.tWHR);
        Exchange.std.__print("* - tWW : %d\n", nand_config->_f.timing.async.tWW);
        Exchange.std.__print("* - tRR : %d\n", nand_config->_f.timing.async.tRR);
        Exchange.std.__print("* - tFEAT : %d\n", nand_config->_f.timing.async.tFEAT);

        Exchange.std.__print("*\n");
        Exchange.std.__print("* - tCS : %d\n", nand_config->_f.timing.async.tCS);
        Exchange.std.__print("* - tCH : %d\n", nand_config->_f.timing.async.tCH);
        Exchange.std.__print("* - tCLS : %d\n", nand_config->_f.timing.async.tCLS);
        Exchange.std.__print("* - tALS : %d\n", nand_config->_f.timing.async.tALS);
        Exchange.std.__print("* - tCLH : %d\n", nand_config->_f.timing.async.tCLH);
        Exchange.std.__print("* - tALH : %d\n", nand_config->_f.timing.async.tALH);
        Exchange.std.__print("* - tWP : %d\n", nand_config->_f.timing.async.tWP);
        Exchange.std.__print("* - tWH : %d\n", nand_config->_f.timing.async.tWH);
        Exchange.std.__print("* - tWC : %d\n", nand_config->_f.timing.async.tWC);
        Exchange.std.__print("* - tDS : %d\n", nand_config->_f.timing.async.tDS);
        Exchange.std.__print("* - tDH : %d\n", nand_config->_f.timing.async.tDH);
        Exchange.std.__print("* - tCEA : %d\n", nand_config->_f.timing.async.tCEA);
        Exchange.std.__print("* - tREA : %d\n", nand_config->_f.timing.async.tREA);
        Exchange.std.__print("* - tRP : %d\n", nand_config->_f.timing.async.tRP);
        Exchange.std.__print("* - tREH : %d\n", nand_config->_f.timing.async.tREH);
        Exchange.std.__print("* - tRC : %d\n", nand_config->_f.timing.async.tRC);
        Exchange.std.__print("* - tCOH : %d\n", nand_config->_f.timing.async.tCOH);

        Exchange.std.__print("*\n");
        Exchange.std.__print("* - Luns Per Ce : %d\n", nand_config->_f.luns_per_ce);
        Exchange.std.__print("* - Databytes Per Page : %d\n", nand_config->_f.databytes_per_page);
        Exchange.std.__print("* - Sparebytes Per Page : %d\n", nand_config->_f.sparebytes_per_page);
        Exchange.std.__print("* - Number Of Planes : %d\n", nand_config->_f.number_of_planes);
        Exchange.std.__print("* - Pages Per Block : %d\n", nand_config->_f.pages_per_block);
        Exchange.std.__print("* - Mainblocks Per Lun : %d\n", nand_config->_f.mainblocks_per_lun);
        Exchange.std.__print("* - Extendedblocks Per Lun : %d\n", nand_config->_f.extendedblocks_per_lun);
        Exchange.std.__print("* - Next Lun Address : %d\n", nand_config->_f.next_lun_address);
        Exchange.std.__print("* - Over Provisioning : %d\n", nand_config->_f.over_provisioning);
        Exchange.std.__print("* - Bits Per Cell : %d\n", nand_config->_f.bits_per_cell);
        Exchange.std.__print("* - Number Of Bits Ecc Correctability : %d\n", nand_config->_f.number_of_bits_ecc_correctability);
        Exchange.std.__print("* - Maindatabytes Per Eccunit : %d\n", nand_config->_f.maindatabytes_per_eccunit);
        Exchange.std.__print("* - Eccbits Per Maindata : %d\n", nand_config->_f.eccbits_per_maindata);
        Exchange.std.__print("* - Eccbits Per Blockinformation : %d\n", nand_config->_f.eccbits_per_blockinformation);
        Exchange.std.__print("* - Block Endurance : %d\n", nand_config->_f.block_endurance);
        Exchange.std.__print("* - Factorybadblocks Per Nand : %d\n", nand_config->_f.factorybadblocks_per_nand);

        Exchange.std.__print("*\n");
        Exchange.std.__print("* - Multiplane Read %d\n", nand_config->_f.support_type.multiplane_read);
        Exchange.std.__print("* - Multiplane Write %d\n", nand_config->_f.support_type.multiplane_write);
        Exchange.std.__print("* - Cache Read %d\n", nand_config->_f.support_type.cache_read);
        Exchange.std.__print("* - Cache Write %d\n", nand_config->_f.support_type.cache_write);
        Exchange.std.__print("* - Interleave %d\n", nand_config->_f.support_type.interleave);
        Exchange.std.__print("* - Paired Page Mapping %d\n", nand_config->_f.support_type.paired_page_mapping);
        Exchange.std.__print("* - Block Indicator %d\n", nand_config->_f.support_type.block_indicator);
        Exchange.std.__print("* - Paired Plane %d\n", nand_config->_f.support_type.paired_plane);
        Exchange.std.__print("* - Multiplane Erase %d\n", nand_config->_f.support_type.multiplane_erase);
        Exchange.std.__print("* - Read Retry %d\n", nand_config->_f.support_type.read_retry);
        Exchange.std.__print("*\n");

        Exchange.std.__print("*******************************************************************************\n");
        Exchange.std.__print("\n");
    }

    return bytes_per_page;
}