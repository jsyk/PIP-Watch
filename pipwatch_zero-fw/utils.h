#ifndef UTILS_H
#define UTILS_H

/* Integer to decimal string conversion.
 * Returns the number of characters written to buf, not counting the final \0.
 * Terminates the string with \0 if there is a space in the buffer.
 */
int itostr(char *buf, int buflen, int x);

/* Integer to decimal string conversion, right-justify to the buffer end, no \0 insertion,
 * fill unused chars with fillch.
 * Returns the number of digits written.
 */
int itostr_rjust(char *buf, int buflen, int x, char fillch);

/* Allocate new string of length n+1 and copy from buf up to n characters.
 */
char *newstrn(const char *buf, int n);

/* Trim string by up to n characters from the end. */
char *strtrimn(char *buf, int n);

#endif
