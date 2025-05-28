#ifndef _MAILBOX_H
#define _MAILBOX_H

#include "types.h"

#define MAILBOX_CLOCK_ID_EMMC	0x1

struct Framebuffer;

void mailbox_create_framebuffer(struct Framebuffer* fb);
u32 mailbox_get_power(u32 device_id, bool domain);
u32 mailbox_set_power(u32 device_id, u8 on, bool domain);
void mailbox_blank_screen(u32 state);
void* mailbox_get_vc_memory(void);
u32 mailbox_get_clock_rate(u32 clock_id);

#endif
