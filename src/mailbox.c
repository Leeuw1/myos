#include "mailbox.h"
#include "io.h"
#include <string.h>

#define MAILBOX0		((const volatile struct MailboxRegs*)	0x3f00b880)
#define MAILBOX1		((volatile struct MailboxRegs*)			0x3f00b8a0)
#define MAILBOX_FULL	(1 << 31)
#define MAILBOX_EMPTY	(1 << 30)

#define LED_PIN_STATUS	42
#define LED_PIN_POWER	130

#define MAILBOX_CHANNEL_PROPERTY_TAGS	8

#define REQUEST_SUCCESS 0x80000000
#define REQUEST_ERROR	0x80000001

#define MAILBOX_TAG_SET_LED_STATUS				0x00038041

#define MAILBOX_TAG_GET_PHYSICAL_WIDTH_HEIGHT	0x00040003
#define MAILBOX_TAG_SET_PHYSICAL_WIDTH_HEIGHT	0x00048003
#define MAILBOX_TAG_GET_VIRTUAL_WIDTH_HEIGHT	0x00040004
#define MAILBOX_TAG_SET_VIRTUAL_WIDTH_HEIGHT	0x00048004
#define MAILBOX_TAG_SET_DEPTH					0x00048005
#define MAILBOX_TAG_GET_PITCH					0x00040008
#define MAILBOX_TAG_SET_ALPHA_MODE				0x00048007
#define MAILBOX_TAG_SET_PIXEL_ORDER				0x00048006
#define MAILBOX_TAG_GET_VIRTUAL_OFFSET			0x00040009
#define MAILBOX_TAG_GET_OVERSCAN				0x0004000a
#define MAILBOX_TAG_BLANK_SCREEN				0x00040002
#define MAILBOX_TAG_ALLOCATE					0x00040001
#define MAILBOX_TAG_GET_COMMAND_LINE			0x00050001
#define MAILBOX_TAG_GET_POWER_STATE				0x00020001
#define MAILBOX_TAG_SET_POWER_STATE				0x00028001
#define MAILBOX_TAG_GET_POWER_DOMAIN_STATE		0x00030030
#define MAILBOX_TAG_SET_POWER_DOMAIN_STATE		0x00038030
#define MAILBOX_TAG_GET_VC_MEMORY				0x00010006
#define MAILBOX_TAG_GET_CLOCK_RATE				0x00030002

struct MailboxRegs {
	u32	rw;
	u32	_pad[3];
	u32	peek;
	u32	sender;
	u32	status;
	u32	config;
} __attribute__((packed));

u32 mailbox_read(u8 channel) {
	u32 data;
	while (1) {
		while (MAILBOX0->status & MAILBOX_EMPTY) {
		}
		data = MAILBOX0->rw;
		u8 c = (u8)(data & 0xf);
		if (c == channel) {
			return data >> 4;
		}
	}
}

void mailbox_write(u8 channel, volatile void* address) {
	while (MAILBOX0->status & MAILBOX_FULL) {
	}
	MAILBOX1->rw = (u32)(u64)address | (u32)channel;
}

struct TagHeader {
	u32 tag;
	u32 size;
	u32 code;
} __attribute__((packed));

union AllocateInfo {
	struct {
		u32 alignment;
		u32 _pad;
	} request;
	struct {
		u32 address;
		u32 size;
	} response;
} __attribute__((packed));

struct FramebufferInfo {
	u32	size;
	u32 code;
	struct TagHeader th0;
	u32 width0;
	u32 height0;
	struct TagHeader th1;
	u32 width1;
	u32 height1;
	struct TagHeader th2;
	u32 depth;
	struct TagHeader th3;
	u32 alpha_mode;
	struct TagHeader th4;
	u32 pixel_order;
	struct TagHeader th5;
	union AllocateInfo alloc;
	struct TagHeader th6;
	u32 pitch;
	/*
	struct TagHeader th7;
	u32 overscan_top;
	u32 overscan_bottom;
	u32 overscan_left;
	u32 overscan_right;
	*/
	u32 end_tag;
} __attribute__((packed));

volatile u8 mailbox_buffer[512] __attribute__((aligned(16)));

struct BlankScreen {
	u32	size;
	u32 code;
	struct TagHeader th;
	u32 state;
	u32 end_tag;
} __attribute__((packed));

void mailbox_blank_screen(u32 state) {
	volatile struct BlankScreen* buf = (void*)mailbox_buffer;
	memset((void*)buf, 0, sizeof *buf);
	buf->size = sizeof(*buf);
	buf->th.tag = MAILBOX_TAG_BLANK_SCREEN;
	buf->th.size = 4;
	buf->state = state & 0x1;

	mailbox_write(MAILBOX_CHANNEL_PROPERTY_TAGS, buf);
	mailbox_read(MAILBOX_CHANNEL_PROPERTY_TAGS);

	if (buf->code != REQUEST_SUCCESS) {
		print("mailbox failed\n");
	}
}

void mailbox_create_framebuffer(struct Framebuffer* fb) {
	const u32 depth = 24;
	volatile struct FramebufferInfo* buf = (void*)mailbox_buffer;
	memset((void*)buf, 0, sizeof *buf);
	buf->size = sizeof(*buf);
	//buf->th0.tag = MAILBOX_TAG_SET_PHYSICAL_WIDTH_HEIGHT;
	buf->th0.tag = MAILBOX_TAG_GET_PHYSICAL_WIDTH_HEIGHT;
	buf->th0.size = 8;
	//buf->width0 = width;
	//buf->height0 = height;
	//buf->th1.tag = MAILBOX_TAG_SET_VIRTUAL_WIDTH_HEIGHT;
	buf->th1.tag = MAILBOX_TAG_GET_VIRTUAL_WIDTH_HEIGHT;
	buf->th1.size = 8;
	//buf->width1 = width;
	//buf->height1 = height;
	buf->th2.tag = MAILBOX_TAG_SET_DEPTH;
	buf->th2.size = 4;
	buf->depth = depth;
	buf->th3.tag = MAILBOX_TAG_SET_ALPHA_MODE;
	buf->th3.size = 4;
	buf->alpha_mode = 2; // alpha ignored
	buf->th4.tag = MAILBOX_TAG_SET_PIXEL_ORDER;
	buf->th4.size = 4;
	buf->pixel_order = 1; // RGB
	buf->th5.tag = MAILBOX_TAG_ALLOCATE;
	buf->th5.size = 8;
	buf->alloc.request.alignment = 16;
	buf->th6.tag = MAILBOX_TAG_GET_PITCH;
	buf->th6.size = 4;
	//buf->th7.tag = MAILBOX_TAG_GET_OVERSCAN;
	//buf->th7.size = 16;

	mailbox_write(MAILBOX_CHANNEL_PROPERTY_TAGS, buf);
	mailbox_read(MAILBOX_CHANNEL_PROPERTY_TAGS);

	/*
	if (buf->width0 != width || buf->height0 != height || buf->width1 != width || buf->height1 != height) {
		print("Dimensions do not match\n");
	}
	*/
	if (buf->depth != depth) {
		print("Depths do not match\n");
	}
	if (buf->alpha_mode != 2) {
		print("Alpha modes do not match\n");
	}
	if (buf->pixel_order != 1) {
		print("Pixel orders do not match\n");
	}

	if (buf->code == REQUEST_SUCCESS) {
		/*
		*address = (void*)(u64)(buf->alloc.response.address & (~0xc0000000));
		*size = buf->alloc.response.size;
		*pitch = buf->pitch;
		*/
		fb->physical_width = buf->width0;
		fb->physical_height = buf->height0;
		fb->width = buf->width1;
		fb->height = buf->height1;
		fb->address = (void*)(u64)(buf->alloc.response.address & (~0xc0000000));
		fb->size = buf->alloc.response.size;
		fb->pitch = buf->pitch;
		/*
		fb->overscan_top = buf->overscan_top;
		fb->overscan_bottom = buf->overscan_bottom;
		fb->overscan_left = buf->overscan_left;
		fb->overscan_right = buf->overscan_right;
		*/
	}
	else {
		print("mailbox failed\n");
	}
}

struct PowerInfo {
	u32	size;
	u32 code;
	struct TagHeader th;
	u32 device_id;
	u32 state;
	u32 end_tag;
};

u32 mailbox_get_power(u32 device_id, bool domain) {
	volatile struct PowerInfo* buf = (void*)mailbox_buffer;
	memset((void*)buf, 0, sizeof *buf);
	buf->size = sizeof *buf;
	buf->th.tag = (domain) ? MAILBOX_TAG_GET_POWER_DOMAIN_STATE : MAILBOX_TAG_GET_POWER_STATE;
	buf->th.size = 8;
	buf->device_id = device_id;
	mailbox_write(MAILBOX_CHANNEL_PROPERTY_TAGS, buf);
	mailbox_read(MAILBOX_CHANNEL_PROPERTY_TAGS);
	if (buf->code != REQUEST_SUCCESS) {
		print("mailbox failed\n");
		return 1;
	}
	if (buf->state & 0x2) {
		printf("Mailbox: Device % does not exist\n", (u64)device_id);
		return 1;
	}
	if (buf->state & 0x1) {
		print("Mailbox: Device is ON\n");
	}
	else {
		print("Mailbox: Device is OFF\n");
	}
	return 0;
}

u32 mailbox_set_power(u32 device_id, u8 on, bool domain) {
	volatile struct PowerInfo* buf = (void*)mailbox_buffer;
	memset((void*)buf, 0, sizeof *buf);
	buf->size = sizeof *buf;
	buf->th.tag = (domain) ?  MAILBOX_TAG_SET_POWER_DOMAIN_STATE : MAILBOX_TAG_SET_POWER_STATE;
	buf->th.size = 8;
	buf->device_id = device_id;
	//buf->state = (u32)(on | (on << 1));
	buf->state = (u32)on;
	mailbox_write(MAILBOX_CHANNEL_PROPERTY_TAGS, buf);
	mailbox_read(MAILBOX_CHANNEL_PROPERTY_TAGS);
	if (buf->code != REQUEST_SUCCESS) {
		print("mailbox failed\n");
		return 1;
	}
	if (buf->state & 0x2) {
		printf("Mailbox: Device % does not exist\n", (u64)device_id);
		return 1;
	}
	if ((buf->state & 0x1) == (u32)on) {
		print("Mailbox: Changed device power state\n");
	}
	else {
		print("Mailbox: Failed to change device power state\n");
	}
	return 0;
}

struct MemoryInfo {
	u32	size;
	u32 code;
	struct TagHeader th;
	u32 address;
	u32 memory_size;
	u32 end_tag;
};

void* mailbox_get_vc_memory(void) {
	volatile struct MemoryInfo* buf = (void*)mailbox_buffer;
	memset((void*)buf, 0, sizeof *buf);
	buf->size = sizeof *buf;
	buf->th.tag = MAILBOX_TAG_GET_VC_MEMORY;
	buf->th.size = 8;
	mailbox_write(MAILBOX_CHANNEL_PROPERTY_TAGS, buf);
	mailbox_read(MAILBOX_CHANNEL_PROPERTY_TAGS);
	if (buf->code != REQUEST_SUCCESS) {
		print("mailbox failed\n");
		return NULL;
	}
	return (void*)(u64)buf->address;
}

struct ClockInfo {
	u32	size;
	u32 code;
	struct TagHeader th;
	u32 clock_id;
	u32 clock_rate;
	u32 end_tag;
};

u32 mailbox_get_clock_rate(u32 clock_id) {
	volatile struct ClockInfo* buf = (void*)mailbox_buffer;
	memset((void*)buf, 0, sizeof *buf);
	buf->size = sizeof *buf;
	buf->th.tag = MAILBOX_TAG_GET_CLOCK_RATE;
	buf->th.size = 8;
	buf->clock_id = clock_id;
	mailbox_write(MAILBOX_CHANNEL_PROPERTY_TAGS, buf);
	mailbox_read(MAILBOX_CHANNEL_PROPERTY_TAGS);
	if (buf->code != REQUEST_SUCCESS) {
		print("mailbox failed\n");
		return 0;
	}
	return buf->clock_rate;
}
