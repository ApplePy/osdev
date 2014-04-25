#include "real_mode_sw_int.h"

#define NULL 0

#include "string.h"

extern void my_print( char *s );
extern void pokeb( unsigned long addr, char b );
extern char peekb( unsigned long addr );
extern void stackdump( unsigned long addr, char b );

unsigned short *video = ( unsigned short * ) 0xB8000;

extern void putcc( unsigned char c );
extern char getcc();
extern void reset();
extern void go_real();

void printhex( char x ) {

   unsigned short z;

   z = ( x & 0xF0 ) >> 4;
   if ( z < 10 )
      z += '0';
   else
      z += 'A' - 10;
   putcc( z );
   /*z |= 0x2e00;
   video[ y * 3 ] = z;
   pokeb( ( 0xb8cd0 + y * 6 ), z );
   pokeb( ( 0xb8cd0 + y * 6 ) + 1, 0x2e );*/

   z = x & 0x0F;
   if ( z < 10 )
      z += '0';
   else
      z += 'A' - 10;
   putcc( z );
   /*z |= 0x2e00;
   video[ y * 3 + 1 ] = z;
   pokeb( ( 0xb8cd0 + y * 6 ) + 2, z );
   pokeb( ( 0xb8cd0 + y * 6 ) + 3, 0x2e );*/

}

void printss( char *s ) {

   while ( *s ) {

      putcc( *s );
      s++;
   
   }

}

void printsss( char *s, int n ) {

   while ( n-- ) {

      if ( *s > ' ' )
         putcc( *s );
      else
         putcc( '.' );

      s++;

   }

}

int main(void) {

   char s[ 27 ] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

   printhex( memcmp( "D", "D", 1 ) );
   printhex( memcmp( "D", "A", 1 ) );
   printhex( memcmp( "D", "G", 1 ) );
   putcc( ' ' );
   
   memset( s + 10, '?', 5 );
   
   printsss( s, 27 );

//   printhex( s1 );
   printhex( strlen( s ) );

   //char s2[] = "";
   //printhex( strlen( s2 ) );
   printhex( strlen( "ABC456789" ) );
   printhex( strlen( "" ) );

   

putcc( '\n' );
//reset();
   /*char *yy = (char *)0x40000;
   int k;
   
   putcc( '*' );
   putcc( '\n' );
   putcc( '\n' );
   for ( k = 0; k < 512; k++ ) {
      printhex( yy[ k ] );
      if ( k % 16 == 15 )
         putcc( '\n' );
      else
         putcc( ' ' );
   }
   putcc( '\n' );
   putcc( '\n' );

   // Print char using BIOS

   Real_Mode_SW_Int real_mode_int;
   
   real_mode_int.ax = 0x0E24;
printhex(real_mode_int.ah);
printhex(real_mode_int.al);
   
   real_mode_int.opcode = 0xCD;
   real_mode_int.operand = 0x10;
   
   real_mode_sw_int_call( &real_mode_int, NULL );
   
   // Read disk sector using BIOS

   Disk_Address_Packet dap;
   
   dap.packet_size = 0x10;
   dap.resv1 = 0x00;
   dap.num_blocks = 0x01;
   dap.resv2 = 0x00;
   dap.offset = 0x0000;
   dap.segment = 0x4000;
//   dap.LBA_sector = 0x0000000002ED5EB1ULL;
   dap.LBA_sector = 0x0000000002FFC13CULL;

   real_mode_int.ah = 0x42; // Extended read
   real_mode_int.dl = 0x80; // BIOS disk number
   real_mode_int.si = 0x0000; // Pointer to dap (in real mode memory...DS:SI)
   real_mode_int.opcode = 0xCD;
   real_mode_int.operand = 0x13;
   
   real_mode_sw_int_memset( 0x0000, &dap, sizeof( dap ) );
   real_mode_sw_int_call( &real_mode_int, NULL );
   
   printss("\nBiatch!\n");
   printhex( 0xA5 );
   putcc( '\n' );
   putcc( '\n' );
   for ( k = 0; k < 512; k++ ) {
      printhex( yy[ k ] );
      if ( k % 16 == 15 )
         putcc( '\n' );
      else
         putcc( ' ' );
   }
   putcc( '\n' );
   putcc( '-' );
   printhex( sizeof( real_mode_int ) );
      putcc( '-' );
*/
   
   putcc( '\n' );
   char buf[ 100 ], s1[ 100 ], s2[ 100 ];
   int count = 0, i, n;
   
   while ( 1 ) {

      buf[ count++ ] = getcc();

      if ( buf[ count - 1 ] == '\b' ) {

         putcc( ' ' );
         putcc( '\b' );

      }

      else if ( buf[ count - 1 ] == '\r' ) {

         buf[ count - 1 ] = '\0';

         putcc( '\n' );
         
         printss( "You entered: " );
         printss( buf );
         printss( "\r\n" );

         for ( i = 0; i < count; i++ )
            if ( ( buf[ i ] >= 'A' ) && ( buf[ i ] <= 'Z' ) )
	   buf[ i ] += 'a' - 'A';

         if ( memcmp( buf, "reset", 5 ) == 0 ) {

            printss( "Rebooting!!\r\n" );
            reset();

         }
         
         else if ( memcmp( buf, "memcmp ", 7 ) == 0 ) {
         
            for ( i = 0; ( buf[ 7 + i ] && ( buf[ 7 + i ] != ' ' ) ); i++ )
	   s1[ i ] = buf[ 7 + i ];
	buf[ 7 + i ]= '\0';
	
	printss( "s1=" );
            printss( s1 );
            printss( "\r\n" );

         }
         
         else
            printss( "Bad command or file name.\r\n" );

         count = 0;
         
      }

   }

   /*my_print("Stage 3 C Program!!");*/
   /*pokeb( 0xB8B90, 0x58 );*/
   /*stackdump( 0xB8B90, 0x58 );*/
return 0;
   char j;
   //unsigned long i;

   for ( i = 0; i < 48; i++ ) {

      j = peekb( 0x00010100 + i );
   //  printhex( j, i );

   }

   return 0;

}
