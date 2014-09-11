/*
 * printf.h
 *
 *	Printf code downloaded from: http://forum.43oh.com/topic/1289-tiny-printf-c-version/
 *
 *	This is a tiny printf() function that can be used with the chips that come with the Launchpad. Code size is about 640 bytes with CCS.
 *	There are 7 format specifiers:
 *	%c - Character
 *	%s - String
 *	%i - signed Integer (16 bit)
 *	%u - Unsigned integer (16 bit)
 *	%l - signed Long (32 bit)
 *	%n - uNsigned loNg (32 bit)
 *	%x - heXadecimal (16 bit)
 *
 *	Thank you oPossum!
 *
 *
 *  Created on: 22.3.2013
 *      Author: Mika Väyrynen
 */

#ifndef PRINTF_H_
#define PRINTF_H_

void printf(char *, ...);

#endif /* PRINTF_H_ */
