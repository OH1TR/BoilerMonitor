#ifndef __USB_H
#define __USB_H

extern usbd_device *usbd_dev;

void usb_init(void);
void usb_poll(void);
void usb_set_nak(uint8_t addr, uint8_t nak);

#endif

