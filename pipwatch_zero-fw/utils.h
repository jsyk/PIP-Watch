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

/* integer minima */
static inline int simin(int a, int b)
{ return (a < b) ? a : b; }

/* integer minima */
static inline unsigned int uimin(unsigned int a, unsigned int b)
{ return (a < b) ? a : b; }

/* -------------------------------------------------------------------------------- */

/* Array of text lines */
struct TextLines {
    /* array of text lines */
    char **textlines;
    /* number of lines */
    int nlines;
};

/* initialize textlines */
void textlines_init(struct TextLines *txt, int nlines);

/* Append a new line to the textline, scrolling the textlines up if needed.
 * The total number of lines does not exceed nlines. 
 * The string str is NOT copyed! */
void textlines_scroll_add(struct TextLines *tbox, char *str);


/* Iterator to characters in textlines
 * Rules: NULL lines are skipped entirely.
 *  All other characters including termitating '\0's are passed through.
 */
struct TextLines_iterator {
    const struct TextLines *txt;
    /* current line */
    int k;
    /* current character on the line */
    int m;
};


/* initialize textline iterator to the beginning */
void textlines_iterator_init(struct TextLines_iterator *it, const struct TextLines *txt);

/* return the current character, or -1 if at the end */
int textlines_iterator_peekc(struct TextLines_iterator *it);

/* advance to the next character; return -1 if at the end */
int textlines_iterator_next(struct TextLines_iterator *it);



#endif
