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

#include "bard.h"

const SDL_Color *bard_color_get(bard_colors *bc, const char *colorname)
{
    /* Return a color in br->colors for this colorname */
    /* Create new colors from 0xFFFFFF specifications -- RGB */
    /* or lookup names for some builtin colors */
    int i;
    int c,r,b,g;

    if (colorname && colorname[0] == '0' && colorname[1] == 'x')
    {
        sscanf(colorname,"%x",&c);
        b = c%256; c /= 256;
        g = c%256; c /= 256;
        r = c%256;
        return bc->colors[bard_color_add(bc,colorname,r,g,b)];
    }
    else
    {
        for (i=0; i<bc->num_colors; i++)
            if (cst_streq(colorname,bc->color_names[i]))
                return bc->colors[i];
    }

    return bc->colors[0];
}

const char *bard_color_name(bard_colors *bc,const SDL_Color *c)
{
    int i;

    for (i=0; i<bc->num_colors; i++)
    {
        if (c == bc->colors[i])
            return bc->color_names[i];
    }

    /* This shouldn't happen ... */
    return bc->color_names[0]; /* so you get white */
}

bard_colors *bard_color_setup(void)
{
    bard_colors *bc = cst_alloc(bard_colors,1);

    bc->max_num_colors = BARD_MAX_NUM_COLORS;
    bc->colors = cst_alloc(SDL_Color *,BARD_MAX_NUM_COLORS);
    bc->color_names = cst_alloc(char *,BARD_MAX_NUM_COLORS);
    bc->num_colors = 0;

    bard_color_add(bc,"white",255,255,255);
    bard_color_add(bc,"black",0,0,0);
    bard_color_add(bc,"cornsilk",255,248,220);
    bard_color_add(bc,"steelblue",70,130,220);
    bard_color_add(bc,"red",255,0,0);
    bard_color_add(bc,"green",0,255,0);
    bard_color_add(bc,"blue",0,0,255);
    bard_color_add(bc,"gray",104,104,104);

    return bc;
}

void bard_color_delete(bard_colors *bc)
{
    int i;

    for (i=0; i<bc->num_colors; i++)
    {
        cst_free(bc->color_names[i]);
        cst_free(bc->colors[i]);
    }
    cst_free(bc->colors);
    cst_free(bc->color_names);
    cst_free(bc);

    return;
}

int bard_color_add(bard_colors *bc, 
                   const char *colorname,
                   unsigned char r, 
                   unsigned char g, 
                   unsigned char b)
{
    /* Add new color to color list, and return its index */
    int i;

    for (i=0; i<bc->num_colors; i++)
    {
        if (cst_streq(colorname,bc->color_names[i]))
            return i;
    }
    if (i < bc->max_num_colors)
    {
        bc->color_names[i] = cst_strdup(colorname);
        bc->colors[i] = cst_alloc(SDL_Color,1);
        bc->colors[i]->r = (unsigned char)r;
        bc->colors[i]->g = (unsigned char)g;
        bc->colors[i]->b = (unsigned char)b;
        bc->num_colors++;
        return i;
    }

    return 0; /* White */
}


