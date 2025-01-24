#ifndef __BASE_H__
#define __BASE_H__
#include "net-common.h"

size_t base64_encoded_size(const uint8_t *data, size_t size);
size_t base64_decoded_size(const uint8_t *encoded_data, size_t encoded_size);

int base64_encode(const uint8_t *data, size_t data_size, uint8_t *encoded_data);
int base64_decode(const uint8_t *encoded_data, size_t encoded_size, uint8_t *data);

#endif // !__BASE_H__


