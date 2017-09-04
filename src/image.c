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
/*               Date:  February 2012                                    */
/*************************************************************************/
/*                                                                       */
/*  For image loading/rendering                                          */
/*    who would think a speech program would need image functions ...    */
/*                                                                       */
/*************************************************************************/

#ifndef BARD_EPUB_IMAGES
/* No EPUB Support */
#include "bard.h"

SDL_Surface *bard_image_render(bard_window *w,char *image,int size)
{
    return NULL;
}

SDL_Surface *bard_image_fit_window(bard_window *w, SDL_Surface *image)
{
    return NULL;
}

#else
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include "bard.h"

SDL_Surface *bard_image_render(bard_window *w,char *image,int size)
{
    /* Render the given image at some sort of size */
    SDL_Surface *imagebox;
    SDL_RWops *s;

    s = SDL_RWFromMem(image,size);
    imagebox = IMG_Load_RW(s, 0);
    SDL_FreeRW(s);

    return imagebox;
}

SDL_Surface *bard_image_fit_window(bard_window *w, SDL_Surface *image)
{   /* Return a scaled image that fits in the screen */
    SDL_Surface *resize_image;
    int desired_x, desired_y;
    int max_x, max_y;

    if (image == NULL) return NULL;

    desired_x = image->w;
    desired_y = image->h;
    /* Try to always get so text on the screen too, so we can page */
    /* properly -- not sure this is right though */
    max_x = w->width-(6*w->font->size);
    max_y = w->height-(6*w->font->size);

    if (desired_x > max_x)
    {
        desired_y = (int)(desired_y*(max_x/(float)desired_x));
        desired_x = max_x;
    }

    if (desired_y > max_y)
    {
        desired_x = (int)(desired_x*(max_y/(float)desired_y));
        desired_y = max_y;
    }

    resize_image = zoomSurface(image,desired_x/(float)image->w,
                               desired_y/(float)image->h,1);

    if (bard_debug)
        printf("bard_debug image was %d %d now %d %d\n",
               image->w, image->h, resize_image->w, resize_image->h);

    return resize_image;
}

#endif
