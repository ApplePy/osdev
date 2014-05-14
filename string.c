#include "string.h"

char   *strcpy   ( char *s1,       const char *s2           )
{
    strncpy(s1, s2, strlen(s2) + 1);
    s1[strlen(s2)] = '\0'; //tack on the null terminating character if it wasn't already done
    return s1;
}

char   *strncpy  ( char *s1,       const char *s2, size_t n )
{
    unsigned int extern_iter = 0; //when s2's length is shorter than n, this allows the function to continue padding null characters
    
    for (unsigned int iterator = 0; iterator < n; iterator++) //iterate through s2 up to char n, copying them to s1
    {
        if (s2[iterator] != '\0')
            s1[iterator] = s2[iterator];
        else //the end of s2 was found prematurely - copy the null character, update external iterator and quit for loop
        {
            s1[iterator] = s2[iterator];
            extern_iter = iterator + 1;
            break;
        }
    }
    
    while (extern_iter < n) //while there are still spaces that need to be filled with null characters, fill them
    {
        s1[extern_iter] = '\0';
        extern_iter++;
    }
    
    return s1;
}