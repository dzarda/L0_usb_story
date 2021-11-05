#include "board.h"
#include "hal.h"

/* Virtual serial port over USB*/
SerialUSBDriver SDU1;

/*
 * Endpoints to be used for USBD1
 */
#define USBD1_DATA_REQUEST_EP 1
#define USBD1_DATA_AVAILABLE_EP 1
#define USBD1_INTERRUPT_REQUEST_EP 2

/*
 * USB Device Descriptor
 */
static const uint8_t device_descriptor_data[18] = {
    USB_DESC_DEVICE(0x0200, // bcdUSB (2.0)
                    0x02, // bDeviceClass (CDC)
                    0x00, // bDeviceSubClass
                    0x00, // bDeviceProtocol
                    0x40, // bMaxPacketSize
                    0x2C99, // idVendor
                    0x00FF, // idProduct
                    0x0000, // bcdDevice
                    1, // iManufacturer
                    2, // iProduct
                    3, // iSerialNumber
                    1) // bNumConfigurations
};

/*
 * Device Descriptor wrapper
 */
static const USBDescriptor device_descriptor = {sizeof device_descriptor_data,
                                                device_descriptor_data};

// Configuration Descriptor tree for CDC
static const uint8_t configuration_descriptor_data[67] = {
    // Configuration Descriptor
    USB_DESC_CONFIGURATION(67, // wTotalLength
                           0x02, // bNumInterfaces
                           0x01, // bConfigurationValue
                           0, // iConfiguration
                           0xC0, // bmAttributes (self powered)
                           50), // bMaxPower (100mA)
    // Interface Descriptor
    USB_DESC_INTERFACE(0x00, // bInterfaceNumber
                       0x00, // bAlternateSetting
                       0x01, // bNumEndpoints
                       0x02, // bInterfaceClass (Communications Interface Class, CDC section 42)
                       0x02, // bInterfaceSubClass (Abstract Control Model, CDC section 43)
                       0x01, // bInterfaceProtocol (AT commands, CDC section 44)
                       0), // iInterface
    // Header Functional Descriptor (CDC section 523)
    USB_DESC_BYTE(5), // bLength
    USB_DESC_BYTE(0x24), // bDescriptorType (CS_INTERFACE)
    USB_DESC_BYTE(0x00), // bDescriptorSubtype (Header Functional Descriptor)
    USB_DESC_BCD(0x0110), // bcdCDC
    // Call Management Functional Descriptor
    USB_DESC_BYTE(5), // bFunctionLength
    USB_DESC_BYTE(0x24), // bDescriptorType (CS_INTERFACE)
    USB_DESC_BYTE(0x01), // bDescriptorSubtype (Call Management Functional Descriptor)
    USB_DESC_BYTE(0x00), // bmCapabilities (D0+D1)
    USB_DESC_BYTE(0x01), // bDataInterface
    // ACM Functional Descriptor
    USB_DESC_BYTE(4), // bFunctionLength
    USB_DESC_BYTE(0x24), // bDescriptorType (CS_INTERFACE)
    USB_DESC_BYTE(0x02), // bDescriptorSubtype (Abstract Control Management Descriptor)
    USB_DESC_BYTE(0x02), // bmCapabilities
    // Union Functional Descriptor
    USB_DESC_BYTE(5), // bFunctionLength
    USB_DESC_BYTE(0x24), // bDescriptorType (CS_INTERFACE)
    USB_DESC_BYTE(0x06), // bDescriptorSubtype (Union
    // Functional Descriptor
    USB_DESC_BYTE(0x00), // bMasterInterface (Communication Class Interface)
    USB_DESC_BYTE(0x01), // bSlaveInterface0 (Data Class Interface)
    // Endpoint 2 Descriptor
    USB_DESC_ENDPOINT(USBD1_INTERRUPT_REQUEST_EP | 0x80, 0x03, // bmAttributes (Interrupt)
                      0x0008, // wMaxPacketSize
                      0xFF), // bInterval
    // Interface Descriptor
    USB_DESC_INTERFACE(0x01, // bInterfaceNumber
                       0x00, // bAlternateSetting
                       0x02, // bNumEndpoints
                       0x0A, // bInterfaceClass (Data Class Interface, CDC section 45)
                       0x00, // bInterfaceSubClass (CDC section 46)
                       0x00, // bInterfaceProtocol (CDC section 47)
                       0x00), // iInterface
    // Endpoint 3 Descriptor
    USB_DESC_ENDPOINT(USBD1_DATA_AVAILABLE_EP, // bEndpointAddress
                      0x02, // bmAttributes (Bulk)
                      0x0040, // wMaxPacketSize
                      0x00), // bInterval
    // Endpoint 1 Descriptor
    USB_DESC_ENDPOINT(USBD1_DATA_REQUEST_EP | 0x80, // bEndpointAddress*/
                      0x02, // bmAttributes (Bulk)
                      0x0040, // wMaxPacketSize
                      0x00), // bInterval
};

/*
 * Configuration Descriptor wrapper
 */
static const USBDescriptor configuration_descriptor = {sizeof configuration_descriptor_data,
                                                       configuration_descriptor_data};

/*
 * US English language identifier
 */
static const uint8_t string0[] = {
    USB_DESC_BYTE(4), // bLength
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING), // bDescriptorType
    USB_DESC_WORD(0x0409) // wLANGID (US English)
};

/*
 * Vendor string
 */
static const uint8_t string1[] = {USB_DESC_BYTE(58), /* bLength */
                                  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType */
                                  /* Prusa Research (prusa3d.com) */
                                  'P', 0, 'r', 0, 'u', 0, 's', 0, 'a', 0, ' ', 0, 'R', 0, 'e', 0,
                                  's', 0, 'e', 0, 'a', 0, 'r', 0, 'c', 0, 'h', 0, ' ', 0, '(', 0,
                                  'p', 0, 'r', 0, 'u', 0, 's', 0, 'a', 0, '3', 0, 'd', 0, '.', 0,
                                  'c', 0, 'o', 0, 'm', 0, ')', 0};

/*
 * Device Description string
 */
static const uint8_t string2[] = {USB_DESC_BYTE(38), /* bLength */
                                  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType */
                                  /* Original Prusa Pen */
                                  'O', 0, 'r', 0, 'i', 0, 'g', 0, 'i', 0, 'n', 0, 'a', 0, 'l', 0,
                                  ' ', 0, 'P', 0, 'r', 0, 'u', 0, 's', 0, 'a', 0, ' ', 0, 'B', 0,
                                  'l', 0, 'a', 0};

/*
 * Serial Number string
 */
static const uint8_t string3[] = {USB_DESC_BYTE(8), /* bLength */
                                  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType */
                                  '0' + CH_KERNEL_MAJOR,
                                  0,
                                  '0' + CH_KERNEL_MINOR,
                                  0,
                                  '0' + CH_KERNEL_PATCH,
                                  0};

/*
 * Strings wrappers array
 */
static const USBDescriptor strings[] = {{sizeof string0, string0},
                                        {sizeof string1, string1},
                                        {sizeof string2, string2},
                                        {sizeof string3, string3}};

/*
 * Handles the GET_DESCRIPTOR callback All required descriptors must be
 * handled here
 */
static const USBDescriptor *get_descriptor(USBDriver *usbp, uint8_t dtype, uint8_t dindex,
                                           uint16_t lang) {

    (void)usbp;
    (void)lang;
    switch (dtype) {
    case USB_DESCRIPTOR_DEVICE:
        return &device_descriptor;
    case USB_DESCRIPTOR_CONFIGURATION:
        return &configuration_descriptor;
    case USB_DESCRIPTOR_STRING:
        if (dindex < 4)
            return &strings[dindex];
    }
    return NULL;
}

/**
 * @brief   IN EP1 state
 */
static USBInEndpointState ep1instate;

/**
 * @brief   OUT EP1 state
 */
static USBOutEndpointState ep1outstate;

/**
 * @brief   EP1 initialization structure (both IN and OUT)
 */
static const USBEndpointConfig ep1config = {USB_EP_MODE_TYPE_BULK,
                                            NULL,
                                            sduDataTransmitted,
                                            sduDataReceived,
                                            0x0040,
                                            0x0040,
                                            &ep1instate,
                                            &ep1outstate,
                                            2,
                                            NULL};

/**
 * @brief   IN EP2 state
 */
static USBInEndpointState ep2instate;

/**
 * @brief   EP2 initialization structure (IN only)
 */
static const USBEndpointConfig ep2config = {USB_EP_MODE_TYPE_INTR,
                                            NULL,
                                            sduInterruptTransmitted,
                                            NULL,
                                            0x0010,
                                            0x0000,
                                            &ep2instate,
                                            NULL,
                                            1,
                                            NULL};

/*
 * Handles the USB driver global events
 */
static void usb_event(USBDriver *usbp, usbevent_t event) {
    extern SerialUSBDriver SDU1;

    switch (event) {
    case USB_EVENT_ADDRESS:
        return;
    case USB_EVENT_CONFIGURED:
        chSysLockFromISR();

        /* Enables the endpoints specified into the configuration
           Note, this callback is invoked from an ISR so I-Class functions
           must be used*/
        usbInitEndpointI(usbp, USBD1_DATA_REQUEST_EP, &ep1config);
        usbInitEndpointI(usbp, USBD1_INTERRUPT_REQUEST_EP, &ep2config);

        /* Resetting the state of the CDC subsystem*/
        sduConfigureHookI(&SDU1);

        chSysUnlockFromISR();
        return;
    case USB_EVENT_RESET:
        /* Falls into*/
    case USB_EVENT_UNCONFIGURED:
        /* Falls into*/
    case USB_EVENT_SUSPEND:
        chSysLockFromISR();

        /* Disconnection event on suspend*/
        sduSuspendHookI(&SDU1);

        chSysUnlockFromISR();
        return;
    case USB_EVENT_WAKEUP:
        chSysLockFromISR();

        /* Connection event on wakeup*/
        sduWakeupHookI(&SDU1);

        chSysUnlockFromISR();
        return;
    case USB_EVENT_STALLED:
        return;
    }
    return;
}

/*
 * Handles the USB driver global events
 */
static void sof_handler(USBDriver *usbp) {

    (void)usbp;

    osalSysLockFromISR();
    sduSOFHookI(&SDU1);
    osalSysUnlockFromISR();
}

/*
 * USB driver configuration
 */
USBConfig usbCfg = {
    .event_cb = usb_event,
    .get_descriptor_cb = get_descriptor,
    .requests_hook_cb = sduRequestsHook,
    .sof_cb = sof_handler
};

/*
 * Serial over USB driver configuration
 */
SerialUSBConfig serUsbCfg = {
    .usbp = &USBD1,
    .bulk_in = USBD1_DATA_REQUEST_EP,
    .bulk_out = USBD1_DATA_AVAILABLE_EP,
    .int_in = USBD1_INTERRUPT_REQUEST_EP
};
