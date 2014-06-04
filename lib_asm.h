#ifndef LIB_H_
#define LIB_H_

typedef struct __attribute__((packed)) {
   
   unsigned short limit;
   void           *base;
   
} GDTR;

typedef struct __attribute__((packed)) {
   
   unsigned short limit;
   void           *base;
   
} IDTR;


long           get_eip();
void           halt();
unsigned char  inb( unsigned short port );
unsigned short inw( unsigned short port );
unsigned long  ind( unsigned short port );
void           irq_disable();
void           irq_enable();
void           lgdt( GDTR *init, unsigned short code_segment_selector, unsigned short data_segment_selector );
void           lidt( IDTR *init );
void           outb( unsigned short port, unsigned char  val);
void           outw( unsigned short port, unsigned short val);
void           outd( unsigned short port, unsigned long  val);
void           reset();
void           sgdt( GDTR *save );
void           sidt( IDTR *save );
void           software_interrupt( unsigned char num );

#endif
