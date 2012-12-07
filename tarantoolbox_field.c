#include "tarantoolbox_private.h"

#include <string.h>

static size_t tarantoolbox_varint_uint32_unpack(const uint8_t *buf, uint32_t *value)
{
	*value = 0;
	if (!(buf[0] & 0x80)) {
		*value = buf[0] & 0x7f;
		return 1;
	}
	if (!(buf[1] & 0x80)) {
		*value = (buf[0] & 0x7f) << 7 |
		         (buf[1] & 0x7f);
		return 2;
	}
	if (!(buf[2] & 0x80)) {
		*value = (buf[0] & 0x7f) << 14 |
			 (buf[1] & 0x7f) << 7  |
			 (buf[2] & 0x7f);
		return 3;
	}
	if (!(buf[3] & 0x80)) {
		*value = (buf[0] & 0x7f) << 21 |
			 (buf[1] & 0x7f) << 14 |
			 (buf[2] & 0x7f) << 7  |
			 (buf[3] & 0x7f);
		return 4;
	}
	if (!(buf[4] & 0x80)) {
		*value = (buf[0] & 0x7f) << 28 |
		         (buf[1] & 0x7f) << 21 |
			 (buf[2] & 0x7f) << 14 |
			 (buf[3] & 0x7f) << 7  |
			 (buf[4] & 0x7f);
		return 5;
	}
	return 0;
}

static size_t tarantoolbox_varint_uint32_pack(uint8_t *buf, uint32_t value) {
    uint8_t *buf0 = buf;
	if (value >= (1 << 7)) {
		if (value >= (1 << 14)) {
			if (value >= (1 << 21)) {
				if (value >= (1 << 28))
					*(buf++) = (value >> 28) | 0x80;
				*(buf++) = (value >> 21) | 0x80;
			}
			*(buf++) = ((value >> 14) | 0x80);
		}
		*(buf++) = ((value >> 7) | 0x80);
	}
	*(buf++) = ((value) & 0x7F);
    return buf - buf0;
}

static size_t tarantoolbox_varint_uint32_size(uint32_t value) {
	if (value < (1 << 7))
		return 1;
	if (value < (1 << 14))
		return 2;
	if (value < (1 << 21))
		return 3;
	if (value < (1 << 28))
		return 4;
	return 5;
}

size_t tarantoolbox_field_packed_size(tarantoolbox_field_t *field) {
    return tarantoolbox_varint_uint32_size(field->size) + field->size;
}

size_t tarantoolbox_field_pack(tarantoolbox_field_t *field, void *buf) {
    size_t size = tarantoolbox_varint_uint32_pack(buf, field->size);
    memcpy(buf + size, field->data, field->size);
    return size + field->size;
}

size_t tarantoolbox_field_unpack(tarantoolbox_field_t *field, void *data, size_t maxsize) {
    size_t size = tarantoolbox_varint_uint32_unpack(data, &field->size);
    if (size == 0) {
        tarantoolbox_log(LOG_ERROR, "invalid field size BER");
        return 0;
    }
    if (field->size > maxsize - size) {
        tarantoolbox_log(LOG_ERROR, "field size is too large: %d > %d", field->size, maxsize - size);
        return 0;
    }
    field->data = data + size;
    return size + field->size;
}
