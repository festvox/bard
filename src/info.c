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

static const char *bard_info_format_string =
"Bard Storyteller %s: General\n"
"\n"
"Text:        %s\n"
"Position:   <<< %2d%% >>>\n"
"Time:       %s\n"
"Battery:    %s\n"
"Voice:      %s\n"
"Volume:   <<< %1.2f >>>\n"
"Speed:      <<< %1.2f >>>\n"
"Font:       %s\n"
"\n";

static void bard_window_info_update(bard_window *w)
{
    bard_window_draw_border(w,1,w->foreground_color);

    return;
}

static char *populate_info_string(bard_reader *br)
{
    char *info;
    const char *version;
    char *text;
    int position;
    const char *voice;
    float volume;
    float speed;
    char *battery;
    char *time;
    int s,p;

    version = BARD_PROJECT_VERSION;
    text = cst_strdup(br->text->textfile);

    p = br->text->sop_pos;
    s = ts_get_stream_size(br->text->ts);
    position = (int)(100.0*(((float)p/(float)s)));

    if (br->speech->default_voice)
        voice = br->speech->default_voice->name;
    else
        voice = "unknown";
    volume = br->speech->gain;
    speed = br->speech->speed;
    battery = 
        bard_get_battery(get_param_string(br->config,
                                          "-battery_script",
                                          "battery"));
    time = bard_get_time();

    info = cst_alloc(char,cst_strlen(bard_info_format_string)+
                     cst_strlen(version)+
                     cst_strlen(text)+
                     2+
                     cst_strlen(voice)+
                     10+
                     10+
                     cst_strlen(battery)+
                     cst_strlen(time)+
                     cst_strlen(br->text->font->name)+
                     1);

    cst_sprintf(info,bard_info_format_string,
                version,
                text,
                position,
                time,
                battery,
                voice,
                volume,
                speed,
                br->text->font->name);
    
    cst_free(text);
    cst_free(battery);
    cst_free(time);
    
    return info;
}

bard_window *bard_make_info_window(bard_reader *br)
{
    bard_window *iw;
    int indent = 10;
    const char *font_name;
    int font_size;

    iw = bard_window_new("Info",
                         br->display->screen_width-(2*indent),
                         br->display->screen_height-(2*indent),
                         br->display->screen->format);
    iw->x_offset = indent;
    iw->y_offset = indent;
    font_name = get_param_string(br->config,"-font",BARD_DEFAULT_FONT);
    font_size = get_param_int(br->config,"-info_font_size",
#ifdef GCW0
                      BARD_DEFAULT_MENU_FONT_SIZE
#else
                      get_param_int(br->config,"-font_size", 
                                    BARD_DEFAULT_INFO_FONT_SIZE)
#endif                                    
                                    );
    bard_font_set(&iw->font,font_name,font_size,BARD_FONT_STYLE_NORMAL);

    iw->background_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-info_background_color","cornsilk"));
    iw->foreground_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-info_foreground_color","steelblue"));
    iw->highlight_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-info_highlight_color","blue"));

    iw->tm=10;
    iw->bm=10;
    iw->tlm=10;
    iw->trm=10;
    iw->update = bard_window_info_update;

    br->info = iw;  /* because this hasn't happened yet */

    bard_window_update_info_string(br);

    return iw;
}

void bard_window_update_info_string(bard_reader *br)
{
    char *info_string;

    info_string = populate_info_string(br);

    if (br->info->ts)
        ts_close(br->info->ts);
    br->info->ts = ts_open_string(info_string,
                                  cst_ts_default_whitespacesymbols,
                                  "",
                                  cst_ts_default_prepunctuationsymbols,
                                  cst_ts_default_postpunctuationsymbols);
    br->info->ts_mode = "literal";
    cst_free(info_string);

    /* Put something on the screen */
    bard_window_display_from_pos(br->info,0);

    return;
}

void bard_window_info_params(bard_reader *br)
{   /* This is where we change the values, not just display them */
    const char *linetype = NULL;
    const bard_token *fl, *t;
    int l,w;
    int m, max;

    if (br->info->current_token)
        t = br->info->current_token;
    else
        return;

    fl = bard_token_first(t);
    if (fl)
        linetype = fl->word;
    else
        return;

    if (cst_streq("Position",linetype))
    {   /* Modify file position */
        max = ts_get_stream_size(br->text->ts);
        m = max/20; /* in 5% increments */
        if (cst_streq("<<<",t->word))
        {
            if (br->text->sop_pos-m > 0)
                br->text->sop_pos-=m;
            else
                br->text->sop_pos=0;
        }
        else if (cst_streq(">>>",t->word))
        {
            if (br->text->sop_pos+m < max)
                br->text->sop_pos+=m;
        }
        /* Previous page positions are not relevant now so delete them */
        ts_get(br->text->ts);  /* move forward to a token boundary */
        delete_val((cst_val *)(void *)br->text->previous_pages);
        br->text->previous_pages = NULL;
        br->speak = 0;
        bard_window_display_from_pos(br->text,br->text->sop_pos);
    }
    else if (cst_streq("Volume",linetype))
    {   /* Modify gain/volume -- happens on next buffer (soon) */
        if (cst_streq("<<<",t->word))
            bard_speech_volume_down(br->speech);
        else if (cst_streq(">>>",t->word))
            bard_speech_volume_up(br->speech);
    }
    else if (cst_streq("Speed",linetype))
    {   /* Modify speed happens on next utterance (not so soon) */
        if (cst_streq("<<<",t->word))
        {
            if (br->speech->speed > 0.2)
                br->speech->speed /= 1.1;
        }
        else if (cst_streq(">>>",t->word))
        {
            if (br->speech->speed < 5.0)
                br->speech->speed *= 1.1;
        }
        feat_set_float(br->speech->default_voice->features,
                       "duration_stretch",
                       br->speech->speed);
    }

    l = t->line_number;
    w = t->token_number;
    bard_window_update_info_string(br); /* Will loose current screen pos */
    /* Put cursor position back */
    bard_window_token_goto(br->info,l,w);
    /* We're back where we were: so highlight it */
    bard_window_token_highlight_move(br->info,0);

    return;
}


