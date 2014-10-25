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


/* -------------------------------------------------------------------------------- */

/* Array of text lines */
struct textlines {
    /* array of text lines */
    char **textlines;
    /* number of lines */
    int nlines;
};

/* initialize textlines */
void textlines_init(struct textlines *txt, int nlines);

/* Append a new line to the textline, scrolling the textlines up if needed
 * The total number of lines does not exceed nlines. */
void textlines_scroll_add(struct textlines *tbox, char *str);


/* Iterator to characters in textlines
 * Rules: NULL lines are skipped entirely.
 *  All other characters including termitating '\0's are passed through.
 */
struct textlines_iterator {
    const struct textlines *txt;
    /* current line */
    int k;
    /* current character on the line */
    int m;
};


/* initialize textline iterator to the beginning */
void textlines_iterator_init(struct textlines_iterator *it, const struct textlines *txt);

/* return the current character, or -1 if at the end */
int textlines_iterator_peekc(struct textlines_iterator *it);

/* advance to the next character; return -1 if at the end */
int textlines_iterator_next(struct textlines_iterator *it);



#endif
