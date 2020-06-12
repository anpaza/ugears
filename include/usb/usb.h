/*
    USB library for uGears
    Copyright (C) 2020 Andrey Zabolotnyi
    Basic USB device definitions

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _USB_H
#define _USB_H

#include <stdint.h>

#pragma pack(push,1)

/** USB descriptor type codes */
typedef enum
{
    /// Device descriptor (see usb_device_desc_t)
    USB_DT_DEVICE = 0x01,
    /// Configuration descriptor (see usb_config_desc_t) */
    USB_DT_CONFIG = 0x02,
    /// String descriptor (see usb_string_desc_t)
    USB_DT_STRING = 0x03,
    /// Interface descriptor (see usb_interface_desc_t)
    USB_DT_INTERFACE = 0x04,
    /// Endpoint descriptor (see usb_endpoint_desc_t) */
    USB_DT_ENDPOINT = 0x05,
    /// Qualifier descriptor
    USB_DT_QUALIFIER = 0x06,
    /// OTG descriptor
    USB_DT_OTG = 0x09,
    /// Debug descriptor, whatever it means (ask lsusb authors whats it)
    USB_DT_DEBUG = 0x0A,
    /// Device Capability descriptor
    USB_DT_DEVICE_CAPABILITY = 0x10,
    /// HID descriptor
    USB_DT_HID = 0x21,
    /// HID report descriptor
    USB_DT_HID_REPORT = 0x22,
    /// Physical descriptor
    USB_DT_PHYSICAL = 0x23,
    /// Class-specific interface descriptor
    USB_DT_CS_INTERFACE = 0x24,
    /// Class-specific endpoint descriptor
    USB_DT_CS_ENDPOINT = 0x25,
    /// Hub descriptor
    USB_DT_HUB = 0x29,
} usb_desc_type_t;

/** USB descriptors common header */
typedef struct
{
    /// Size of this descriptor in bytes
    uint8_t bLength;
    /// Descriptor type
    usb_desc_type_t bDescriptorType : 8;
    //... followed by type-specific stuff
} usb_desc_t;

/**
 * Known USB device class identifiers
 */
typedef enum
{
    /// Audio device
    USB_CLASS_AUDIO = 1,
    /// Communication device
    USB_CLASS_CDC = 2,
    /// Human Interface Device
    USB_CLASS_HID = 3,
    /// Display device
    USB_DISPLAY = 4,
    /// Physical device
    USB_CLASS_PHYSICAL = 5,
    /// Still Imaging device
    USB_CLASS_STILL_IMAGE = 6,
    /// Printer device
    USB_CLASS_PRINTER = 7,
    /// Mass Storage device
    USB_CLASS_MASS_STORAGE = 8,
    /// USB hub
    USB_CLASS_HUB = 9,
    /// Data device
    USB_CLASS_DATA = 10,
    /// Smart Card
    USB_CLASS_SMART_CARD = 11,
    /// Content security
    USB_CLASS_CONTENT_SEC = 13,
    /// Video device
    USB_CLASS_VIDEO = 14,
    /// Personal Healthcare
    USB_CLASS_HEALTHCARE = 15,
    /// Diagnostic device
    USB_CLASS_DIAGNOSTIC = 0xDC,
    /// Wireless device
    USB_CLASS_WIRELESS = 0xE0,
    /// Miscelaneous device
    USB_CLASS_MISC = 0xEF,
    /// Application-specific
    USB_CLASS_APPLICATION = 0xFE,
    /// Vendor-specific
    USB_CLASS_VENDOR = 0xFF
} usb_device_class_t;

/**
 * Common USB device subclass identifiers.
 * Device subclass depends on its class; however, some values are
 * valid for all device classes.
 */
typedef enum
{
    /// Subclass not known / doesn't matter
    USB_SUBCLASS_NONE = 0x00,
    /// Vendor-specific subclass
    USB_SUBCLASS_VENDOR = 0xFF,
} usb_device_subclass_t;

/**
 * Common USB device protocol identifiers.
 * Device protocol depends on device class and subclass; however,
 * some values are valid for all kinds of devices.
 */
typedef enum
{
    /// Protocol not known / doesn't matter
    USB_PROTO_NONE = 0x00,
} usb_device_proto_t;

/**
 * Device descriptor format.
 * USB 2.0 spec, Table 9-8. Standard Device Descriptor.
 */
typedef struct
{
    /// Size of this descriptor in bytes
    uint8_t bLength;
    /// Descriptor Type (USB_DT_DEVICE)
    usb_desc_type_t bDescriptorType : 8;
    /// USB Specification Release Number in Binary-Coded Decimal (i.e., 2.10 is 210H)
    uint16_t bcdUSB;
    /// Device class code (see usb_device_class_t)
    usb_device_class_t bDeviceClass : 8;
    /// Device subclass code (depends on bDeviceClass)
    uint8_t bDeviceSubClass;
    /// Device protocol, depends on class & subclass
    uint8_t bDeviceProtocol;
    /// Maximum packet size for endpoint zero (control): 8,16,32 or 64
    uint8_t bMaxPacketSize0;
    /// Vendor ID
    uint16_t idVendor;
    /// Product ID
    uint16_t idProduct;
    /// Device release number in Binary-Coded Decimal
    uint16_t bcdDevice;
    /// String index for device manufacturer or 0
    uint8_t iManufacturer;
    /// String index for product name or 0
    uint8_t iProduct;
    /// String index for serial number or 0
    uint8_t iSerialNumber;
    /// Number of possible configurations
    uint8_t bNumConfigurations;
} usb_device_desc_t;

/** Encode h.l version number into a 16-bit value */
#define USB_BCD_VER(h,l) \
    ((((h) & 25) << 8) | (l & 255))

/**
 * Provides information about a specific device configuration.
 * USB 2.0 spec, Table 9-10. Standard Configuration Descriptor.
 */
typedef struct
{
    /// Size of this descriptor in bytes
    uint8_t bLength;
    /// Descriptor Type (USB_DT_CONFIG)
    usb_desc_type_t bDescriptorType : 8;
    /// Total length of all descriptors for this configuration
    uint16_t wTotalLength;
    /// Number of interfaces for this configuration
    uint8_t bNumInterfaces;
    /// Configuration identifier (used by SET_CONFIGURATION request)
    uint8_t bConfigurationValue;
    /// String index that describes this configuration
    uint8_t iConfiguration;
    /// Various attributes, see USB_CONFIG_ATTR()
    uint8_t bmAttributes;
    /// Bus current consumed by device (if not self-powered), USB_CONFIG_MA()
    uint8_t bMaxPower;
} usb_config_desc_t;

/**
 * Define USB device attributes
 * @param selfpow Zero if device is bus powered, non-zero if self-powered
 * @param canwake Zero if device can not be remotely waken
 */
#define USB_CONFIG_ATTR(selfpow, canwake) \
    (USB_CONFIG_ATTR_ALWAYS | \
     ((selfpow) ? USB_CONFIG_ATTR_SELFPOWERED : 0) | \
     ((canwake) ? USB_CONFIG_ATTR_REMOTEWAKEUP : 0))

/// This bit must always be set
#define USB_CONFIG_ATTR_ALWAYS                  0x80
/// Device is self-powered
#define USB_CONFIG_ATTR_SELFPOWERED             0x40
/// Device supports remote wakeup
#define USB_CONFIG_ATTR_REMOTEWAKEUP            0x20

/**
 * The bMaxPower field defines the max current consumed from the bus
 * by the device
 * @param cur Current in milliamperes
 */
#define USB_CONFIG_MA(cur)                      (cur / 2)

/**
 * Device interface descriptor format.
 * USB 2.0 spec, Table 9-12. Standard Interface Descriptor.
 */
typedef struct
{
    /// Size of this descriptor in bytes
    uint8_t bLength;
    /// Descriptor Type (USB_DT_INTERFACE)
    usb_desc_type_t bDescriptorType : 8;
    /// Zero-based number of this interface
    uint8_t bInterfaceNumber;
    /// Value used to select this alternate setting
    uint8_t bAlternateSetting;
    /// Number of endpoints (excluding EP0) used by this interface
    uint8_t bNumEndpoints;
    /// Device class code
    uint8_t bInterfaceClass;
    /// Device subclass code
    uint8_t bInterfaceSubClass;
    /// Device protocol, depends on class & subclass
    uint8_t bInterfaceProtocol;
    /// Index of string descriptor describing this interface
    uint8_t iInterface;
} usb_interface_desc_t;

/**
 * Device endpoint descriptor format.
 * USB 2.0 spec, Table 9-13. Standard Endpoint Descriptor.
 */
typedef struct
{
    /// Size of this descriptor in bytes
    uint8_t bLength;
    /// Descriptor Type (USB_DT_ENDPOINT)
    usb_desc_type_t bDescriptorType : 8;
    /// The address of the endpoint on the USB device (see USB_EP_ADDR())
    uint8_t bEndpointAddress;
    /// Endpoint attributes
    uint8_t bmAttributes;
    /// Maximum packet size this endpoint is capable of sending or receiving
    uint16_t wMaxPacketSize;
    /// Time interval for data transfer polling (units depend on EP type)
    uint8_t bInterval;
} usb_endpoint_desc_t;

/**
 * bEndpointAddress field is encoded as follows:
 * @li Bit 3...0: The endpoint number
 * @li Bit 7: Direction, ignored for control endpoints
 *     0 = OUT endpoint
 *     1 = IN endpoint
 */
#define USB_EP_ADDR(n,in) \
    (((n) & 0x0f) | ((in) ? 0x80 : 0x00))

/**
 * Encode the bmAttributes field.
 * @a sync and @a usage must be X for any type except ISOCHRONOUS.
 *
 * @param type One of CONTROL, ISOCHRONOUS, BULK, INTERRUPT
 * @param sync One of X, NONE, ASYNC, ADAPT, SYNC
 * @param usage One of X, DATA, FB, IMPFB
 */
#define USB_EP_ATTR(type, sync, usage) \
    JOIN2 (USB_EP_ATTR_TYPE_, type) | \
    JOIN2 (USB_EP_ATTR_SYNC_, sync) | \
    JOIN2 (USB_EP_ATTR_USAGE_, usage)

#define USB_EP_ATTR_TYPE_CONTROL                0x00
#define USB_EP_ATTR_TYPE_ISOCHRONOUS            0x01
#define USB_EP_ATTR_TYPE_BULK                   0x02
#define USB_EP_ATTR_TYPE_INTERRUPT              0x03

// Don't care (for non-isochronous endpoints)
#define USB_EP_ATTR_SYNC_X                      0x00
// No Synchronization
#define USB_EP_ATTR_SYNC_NONE                   (0x00 << 2)
// Asynchronous
#define USB_EP_ATTR_SYNC_ASYNC                  (0x01 << 2)
// Adaptive
#define USB_EP_ATTR_SYNC_ADAPT                  (0x02 << 2)
// Synchronous
#define USB_EP_ATTR_SYNC_SYNC                   (0x03 << 2)

// Don't care (for non-isochronous endpoints)
#define USB_EP_ATTR_USAGE_X                     0x00
// Data endpoint
#define USB_EP_ATTR_USAGE_NONE                  (0x00 << 4)
// Feedback endpoint
#define USB_EP_ATTR_USAGE_FB                    (0x01 << 4)
// Implicit feedback Data endpoint
#define USB_EP_ATTR_USAGE_IMPFB                 (0x02 << 4)

/**
 * For isochronous endpoints wMaxPacketSize field may specify the
 * number of additional transaction opportunities per microframe
 * @param n Maximal packet size for this endpoint, bytes
 * @param xop May be one of X, NONE, 1, 2
 */
#define USB_EP_PKTSIZE(n,xop) \
    ((n) & 2047) | JOIN2 (USB_EP_PKTSIZE_XOP_, xop)

// Don't care (for non-isochronous endpoints)
#define USB_EP_PKTSIZE_XOP_X                    0x00
// None (1 transaction per microframe)
#define USB_EP_PKTSIZE_XOP_NONE                 (0x00 << 11)
// 1 additional (2 per microframe)
#define USB_EP_PKTSIZE_XOP_1                    (0x01 << 11)
// 2 additional (3 per microframe)
#define USB_EP_PKTSIZE_XOP_2                    (0x02 << 11)

/**
 * USB string descriptor provides textual information
 * for the user interface.
 */
typedef struct
{
    /// Size of this descriptor in bytes
    uint8_t bLength;
    /// Descriptor type (USB_DT_STRING)
    usb_desc_type_t bDescriptorType : 8;
    /// Variable-length string
    uint16_t wString [0];
} usb_string_desc_t;

typedef union
{
    struct
    {
        uint8_t u8l : 8;
        uint8_t u8h : 8;
    };
    uint16_t u16;
} usb_uint16_t;

typedef struct
{
    /// Request type, see usb_req_type_mask_t bitmasks
    uint8_t bmRequestType;
    /// Request code, see usb_req_std_t
    uint8_t bRequest;
    usb_uint16_t wValue;
    usb_uint16_t wIndex;
    // Size of DATA packet following the setup packet
    uint16_t wLength;
} usb_setup_packet_t;

/** Bitfields for usb_setup_packet_t.bmRequestType */
typedef enum
{
    /// Recipient of the request, see usb_req_recp_t
    USB_REQ_TYPE_RECP_MASK = 0x1F,
    USB_REQ_TYPE_RECP_SHIFT = 0,
    /// Request type, see usb_req_type_t
    USB_REQ_TYPE_TYPE_MASK = 0x60,
    USB_REQ_TYPE_TYPE_SHIFT = 5,
    /// 0: Host-to-device, 1: Device-to-host
    USB_REQ_TYPE_DIR = 0x80,
    USB_REQ_TYPE_DIR_SHIFT = 7,
} usb_req_type_mask_t;

/** Request type: bmRequestType & USB_REQ_TYPE_TYPE_MASK */
typedef enum
{
    USB_REQ_TYPE_STANDARD = (0 << USB_REQ_TYPE_TYPE_SHIFT),
    USB_REQ_TYPE_CLASS = (1 << USB_REQ_TYPE_TYPE_SHIFT),
    USB_REQ_TYPE_VENDOR = (2 << USB_REQ_TYPE_TYPE_SHIFT),
    USB_REQ_TYPE_RESERVED = (3 << USB_REQ_TYPE_TYPE_SHIFT),
} usb_req_type_t;

/** Request recipient: bmRequestType & USB_REQ_TYPE_RECP_MASK */
typedef enum
{
    /// Device
    USB_REQ_RECP_DEVICE = (0 << USB_REQ_TYPE_RECP_SHIFT),
    /// Interface
    USB_REQ_RECP_INTERFACE = (1 << USB_REQ_TYPE_RECP_SHIFT),
    /// Endpoint
    USB_REQ_RECP_ENDPOINT = (2 << USB_REQ_TYPE_RECP_SHIFT),
    /// Other
    USB_REQ_RECP_OTHER = (3 << USB_REQ_TYPE_RECP_SHIFT),
} usb_req_recp_t;

/** USB Standard Request Codes (usb_setup_packet_t.bRequest) */
typedef enum
{
    USB_REQ_STD_GET_STATUS = 0,
    USB_REQ_STD_CLEAR_FEATURE = 1,
    USB_REQ_STD_SET_FEATURE = 3,
    USB_REQ_STD_SET_ADDRESS = 5,
    USB_REQ_STD_GET_DESCRIPTOR = 6,
    USB_REQ_STD_SET_DESCRIPTOR = 7,
    USB_REQ_STD_GET_CONFIGURATION = 8,
    USB_REQ_STD_SET_CONFIGURATION = 9,
    USB_REQ_STD_GET_INTERFACE = 10,
    USB_REQ_STD_SET_INTERFACE = 11,
    USB_REQ_STD_SYNC_FRAME = 12,
} usb_req_std_t;

/** Bits for reply in USB_REQ_STD_GET_STATUS */
typedef enum
{
    /// Device is currently self-powered
    USB_REQ_STATUS_SELFPOWERED = 0x0001,
    /// Device can remotely wakeup the host (e.g. keyboard)
    USB_REQ_STATUS_REMOTEWAKEUP = 0x0002,
} usb_req_std_get_status_t;

#pragma pack(pop)

#endif /* _USB_H */
