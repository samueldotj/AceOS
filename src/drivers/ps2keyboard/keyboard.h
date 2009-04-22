/*!
  \file		drivers/ps2keyboard/keyboard.h
  \brief	ps2 keyboard structures and function declarations
*/

#ifndef _PS2_KEYBOARD_H
#define _PS2_KEYBOARD_H

#include <ace.h>

/* If u change this value, the data type of the variables cur_kbd_buf_read_index and cur_kbd_buf_write_index should be suitably modified */
#define MAX_KEYBOARD_BUFFER_SIZE 256
#define KEYBOARD_INTERRUPT_NUMBER 1

#define KEYBOARD_CONTROLLER_DATA_PORT		0x60
#define KEYBOARD_CONTROLLER_CONTROL_PORT	0x64
#define MAX_SCANCODE_BYTES	6

typedef struct keyboard_buffer
{
	unsigned char scancode[MAX_SCANCODE_BYTES]; /* Most recent scan code entry will be in 0 */
	unsigned char keycode;
	unsigned char ascii_value;
}KEYBOARD_BUFFER, *KEYBOARD_BUFFER_PTR;

static ERROR_CODE AddDevice(DRIVER_OBJECT_PTR drv_obj, DEVICE_OBJECT_PTR parent_dev_obj);
static ERROR_CODE MajorFunctionPnP(DEVICE_OBJECT_PTR dev_obj, IRP_PTR irp);
static ERROR_CODE MajorFunctionRead(DEVICE_OBJECT_PTR dev_obj, IRP_PTR irp);
static ERROR_CODE MajorFunctionFlush(DEVICE_OBJECT_PTR dev_obj, IRP_PTR irp);
static ISR_RETURN_CODE KeyboardInterruptHandler(INTERRUPT_INFO_PTR interrupt_info, void * arg);
static ERROR_CODE ReadFromKeyboardBuffer(unsigned char **scancode, unsigned char *keycode, unsigned char *ascii_value);
static void WriteToKeyboardBuffer(unsigned char* scancode, unsigned char keycode, unsigned char ascii_value);

#define SCANCODE_LEFT_SHIFT_PRESS		0x2A
#define SCANCODE_LEFT_SHIFT_RELEASE		0xAA

#define SCANCODE_RIGHT_SHIFT_PRESS		0x36
#define SCANCODE_RIGHT_SHIFT_RELEASE	0xB6

#define SCANCODE_CAPS_LOCK_PRESS		0x3A
#define SCANCODE_CAPS_LOCK_RELEASE		0xBA

#define SCANCODE_NUM_LOCK_PRESS			0x45
#define SCANCODE_NUM_LOCK_RELEASE		0xC5

#define SCANCODE_ENTER					0x1C

#define SCANCODE_CTRL_PRESS				0x1D
#define SCANCODE_CTRL_RELEASE			0x9D

#define SCANCODE_ALT_PRESS				0x38
#define SCANCODE_ALT_RELEASE			0xB8


enum KEYCODE
{
	/* Keycodes */
	KEYCODE_UP_ARROW = 128,
	KEYCODE_DOWN_ARROW,
	KEYCODE_LEFT_ARROW,	
	KEYCODE_RIGHT_ARROW,	
	KEYCODE_F1,
	KEYCODE_F2,
	KEYCODE_F3,
	KEYCODE_F4,
	KEYCODE_F5,
	KEYCODE_F6,
	KEYCODE_F7,
	KEYCODE_F8,
	KEYCODE_F9,
	KEYCODE_F10,
	KEYCODE_F11,
	KEYCODE_F12,
	KEYCODE_PAUSE_BREAK,
	KEYCODE_WIN_START,
	KEYCODE_INSERT,
	KEYCODE_HOME,
	KEYCODE_PAGEUP,
	KEYCODE_END,
	KEYCODE_PAGEDOWN,
	KEYCODE_NUM_LOCK,
	KEYCODE_SCROLL_LOCK,
	KEYCODE_CAPS_LOCK,
	KEYCODE_LEFT_SHIFT,
	KEYCODE_RIGHT_SHIFT,
	KEYCODE_LEFT_ALT,
	KEYCODE_RIGHT_ALT,
	KEYCODE_LEFT_CTRL,
	KEYCODE_RIGHT_CTRL,
	KEYCODE_SYSRQ,
	KEYCODE_LEFT_WIN,
	KEYCODE_RIGHT_WIN,
	KEYCODE_NUMERIC_HOME,
	KEYCODE_NUMERIC_UP_ARROW,
	KEYCODE_NUMERIC_PAGEUP,
	KEYCODE_NUMERIC_LEFT_ARROW,
	KEYCODE_NUMERIC_RIGHT_ARROW,
	KEYCODE_NUMERIC_END,
	KEYCODE_NUMERIC_DOWN_ARROW,
	KEYCODE_NUMERIC_PAGEDOWN,
	KEYCODE_NUMERIC_INSERT,
	KEYCODE_NUMERIC_DELETE	//172
};

#endif
