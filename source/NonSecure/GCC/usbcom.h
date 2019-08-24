#ifndef _USBCOM_H_
#define _USBCOM_H_


uint8_t USB_available();
void USB_flush();
char USB_read();
void USB_send(char c);
void USB_output(char character, void* arg);

#endif // _USBCOM_H_
