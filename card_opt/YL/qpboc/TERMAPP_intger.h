/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _INTEGER
#define _INTEGER

/* These types must be 16-bit, 32-bit or larger integer */
typedef int				INT;
typedef unsigned int	uint;

/* These types must be 8-bit integer */
typedef signed char		CHAR;
typedef unsigned char	uchar;

//typedef unsigned char	byte;

/* These types must be 16-bit integer */
typedef short			SHORT;
typedef unsigned short	u16;
//typedef unsigned short	WORD;

/* These types must be 32-bit integer */
typedef long			LONG;
typedef unsigned long	u32;
typedef unsigned long	ulong;
typedef unsigned long	DWORD;
/* Boolean type */
//typedef enum { FALSE = 0, TRUE } BOOL;

#ifndef NULL
#define NULL 		((void*)0)
#endif

#endif

