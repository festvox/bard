/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2013                            */
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
/*               Date:  December 2013                                    */
/*                                                                       */
/*  A menu window to select between the windows, useful when there isn't */
/*  a keyboard on the device                                             */
/*                                                                       */
/*************************************************************************/

#include "bard.h"
#include "bard_system.h"

void bard_window_update_menu_string(bard_reader *br);

static const char *bard_menu_format_string =
"Bard Storyteller: Window Menu\n"
"\n"
"File:        select a file\n"
"General:  info on current file\n"
"Recent:   select a recent file\n"
"Help:       help screen\n"
"%s"
"%s"
"Text:       main screen\n"
"Quit:       exit Bard\n";

static const char *voices_menu_option =
    "Voices:    select a voice\n";
static const char *font_menu_option =
    "Font:      select a font\n";

static void bard_window_menu_update(bard_window *w)
{
    bard_window_draw_border(w,1,w->foreground_color);

    return;
}

bard_window *bard_make_menu_window(bard_reader *br)
{
    bard_window *mw;
    int indent = 10;
    const char *font_name;
    int font_size;

    mw = bard_window_new("Menu",
                         br->display->screen_width-(10*indent),
                         br->display->screen_height-(10*indent),
                         br->display->screen->format);
    /* Menu window is deliberately (more) smaller than the other windows */
    mw->x_offset = 5*indent;
    mw->y_offset = 5*indent;
    font_name = get_param_string(br->config,"-font",BARD_DEFAULT_FONT);
    /* Its quite important for the menu font size to be small enough to */
    /* display all the main options, especially the first time you see it */
    /* so we allow for device specific setting here */
    font_size = 
        get_param_int(br->config,"-menu_font_size",
#ifdef GCW0
                      BARD_DEFAULT_MENU_FONT_SIZE
#else
                      get_param_int(br->config,"-font_size", 
                                    BARD_DEFAULT_MENU_FONT_SIZE)
#endif                                    
                                    );
    bard_font_set(&mw->font,font_name,font_size,BARD_FONT_STYLE_NORMAL);

    mw->background_color = 
        bard_color_get(br->colors,                          
         get_param_string(br->config,"-menu_background_color","gray"));
    mw->foreground_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-menu_foreground_color","white"));
    mw->highlight_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-menu_highlight_color","blue"));

    mw->tm=10;
    mw->bm=10;
    mw->tlm=10;
    mw->trm=10;
    mw->update = bard_window_menu_update;

    br->menu = mw;  /* because this hasn't happened yet */

    bard_window_update_menu_string(br);

    return mw;
}

void bard_window_update_menu_string(bard_reader *br)
{
    char *menu_string = NULL;

    if (br->menu->ts)
        ts_close(br->menu->ts);

    menu_string = cst_alloc(char,
                            cst_strlen(bard_menu_format_string)+
                            cst_strlen(voices_menu_option)+
                            cst_strlen(font_menu_option)+
                            1);

    cst_sprintf(menu_string,bard_menu_format_string,
                (feat_present(br->config,"-voices_dir")?voices_menu_option:""),
                (feat_present(br->config,"-font_dir")?font_menu_option:""));

    br->menu->ts = ts_open_string(menu_string,
                                  cst_ts_default_whitespacesymbols,
                                  "",
                                  cst_ts_default_prepunctuationsymbols,
                                  cst_ts_default_postpunctuationsymbols);
    br->menu->ts_mode = "literal";
    cst_free(menu_string);

    /* Put something on the screen */
    bard_window_display_from_pos(br->menu,0);

    return;
}

void bard_window_menu_select_window(bard_reader *br)
{
    /* Jump to selected window */
    const bard_token  *t, *fl;
    const char *desired_window_name = NULL;

    if (br->menu->current_token)
        t = br->menu->current_token;
    else
        return;

    fl = bard_token_first(t);  /* find first token in line */
    if (fl)
        desired_window_name = fl->word;
    else
        return;

    if (cst_streq("Text",desired_window_name))
        br->display->current = br->text;
    else if (cst_streq("File",desired_window_name))
        br->display->current = br->file_select;
    else if (cst_streq("Recent",desired_window_name))
        br->display->current = br->recent;
    else if (cst_streq("Help",desired_window_name))
        br->display->current = br->help;
    else if (cst_streq("General",desired_window_name))
    {
        bard_window_update_info_string(br);
        br->display->current = br->info;
    }
    else if (cst_streq("Voices",desired_window_name))
        br->display->current = br->voice_select;
    else if (cst_streq("Font",desired_window_name))
        br->display->current = br->font_select;
    else if (cst_streq("Quit",desired_window_name))
        br->quit = 1;

    return;
}




