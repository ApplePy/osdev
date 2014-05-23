#include "lib_asm.h"
#include "lib_c.h"
#include "string.h"

unsigned short *video = ( unsigned short * ) 0xB8000;

void isr_default( unsigned char num, ISR_Stack_Frame isf );
void isr_pit    ( unsigned char num, ISR_Stack_Frame isf );

int main(void) {

   char buf, *addr, checksum;
   int i, count;
 
   printss( "\nMiniOS32 Kernel is Active!!!\n" );
   printss( "Copyright (C) 2014\n" );
   printss( "Enter \"?\" for help\n" );
   printss( "Boot ROM date = " );
   printsss( ( char * ) 0xFFFF5, 8 );
   printss( "\nEIP = " );
   printhex( get_eip(), 8 );
   
   char new_gdt[ 24 ] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00,
                          0xFF, 0xFF, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00 };
   
   GDTR gdtr;
   gdtr.limit = sizeof( new_gdt ) - 1;
   gdtr.base = new_gdt;
   lgdt( &gdtr, 0x0008, 0x0010 );
   
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

         // The line below treats addr as a pointer to a function
         ( ( void ( * ) () ) addr )();

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
      
      else if ( buf == 'w' ) {
         
         if ( gethex( ( void * ) &addr, 2, -1 ) != 0 )
            continue;
         
         software_interrupt( ( unsigned char ) ( unsigned long ) addr );

      }
      
      else if ( buf == 'x' ) {
      
/*
         printss( "\nisr_template_start = " );
         printhex( ( unsigned long ) &isr_template_start, 8 );

         printss( "\nisr_template_error_start = " );
         printhex( ( unsigned long ) &isr_template_error_start, 8 );
         printss( "\nisr_template_error_end = " );
         printhex( ( unsigned long ) &isr_template_error_end, 8 );
         printss( "\nisr_template_num = " );
         printhex( ( unsigned long ) &isr_template_num, 8 );
         printss( "\nisr_template_cfunc_start = " );
         printhex( ( unsigned long ) &isr_template_cfunc_start, 8 );
         printss( "\nisr_template_cfunc_end = " );
         printhex( ( unsigned long ) &isr_template_cfunc_end, 8 );
         
         printss( "\nisr_template_end = " );
         printhex( ( unsigned long ) &isr_template_end, 8 );
         printss( "\nlength = " );
         printhex( ( unsigned long ) ( &isr_template_end - &isr_template_start ), 8 );
*/
         
         // Prevent an unfortunately-timed IRQ from messing up the whole works
         irq_disable();
         
         // Setup IDT and all associated supports
         idt_install();
         
         // Install some elementary ISRs
         idt_set_gate( 0x0F, isr_default, 0x0008, 0x8E );
         idt_set_gate( 0x20, isr_pit, 0x0008, 0x8E );
         
         // ...and respond to IRQs again
         irq_enable();
         
      }
      
      else if ( buf == 'y' ) {
         
         IDTR idtr;

         sidt( &idtr );
         
         printss( "\nidtr.limit=" );
         printhex( idtr.limit, 4 );
         printss( "\nidtr.base=" );
         printhex( ( unsigned long ) idtr.base, 8 );
         
      }
      
      else if ( buf == 'z' ) {
         
         for ( i = 0; i < 256; i++ ) {
            
            if ( i % 4 == 0 )
               printss( "\n" );
            else
               printss( "    " );
            printhex( i, 2 );
            printss( ": " );
            printhex( irq_count[ i ], 8 );
            
         }
         
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
         printss( "\ns <addr>                    - \"silent\" fill (same as f without echo)" );
         
      }

   }

   return 0;

}

void isr_pit    ( unsigned char num, ISR_Stack_Frame isf ) {
   
   // Send EOI to PIC
   outb( 0x20, 0x20 );
   
}

void isr_default( unsigned char num, ISR_Stack_Frame isf ) {

   unsigned long *y = (unsigned long *)&isf;
   
   int i;
   
   printss( "\nsizeof( ISR_Stack_Frame ) = " );
   printhex( sizeof( ISR_Stack_Frame ), 8 );
   printss( "\nnum = " );
   printhex( num, 2 );
   printss( "\nisf.gs = " );
   printhex( isf.gs, 4 );
   printss( "\nerror code = " );
   printhex( isf.error_code, 2 );
   printss( "\nisf.eip:" );
   printhex( isf.eip, 8 );
   printss( "\nisf.cs = " );
   printhex( isf.cs, 4 );
   printss( "\nisf.eflags = " );
   printhex( isf.eflags, 8 );
   printss( "\nStack Frame:" );
   
   for ( i = 0; i < 32; i++ ) {
      
      printss( "\n" );
      printhex( i, 2 );
      printss( " " );
      printhex( ( unsigned long ) y, 8 );
      printss( ": " );
      printhex( *y, 8 );
      
      y++;
      
   }
   
   reset();

}
