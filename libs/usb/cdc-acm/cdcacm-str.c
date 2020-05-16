/*
    Barebone USB CDC ACM (USB-UART) implementation
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#define _CDCACM_STR_C
#include "cdcacm-priv.h"

typedef struct
{
#if defined USB_CDC_STR_MANUFACTURER || defined USB_CDC_STR_PRODUCT || defined USB_CDC_STR_SERIALNUMBER
    usb_desc_t hdr_lang_id;
    uint16_t lang_id [1];
#endif
#ifdef USB_CDC_STR_MANUFACTURER
    usb_desc_t hdr_manufacturer;
    uint16_t manufacturer [sizeof (USB_CDC_STR_MANUFACTURER) / 2 - 1];
#endif
#ifdef USB_CDC_STR_PRODUCT
    usb_desc_t hdr_product;
    uint16_t product [sizeof (USB_CDC_STR_PRODUCT) / 2 - 1];
#endif
#ifdef USB_CDC_STR_SERIALNUMBER
    usb_desc_t hdr_serialnumber;
    uint16_t serialnumber [sizeof (USB_CDC_STR_SERIALNUMBER) / 2 - 1];
#endif
    uint8_t eor;
} usb_desc_strings_t;

const usb_desc_strings_t usb_desc_strings =
{
#if defined USB_CDC_STR_MANUFACTURER || defined USB_CDC_STR_PRODUCT || defined USB_CDC_STR_SERIALNUMBER
    .hdr_lang_id =
    {
        sizeof (usb_desc_t) + sizeof (uint16_t),
        USB_DT_STRING,
    },
    .lang_id = {USB_CDC_STR_LANGID},
#endif
#ifdef USB_CDC_STR_MANUFACTURER
    .hdr_manufacturer =
    {
        sizeof (usb_desc_t) + sizeof (USB_CDC_STR_MANUFACTURER) - 2,
        USB_DT_STRING,
    },
    .manufacturer = {USB_CDC_STR_MANUFACTURER},
#endif
#ifdef USB_CDC_STR_PRODUCT
    .hdr_product =
    {
        sizeof (usb_desc_t) + sizeof (USB_CDC_STR_PRODUCT) - 2,
        USB_DT_STRING,
    },
    .product = {USB_CDC_STR_PRODUCT},
#endif
#ifdef USB_CDC_STR_SERIALNUMBER
    .hdr_serialnumber =
    {
        sizeof (usb_desc_t) + sizeof (USB_CDC_STR_SERIALNUMBER) - 2,
        USB_DT_STRING,
    },
    .serialnumber = {USB_CDC_STR_SERIALNUMBER},
#endif
    .eor = 0
};
