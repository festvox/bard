/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2012                            */
/*                        All Rights Reserved.                           */
/*                                                                       */
/*  Permission is hereby granted, free of charge, to use and distribute  */
/*  this software and its documentation without restriction, including   */
/*  without limitation the rights to use, copy, modify, merge, publish,  */
/*  distribute, sublicense, and/or sell copies of this work, and to      */
/*  permit persons to whom this work is furnished to do so, subject to   */
/*  the following conditions:                                            */
/*   1. The code must retain the above copyright notice, this list of    */
/*      conditions and the following disclaimer.                         */
/*   2. Any modifications must be clearly marked as such.                */
/*   3. Original authors' names are not deleted.                         */
/*   4. The authors' names are not used to endorse or promote products   */
/*      derived from this software without specific prior written        */
/*      permission.                                                      */
/*                                                                       */
/*  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         */
/*  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      */
/*  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    */
/*  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   */
/*  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          */
/*  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       */
/*  THIS SOFTWARE.                                                       */
/*                                                                       */
/*************************************************************************/
/*             Author:  Alan W Black (awb@cs.cmu.edu)                    */
/*               Date:  January 2012                                     */
/*************************************************************************/
/*  Windows                                                              */
/*************************************************************************/
#ifndef _BARD_WINDOW_
#define _BARD_WINDOW_

#include <SDL.h>
#include <SDL_ttf.h>

#include "cst_alloc.h"
#include "cst_features.h"
#include "cst_tokenstream.h"
#include "bard_color.h"
#include "bard_display.h"
#include "bard_token.h"

struct bard_window_struct {
    const char *name;

    SDL_Surface *surface;
    int width;
    int height;
    int x_offset;  /* x position on screen */
    int y_offset;  /* y position on screen */

    int tm;  /* top margin */
    int bm;  /* bottom margin */
    int tlm; /* text left margin */
    int trm; /* text right margin */
    int tls; /* text line separator */
    int tlh; /* text line height */

    const SDL_Color *background_color;
    const SDL_Color *foreground_color;
    const SDL_Color *highlight_color;

    bard_font *font;

    const char *ts_mode;
    char *textfile;  
    int sop_pos; /* start of page position */
    cst_tokenstream *ts;
    bard_token *text_screen; /* structure for tokens on the screen */
    const bard_token *current_token;
    const cst_val *previous_pages;

    int scroll_offset;

    void *data;  /* for window specific context */

    void (*update)(struct bard_window_struct *w);
    void (*data_delete)(struct bard_window_struct *w);
};
typedef struct bard_window_struct bard_window;    

bard_window *bard_window_new(const char *name,
                             int width, int height, 
                             const SDL_PixelFormat *format);
void bard_window_delete(bard_window *bw);
void bard_window_clear(bard_window *bw);
void bard_window_display_from_pos(bard_window *w, int pos);
int bard_window_fill_text(bard_window *w);
int bard_window_draw_border(bard_window *w, int thickness, 
                            const SDL_Color *color);
void bard_window_token_highlight_move(bard_window *w,int direction);
void bard_window_token_goto(bard_window *w,int line_number,int token_number);
void bard_window_page_up(bard_window *cw);
void bard_window_page_down(bard_window *cw);

#endif
