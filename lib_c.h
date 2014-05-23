#ifndef LIB_C_H_
#define LIB_C_H_

#define NULL 0

unsigned long irq_count[ 256 ];

typedef struct {

   unsigned short gs __attribute__((aligned(4)));
   unsigned short fs __attribute__((aligned(4)));
   unsigned short es __attribute__((aligned(4)));
   unsigned short ds __attribute__((aligned(4)));
   
   unsigned long  edi;
   unsigned long  esi;
   unsigned long  ebp;
   unsigned long  esp;
   unsigned long  ebx;
   unsigned long  edx;
   unsigned long  ecx;
   unsigned long  eax;
   
   unsigned char  error_code __attribute__((aligned(4)));

   unsigned long  eip;
   unsigned short cs;
   unsigned long  eflags;
   
   // May or may not be filled depending on privilege level of caller
   unsigned long  user_esp;
   unsigned short user_ss __attribute__((aligned(4)));
   
} ISR_Stack_Frame;

char getcc( int echo );
int  gethex( unsigned long *num, int digits, int echo );
void idt_install();
void idt_set_gate( unsigned char num, void ( *handler )( unsigned char num, ISR_Stack_Frame isf ), unsigned short selector, unsigned char type );
char lowercase( char c );
void putcc( char c );
void printhex( unsigned long num, int digits );
void printss( char *s );
void printsss( char *s, int n );


#endif
