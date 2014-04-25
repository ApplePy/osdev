#include "real_mode_sw_int.h"
#include "string.h"

extern void real_mode_linear_base;

/*
 * real_mode_sw_int_memget(): copy bytes from the real-mode interrupt
 *                            wrapper data segment into a buffer.
 */
void real_mode_sw_int_memget( void *dst, unsigned short src_offset, unsigned short n ) {

   memcpy( dst, &real_mode_linear_base + src_offset, n );

}

/*
 * real_mode_sw_int_memget(): copy bytes from a buffer into the real-mode
 *                            interrupt wrapper data segment. No checks
 *                            are made to ensure that the data does not
 *                            overwrite code or stack in the segment!
 */
void real_mode_sw_int_memset( unsigned short dst_offset, const void *src, unsigned short n ) {

   memcpy( &real_mode_linear_base + dst_offset, src, n );

}
