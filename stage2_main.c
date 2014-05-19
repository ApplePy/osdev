#define NULL 0

#include "string.h"
#include "FAT.h"

char inb( unsigned short port );
void callfn();
long get_eip();
void irq_disable();
void irq_enable();
void reset();
void real_mode_sw_int_call();

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
   unsigned char      resv1;	      // 0x00
   unsigned char      num_blocks;	// Maximum 0x7F
   unsigned char      resv2;	      // 0x00
   unsigned short     offset;	      // Real-mode transfer buffer
   unsigned short     segment;	   // Real-mode transfer buffer
   unsigned long long LBA_sector;	// Sector to read/write

} Disk_Address_Packet;

extern unsigned long part_start_lba;
extern unsigned long part_length;
extern unsigned char bios_disk_number;
extern unsigned long _start;
extern Real_Mode_SW_Int real_mode_linear_sw_int;
extern unsigned long real_mode_linear_esp;
extern unsigned long XXX;
extern unsigned long YYY;
extern Disk_Address_Packet disk_address_packet;

unsigned short *video = ( unsigned short * ) 0xB8000;

#define TTY 0x03F8
#define DLLB	 0
#define IER	 1
#define DLHB	 1
#define IIR	 2
#define FCR	 2
#define LCR	 3
#define MCR	 4
#define LSR	 5
#define MSR	 6
#define SCRATCH	 7

// send byte to I/O port
static inline void outb( unsigned short port, unsigned char val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* TODO: Is it wrong to use 'N' for the port? It's not a 8-bit constant. */
    /* TODO: Should %1 be %w1? */
    /* TODO: Is there any reason to force the use of eax and edx? */
}

// send a character to the serial port console
void putcc( char c ) {

   if ( c == 0x0A )
      putcc( 0x0D );

   while ( ( inb( TTY + LSR ) & 0x20 ) == 0x00 )
      ;
   
   outb( TTY, c );

}

// get a character from the serial port console
// echoes back to the console if echo is non-zero
char getcc( int echo ) {

   char c;

   while ( ( inb( TTY + LSR ) & 0x01 ) == 0x00 )
      ;
   
   c = inb( TTY );
   
   if ( echo )
      putcc( c );
   
   return c;

}

// print a number in hexadecimal format
void printhex( unsigned long num, int digits ) {

   int i, buf;

   for ( i = 0; i < digits; i++ ) {
   
      buf = ( num >> ( ( digits - i - 1 ) * 4 ) ) & 0xF;
      if ( buf < 10 )
         buf += '0';
      else
         buf += 'A' - 10;
      putcc( buf );
   
   }
   
}

// print a null-terminated string
void printss( char *s ) {

   while ( *s ) {

      putcc( *s );
      s++;
   
   }

}

// print n characters from the string pointed to by s
// protect against control characters < 32 and > 127
void printsss( char *s, int n ) {

   while ( n-- ) {

      if ( *s > ' ' )
         putcc( *s );
      else
         putcc( '.' );

      s++;

   }

}

// converts to lowercase
char lowercase( char c ) {

   if ( ( c >= 'A' ) && ( c <= 'Z' ) )
      c += 'a' - 'A';

   return c;

}

// gets up to 8 hex digits & stores in num
// returns 0 on success and non-zero on invalid input
int gethex( unsigned long *num, int digits, int echo ) {

   char buf;
   int i;

   if ( echo )
      printss( " " );
   *num = 0;

   for ( i = 0; i < digits; i++ ) {
   
      buf = lowercase( getcc( echo ) );
      if ( ( buf < '0' ) || ( buf > 'f' ) || ( ( buf > '9' ) && ( buf < 'a' ) ) )
         return -1;
      if ( buf > '9' )
         buf -= 'a' - 10;
      else
         buf -= '0';
      *num = ( *num << 4 ) + buf;
      
   }

   return 0;
   
}

// To test 'c' command
void testfn() {

   putcc('A');

}

// enable A20 address line using the keyboard controller,
// to allow access to RAM > 1MB.
// Returns 0 on success, non-zero on failure.
int enableA20() {
   
//   irq_disable();
   
   int i;
   char status;
   
   // Make 5 attempts to enable A20
   for ( i = 0; i < 5; i++ ) {
      
      // Wait for controller to be ready for a command
      while ( inb( 0x0064 ) & 0x02 )
         ;
      
      // Tell controller we want to read the current status
      outb( 0x0064, 0xD0 );
      
      // Wait for controller to be ready with data
      while ( ( inb( 0x0064 ) & 0x01 ) == 0 )
         ;
      
      // Read current port status
      status = inb( 0x0060 );
      
      // Wait for controller to be ready for a command
      while ( inb( 0x0064 ) & 0x02 )
         ;
      
      // Tell controller we want to write status byte
      outb( 0x0064, 0xD1 );
      
      // Wait for controller to be ready for data
      while ( inb( 0x0064 ) & 0x02 )
         ;
      
      // Turn on A20 enable bit
      status |= 0x02;
      
      // Write new value
      outb( 0x0060, status );
      
      // Check that it was enabled
      
      // Wait for controller to be ready for a command
      while ( inb( 0x0064 ) & 0x02 )
         ;
      
      // Tell controller we want to read the current status
      outb( 0x0064, 0xD0 );
      
      // Wait for controller to be ready with data
      while ( ( inb( 0x0064 ) & 0x01 ) == 0 )
         ;
      
      // Read current port status
      status = inb( 0x0060 );
      
      // Is A20 enabled?
      if ( status & 0x02 ) {
         
         printss( "\nA20 enabled using PRIMARY method on attempt " );
         printhex( i + 1, 1 );
         printss( "." );
         
//         irq_enable();
         
         return 0;
         
      }
      
   }
   
   // Initial attempt(s) to enable A20 has failed. Try a backup
   // method that is not supported on many chipsets but is the
   // only method that works on other chipsets.
   
   // Make 5 more attempts to enable A20
   for ( i = 0; i < 5; i++ ) {
      
      // Wait for controller to be ready for a command
      while ( inb( 0x0064 ) & 0x02 )
         ;
      
      // Tell controller we want to turn on A20
      outb( 0x0064, 0xDF );
      
      // Wait for controller to be ready for a command
      while ( inb( 0x0064 ) & 0x02 )
         ;
      
      // Tell controller we want to read the current status
      outb( 0x0064, 0xD0 );
      
      // Wait for controller to be ready with data
      while ( ( inb( 0x0064 ) & 0x01 ) == 0 )
         ;
      
      // Read current port status
      status = inb( 0x0060 );
      
      // Is A20 enabled?
      if ( status & 0x02 ) {
         
         printss( "\nA20 enabled using SECONDARY method on attempt " );
         printhex( i + 1, 1 );
         printss( "." );
         
//         irq_enable();
         
         return 0;
         
      }
      
   }
   
   printss( "\nAll attempts to enable A20 FAILED!" );
   
//   irq_enable();
   
   return -1;
   
}

// Read sector(s) from booted partition to 0x4000:0000
// Returns 0 on success and non-zero on failure
int int13h_read( unsigned long sector_offset, unsigned char num_blocks ) {
   
   // Bounds check; only read sectors on the booted partition
   if ( sector_offset + num_blocks > part_length )
      return -1;
   
   // Sanity check; BIOS int13h can only read up to 0x7F blocks
   if ( num_blocks > 0x7F )
      return -1;
   
   disk_address_packet.packet_size = 0x10;
   disk_address_packet.resv1 = 0x00;
   disk_address_packet.num_blocks = num_blocks;
   disk_address_packet.resv2 = 0x00;
   disk_address_packet.offset = 0x0000;
   disk_address_packet.segment = 0x4000;
   disk_address_packet.LBA_sector = ( unsigned long long ) part_start_lba + ( unsigned long long ) sector_offset;
   
   real_mode_linear_sw_int.ah = 0x42;   // Extended read
   real_mode_linear_sw_int.dl = bios_disk_number;
   real_mode_linear_sw_int.si = 0x0040; // Pointer to dap (in real mode memory...DS:SI)
   real_mode_linear_sw_int.opcode = 0xCD;
   real_mode_linear_sw_int.operand = 0x13;
   
   real_mode_sw_int_call();
   
   return real_mode_linear_sw_int.ah;
   
}

// Write sector(s) to booted partition from 0x4000:0100
// Returns 0 on success and non-zero on failure
int int13h_write( unsigned long sector_offset, char num_blocks ) {
   
   // Bounds check; only write sectors on the booted partition
   if ( sector_offset + num_blocks > part_length )
      return -1;
   
   // Sanity check; BIOS int13h can only write up to 0x7F blocks
   if ( num_blocks > 0x7F )
      return -1;
   
   disk_address_packet.packet_size = 0x10;
   disk_address_packet.resv1 = 0x00;
   disk_address_packet.num_blocks = num_blocks;
   disk_address_packet.resv2 = 0x00;
   disk_address_packet.offset = 0x0000;
   disk_address_packet.segment = 0x4000;
   disk_address_packet.LBA_sector = ( unsigned long long ) part_start_lba + ( unsigned long long ) sector_offset;
   
   real_mode_linear_sw_int.al = 0x00;   // Write with verify off
   real_mode_linear_sw_int.ah = 0x43;   // Extended write
   real_mode_linear_sw_int.dl = bios_disk_number;
   real_mode_linear_sw_int.si = 0x0040; // Pointer to dap (in real mode memory...DS:SI)
   real_mode_linear_sw_int.opcode = 0xCD;
   real_mode_linear_sw_int.operand = 0x13;
   
   real_mode_sw_int_call();
   
   return real_mode_linear_sw_int.ah;
   
}

int main(void) {

   char buf, *addr, checksum;
   int i, count, lba_sector, num_blocks;
 
   printss( "\nSecureOS Bootloader v2.0 -> Stage 2\n" );
   printss( "Copyright (C) 2001, 2013\n" );
   printss( "Enter \"?\" for help\n" );
   printss( "Boot ROM date = " );
   printsss( ( char * ) 0xFFFF5, 8 );
   printss( "\nEIP = " );
   printhex( get_eip(), 8 );
   enableA20();
   
   printss( "\npart_start_lba=" );
   printhex( part_start_lba, 8 );
   printss( "\npart_length=" );
   printhex( part_length, 8 );
   printss( "\nbios_disk_number=" );
   printhex( bios_disk_number, 2 );
   
   printss( "\n_start=" );
   printhex( ( unsigned long ) &_start, 8 );
   printss( "\nreal_mode_linear_sw_int=" );
   printhex( ( unsigned long ) &real_mode_linear_sw_int, 8 );
   printss( "\nreal_mode_linear_esp=" );
   printhex( ( unsigned long ) &real_mode_linear_esp, 8 );
   printss( "\nXXX=" );
   printhex( ( unsigned long ) &XXX, 8 );
   printss( "\nYYY=" );
   printhex( ( unsigned long ) &YYY, 8 );
   
   while ( 1 ) {

      printss( "\n>" );

      buf = lowercase( getcc( 1 ) );

      if ( buf == 'b' ) {

         printss( "\nRebooting!!\n" );
         reset();

      }
      
      else if ( buf == 'c' ) {
      
         if ( gethex( ( void * ) &addr, 8, -1 ) != 0 )
            continue;

         callfn( addr );

      }
      
      else if ( buf == 'd' ) {
      
         if ( gethex( ( void * ) &addr, 8, -1 ) != 0 )
            continue;

         if ( gethex( ( void * ) &count, 8, -1 ) != 0 )
            continue;

         count += ( unsigned long ) addr & 0x0000000F;
         addr = ( char * ) ( ( unsigned long ) addr & 0xFFFFFFF0 );
         checksum = 0;
         
         while ( count > 0 ) {
         
            printss( "\n" );
            printhex( ( unsigned long ) addr, 8 );
            printss( " " );
            
            for ( i = 0; i < 16; i++, addr++ ) {
            
               printhex( *addr, 2 );
               printss( " " );
            
            }
            
            addr -= 16;
            
            for ( i = 0; i < 16; i++, addr++ ) {
            
               printsss( addr, 1 );
               checksum += *addr;
            
            }
            
            count -= 16;
         
         }

         printss( "\nChecksum = " );
         printhex( checksum, 2 );
         
      }
      
      else if ( ( buf == 'f' ) || ( buf == 's' ) ) {
      
         if ( gethex( ( void * ) &addr, 8, ( buf == 's' ) ? 0 : -1 ) != 0 )
            continue;

         addr = ( char * ) ( ( unsigned long ) addr & 0xFFFFFFF0 );

         for ( i = 0; ; i++, addr++ ) {
         
            if ( ( i % 16 == 0 ) && ( buf != 's' ) ) {
            
               printss( "\n" );
               printhex( ( unsigned long ) addr, 8 );
            
            }
            
            if ( gethex( ( void * ) &count, 2, ( buf == 's' ) ? 0 : -1 ) != 0 )
               break;
      
            *addr = *( ( char * ) &count );
      
         }
      
      }
      
      else if ( buf == 'i' ) {
      
         if ( gethex( ( void * ) &addr, 4, -1 ) != 0 )
            continue;
         
         printss( "\n" );
         printhex( inb( ( unsigned long ) addr ), 2 );
      
      }
         
      else if ( buf == 'o' ) {
      
         if ( gethex( ( void * ) &addr, 4, -1 ) != 0 )
            continue;
         
         if ( gethex( ( void * ) &count, 2, -1 ) != 0 )
            continue;
         
         outb( ( unsigned long ) addr, count );
      
      }
      
      else if ( buf == 'm' ) {
      
         if ( gethex( ( void * ) &addr, 8, -1 ) != 0 )
            continue;
         
         printss( "\n" );
         printhex( *addr, 2 );
         
         if ( gethex( ( void * ) &count, 2, -1 ) != 0 )
            continue;
         
         *addr = count;
      
      }
      
      else if ( buf == 'r' ) {
         
         if ( gethex( ( void * ) &lba_sector, 8, -1 ) != 0 )
            continue;
         
         if ( gethex( ( void * ) &num_blocks, 2, -1 ) != 0 )
            continue;
         
         int13h_read( lba_sector, num_blocks );

      }
      
      else if ( buf == 'w' ) {
         
         if ( gethex( ( void * ) &lba_sector, 8, -1 ) != 0 )
            continue;
         
         if ( gethex( ( void * ) &num_blocks, 2, -1 ) != 0 )
            continue;
         
         int13h_write( lba_sector, num_blocks );

      }
      
      else if ( buf == '?' ) {
         
         printss( "\nInternal commands:" );
         printss( "\nb                           - reboot" );
         printss( "\nc <addr>                    - call function" );
         printss( "\nd <addr> <count>            - dump range of memory" );
         printss( "\nf <addr>                    - fill range of memory" );
         printss( "\ni <port>                    - read byte from I/O port" );
         printss( "\nm <addr>                    - modify memory location" );
         printss( "\no <port> <byte>             - write byte to I/O port" );
         printss( "\nr <lba_sector> <num_blocks> - read disk sector(s) to 0x00040000" );
         printss( "\ns <addr>                    - \"silent\" fill (same as f without echo)" );
         printss( "\nw <lba_sector> <num_blocks> - write disk sector(s) from 0x00040000" );
         
      }

   }

   return 0;

}