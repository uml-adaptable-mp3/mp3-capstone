#ifndef PARSE_FILE_PARAM_H
#define PARSE_FILE_PARAM_H

#include <vstypes.h>

auto const char *FileParamSeek(register const char *haystack,
			       register const char *needle,
			       register s_int16 *n);

/**
   Finds integer parameter needle in the haystack if it exists.

   \param haystack The string where to search the parameter from.
   \param needle The string to look for.
   \param defaultVal The value the function returns if needle is not found
	in the haystack.
   \return The value, or \a defaultVal if needle is not found in haystack.
   \example FileParamInt("or=24000/oc=2", "or", 48000) = 24000.
   \example FileParamInt("or=24000/oc=2", "oc", 1) = 2.
   \example FileParamInt("or=24000/oc=2", "ir", 48000) = 48000.
*/
auto u_int32 FileParamInt(register const char *haystack,
			  register const char *needle,
			  register u_int32 defaultVal);

/**
   Finds string parameter needle in the haystack if it exists.

   \param haystack The string where to search the parameter from.
   \param needle The string to look for.
   \param dst Destination where to copy the result strings to.
   \param n Number of character space in the destination string.
   \return Absolute value tells the number of zero-terminated elements
	written to \a dst. If number is negative, \a dst didn't contain
	enough space for all values. Note that the function will always
	null-terminate \a dst.
   \example FileParamStr("of=par1,par2,par3/out=1212", "if", s, 16) = 0.
   \example FileParamStr("of=par1,par2,par3/out=1212", "of", s, 16) = 3.
	\a dst will contain "par1\0par2\0par3".
   \example FileParamStr("of=par1,par2,par3/out=1212", "of", s, 14) = -3.
	\a dst will contain "par1\0par2\0par".
*/
auto s_int16 FileParamStr(register const char *haystack,
			  register const char *needle,
			  register char *dst,
			  register s_int16 n);

#endif /* !PARSE_FILE_PARAM_H */
