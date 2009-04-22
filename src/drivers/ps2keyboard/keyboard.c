/*!
	\file	drivers/keyboard.c
	\brief	PS/2 keyboard driver for 101-key keyboard
*/
#include <ace.h>
#include <string.h>
#include <kernel/io.h>
#include <kernel/error.h>
#include <kernel/iom/iom.h>
#include <kernel/interrupt.h>
#include <kernel/printf.h>
#include "keyboard.h"

KEYBOARD_BUFFER kbd_buf[MAX_KEYBOARD_BUFFER_SIZE];

/* These 2 variables are dependent on the size of the macro MAX_KEYBOARD_BUFFER_SIZE */
int cur_kbd_buf_read_index=0;
int cur_kbd_buf_write_index=0;

/* www.microsoft.com/whdc/archive/scancode.mspx
 * http://www.osdever.net/bkerndev/Docs/keyboard.htm
 */
/* This is a scancode table used to layout a standard US keyboard.
 * Index with a scan code to get the corresponding ascii value.
 * First column contains ascii values or key codes WITHOUT SHIFT key pressed and 2nd column is with SHIFT key pressed.
 */
unsigned char scancode_to_ascii[][2] =
{
	{0, 0},
	{27, 27}, 						/* Escape */
	{'1', '!'},
	{'2', '@'},
	{'3', '#'},
	{'4', '$'},
	{'5', '%'},
	{'6', '^'},
	{'7', '&'},
	{'8', '*'},
	{'9', '('},
	{'0', ')'},
	{'-', '_'},
	{'=', '+'},
	{'\b', '\b'},					/* Backspace */
	{'\t', '\t'},					/* Tab */
	{'q', 'Q'},
	{'w', 'W'},
	{'e', 'E'},
	{'r', 'R'},					/* 19 */
	{'t', 'T'},
	{'y', 'Y'},
	{'u', 'U'},
	{'i', 'I'},
	{'o', 'O'},
	{'p', 'P'},
	{'[', '{'},
	{']', '}'},
	{'\n', '\n'},					/* Enter 1c*/
	{KEYCODE_LEFT_CTRL, 0},		/* 1d Left ctrl*/
	{'a', 'A'},
	{'s', 'S'},
	{'d', 'D'},
	{'f', 'F'},
	{'g', 'G'},
	{'h', 'H'},
	{'j', 'J'},
	{'k', 'K'},
	{'l', 'L'},
	{';', ':'},					/* 0x27 */
	{'\'','"'},
	{'`', '~'},					/*0x29 is available on US kbd and undefined on international keyboard */
	{KEYCODE_LEFT_SHIFT, 0},		/* Left shift */
	{'\\',  '|'},						/* 0x2b not available on US kbd */
	{'z', 'Z'},
	{'x', 'X'},
	{'c', 'C'},
	{'v', 'V'},
	{'b', 'B'},
	{'n', 'N'},					/* 0x31 */
	{'m', 'M'},
	{',', '<'},
	{'.', '>'},
	{'/', '?'},
	{KEYCODE_RIGHT_SHIFT, 0},		/* Right shift */
	{'*', '*'},					/* 0x37 Numeric * */
	{KEYCODE_LEFT_ALT, 0},		/* Left Alt */
	{' ',  ' '},					/* Space bar */
	{KEYCODE_CAPS_LOCK,  0},		/* 0x3A Caps lock */
	{KEYCODE_F1, 0},				/* 0x3B - F1 key ... > */
	{KEYCODE_F2, 0},
	{KEYCODE_F3, 0},
	{KEYCODE_F4, 0},
	{KEYCODE_F5, 0},
	{KEYCODE_F6, 0},
	{KEYCODE_F7, 0},
	{KEYCODE_F8, 0},
	{KEYCODE_F9, 0},
	{KEYCODE_F10, 0},				/* < 0x44 F10 */
	{KEYCODE_NUM_LOCK, 0},		/* 0x45  Num lock*/
	{KEYCODE_SCROLL_LOCK, 0},	/* Scroll Lock */
	{KEYCODE_NUMERIC_HOME, '7'},	/* 0x47  Numeric 7 */
	{KEYCODE_NUMERIC_UP_ARROW, '8'},		/* Numeric 8 */
	{KEYCODE_NUMERIC_PAGEUP, '9'},		/* Numeric 9 */
	{'-', '-'},						/* Numeric - */
	{KEYCODE_NUMERIC_LEFT_ARROW, '4'},	/* Numeric 4 */	
	{0, '5'},						/* Numeric 5 */
	{KEYCODE_NUMERIC_RIGHT_ARROW, '6'},   /* 0x4D Numeric 6 */
	{'+', '+'},					/* Numeric + */
	{KEYCODE_NUMERIC_END, '1'},			/* Numeric 1 */
	{KEYCODE_NUMERIC_DOWN_ARROW, '2'},	/* 0x50 Numeric 2 */
	{KEYCODE_NUMERIC_PAGEDOWN, '3'},		/* Numeric 3 */
	{KEYCODE_NUMERIC_INSERT, '0'},		/* Numeric 0 */
	{127, '.'}					/* 0x53 Numeric DELETE, Numeric . Take care of special case: numlock_DEL, there will be no modifier key for this*/
};

/*!
 * \brief	Entry point - called during driver initialization
 * \param	drv_obj	Driver object pointer which should be initialised
 * Returns standard error code.
 */
ERROR_CODE DriverEntry(DRIVER_OBJECT_PTR drv_obj)
{
	strcpy( drv_obj->driver_name, "Keyboard" );
	drv_obj->driver_extension = NULL;
	drv_obj->fn.AddDevice = AddDevice;
	drv_obj->fn.MajorFunctions[IRP_MJ_READ] = MajorFunctionRead;
	drv_obj->fn.MajorFunctions[IRP_MJ_PNP] = MajorFunctionPnP;
	drv_obj->fn.MajorFunctions[IRP_MJ_FLUSH_BUFFERS] = MajorFunctionFlush;

	/* Register keyboard interrupt handler */
	InstallInterruptHandler( KEYBOARD_INTERRUPT_NUMBER, KeyboardInterruptHandler, 0);
	return ERROR_SUCCESS;
}

/*!
 * \brief	Adds a new device to the device hierarchy and binds the device with the given driver.
 * \param	drv_obj	Driver object to which the new device must be associated.
 * \param	parent_dev_obj	Device object's parent pointer.
 * Returns a standard error code
 */
static ERROR_CODE AddDevice(DRIVER_OBJECT_PTR drv_obj, DEVICE_OBJECT_PTR parent_dev_obj)
{
	DEVICE_OBJECT_PTR dev_obj;
	ERROR_CODE err;

	err = CreateDevice(drv_obj, 0, &dev_obj);
	if( err != ERROR_SUCCESS )
		return err;

	/* Now establish the hierarchy between parent_dev_obj and dev_obj */
	(void)AttachDeviceToDeviceStack(dev_obj, parent_dev_obj);

	return ERROR_SUCCESS;
}

/*!
 * \brief	Function to handle all Plug and play activities associated with keyboard.
 * \param	dev_obj	Device for which request is made.
 * \param	irp		Interrupt request packet pointer containing user request details
 * Returns SUCCESS if request can be processed or else suitable failure code is returned.
 */
static ERROR_CODE MajorFunctionPnP(DEVICE_OBJECT_PTR dev_obj, IRP_PTR irp)
{
	return ERROR_SUCCESS;
}

/*!
 * \brief	Function to handle Read activities activities.
 * \param	dev_obj	Device for which request is made.
 * \param	irp		Interrupt request packet pointer containing user request details
 * Returns SUCCESS if request can be processed or else suitable failure code is returned.
 */
static ERROR_CODE MajorFunctionRead(DEVICE_OBJECT_PTR dev_obj, IRP_PTR irp)
{
	unsigned char *scancode, *keycode, *ascii;

	irp->io_status.information = kmalloc(sizeof(KEYBOARD_BUFFER), 0);
	if ( irp->io_status.information  == NULL )
	{
		irp->io_status.status = ERROR_NOT_ENOUGH_MEMORY;
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	scancode = irp->io_status.information;
	keycode = scancode + MAX_SCANCODE_BYTES;
	ascii = keycode + 1;
	return ReadFromKeyboardBuffer(&scancode, keycode, ascii);
}

/*!
 * \brief	Read a BYTE from keyboard buffer.
 * \param	ch	Pointer to a BYTE at which the read data has to be placed.
 * Returns ERROR_READ_FAULT if there is no data available or else returns ERROR_SUCCESS.
 */
static ERROR_CODE ReadFromKeyboardBuffer(unsigned char **scancode, unsigned char *keycode, unsigned char *ascii_value)
{
	int i;
	if(cur_kbd_buf_read_index == cur_kbd_buf_write_index)
		return ERROR_READ_FAULT;

	*ascii_value = kbd_buf[cur_kbd_buf_read_index].ascii_value;
	*keycode = kbd_buf[cur_kbd_buf_read_index].keycode;
	for(i=0; i < MAX_SCANCODE_BYTES; i++)
	{
		*scancode[i] = kbd_buf[cur_kbd_buf_read_index].scancode[i];
	}
	cur_kbd_buf_read_index = (cur_kbd_buf_read_index + 1) % MAX_KEYBOARD_BUFFER_SIZE;

	return ERROR_SUCCESS;
}

/*!
 * \brief	Writes a BYTE to keyboard buffer. Called from interrupt handler as soon as data is entered on keyboard.
 * \param	ch	BYTE data which has to be written into the keyboard buffer.
 */

static void WriteToKeyboardBuffer(unsigned char* scancode, unsigned char keycode, unsigned char ascii_value)
{
	int i;

	kbd_buf[cur_kbd_buf_write_index].ascii_value = ascii_value;
	kbd_buf[cur_kbd_buf_write_index].keycode = keycode;
	for(i=0; i < MAX_SCANCODE_BYTES; i++)
	{
		kbd_buf[cur_kbd_buf_write_index].scancode[i] = scancode[i];
	}

	cur_kbd_buf_write_index = (cur_kbd_buf_write_index + 1) % MAX_KEYBOARD_BUFFER_SIZE;

	if(cur_kbd_buf_write_index == cur_kbd_buf_read_index)
		cur_kbd_buf_read_index = (cur_kbd_buf_read_index + 1) % MAX_KEYBOARD_BUFFER_SIZE;
}

/*!
 * \brief	Resets the keyboard buffer by flushing it. This just resets the read and write pointers to the keyboard buffer.
 * \param	dev_obj	Device for which request is made.
 * \param	irp		Interrupt request packet pointer containing user request details
 * Returns SUCCESS if request can be processed or else suitable failure code is returned.
 */
static ERROR_CODE MajorFunctionFlush(DEVICE_OBJECT_PTR dev_obj, IRP_PTR irp)
{
	cur_kbd_buf_read_index = cur_kbd_buf_write_index = 0;
	return ERROR_SUCCESS;
}

/*!
 * \brief	Handles all interrupts from keyboard device.
 * \brief	interrupt_info	Contains details on this interrupt: number, device, priority level
 * \brief	arg				Contains optional arguments. It's NULL for keyboard device.
 */
static ISR_RETURN_CODE KeyboardInterruptHandler(INTERRUPT_INFO_PTR interrupt_info, void * arg)
{
	unsigned char scancode;
	unsigned char ascii_value=0;
	static BYTE key_e0=0;
	static unsigned char scancode_buffer[MAX_SCANCODE_BYTES]; /*Usage of this variable is incomplete. Somebody do it! */
	char keycode;
	int col;
	static unsigned char modifier_keys=0; /* bit 0=left SHIFT, 1=right SHIFT, 2=left ALT, 3=right ALT, 4=left CTRL, 5=right CTRL, 6=CAPS lock 7=NUM lock*/
#define LEFT_SHIFT_BIT	1
#define RIGHT_SHIFT_BIT	2
#define LEFT_ALT_BIT	4
#define	RIGHT_ALT_BIT	8
#define LEFT_CTRL_BIT	16
#define RIGHT_CTRL_BIT	32
#define CAPS_LOCK_BIT	64
#define NUM_LOCK_BIT	128

	scancode = _inp(KEYBOARD_CONTROLLER_DATA_PORT);
	
	if(scancode == 0xE0)
	{
		key_e0 = 1;
		return ISR_END_PROCESSING;
	}

	if(key_e0 == 1)
	{
		key_e0 = 0;
		/* These scancodes have special meaning, so do not search in scancode_to_ascii table
		 * This list is not exhaustive. Many other cases are ignored. Ex: LEFT_SHIFT + RIGHT_SHIFT + INSERT = 2A, 36, E0AA EOB6 E052
		 * In that case, one has to record what was the previous action with E0... coz we have now made e0=0.
		 * If there's anybody without any damn work, then scrath this hard.
		 */
		switch(scancode)
		{
			case 0x52:	keycode = KEYCODE_INSERT;	break;
			case 0x53:	keycode = modifier_keys; ascii_value = 127;	break;
			case 0x4B:	keycode = KEYCODE_LEFT_ARROW;	break;
			case 0x47:	keycode = KEYCODE_HOME;	break;
			case 0x4F:	keycode = KEYCODE_END;	break;
			case 0x48:	keycode = KEYCODE_UP_ARROW;	break;
			case 0x50:	keycode = KEYCODE_DOWN_ARROW;	break;
			case 0x49:	keycode = KEYCODE_PAGEUP;	break;
			case 0x51:	keycode = KEYCODE_PAGEDOWN;	break; 
			case 0x4D:	keycode = KEYCODE_RIGHT_ARROW;	break;
			case 0x5B:	keycode = KEYCODE_LEFT_WIN;	break;
			case 0x5C:	keycode = KEYCODE_RIGHT_WIN; break;
			case SCANCODE_ENTER: keycode = modifier_keys; ascii_value = '\n'; break; 
			case SCANCODE_ALT_PRESS: modifier_keys |= RIGHT_ALT_BIT; goto modifier_keys_only; 
			case SCANCODE_ALT_RELEASE: modifier_keys &= ~(RIGHT_ALT_BIT); goto modifier_keys_only; 
			case SCANCODE_CTRL_PRESS: modifier_keys |= RIGHT_CTRL_BIT; goto modifier_keys_only; 
			case SCANCODE_CTRL_RELEASE: modifier_keys &= ~(RIGHT_CTRL_BIT); goto modifier_keys_only; 
			default:
				/* Unknown key typed.. so skip it */
				return ISR_END_PROCESSING;
		}
		scancode_buffer[0] = scancode;
		scancode_buffer[1] = 0xE0;
		if(modifier_keys)
			scancode_buffer[2] = modifier_keys;
		goto send_buffer;
	}

	switch(scancode)
	{
		case SCANCODE_LEFT_SHIFT_PRESS: 	modifier_keys |= LEFT_SHIFT_BIT;	break;
		case SCANCODE_LEFT_SHIFT_RELEASE:	modifier_keys &= ~(LEFT_SHIFT_BIT);	break;
		case SCANCODE_RIGHT_SHIFT_PRESS:	modifier_keys |= RIGHT_SHIFT_BIT;	break;
		case SCANCODE_RIGHT_SHIFT_RELEASE:	modifier_keys &= ~(RIGHT_SHIFT_BIT);break;
		case SCANCODE_ALT_PRESS:
			modifier_keys |= LEFT_ALT_BIT;
			break;
		case SCANCODE_ALT_RELEASE:
			modifier_keys &= ~(LEFT_ALT_BIT);
			break;
		case SCANCODE_CTRL_PRESS:
			modifier_keys |= LEFT_CTRL_BIT;
			break;
		case SCANCODE_CTRL_RELEASE:
			modifier_keys &= ~(LEFT_CTRL_BIT);
			break;
		case SCANCODE_CAPS_LOCK_PRESS:
			if(modifier_keys & CAPS_LOCK_BIT)
				modifier_keys &= ~(CAPS_LOCK_BIT);
			else
				modifier_keys |= CAPS_LOCK_BIT;
			break;

		case SCANCODE_CAPS_LOCK_RELEASE:
			break;

		case SCANCODE_NUM_LOCK_PRESS:
			if(modifier_keys & NUM_LOCK_BIT)
				modifier_keys &= ~(NUM_LOCK_BIT);
			else
				modifier_keys |= NUM_LOCK_BIT;
			break;

		case SCANCODE_NUM_LOCK_RELEASE:
			break;

		default: /* This is not a modifier key, but an ascii key */
			goto find_ascii;
	}

modifier_keys_only:
	return ISR_END_PROCESSING; /* Return if scan code is a modifier key */

find_ascii:
	ascii_value = 0;
	keycode = modifier_keys;
	
	/* Convert scan code to ascii and store it in keyboard buffer. For that select the index(column) into scancode table. */
	if(scancode <= 0x53)
	{
		if( modifier_keys & LEFT_SHIFT_BIT || (modifier_keys & RIGHT_SHIFT_BIT) )
		{
			if (modifier_keys & NUM_LOCK_BIT)
			{
				if ( scancode==0x37 || (scancode>=0x47 && scancode<=0x53) )
				{
					col = 0;
				}
				else
					col = 1;
			}
			else
				col = 1;
		}
		else
		{
			if ( (modifier_keys & NUM_LOCK_BIT) && (scancode==0x37 || (scancode>=0x47 && scancode<=0x53) ) )
				col = 1;
			else
				col = 0;
		}

		ascii_value = scancode_to_ascii[scancode][col];

		if( (modifier_keys & CAPS_LOCK_BIT) && ascii_value >=97 && ascii_value <= 122) /* get the UPPER case */
		{
				ascii_value -= 32;
		}
		else if( (modifier_keys & CAPS_LOCK_BIT) && ascii_value >=65 && ascii_value <= 90) /* get the LOWER case */
		{
				ascii_value += 32;
		}
		
	}
	else
		return ISR_END_PROCESSING;

send_buffer:
	/* if ascii_value > 127, then user should check for keycode also. That would show that it's one of NUMERIC keys. */
	WriteToKeyboardBuffer(scancode_buffer, keycode, ascii_value);
	kprintf("Wrote to keyboard buffer\n");
	
	return ISR_END_PROCESSING; 
}

