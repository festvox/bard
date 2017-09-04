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

static const char *bard_help_string =
"Bard Storyteller: Help\n"
"\n"
#ifdef GCW0
"GCW-Zero Game Console:\n"
"select  Menu of other windows\n"
"start   Return to main text window\n"
"A       toggle scrolling/select\n"
"X       toggle speaking\n"
"Y       Page Up\n"
"B       Page Down\n"
"R1/L1    Increase/Decrease font/speech/volume\n"
"d-pad and A to select in Menu window\n"
"d-pad in General Menu to select position etc\n"
#endif
"Keyboard:\n"
"tab   Toggle speaking\n"
"space Toggle scrolling\n"
"i      Increase font size\n"
"o     Decrease font size\n"
"p     Toggle pause\n"
"q     Quit\n"
"g     General parameters\n"
"h     Help screen\n"
"f      File select\n"
"r      Recent files\n"
"m      Menu of windows\n"
"v      voice selection\n"
"c      font selection\n"
"      use arrow keys, home, pgup, pgdn\n"
"      enter to select\n"
"F11/F12 vol/speed/page\n"
"\n"
#ifdef Pandora
"Game Console:\n"
"menu    Menu of other windows\n"
"X       toggle scrolling/select\n"
"O       toggle speaking\n"
"Triangle Page Up\n"
"Square   Page Down\n"
"R1/L1    Increase/Decrease font size\n"
#endif
"Bard-%s-%s %s\n"
"Copyright (C) 2012-2014\n"
"Carnegie Mellon University\n"
"All Rights Reserved\n"
"http://festvox.org/bard/\n"
"\n";

static void bard_window_help_update(bard_window *w)
{
    bard_window_draw_border(w,1,w->foreground_color);

    return;
}

bard_window *bard_make_help_window(bard_reader *br)
{
    bard_window *hw;
    int indent = 10;
    char *bhs;
    const char *font_name;
    int font_size;

    hw = bard_window_new("help",
                         br->display->screen_width-(2*indent),
                         br->display->screen_height-(2*indent),
                         br->display->screen->format);
    hw->x_offset = indent;
    hw->y_offset = indent;
    font_name = get_param_string(br->config,"-font",BARD_DEFAULT_FONT);
    font_size =
        get_param_int(br->config,"-help_font_size",
                      get_param_int(br->config,"-font_size",
                                    BARD_DEFAULT_FONT_SIZE));
    /* We assume we can open this font, as if we couldn't when initializing */
    /* the display window, we'd have failed by now */
    bard_font_set(&hw->font,font_name,font_size,BARD_FONT_STYLE_NORMAL);

    hw->background_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-help_background_color","cornsilk"));
    hw->foreground_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-help_foreground_color","steelblue"));
    hw->highlight_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-help_highlight_color","blue"));

    hw->tm=10;
    hw->bm=10;
    hw->tlm=10;
    hw->trm=10;

    bhs = cst_alloc(char,cst_strlen(bard_help_string)+
                    cst_strlen(BARD_PROJECT_VERSION)+
                    cst_strlen(BARD_PROJECT_STATE)+
                    cst_strlen(BARD_PROJECT_DATE)+1);
    cst_sprintf(bhs,bard_help_string,BARD_PROJECT_VERSION,
                BARD_PROJECT_STATE,
                BARD_PROJECT_DATE);

    hw->ts = ts_open_string(bhs,
                     cst_ts_default_whitespacesymbols,
                     "",
                     cst_ts_default_prepunctuationsymbols,
                     cst_ts_default_postpunctuationsymbols);
    hw->ts_mode = "literal";
    cst_free(bhs);

    hw->update = bard_window_help_update;

    /* Put something on the screen */
    bard_window_display_from_pos(hw,0);

    return hw;
}

