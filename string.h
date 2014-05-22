#ifndef STRING_H_
#define STRING_H_

#define size_t unsigned long

void   *memcpy   ( void *s1,       const void *s2, size_t n ); // Implemented!
void   *memmove  ( void *s1,       const void *s2, size_t n ); // Implemented!
char   *strcpy   ( char *s1,       const char *s2           ); // Priority
char   *strncpy  ( char *s1,       const char *s2, size_t n ); // Priority
char   *strcat   ( char *s1,       const char *s2           ); // Priority
char   *strncat  ( char *s1,       const char *s2, size_t n ); // Priority
int    memcmp    ( const void *s1, const void *s2, size_t n ); // Implemented!
int    strcmp    ( const char *s1, const char *s2           ); // Implemented!
int    strcoll   ( const char *s1, const char *s2           );
int    strncmp   ( const char *s1, const char *s2, size_t n ); // Implemented!
size_t strxfrm   ( char *s1,       const char *s2, size_t n );
void   *memchr   ( const void *s,  int c,          size_t n );
char   *strchr   ( const char *s,  int c                    );
size_t strcspn   ( const char *s1, const char *s2           );
char   *strpbrk  ( const char *s1, const char *s2           );
char   *strrchr  ( const char *s,  int c                    );
size_t strspn    ( const char *s1, const char *s2           );
char   *strstr   ( const char *s1, const char *s2           );
char   *strtok   ( char *s1,       const char *s2           );
void   *memset   ( void *s,        int c,          size_t n ); // Implemented!
char   *strerror ( int errnum                               );
size_t strlen    ( const char *s                            ); // Implemented!

#endif
