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

#include "bard_display.h"
#include "bard_render.h"
#include "bard_window.h"
#include "bard_color.h"

static void bard_display_hardware_screen_size(bard_display *d);

bard_display *bard_display_open(cst_features *config)
{
    bard_display *display;
    int screen_width, screen_height;
    int default_font_size;
    const char *font_name;

    display = cst_alloc(bard_display,1);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_WM_SetCaption("Bard", "Bard");
    display->name = "Bard";

    bard_display_hardware_screen_size(display);
    /* Respect hardware size if it can be determined */
    /* But not too much if its a big screen */
    /* But respect command line specification more */
    /* as long as its less than the hardware size (if known) */

    screen_width = display->hardware_width;
    if ((screen_width == 0) || 
        (screen_width > BARD_DISPLAY_DEFAULT_MAX_WIDTH))
        screen_width = BARD_DISPLAY_DEFAULT_MAX_WIDTH;
    screen_height = display->hardware_height;
    if ((screen_height == 0) || 
        (screen_height > BARD_DISPLAY_DEFAULT_MAX_HEIGHT))
        screen_height = BARD_DISPLAY_DEFAULT_MAX_HEIGHT;

    display->screen_width = 
        get_param_int(config,"-screen_width",screen_width);
    if ((display->hardware_width > 0) && 
        (display->screen_width > display->hardware_width))
        display->screen_width = display->hardware_width;
    display->screen_height = 
        get_param_int(config,"-screen_height",screen_height);
    if ((display->hardware_height > 0) &&
        (display->screen_height > display->hardware_height))
        display->screen_height = display->hardware_height;

    display->screen_depth = BARD_DISPLAY_SCREEN_DEPTH;
    display->screen = SDL_SetVideoMode(display->screen_width,
                                       display->screen_height,
                                       display->screen_depth,
                                       SDL_SWSURFACE);
    if (display->screen == NULL)
    {
        bard_display_close(display);
        return NULL;
    }

    if (!bard_font_init())
    {
        bard_display_close(display);
        return NULL;
    }

    /* Allow repeat keys (maybe don't want every key to autorepeat) */
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);

    font_name = feat_string(config,"-font");
    /* This is sort of a guess for big screens and nothing specified */
    /* It might be too big for people with good eyesight, but they */
    /* can change it afterwards */
    default_font_size = display->screen_height/12;
    default_font_size = get_param_int(config,"-font_size",default_font_size);
    bard_font_set(&display->font,font_name,default_font_size,BARD_FONT_STYLE_NORMAL);
    if (display->font == NULL)
    {
        printf("Bard: failed to load font %s\n",font_name);
        bard_display_close(display);
        return NULL;
    }

    /* We don't support the mouse cursor so just switch it off */
    /* unless you move it */
    SDL_ShowCursor(SDL_DISABLE);

    return display;
}

void bard_display_close(bard_display *display)
{
    if (display == NULL)
        return;

    if (display->screen)
        SDL_FreeSurface(display->screen);
    if (display->font)
        bard_font_delete(display->font);

    bard_font_quit();
    SDL_Quit();

    cst_free(display);
}

int bard_display_clear(bard_display *d)
{
    SDL_FillRect(d->screen, NULL, 
                 SDL_MapRGB(d->screen->format,
                            d->background_color->r,
                            d->background_color->g,
                            d->background_color->b));
    return 0;
}

static void bard_display_hardware_screen_size(bard_display *d)
{
    /* Get Hardware screen size if available */
    /* Its not really the hardware its the current display resolution */
    const SDL_VideoInfo *vi;

    vi = SDL_GetVideoInfo();

    d->hardware_height = 0;
    d->hardware_width = 0;

    /* Earlier versions of SDL don't have this */
#if ((SDL_MAJOR_VERSION == 1) && (SDL_MINOR_VERSION == 2) && (SDL_PATCHLEVEL > 12))
    d->hardware_height = vi->current_h;
    d->hardware_width = vi->current_w;
#endif

    return;
}

void bard_display_update(bard_display *d)
{
    SDL_Rect offset;
    SDL_Rect offset2;

    /* Window specific update function */
    if (d->current->update) (d->current->update)(d->current);
    
    /* Render current window to display and display on device */
    offset.x = d->current->x_offset;
    offset.y = d->current->y_offset;

    offset2.x = 0;
    offset2.y = d->current->scroll_offset;
    offset2.w = d->current->surface->w;
    offset2.h = d->current->surface->h;

    SDL_BlitSurface(d->current->surface,&offset2,d->screen,&offset);

    SDL_Flip(d->screen);
    return;
}
