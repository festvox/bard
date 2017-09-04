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
/*  Some Global Colors                                                   */
/*************************************************************************/
#ifndef _BARD_COLOR_
#define _BARD_COLOR_

#include <SDL.h>
#include "cst_alloc.h"
#include "cst_string.h"

/* At present this could never be more than num_windows * 3 */
/* if every color was different (fore, back and highlight) */
/* If you need more colors you can increase this */
#define BARD_MAX_NUM_COLORS 20

struct bard_colors_struct {
    SDL_Color **colors;
    char **color_names;
    int num_colors;
    int max_num_colors;
};
typedef struct bard_colors_struct bard_colors;

bard_colors *bard_color_setup();
void bard_color_delete(bard_colors *bc);
const SDL_Color *bard_color_get(bard_colors *bc, const char *colorname);
int bard_color_add(bard_colors *bc, 
                   const char *colorname,
                   unsigned char r, 
                   unsigned char g, 
                   unsigned char b);
const char *bard_color_name(bard_colors *bc,const SDL_Color *c);

#endif
