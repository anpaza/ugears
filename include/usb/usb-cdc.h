/*
    USB library for uGears
    Copyright (C) 2020 Andrey Zabolotnyi
    USB CDC device definitions

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
*/

#ifndef _USBCDC_H
#define _USBCDC_H

#include "usb.h"

#pragma pack(push, 1)

/**
 * USB CDC functional descriptors in bDescriptorSubType descriptor field.
 * USB CDC 1.1 spec, Table 25: bDescriptorSubType in Functional Descriptors
 */
typedef enum
{
    /// Header Functional Descriptor
    USB_CDC_CLASS_HEADER = 0x00,
    /// Call Management
    USB_CDC_CLASS_CALL_MGMT = 0x01,
    /// Abstract Control Management
    USB_CDC_CLASS_ACM = 0x02,
    /// Direct Line Management
    USB_CDC_CLASS_DIRECT_LINE_MGMT = 0x03,
    /// Telephone Ringer
    USB_CDC_CLASS_PHONE_RINGER = 0x04,
    /// Telephone Call and Line State Reporting Capabilities
    USB_CDC_CLASS_PHONE_CALL = 0x05,
    /// Union
    USB_CDC_CLASS_UNION = 0x06,
    /// Country Selection
    USB_CDC_CLASS_COUNTRY = 0x07,
    /// Telephone Operational Modes
    USB_CDC_CLASS_PHONE_OPMODE = 0x08,
    /// USB Terminal Functional
    USB_CDC_CLASS_TERMINAL = 0x09,
    /// Network Channel Terminal
    USB_CDC_CLASS_NETWORK_TERMINAL = 0x0A,
    /// Protocol Unit
    USB_CDC_CLASS_PROTOCOL_UNIT = 0x0B,
    /// Extension Unit
    USB_CDC_CLASS_EXTENSION_UNIT = 0x0C,
    /// Multi-Channel Management
    USB_CDC_CLASS_MULTI_CHANNEL_MGMT = 0x0D,
    /// CAPI Control Management
    USB_CDC_CLASS_CAPI_CONTROL_MGMT = 0x0E,
    /// Ethernet Networking
    USB_CDC_CLASS_ETHERNET = 0x0F,
    /// ATM Networking
    USB_CDC_CLASS_ATM = 0x010,
} usb_cdc_desc_subtype_t;

/**
 * usb_interface_desc_t.bInterfaceProtocol values as per USB CDC 1.1 spec,
 * Table 17: Communication Interface Class Control Protocol Codes
 */
typedef enum
{
    // No class specific protocol required
    USB_CDC_PROTO_NONE = 0x00,
    // Common AT commands (also known as “HayesTM compatible”)
    USB_CDC_PROTO_V25TER = 0x01,
    // Vendor-specific
    USB_CDC_PROTO_VENDOR = 0xFF,
} usb_cdc_proto_t;

#if 0
WTF ?????
/**
 * The usb_interface_desc_t.bInterfaceSubClass values.
 * USB CDC 1.1 spec, Table 16: Communication Interface Class SubClass Codes
 */
typedef enum
{
    // Direct Line Control Model
    USB_CDC_IFCLASS_DLCM = 0x01,
    // Abstract Control Model
    USB_CDC_IFCLASS_ACM = 0x02,
    // Telephone Control Model
    USB_CDC_IFCLASS_TCM = 0x03,
    // Multi-Channel Control Model
    USB_CDC_IFCLASS_MCCM = 0x04,
    // CAPI Control Model
    USB_CDC_IFCLASS_CAPICM = 0x05,
    // Ethernet Networking Control Model
    USB_CDC_IFCLASS_ETHERNET = 0x06,
    // ATM Networking Control Model
    USB_CDC_IFCLASS_ATM = 0x07,
} usb_iface_cdc_subclass_t;
#endif

/**
 * Header Functional Descriptor, which marks the beginning of the
 * concatenated set of functional descriptors for the interface.
 * USB CDC 1.1 spec, Table 26: Class-Specific Descriptor Header Format
 */
typedef struct
{
    /// Size of this descriptor in bytes
    uint8_t bFunctionLength;
    /// Descriptor type (USB_DT_CS_INTERFACE)
    usb_desc_type_t bDescriptorType : 8;
    /// Device sub-type (USB_CDC_CLASS_HEADER)
    usb_cdc_desc_subtype_t bDescriptorSubType : 8;
    /// USB CDC standard version in BCD
    uint16_t bcdCDC;
} usb_cdc_header_desc_t;

/**
 * Describes the processing of calls for the Communications Class interface.
 * CDC PSTN Subclass, 5.3.1: Call Management Functional Descriptor
 */
typedef struct
{
    /// Size of this descriptor in bytes
    uint8_t bFunctionLength;
    /// Descriptor type (USB_DT_CS_INTERFACE)
    usb_desc_type_t bDescriptorType : 8;
    /// Device sub-type (USB_CDC_CLASS_CALL_MGMT)
    usb_cdc_desc_subtype_t bDescriptorSubType : 8;
    /// The capabilities that this configuration supports, see USB_CDC_CALLMGMT_CAP()
    uint8_t bmCapabilities;
    /// Interface number of Data Class interface optionally used for call management
    uint8_t bDataInterface;
} usb_cdc_callmgmt_desc_t;

/**
 * @param callself Non-zero if device handles call management itself
 * @param dataif Non-zero if device can send/receive call management
 *      information over a Data Class interface (e.g. using AT-commands
 *      via the data channel), zero if only over the Communications Class
 *      interface.
 */
#define USB_CDC_CALLMGMT_CAP(callself, dataif) \
    (((callself) ? USB_CDC_CALLMGMT_CAP_CALLMGMT_SELF : 0) | \
     ((dataif) ? USB_CDC_CALLMGMT_CAP_CALLMGMT_DATA : 0))

#define USB_CDC_CALLMGMT_CAP_CALLMGMT_SELF      (0x01)
#define USB_CDC_CALLMGMT_CAP_CALLMGMT_DATA      (0x02)

/**
 * Describes the commands supported by the Communications Class interface
 * with the SubClass code of Abstract Control Model.
 * CDC PSTN Subclass, 5.3.2 Abstract Control Management Functional Descriptor
 */
typedef struct
{
    /// Size of this descriptor in bytes
    uint8_t bFunctionLength;
    /// Descriptor type (USB_DT_CS_INTERFACE)
    usb_desc_type_t bDescriptorType : 8;
    /// Device sub-type (USB_CDC_CLASS_ACM)
    usb_cdc_desc_subtype_t bDescriptorSubType : 8;
    /// Capabilities of the CDC ACM device, see USB_CDC_ACM_CAP()
    uint8_t bmCapabilities;
} usb_cdc_acm_desc_t;

/**
 * Build the value of usb_cdc_acm_desc_t.bmCapabilities field
 * @param comm Non-zero if device supports Set_Comm_Feature,
 *      Clear_Comm_Feature, Get_Comm_Feature
 * @param line Non-zero if device supports Set_Line_Coding,
 *      Set_Control_Line_State, Get_Line_Coding, Serial_State
 * @param brk Non-zero if device supports the request Send_Break
 * @param conn Non-zero if device supports the notification Network_Connection
 */
#define USB_CDC_ACM_CAP(comm, line, brk, conn) \
    (((comm) ? USB_CDC_ACM_CAP_COMM : 0) | \
     ((line) ? USB_CDC_ACM_CAP_LINE : 0) | \
     ((brk) ? USB_CDC_ACM_CAP_BREAK : 0) | \
     ((conn) ? USB_CDC_ACM_CAP_NETCONN : 0))

/// Supports Set_Comm_Feature, Clear_Comm_Feature, Get_Comm_Feature
#define USB_CDC_ACM_CAP_COMM                    0x01
/// Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding, Serial_State
#define USB_CDC_ACM_CAP_LINE                    0x02
/// Device supports the request Send_Break
#define USB_CDC_ACM_CAP_BREAK                   0x04
/// Device supports the notification Network_Connection
#define USB_CDC_ACM_CAP_NETCONN                 0x08

/**
 * The Union functional descriptor describes the relationship between
 * a group of interfaces that can be considered to form a functional unit.
 * USB CDC 1.1 spec, Table 33: Union Interface Functional Descriptor
 */
typedef struct
{
    /// Size of this descriptor in bytes
    uint8_t bFunctionLength;
    /// Descriptor type (USB_DT_CS_INTERFACE)
    usb_desc_type_t bDescriptorType : 8;
    /// Device sub-type (USB_CDC_CLASS_UNION)
    usb_cdc_desc_subtype_t bDescriptorSubType : 8;
    /// CDC interface number designated as the controlling interface
    uint8_t bMasterInterface;
    /// Interface numbers associated with controlling interface
    uint8_t bSlaveInterface [0];
} usb_cdc_union_desc_t;

/*
 * USB CDC request codes
 */
enum
{
    USB_REQ_CDC_SEND_ENCAPSULATED_COMMAND = 0x00,
    USB_REQ_CDC_GET_ENCAPSULATED_RESPONSE = 0x01,
    USB_REQ_CDC_SET_COMM_FEATURE = 0x02,
    USB_REQ_CDC_GET_COMM_FEATURE = 0x03,
    USB_REQ_CDC_CLEAR_COMM_FEATURE = 0x04,
    USB_REQ_CDC_SET_AUX_LINE_STATE = 0x10,
    USB_REQ_CDC_SET_HOOK_STATE = 0x11,
    USB_REQ_CDC_PULSE_SETUP = 0x12,
    USB_REQ_CDC_SEND_PULSE = 0x13,
    USB_REQ_CDC_SET_PULSE_TIME = 0x14,
    USB_REQ_CDC_RING_AUX_JACK = 0x15,
    USB_REQ_CDC_SET_LINE_CODING = 0x20,
    USB_REQ_CDC_GET_LINE_CODING = 0x21,
    USB_REQ_CDC_SET_CONTROL_LINE_STATE = 0x22,
    USB_REQ_CDC_SEND_BREAK = 0x23,
    USB_REQ_CDC_SET_RINGER_PARAMS = 0x30,
    USB_REQ_CDC_GET_RINGER_PARAMS = 0x31,
    USB_REQ_CDC_SET_OPERATION_PARAM = 0x32,
    USB_REQ_CDC_GET_OPERATION_PARAM = 0x33,
    USB_REQ_CDC_SET_LINE_PARAMS = 0x34,
    USB_REQ_CDC_GET_LINE_PARAMS = 0x35,
    USB_REQ_CDC_DIAL_DIGITS = 0x36,
    USB_REQ_CDC_SET_UNIT_PARAMETER = 0x37,
    USB_REQ_CDC_GET_UNIT_PARAMETER = 0x38,
    USB_REQ_CDC_CLEAR_UNIT_PARAMETER = 0x39,
    USB_REQ_CDC_SET_ETHERNET_MULTICAST_FILTERS = 0x40,
    USB_REQ_CDC_SET_ETHERNET_POW_PATTER_FILTER = 0x41,
    USB_REQ_CDC_GET_ETHERNET_POW_PATTER_FILTER = 0x42,
    USB_REQ_CDC_SET_ETHERNET_PACKET_FILTER = 0x43,
    USB_REQ_CDC_GET_ETHERNET_STATISTIC = 0x44,
    USB_REQ_CDC_SET_ATM_DATA_FORMAT = 0x50,
    USB_REQ_CDC_GET_ATM_DEVICE_STATISTICS = 0x51,
    USB_REQ_CDC_SET_ATM_DEFAULT_VC = 0x52,
    USB_REQ_CDC_GET_ATM_VC_STATISTICS = 0x53,
    USB_REQ_CDC_MDLM_SPECIFIC_REQUESTS_MASK = 0x7F,
};

/** SET_CONTROL_LINE_STATE bitflags */
typedef enum
{
    USB_CDC_LINE_STATE_DTR = 0x01,
    USB_CDC_LINE_STATE_RTS = 0x02,
} usb_req_cdc_set_control_line_state_t;

/** CDC ACM notification codes */
typedef enum
{
    /// Notification to host of network connection status
    USB_CDC_ACM_NOTIF_NETWORK_CONNECTION = 0x00,
    /// Notification to host to issue a GET_ENCAPSULATED_RESPONSE request
    USB_CDC_ACM_NOTIF_RESPONSE_AVAILABLE = 0x01,
    /// Returns the current state of the carrier detect, DSR, break, and ring signal
    USB_CDC_ACM_NOTIF_SERIAL_STATE = 0x20,
} usb_cdc_acm_notifications_t;

/** UART state bitmap flags for USB_CDC_ACM_NOTIF_SERIAL_STATE */
typedef enum
{
    /// Data Carrier Detected flag
    USB_CDC_ACM_SERIAL_STATE_DCD = 0x01,
    /// Data Set Ready flag
    USB_CDC_ACM_SERIAL_STATE_DSR = 0x02,
    /// A BREAK signal has been detected on line
    USB_CDC_ACM_SERIAL_STATE_BREAK = 0x04,
    /// The RING modem signal
    USB_CDC_ACM_SERIAL_STATE_RING = 0x08,
    /// A framing error has been detected
    USB_CDC_ACM_SERIAL_STATE_FRAMING = 0x10,
    /// A parity error has been detected
    USB_CDC_ACM_SERIAL_STATE_PARITY = 0x20,
    /// Some incoming data has been discarded due to overrun
    USB_CDC_ACM_SERIAL_STATE_OVERRUN = 0x40,
} usb_cdc_acm_notif_serial_state_t;

#pragma pack(pop)

#endif /* _USBCDC_H */
