/******************************************************************************
 *
 * (C) COPYRIGHT 2008-2014 EASTWHO CO., LTD ALL RIGHTS RESERVED
 *
 * File name    : mio.definition.h
 * Date         : 2014.07.11
 * Author       : SD.LEE (mcdu1214@eastwho.com)
 * Abstraction  :
 * Revision     : V1.0 (2014.07.11 SD.LEE)
 *
 * Description  :
 *
 ******************************************************************************/
#pragma once

#include "media/exchange.config.h"

/******************************************************************************
 *
 ******************************************************************************/
#define __POW(B,P)              ((unsigned int)(B)<<(P))
#define __NROOT(B,R)            ((B)>>(R))

/******************************************************************************
 *
 ******************************************************************************/
#define __GB(X)                 __POW(X,30)
#define __MB(X)                 __POW(X,20)
#define __KB(X)                 __POW(X,10)
#define __B(X)                  __POW(X,0)

/******************************************************************************
 *
 ******************************************************************************/
#define __GHZ(X)                ((X)*1000000000)
#define __MHZ(X)                ((X)*1000000)
#define __KHZ(X)                ((X)*1000)
#define __HZ(X)                 ((X)*1)

/******************************************************************************
 *
 ******************************************************************************/
#define __SECTOR_SIZEOF(S)      __POW(S,9)      // Seccnt -> Byte
#define __SECTOR_OF_BYTE(B)     __NROOT(B,9)    // Byte -> Seccnt

/******************************************************************************
 *
 ******************************************************************************/
#define __TRUE  (1)
#define __FALSE (0)
#define __NULL  (0)

/******************************************************************************
 * Debug Print Options
 ******************************************************************************/

#if defined (__BUILD_MODE_ARM_LINUX_DEVICE_DRIVER__)
#define __PRINT     printk

#elif defined (__BUILD_MODE_ARM_UBOOT_DEVICE_DRIVER__)
#define __PRINT     printf

#endif

#define __PRINTK_DBG
#define __DBG_BLK
#define __DBG_BLK_REQ
//#define __DBG_MEDIA
#define __DBG_NFC_PHY
#define __DBG_NFC_PHY_FEATURE
#define __DBG_NFC_PHY_ECC
#define __DBG_NFC_PHY_ECC_CORRECTION
#define __DBG_NFC_PHY_ECC_CORRECTED
//#define __DBG_NFC_PHY_ECC_UNCORRECTABLE

#ifndef __PRINTK_DBG
#define PRINTK_DBG(fmt, args...) __PRINT(fmt, ##args)
#else
#define PRINTK_DBG(fmt, args...)
#endif

#ifndef __DBG_BLK
#define DBG_BLK(fmt, args...) __PRINT(fmt, ##args)
#else
#define DBG_BLK(fmt, args...)
#endif

#ifndef __DBG_BLK_REQ
#define DBG_BLK_REQ(fmt, args...) __PRINT(fmt, ##args)
#else
#define DBG_BLK_REQ(fmt, args...)
#endif

#ifndef __DBG_MEDIA
#define DBG_MEDIA(fmt, args...) __PRINT(fmt, ##args)
#else
#define DBG_MEDIA(fmt, args...)
#endif

#ifndef __DBG_NFC_PHY
#define DBG_NFC_PHY(fmt, args...) __PRINT(fmt, ##args)
#else
#define DBG_NFC_PHY(fmt, args...)
#endif

#ifndef __DBG_NFC_PHY_FEATURE
#define DBG_NFC_PHY_FEATURE(fmt, args...) __PRINT(fmt, ##args)
#else
#define DBG_NFC_PHY_FEATURE(fmt, args...)
#endif

#ifndef __DBG_NFC_PHY_ECC
#define DBG_NFC_PHY_ECC(fmt, args...) __PRINT(fmt, ##args)
#else
#define DBG_NFC_PHY_ECC(fmt, args...)
#endif

#ifndef __DBG_NFC_PHY_ECC_CORRECTION
#define DBG_NFC_PHY_ECC_CORRECTION(fmt, args...) __PRINT(fmt, ##args)
#else
#define DBG_NFC_PHY_ECC_CORRECTION(fmt, args...)
#endif

#ifndef __DBG_NFC_PHY_ECC_CORRECTED
#define DBG_NFC_PHY_ECC_CORRECTED(fmt, args...) __PRINT(fmt, ##args)
#else
#define DBG_NFC_PHY_ECC_CORRECTED(fmt, args...)
#endif

#ifndef __DBG_NFC_PHY_ECC_UNCORRECTABLE
#define DBG_NFC_PHY_ECC_UNCORRECTABLE(fmt, args...) __PRINT(fmt, ##args)
#else
#define DBG_NFC_PHY_ECC_UNCORRECTABLE(fmt, args...)
#endif
