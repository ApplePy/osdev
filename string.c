#include "string.h"

char *strcpy(char *s1, const char *s2)
{
	strncpy(s1, s2, strlen(s2) + 1);
	s1[strlen(s2)] = '\0'; //tack on the null terminating character if it wasn't already done
	return s1;
}

char *strncpy(char *s1, const char *s2, size_t n)
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

int strcmp(const char *s1, const char *s2)
{
	return strncmp(s1, s2, strlen(s1)); //It doesn't matter what the n is at this point - they should be the same length anyways
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	unsigned int count = 0;
	while (count <= n) //its <= because we want to compare the '\0' too in case one string is a bit shorter than the other
	{
		if (s1[count] == s2[count])
		{
			if (s1[count] == '\0' || s2[count] == '\0') //quit early because null-termination found
				return s1[count] - s2[count];
			else
				count++;
		}
		else
			return s1[count] - s2[count];
	}
	
	return 0;
}