
#include "board.h"
#include "usb.hpp"

#include "ch.h"
#include "hal.h"

int main() {
    stm32_clock_init();
    chSysInit();
    halInit();
    
    rccEnableIOP(RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN, true);

    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serUsbCfg);

    usbDisconnectBus(serUsbCfg.usbp);
    chThdSleepMilliseconds(1500);
    usbStart(serUsbCfg.usbp, &usbCfg);
    usbConnectBus(serUsbCfg.usbp);

    while (true) {
    }
}
