/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2014                            */
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
/*               Date:  August 2014                                      */
/*************************************************************************/
/*  Bard render api for different ttf rendering techniques               */
/*************************************************************************/
#ifndef _BARD_RENDER_
#define _BARD_RENDER_

#ifndef BARD_HARFBUZZ
/* If we have no harfbuff support still do standard SDL_TFF */
#define BARD_SDL_TTF 1
#endif

#ifdef BARD_SDL_TTF
#include <SDL_ttf.h>
#endif

#ifdef BARD_HARFBUZZ
#define FREETYPE
#include <ft2build.h>
#include <freetype/freetype.h>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb-icu.h>
#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>
#endif

#ifdef BARD_SDL_TTF
#define BARD_FONT_STYLE_NORMAL TTF_STYLE_NORMAL
#define BARD_FONT_STYLE_ITALIC TTF_STYLE_ITALIC
#define BARD_FONT_STYLE_BOLD TTF_STYLE_BOLD
#else
#define BARD_FONT_STYLE_NORMAL 0
#define BARD_FONT_STYLE_ITALIC 1
#define BARD_FONT_STYLE_BOLD 2
#endif

#ifdef BARD_HARFBUZZ
/* Script direction */
#define BARD_FONT_DIRECTION_LTR HB_DIRECTION_LTR
#define BARD_FONT_DIRECTION_RTL HB_DIRECTION_RTL
#define BARD_FONT_DIRECTION_TTB HB_DIRECTION_TTB

#define BARD_SCRIPT_LATIN  HB_SCRIPT_LATIN
#define BARD_SCRIPT_ARABIC HB_SCRIPT_ARABIC
#define BARD_SCRIPT_HAN    HB_SCRIPT_HAN
#define BARD_SCRIPT_DEVANAGARI HB_SCRIPT_DEVANAGARI
#define BARD_SCRIPT_TAMIL HB_SCRIPT_TAMIL
#define BARD_SCRIPT_TELUGU HB_SCRIPT_TELUGU
#define BARD_SCRIPT_KANNADA HB_SCRIPT_KANNADA
#endif

struct bard_font_struct {
    char *name;
    int size;
    int style;

#ifdef BARD_SDL_TTF
    TTF_Font *ttf_font;
#endif

#ifdef BARD_HARFBUZZ
    FT_Library ft_library; /* maybe this should be global */

    FT_Face ft_face;
    cairo_font_face_t *cairo_ft_face;

    hb_font_t *hb_ft_font;

    int text_direction;
    hb_script_t script;
    const char *language;

#endif
};
typedef struct bard_font_struct bard_font;

struct bard_surface_struct {
    SDL_Surface *sdl_surface;
#ifdef BARD_HARFBUZZ
    cairo_surface_t *cairo_surface;
    cairo_t *cr;
#endif
};
typedef struct bard_surface_struct bard_surface;

int bard_font_init();
int bard_font_quit();

bard_font *bard_font_new();
int bard_font_delete(bard_font *bf);
bard_surface *bard_surface_new();
int bard_surface_delete(bard_surface *bf);

int bard_font_set(bard_font **tbf,
                  const char *font_name,
                  int font_size,
                  int style);

int bard_font_size(bard_font *bf);
const char *bard_font_name(bard_font *bf);
int bard_font_style(bard_font *bf);
int bard_font_set_style(bard_font **font, int style);
bard_surface *bard_render_text(bard_font *font,char *text,SDL_Color c);

#endif
