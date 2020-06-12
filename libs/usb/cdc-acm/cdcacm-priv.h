/*
    Barebone USB CDC ACM (USB-UART) implementation
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _CDCACM_PRIV_H
#define _CDCACM_PRIV_H

#include "usb.h"
#include "usb-cdc.h"
#include "usb-cdc-acm.h"

/* Number of endpoints */
#define EP_COUNT		4

/* Control endpoint */
#define EP_CTL                  0
/* Interrupt endpoint */
#define EP_INT                  1
/* Data endpoint */
#define EP_DATA                 2

/* Endpoint max packet size, must be either <64, or a multiple of 32 */
#define EP_CTL_PKT_SIZE         8
#define EP_INT_PKT_SIZE         8
#define EP_DATA_PKT_SIZE        UCA_RECEIVED_MAX

#define USB_CDC_SPEC            0x0120

/* Handy alias to USB->EPxR as array */
typedef uint32_t usb_epr_t [8];
#define USBEPxR(n)              ((*(volatile usb_epr_t *)&USB->EP0R) [n])

#define LOBYTE(x) ((uint8_t)(x & 0x00FF))
#define HIBYTE(x) ((uint8_t)((x & 0xFF00) >> 8))

/* USB_COUNTn_RX bits */
#define USB_COUNT_RX_COUNT_RX           USB_COUNT0_RX_COUNT0_RX
#define USB_COUNT_RX_NUM_BLOCK_Pos      USB_COUNT0_RX_NUM_BLOCK_Pos
#define USB_COUNT_RX_NUM_BLOCK          USB_COUNT0_RX_NUM_BLOCK
#define USB_COUNT_RX_BLSIZE             USB_COUNT0_RX_BLSIZE

#pragma pack (push, 1)

/** Endpoint configuration structure */
typedef struct
{
    // USB_EP_ADDR | USB_EP_TYPE
    uint16_t epn_type;
    // Max TX EP Buffer Size (in PMA, fragment size)
    uint8_t tx_max;
    // Max RX EP Buffer Size (in PMA, fragment size)
    uint8_t rx_max;
} uca_ep_config_t;

/** Endpoint status structure - one per each uca_ep_config_t */
typedef struct
{
    // TX data pointer, NULL when transmission complete
    const void *tx_buff;
    // RX buffer pointer, statically allocated at initialization
    void *rx_buff;
    // TX data length (up to 64k)
    uint16_t tx_len;
    // RX data length
    uint8_t rx_len;
    // true if last initiated TX transaction has been completed by hw
    bool tx_active : 1;
} uca_ep_status_t;

/* --- Dedicated Packet Buffer Memory (PMA) SRAM --- */

/* PMA size differs from MCU to MCU */
#if defined STM32F302x8 || defined STM32F302xE || defined STM32F303xE

// STM32F302x6/8/D/E and STM32F303xD/E have 1024 bytes PMA
// and 2 x 16bits PMA access scheme
#define USB_PMA_ACCESS          1

#ifndef USB_PMA_SIZE
#  define USB_PMA_SIZE          1024
#endif

#else

// 1 x 16bits PMA access scheme
#define USB_PMA_ACCESS          2

#ifndef USB_PMA_SIZE
#  define USB_PMA_SIZE          512
#endif

#endif

/** PMA descriptor descriptor table - single TX or RX record */
typedef struct
{
    uint16_t addr;
#if USB_PMA_ACCESS == 2
    unsigned :16;
#endif
    uint16_t count;
#if USB_PMA_ACCESS == 2
    unsigned :16;
#endif
} usb_pma_rec_t;

/** PMA descriptor descriptor table - endpoint record */
typedef struct
{
    usb_pma_rec_t tx;
    usb_pma_rec_t rx;
} usb_pma_epbuf_t;

/* We statically allocate buffer descriptors at offset 0 in PMA */
typedef usb_pma_epbuf_t uca_epbuf_table_t [EP_COUNT];
#define USBEPBUF (*(volatile uca_epbuf_table_t *)USB_PMAADDR)

enum
{
    USB_CDC_STR_IDX_LANG_ID = 0,
#ifdef USB_CDC_STR_MANUFACTURER
    USB_CDC_STR_IDX_MANUFACTURER,
#endif
#ifdef USB_CDC_STR_PRODUCT
    USB_CDC_STR_IDX_PRODUCT,
#endif
#ifdef USB_CDC_STR_SERIALNUMBER
    USB_CDC_STR_IDX_SERIALNUMBER,
#endif

#ifndef USB_CDC_STR_MANUFACTURER
    USB_CDC_STR_IDX_MANUFACTURER = 0,
#endif
#ifndef USB_CDC_STR_PRODUCT
    USB_CDC_STR_IDX_PRODUCT = 0,
#endif
#ifndef USB_CDC_STR_SERIALNUMBER
    USB_CDC_STR_IDX_SERIALNUMBER = 0,
#endif
};

/// USB device descriptor
extern const usb_device_desc_t usb_cdc_acm_desc;

/** USB CDC ACM function configuration descriptor */
typedef struct
{
    usb_config_desc_t config;
    // comm interface
    usb_interface_desc_t iface;
    usb_cdc_header_desc_t func;
    usb_cdc_callmgmt_desc_t call;
    usb_cdc_acm_desc_t acm;
    usb_cdc_union_desc_t ifunion;
    uint8_t ifunion_slaves [1];
    usb_endpoint_desc_t ep1;
    // data interface
    usb_interface_desc_t data;
    usb_endpoint_desc_t ep2;
    usb_endpoint_desc_t ep3;
} usb_cdc_acm_config_t;

/// USB CDC ACM function configuration descriptor
extern const usb_cdc_acm_config_t usb_cdc_acm_config;

#ifndef _CDCACM_STR_C
/// USB string descriptors
extern const usb_string_desc_t usb_desc_strings;
#endif

#pragma pack(pop)

#endif /* _CDCACM_PRIV_H */
