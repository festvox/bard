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
#include "bard_system.h"

static void bard_window_text_update(bard_window *w)
{

    if (w->current_token)
        if (bard_debug)
            printf("bard_debug text update at %s\n",w->current_token->token);

    return;
}

bard_window *bard_make_text_window(bard_reader *br)
{
    bard_window *tw;
    const char* pwd;
    const char *font_name;
    int font_size;
    char *fp;
    short reladdr = 1;
    size_t lnth;

    tw = bard_window_new("text",
                         br->display->screen_width,
                         br->display->screen_height,
                         br->display->screen->format);
    font_name = 
        /* There might be a language specific text window font */
        get_param_string(br->config,"-text_font",
                         get_param_string(br->config,"-font",BARD_DEFAULT_FONT));
    font_size = get_param_int(br->config,"-font_size",20);
    bard_font_set(&tw->font,font_name,font_size,BARD_FONT_STYLE_NORMAL);

    tw->textfile = cst_strdup(get_param_string(br->config,"-text","<no file>"));
    lnth = cst_strlen (tw->textfile);
    if (lnth >= 1)
       if ((tw->textfile[0] == '/') || /* it's not a relative address */
           (tw->textfile[0] == '<'))   /* or it's a directive */
          reladdr = 0; 
    if (reladdr)
    {
        pwd = bard_getwd();
        fp=cst_alloc(char,cst_strlen(pwd)+1+cst_strlen(tw->textfile)+1);
        cst_sprintf(fp,"%s/%s",pwd,tw->textfile);
        cst_free(tw->textfile);
        tw->textfile = fp;
    }
    tw->ts = bard_open_textfile(tw->textfile);
    if (tw->ts == NULL)  /* the desired file cannot be opened */
    {
        tw->ts = ts_open_string("No text file found:\n"
                                "use 'f' for file selection\n"
                                "use 'h' for help\n",
                 cst_ts_default_whitespacesymbols,
                 "",
                 cst_ts_default_prepunctuationsymbols,
                 cst_ts_default_postpunctuationsymbols);
        tw->sop_pos = 0;
    }
    else
        tw->sop_pos = get_param_int(br->config,"-text_pos",0);

    tw->background_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-text_background_color","white"));
    tw->foreground_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-text_foreground_color","black"));
    tw->highlight_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-text_highlight_color","blue"));

    tw->update = bard_window_text_update;

    /* Put something on the screen */
    bard_window_display_from_pos(tw,tw->sop_pos);
    return tw;
}

void bard_text_scroll_speed_up(bard_reader *br)
{
    if (br->scroll_delay * 0.95 > 10)
    {
        br->scroll_delay -= 1;     /* ensure it changes */
        br->scroll_delay *= 0.95;
    }
    if (bard_debug)
        printf("bard_debug scroll_delay %d\n",br->scroll_delay);
}

void bard_text_scroll_speed_down(bard_reader *br)
{
    if (br->scroll_delay * 1.05 < 180)
    {
        br->scroll_delay += 1;     /* ensure it changes */
        br->scroll_delay *= 1.05;
    }
    if (bard_debug)
        printf("bard_debug scroll_delay %d\n",br->scroll_delay);
}

int bard_text_scroll(bard_window *tw)
{
    int next_line_pos;
    const bard_token *t;
    int h=0;

    tw->scroll_offset++;
    /* Scroll to the start pos of the next line (including any new lines) */
    /* before the next line */
    t = tw->text_screen;  /* from first token on screen */
    if (t && (t->d))
        h = t->d->y-1; 
    /* But if there is an image on the screen this isn't really correct */

    if (tw->scroll_offset > h)
    {   
        if (!t || !t->d)
            return 0;  /* end of file */
        next_line_pos = t->d->file_pos;
        bard_window_display_from_pos(tw,next_line_pos);
        tw->scroll_offset = 0;
    }

    return 1;
}
