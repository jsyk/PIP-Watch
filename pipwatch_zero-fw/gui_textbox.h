#ifndef GUI_TEXTBOX_H
#define GUI_TEXTBOX_H

#include "gui_window.h"
#include "utils.h"


typedef enum {
	FF_HELV,
} fontfamily_t;

typedef enum {
	COLOR_FOREGR,
	COLOR_MID,
	COLOR_BACKGR
} color_t;


struct fontstyle {
	int bold;
	int italic;
	int underline;
	int font_size;
	int extra_lnspc;
	fontfamily_t font_family;
	color_t color;
};

/* init default font style */
void fontstyle_init(struct fontstyle *fs);


/* ---------------------------------------------------------------------- */

struct GuiTextbox {
    /* the window base class; MUST BE THE FIRST ELEMENT */
    struct GuiWindow win;
    /* text lines */
    struct TextLines txt;
    /* interpret formating markup (according to rfc1896) ? */
    int markup;
    /* spread long textlines over several print lines (1),
     * or trim them at the boundary (0) ? */
    int wraplines;
    /* expand escape codes entered using '\' */
    int escexpand;
    /* default font style - initial state when drawing */
    struct fontstyle ini_fontstyle;
};


/*
	RFC1896 text/enriched

	Token		Meaning
	<< 			Literal <
	'\n'		Line break
	<param>
	<bold>, <b>
	<italic>, <i>
	<underline>, <u>
	<fixed>, <fi>
	<fontfamily>, <fo>
	<color>
	<smaller>, <sm>
	<bigger>, <bi>
	<center>
	<flushleft>
	<flushright>
	<flushboth>
	<paraindent>
	<nofill>


  BNF:
	
	<echar> ::=
		normal char except '\'
		| '\' <esccode>
	
	<esccode> ::=
		'n' | 'r' | '\' | '"' | {'u' <unicode_seq>}

	<unicode_seq> ::=
		<digit> <digit> <digit>

	<token> ::=
		{'<' '<'}
		| '<' ['/'] <tokname>
		| <echar\{'<'}>

	<tokname> ::=
		{ <echar\{'<','>'}> * } '>'
*/


/* allocate new textbox gui element */
struct GuiTextbox *gui_textbox_alloc(int nlines);


/* drawing callback for textbox */
int gui_textbox_draw_cb(u8g_t *pu8g, struct GuiWindow *win,
                struct GuiPoint abspos);


#endif
