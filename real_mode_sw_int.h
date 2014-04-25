#ifndef REAL_MODE_SW_INT_H_
#define REAL_MODE_SW_INT_H_

// For use with real_mode_sw_int_call function; holds register parameters
// and interrupt type. It is also used to return any register values. All
// general purpose registers (AX, BX, CX and DX) can also be accessed via
// their high and low bytes, e.g., AH and AL.
typedef struct real_mode_sw_int {

   unsigned short flags;
   
   unsigned short di;
   unsigned short si;
   unsigned short bp;
   unsigned short sp_ignored;	// SP is not restored from POPA
   
   union {
      unsigned short bx;
      struct {
         unsigned char bl;
         unsigned char bh;
      };
   };
   
   union {
      unsigned short dx;
      struct {
         unsigned char dl;
         unsigned char dh;
      };
   };
   
   union {
      unsigned short cx;
      struct {
         unsigned char cl;
         unsigned char ch;
      };
   };
   
   union {
      unsigned short ax;
      struct {
         unsigned char al;
         unsigned char ah;
      };
   };
   
   unsigned char  opcode;	// Suggestion: 0xCD
   unsigned char  operand;

} Real_Mode_SW_Int;

// For use with Int 13h extensions, functions 42h (extended read) and
// 43h (extended write).
typedef struct disk_address_packet {

   unsigned char      packet_size;	// 0x10
   unsigned char      resv1;	// 0x00
   unsigned char      num_blocks;	// Maximum 0x7F
   unsigned char      resv2;	// 0x00
   unsigned short     offset;	// Real-mode transfer buffer
   unsigned short     segment;	// Real-mode transfer buffer
   unsigned long long LBA_sector;	// Sector to read/write

} Disk_Address_Packet;

void real_mode_sw_int_memget( void *dst, unsigned short src_offset, unsigned short n );
void real_mode_sw_int_memset( unsigned short dst_offset, const void *src, unsigned short n );
void real_mode_sw_int_call( const Real_Mode_SW_Int *param, Real_Mode_SW_Int *ret );

#endif
