/*
    Barebone USB CDC ACM (USB-UART) implementation
    Copyright (C) 2020 Andrey Zabolotnyi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#include "cdcacm-priv.h"

const usb_device_desc_t usb_cdc_acm_desc =
{
    /// Size of this descriptor in bytes
    .bLength = sizeof (usb_device_desc_t),
    /// Descriptor Type
    .bDescriptorType = USB_DT_DEVICE,
    /// USB Specification Release Number in Binary-Coded Decimal (i.e., 2.10 is 210H)
    .bcdUSB = USB_BCD_VER (2,0),
    /// Device class code
    .bDeviceClass = USB_CLASS_CDC,
    /// Device subclass code
    .bDeviceSubClass = 0,
    /// Device protocol, depends on class & subclass
    .bDeviceProtocol = 0,
    /// Maximum packet size for endpoint zero (control): 8,16,32 or 64
    .bMaxPacketSize0 = EP_CTL_PKT_SIZE,
    /// Vendor ID
    .idVendor = USB_CDC_VENDOR_ID,
    /// Product ID
    .idProduct = USB_CDC_PRODUCT_ID,
    /// Device release number in Binary-Coded Decimal
    .bcdDevice = USB_CDC_VER,
    /// USB string index for device manufacturer or 0
    .iManufacturer = USB_CDC_STR_IDX_MANUFACTURER,
    /// USB string index for product name or 0
    .iProduct = USB_CDC_STR_IDX_PRODUCT,
    /// USB string index for serial number or 0
    .iSerialNumber = USB_CDC_STR_IDX_SERIALNUMBER,
    /// Number of possible configurations
    .bNumConfigurations = 1,
};

const usb_cdc_acm_config_t usb_cdc_acm_config =
{
    .config =
    {
        /// Size of this descriptor in bytes
        .bLength = sizeof (usb_config_desc_t),
        /// Descriptor Type (USB_DT_CONFIG)
        .bDescriptorType = USB_DT_CONFIG,
        /// Total length of all descriptors for this configuration
        .wTotalLength = sizeof (usb_cdc_acm_config_t),
        /// Number of interfaces for this configuration
        .bNumInterfaces = 2,
        /// Configuration identifier (used by SET_CONFIGURATION request)
        .bConfigurationValue = 1,
        /// String index that describes this configuration
        .iConfiguration = 0,
        /// Various attributes, see USB_CONFIG_ATTR()
        .bmAttributes = USB_CONFIG_ATTR (USB_CDC_POWER_MA, 0),
        /// Bus current consumed by device (if not self-powered), USB_CONFIG_MA()
        .bMaxPower = USB_CONFIG_MA (USB_CDC_POWER_MA),
    },
    .iface =
    {
        /// Size of this descriptor in bytes
        .bLength = sizeof (usb_interface_desc_t),
        /// Descriptor Type (USB_DT_INTERFACE)
        .bDescriptorType = USB_DT_INTERFACE,
        /// Zero-based number of this interface
        .bInterfaceNumber = 0,
        /// Value used to select this alternate setting
        .bAlternateSetting = 0,
        /// Number of endpoints (excluding EP0) used by this interface
        .bNumEndpoints = 1,
        /// Device class code
        .bInterfaceClass = USB_CLASS_CDC,
        /// Device subclass code
        .bInterfaceSubClass = USB_CDC_CLASS_ACM,
        /// Device protocol, depends on class & subclass
        .bInterfaceProtocol = USB_CDC_PROTO_V25TER,
        /// Index of string descriptor describing this interface
        .iInterface = 0,
    },
    .func =
    {
        /// Size of this descriptor in bytes
        .bFunctionLength = sizeof (usb_cdc_header_desc_t),
        /// Descriptor type (USB_DT_CS_INTERFACE)
        .bDescriptorType = USB_DT_CS_INTERFACE,
        /// Device sub-type (USB_CDC_CLASS_HEADER)
        .bDescriptorSubType = USB_CDC_CLASS_HEADER,
        /// USB CDC standard version in BCD
        .bcdCDC = USB_CDC_SPEC,
    },
    .call =
    {
        /// Size of this descriptor in bytes
        .bFunctionLength = sizeof (usb_cdc_callmgmt_desc_t),
        /// Descriptor type (USB_DT_CS_INTERFACE)
        .bDescriptorType = USB_DT_CS_INTERFACE,
        /// Device sub-type (USB_CDC_CLASS_CALL_MGMT)
        .bDescriptorSubType = USB_CDC_CLASS_CALL_MGMT,
        /// The capabilities that this configuration supports, see USB_CDC_CALLMGMT_CAP()
        .bmCapabilities = USB_CDC_CALLMGMT_CAP (0, 0),
        /// Interface number of Data Class interface optionally used for call management
        .bDataInterface = 1,
    },
    .acm =
    {
        /// Size of this descriptor in bytes
        .bFunctionLength = sizeof (usb_cdc_acm_desc_t),
        /// Descriptor type (USB_DT_CS_INTERFACE)
        .bDescriptorType = USB_DT_CS_INTERFACE,
        /// Device sub-type (USB_CDC_CLASS_ACM)
        .bDescriptorSubType = USB_CDC_CLASS_ACM,
        /// Capabilities of the CDC ACM device, see USB_CDC_ACM_CAP()
        .bmCapabilities = USB_CDC_ACM_CAP (0, USB_CDC_LINE_CODING, 0, 0),
    },
    .ifunion =
    {
        /// Size of this descriptor in bytes
        .bFunctionLength = sizeof (usb_cdc_union_desc_t) + 1,
        /// Descriptor type (USB_DT_CS_INTERFACE)
        .bDescriptorType = USB_DT_CS_INTERFACE,
        /// Device sub-type (USB_CDC_CLASS_UNION)
        .bDescriptorSubType = USB_CDC_CLASS_UNION,
        /// CDC interface number designated as the controlling interface
        .bMasterInterface = 0,
    },
    .ifunion_slaves =
    {
        1
    },
    .ep1 =
    {
        /// Size of this descriptor in bytes
        .bLength = sizeof (usb_endpoint_desc_t),
        /// Descriptor Type (USB_DT_ENDPOINT)
        .bDescriptorType = USB_DT_ENDPOINT,
        /// The address of the endpoint on the USB device (see USB_EP_ADDR())
        .bEndpointAddress = USB_EP_ADDR (EP_INT, true),
        /// Endpoint attributes
        .bmAttributes = USB_EP_ATTR (INTERRUPT, X, X),
        /// Maximum packet size this endpoint is capable of sending or receiving
        .wMaxPacketSize = EP_INT_PKT_SIZE,
        /// Time interval for data transfer polling (units depend on EP type)
        .bInterval = 255,
    },
    .data =
    {
        /// Size of this descriptor in bytes
        .bLength = sizeof (usb_interface_desc_t),
        /// Descriptor Type (USB_DT_INTERFACE)
        .bDescriptorType = USB_DT_INTERFACE,
        /// Zero-based number of this interface
        .bInterfaceNumber = 1,
        /// Value used to select this alternate setting
        .bAlternateSetting = 0,
        /// Number of endpoints (excluding EP0) used by this interface
        .bNumEndpoints = 2,
        /// Device class code
        .bInterfaceClass = USB_CLASS_DATA,
        /// Device subclass code
        .bInterfaceSubClass = USB_SUBCLASS_NONE,
        /// Device protocol, depends on class & subclass
        .bInterfaceProtocol = USB_PROTO_NONE,
        /// Index of string descriptor describing this interface
        .iInterface = 0,
    },
    .ep2 =
    {
        /// Size of this descriptor in bytes
        .bLength = sizeof (usb_endpoint_desc_t),
        /// Descriptor Type (USB_DT_ENDPOINT)
        .bDescriptorType = USB_DT_ENDPOINT,
        /// The address of the endpoint on the USB device (see USB_EP_ADDR())
        .bEndpointAddress = USB_EP_ADDR (EP_DATA, true),
        /// Endpoint attributes
        .bmAttributes = USB_EP_ATTR (BULK, X, X),
        /// Maximum packet size this endpoint is capable of sending or receiving
        .wMaxPacketSize = EP_DATA_PKT_SIZE,
        /// Time interval for data transfer polling (units depend on EP type)
        .bInterval = 0,
    },
    .ep3 =
    {
        /// Size of this descriptor in bytes
        .bLength = sizeof (usb_endpoint_desc_t),
        /// Descriptor Type (USB_DT_ENDPOINT)
        .bDescriptorType = USB_DT_ENDPOINT,
        /// The address of the endpoint on the USB device (see USB_EP_ADDR())
        .bEndpointAddress = USB_EP_ADDR (EP_DATA, false),
        /// Endpoint attributes
        .bmAttributes = USB_EP_ATTR (BULK, X, X),
        /// Maximum packet size this endpoint is capable of sending or receiving
        .wMaxPacketSize = EP_DATA_PKT_SIZE,
        /// Time interval for data transfer polling (units depend on EP type)
        .bInterval = 0,
    },
};
