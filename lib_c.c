#include "lib_asm.h"
#include "lib_c.h"
#include "string.h"

#define TTY       0x03F8
#define DLLB	   0
#define IER	      1
#define DLHB	   1
#define IIR	      2
#define FCR	      2
#define LCR	      3
#define MCR	      4
#define LSR	      5
#define MSR	      6
#define SCRATCH   7

typedef struct __attribute__((packed)) {
   
   unsigned short handler_15_0;
   unsigned short selector;
   unsigned char  always_zero;
   unsigned char  type;
   unsigned short handler_31_16;
   
} IDT_Entry;

IDT_Entry idt[ 256 ];

extern const unsigned char ISR_Template_Start;
extern const unsigned char ISR_Template_Error_Start;
extern const unsigned char ISR_Template_Error_End;
extern const unsigned char ISR_Template_Num;
extern const unsigned char ISR_Template_CFunc_Start;
extern const unsigned char ISR_Template_CFunc_Addr;
extern const unsigned char ISR_Template_CFunc_End;
extern const unsigned char ISR_Template_End;


//void putstring(char * str);
//void _printInt(int val, int base);
//void _printf(char * fmt, ...);


// 0x40 is bad!! Re-code to handle changing size of ISR stub code
// based on ( ISR_template_end - ISR_template_start )
#define ISR_STUB_LENGTH 0x40
unsigned char irq_handler[ 256 * ISR_STUB_LENGTH ];

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

void idt_install() {
   

printss( "\nidt_install()" );

printss( "\n&irq_handler = " );
printhex( (unsigned long) &irq_handler, 8 );
printss( "\nsizeof( irq_handler ) = " );
printhex( sizeof( irq_handler ), 8 );

printss( "\n&irq_count = " );
printhex( (unsigned long) &irq_count, 8 );
printss( "\nsizeof( irq_count ) = " );
printhex( sizeof( irq_count ), 8 );

printss( "\n&idt = " );
printhex( (unsigned long) &idt, 8 );
printss( "\nsizeof( idt ) = " );
printhex( sizeof( idt ), 8 );

printss( "\n&idt = " );
printhex( (unsigned long) &idt, 8 );

   IDTR idtr;
   
   idtr.limit = sizeof( idt ) - 1;
   idtr.base = &idt;
   
   lidt( &idtr );
   
   // Initialize IDT and by extension, IRQ count and ISR stubs
   int i;
   for ( i = 0; i < 256; i++ )
      idt_set_gate( i, NULL, 0x0008, 0x8E );
   
   // Re-map BIOS-mapped IRQs so as not to conflict with reserved
   // IDT mappings < 32...specifically, IRQ0-15 are re-mapped to
   // IDT entries 32-47. Otherwise, exceptions, such as double
   // fault (IDT 8) would be a problem!
   outb( 0x20, 0x11 );
   outb( 0xA0, 0x11 );
   outb( 0x21, 0x20 );
   outb( 0xA1, 0x28 );
   outb( 0x21, 0x04 );
   outb( 0xA1, 0x02 );
   outb( 0x21, 0x01 );
   outb( 0xA1, 0x01 );
   outb( 0x21, 0x00 );
   outb( 0xA1, 0x00 );
   
}

void idt_set_gate( unsigned char num, void ( *handler )(  unsigned char num, ISR_Stack_Frame isf  ), unsigned short selector, unsigned char type ) {
   
   idt[ num ].handler_15_0 = ( unsigned long ) &irq_handler[ num * ISR_STUB_LENGTH ];
   idt[ num ].selector = selector;
   idt[ num ].always_zero = 0x00;
   idt[ num ].type = type;
   idt[ num ].handler_31_16 = ( unsigned long ) &irq_handler[ num * ISR_STUB_LENGTH ] >> 16;
   
   irq_count[ num ] = 0;
   
   const unsigned char *src = &ISR_Template_Start;
   const unsigned char *cfunc = ( unsigned char * ) &handler;
   unsigned char *dst = &irq_handler[ num * ISR_STUB_LENGTH ];
   
   while ( src < &ISR_Template_Start + ISR_STUB_LENGTH ) {
      
      // Blank memory if entire ISR stub is already copied
      if ( src >= &ISR_Template_End )
         *dst = 0x00;
      
      // CPU pushes error code automatically; replace error push code with no-op's
      else if ( ( src >= &ISR_Template_Error_Start ) && ( src < &ISR_Template_Error_End ) &&
                ( ( num == 8 ) || ( ( num >= 10 ) && ( num <= 14 ) ) ) )
         *dst = 0x90; // No-op
      
      // Insert ISR number in code
      else if ( src == &ISR_Template_Num )
         *dst = num;
      
      // Have hook to C language function? Copy function pointer to call code
      else if ( ( src >= &ISR_Template_CFunc_Addr ) && ( src < &ISR_Template_CFunc_Addr + 4 ) &&
                ( handler != NULL ) )
         *dst = *cfunc++;
      
      // No hook to C language function, replace call code with no-op's
      else if ( ( src >= &ISR_Template_CFunc_Start ) && ( src < &ISR_Template_CFunc_End ) &&
                ( handler == NULL ) )
         *dst = 0x90; // No-op
      
      // Otherwise, just copy the code byte-for-byte
      else
         *dst = *src;
      
      src++;
      dst++;
      
   }
   
}

// converts to lowercase
char lowercase( char c ) {

   if ( ( c >= 'A' ) && ( c <= 'Z' ) )
      c += 'a' - 'A';

   return c;

}

char uppercase( char c ) {

   if ( ( c >= 'a' ) && ( c <= 'z' ) )
      c -= 'a' - 'A';

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

// send a character to the serial port console
void putcc( char c ) {

   if ( c == 0x0A )
      putcc( 0x0D );

   while ( ( inb( TTY + LSR ) & 0x20 ) == 0x00 )
      ;
   
   outb( TTY, c );
}
// send a string
void putstring(char * str)
{
	while (*str != '\0')
	{
		putcc(*str);
		str++;
	}
}

void _printInt(int val, int base)
{
	char buff[32];
	char * ptr;
	int n;
	ptr = &buff[sizeof(buff)-1];
	*ptr = '\0';

	for(; val != 0; val /= base)
	{
		ptr--;
		n = val%base;
		if (n < 10)
		{
			*ptr = n + '0';
		}

		else
		{
			*ptr = n - 10 + 'A';
		}
	}

	putstring(ptr);
}

void _printf(char * fmt, ...)
{
	int i = 0;
	char *s, *p = (char *)fmt;
	unsigned long *argptr = (unsigned long *)&fmt;
	argptr++;
	while (*p != '\0') // is not NULL
	{
		if (*p != '%')
		{
			putcc(*p);
			p++;
			continue;
		}

		else if (*p == '%')
		{
			switch (*(p+1))
			{
			case 'd':
				i = *(int *)argptr;
				_printInt(i, 10);
				argptr++;
				p+=2;
				break;
			case 's':
				s = (char *)*argptr;
				putstring(s);
				argptr++;
				p+=2;
				break;
			case 'x': 
				i = *(int *)argptr;
				_printInt(i, 16);
				argptr++;
				p+=2;
				break;
			default:
				putcc(*p);
				p++;
				break;
			}
		}
	}
}

char* uppercase_str(char* input)
{
	unsigned int counter = 0;
	unsigned int inputLength = strlen(input);

	while (counter < inputLength) //iterate through input, converting the characters to uppercase
	{
		if ((short)input[counter] >= 97 && (short)input[counter] <= 122)
			input[counter] -= 32;
		counter++;
	}
	return input;
}

char* lowercase_str(char* input)
{
	unsigned int counter = 0;
	unsigned int inputLength = strlen(input);

	while (counter < inputLength) //iterate through input, converting the characters to uppercase
	{
		if ((short)input[counter] >= 65 && (short)input[counter] <= 90)
			input[counter] += 32;
		counter++;
	}
	return input;
}