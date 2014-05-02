
extern void my_print( char *s );
extern void pokeb( unsigned long addr, char b );
extern char peekb( unsigned long addr );
extern void stackdump( unsigned long addr, char b );

unsigned short *video = ( unsigned short * ) 0xB8000;

void printhex( char x, int y ) {

   unsigned short z;

   z = ( x & 0xF0 ) >> 4;
   if ( z < 10 )
      z += '0';
   else
      z += 'A' - 10;
   z |= 0x2e00;
   ;video[ y * 3 ] = z;
   ;pokeb( ( 0xb8cd0 + y * 6 ), z );
   ;pokeb( ( 0xb8cd0 + y * 6 ) + 1, 0x2e );

   z = x & 0x0F;
   if ( z < 10 )
      z += '0';
   else
      z += 'A' - 10;
   z |= 0x2e00;
   ;video[ y * 3 + 1 ] = z;
   ;pokeb( ( 0xb8cd0 + y * 6 ) + 2, z );
   ;pokeb( ( 0xb8cd0 + y * 6 ) + 3, 0x2e );

}

int main(void) {

   /*my_print("Stage 3 C Program!!");*/
   /*pokeb( 0xB8B90, 0x58 );*/
   stackdump( 0xB8B90, 0x58 );
return 0;
   char j;
   unsigned long i;

   for ( i = 0; i < 48; i++ ) {

      j = peekb( 0x00010100 + i );
      printhex( j, i );

   }

   return 0;

}
