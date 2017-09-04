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
/*  Display interface (SDL, terminal, etc)                               */
/*************************************************************************/
#ifndef _BARD_DISPLAY_
#define _BARD_DISPLAY_

#include <SDL.h>

#include "cst_alloc.h"
#include "cst_features.h"
#include "cst_tokenstream.h"
#include "bard_render.h"

struct bard_display_struct {
    const char *name;

    int screen_width, screen_height, screen_depth;
    SDL_Surface *screen;
    struct bard_window_struct *current;

    const SDL_Color *background_color;
    const SDL_Color *foreground_color;

    bard_font *font;

    /* Non-zero if they can be determined */
    int hardware_height;
    int hardware_width; 
};
typedef struct bard_display_struct bard_display;

#define BARD_DISPLAY_DEFAULT_MAX_WIDTH 800
#define BARD_DISPLAY_DEFAULT_MAX_HEIGHT 600
#define BARD_DISPLAY_SCREEN_DEPTH 16

bard_display *bard_display_open(cst_features *config);
void bard_display_close(bard_display *d);
int bard_display_clear(bard_display *d);
void bard_display_update(bard_display *d);

#endif
