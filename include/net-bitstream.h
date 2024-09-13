#ifndef __BS_STREAN_H__
#define __BS_STREAN_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "net-common.h"

#define get_bit(flag, bit) (flag & (1 << (bit)))
#define set_bit(flag, bit) {flag |= (1 << (bit));}
#define clt_bit(flag, bit) {flag &= ~(1 << (bit));}

typedef struct
{
	uint8_t* start;
	uint8_t* p;
	uint8_t* end;
	int bits_left;
} bs_t;

bs_t* bs_new(uint8_t* buf, size_t size);
void bs_reset(bs_t *bs);
bs_t *bs_new_buffer(size_t size);

void bs_free(bs_t* b);
bs_t* bs_clone(bs_t* dest, const bs_t* src);
int bs_clone_bs(bs_t *dest, bs_t *src);

uint8_t *bs_start_ptr(bs_t *b);
uint8_t *bs_p_ptr(bs_t *b);
uint8_t *bs_end_ptr(bs_t *b);

uint32_t bs_byte_aligned(bs_t* b);  //判断当前是不是第8位
int bs_eof(bs_t* b);                //判断是不是已经结束了
int bs_overrun(bs_t* b);            //判断是不是已经结束了
int bs_pos(bs_t* b);                //当前位置
int bs_bytes_left(bs_t *b);         //剩余

uint32_t bs_peek_u1(bs_t* b);       //向前读取1个字节
uint32_t bs_read_u1(bs_t* b);       //读取一个字节
uint32_t bs_read_u(bs_t* b, int n); //读取n个字节
uint32_t bs_read_ru(bs_t* b, int n);//读取n个字节,不移动
uint32_t bs_read_ru1(bs_t *b);

uint32_t bs_read_u8(bs_t* b);       //读取8个字节
uint32_t bs_read_ue(bs_t* b);       //UE编码结果
int32_t  bs_read_se(bs_t* b);       //SE编码结果
int      bs_read_string(bs_t *bs, int str_size, uint8_t  *string, int size);

void bs_write_u1(bs_t* b, uint32_t v);
void bs_write_u(bs_t* b, int n, uint32_t v);
void bs_write_f(bs_t* b, int n, uint32_t v);
void bs_write_u8(bs_t* b, uint32_t v);
void bs_write_ue(bs_t* b, uint32_t v);
void bs_write_se(bs_t* b, int32_t v);

int bs_read_bytes(bs_t* b, uint8_t* buf, int len);
int bs_write_bytes(bs_t* b, uint8_t* buf, int len);
int bs_skip_bytes(bs_t* b, int len);
uint32_t bs_next_bits(bs_t* b, int nbits);

static inline void bs_skip_u1(bs_t* b)
{    
    b->bits_left--;
    if (b->bits_left == 0) { b->p ++; b->bits_left = 8; }
}

static inline void bs_skip_u(bs_t* b, int n)
{
    int i;
    for ( i = 0; i < n; i++ ) 
    {
        bs_skip_u1( b );
    }
}

#ifdef __cplusplus
}
#endif

#endif
