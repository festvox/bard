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

static void bard_delete_recent_file(bard_window *recent, const char *fn);

static void bard_window_recent_update(bard_window *w)
{
    bard_window_draw_border(w,1,w->foreground_color);

    return;
}

static void bard_window_recent_data_delete(bard_window *w)
{
    cst_val *rf;

    rf = (cst_val *)w->data;
    delete_val(rf);

    return;
}

static void bard_window_recent_select(bard_window *w)
{
    char *recentfiles;
    const char *header = "Recent Files:\n\n";
    const cst_val *f;
    const cst_val *rf;
    int size;

    rf = (const cst_val *)w->data;
    for (size=0,f=rf; f; f=val_cdr(f))
        size += 1+cst_strlen(val_string(val_car(val_car(f))));

    recentfiles = cst_alloc(char,cst_strlen(header)+size+1);
    cst_sprintf(recentfiles,"%s",header);
    for (f=rf; f; f=val_cdr(f))
    {
        cst_sprintf(&recentfiles[cst_strlen(recentfiles)],"%s",
                    val_string(val_car(val_car(f))));
        cst_sprintf(&recentfiles[cst_strlen(recentfiles)],"%s","\n");
    }

    if (w->ts) ts_close(w->ts);
    w->ts = ts_open_string(recentfiles,
                     bard_filename_whitespacesymbols,
                     bard_filename_singlecharsymbols,
                     bard_filename_prepunctuationsymbols,
                     bard_filename_postpunctuationsymbols);
    w->ts_mode = "literal";
    w->sop_pos = 0;

    bard_window_display_from_pos(w,w->sop_pos);

    cst_free(recentfiles);
}

void bard_window_recent_select_file(bard_reader *br)
{
    /* User selected current token */
    const cst_val *it;
    const cst_val *rf;
    bard_window *rw, *tw;
    cst_tokenstream *ts;
    char *rfn;
    char *rvoice;
    char *rdir;
    char *rfont_name;
    char *r;
    int rpos, rfs;

    rw = br->recent;
    tw = br->text;
    rf = (const cst_val *)rw->data;

    it = val_assoc_string(rw->current_token->token,rf);
    if (it)
    {
        rfn = cst_strdup(val_string(val_car(it)));
        rpos = val_int(val_car(val_cdr(it)));
        rfs = val_int(val_car(val_cdr(val_cdr(it))));
        rvoice = cst_strdup(val_string(val_car(val_cdr(val_cdr(val_cdr(it))))));
        rfont_name = cst_strdup(val_string(val_car(val_cdr(val_cdr(val_cdr(val_cdr(it)))))));
        /* We extract the dir name from the filename */
        rdir = cst_strdup(rfn);
        r = cst_strrchr(rdir,'/');
        if (r)
            r[0] ='\0';
        else
        {   /* There isn't a directory, so make it the current directory */
            cst_free(rdir);
            rdir = cst_strdup(".");
        }
        ts = bard_open_textfile(rfn);
        if (ts == NULL) /* file not openable */
        {
            cst_free(rfn);
            cst_free(rvoice);
            return;
        }
        /* Delete this new current from the recent list */
        bard_delete_recent_file(br->recent,rfn);
        /* Add current file to recent list */
        bard_add_recent_file(br->recent,br->text,
                             br->speech->default_voice_pathname);
        /* Update text window to newly selected file */
        if (tw->ts)
        {
            cst_free(tw->textfile);
            ts_close(tw->ts);
        }

        tw->textfile = rfn;
        tw->ts = ts;
        tw->sop_pos = rpos;
        /* Delete list of previous page points -- thats for the old file */
        delete_val((cst_val *)(void *)tw->previous_pages);
        tw->previous_pages = NULL;
        /* Select the new font */
        bard_font_set(&tw->font,rfont_name,rfs,BARD_FONT_STYLE_NORMAL);
        cst_free(rfont_name);
        br->speak = 0;
        /* update text dir for this file */
        cst_free(br->file_select->textfile);
        br->file_select->textfile = rdir;
        br->file_select->sop_pos = 0;
        bard_window_file_select_select(br->file_select);

        /* Select voice */
        if (bard_voice_select(br->speech,rvoice) == FALSE)
        {
            printf("bard_debug: not able to load voice %s\n", rvoice);
            /* beep */
        }
        cst_free(rvoice);
        
        br->display->current = tw;
        bard_window_display_from_pos(tw,tw->sop_pos);
    }

    return;
}

bard_window *bard_make_recent_window(bard_reader *br)
{
    bard_window *rw;
    int indent = 10;
    cst_val *rf = NULL;
    int i, rf_pos, rf_font_size;
    const cst_val *ne;
    const char *rf_filename;
    const char *rf_voice_name;
    const char *rf_font_name;
    const char *font_name;
    int font_size;
    char featname[20];

    rw = bard_window_new("recent",
                         br->display->screen_width-(2*indent),
                         br->display->screen_height-(2*indent),
                         br->display->screen->format);
    rw->x_offset = indent;
    rw->y_offset = indent;
    font_name = get_param_string(br->config,"-font",BARD_DEFAULT_FONT);
    font_size = 
        get_param_int(br->config,"-recent_font_size",
                      get_param_int(br->config,"-font_size",
                                    BARD_DEFAULT_FONT_SIZE));
    bard_font_set(&rw->font,font_name,font_size,BARD_FONT_STYLE_NORMAL);
    rw->background_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-recent_background_color","black"));
    rw->foreground_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-recent_foreground_color","white"));
    rw->highlight_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-recent_highlight_color","blue"));

    rw->tm=indent;
    rw->bm=indent;
    rw->tlm=indent;
    rw->trm=indent;

    /* Populate the recent files list */
    for (i=0; i<BARD_MAX_NUM_RECENT_FILES; i++)
    {
        cst_sprintf(featname,"-rf%d_text",i);
        if (feat_present(br->config,featname))
        {
            rf_filename = feat_string(br->config,featname);
            cst_sprintf(featname,"-rf%d_text_pos",i);
            rf_pos = feat_int(br->config,featname);
            cst_sprintf(featname,"-rf%d_font_size",i);
            rf_font_size = feat_int(br->config,featname);
            cst_sprintf(featname,"-rf%d_voice_name",i);
            rf_voice_name = get_param_string(br->config,featname,"kal");
            cst_sprintf(featname,"-rf%d_font_name",i);
            rf_font_name = get_param_string(br->config,featname,
                                            BARD_DEFAULT_FONT);
            
            ne = cons_val(string_val(rf_filename),
                  cons_val(int_val(rf_pos),
                   cons_val(int_val(rf_font_size),
                    cons_val(string_val(rf_voice_name),
                     cons_val(string_val(rf_font_name),
                              NULL)))));
            rf = cons_val(ne,rf);
        }
    }

    rw->data = (void *)val_reverse(rf);

    rw->update = bard_window_recent_update;
    rw->data_delete = bard_window_recent_data_delete;

    /* Put something on the screen */
    bard_window_recent_select(rw);

    return rw;
}

static void bard_delete_recent_file(bard_window *recent, const char *fn)
{
    /* When file is selected you want to remove it from the recent list */
    const cst_val *rf;
    const cst_val *v, *l;
    cst_val *x;

    rf = (const cst_val *)recent->data;
    l=NULL;
    for (v=rf; v; v=val_cdr(v))
    {
        if (cst_streq(fn,val_string(val_car(val_car(v)))))
        {
            x = (cst_val *)(void *)v;
            if (l==NULL) /* its the first one */
                rf = val_cdr(v);
            else
                set_cdr((cst_val *)(void *)l,val_cdr(v));
            set_cdr(x,NULL);
            delete_val(x);
            break;
        }
        l=v;
    }
    recent->data = (void *)rf;

    return;
}

void bard_add_recent_file(bard_window *recent, bard_window *text,
                          const char *voicepath)
{
    /* Add the (new) current file to the recent list */
    /* Deleting it if it appears further down the list */
    const cst_val *rf;
    const cst_val *ne;
    const cst_val *v;

    if (cst_streq(text->textfile,"<no file>"))
        return;
    /* Recent files in an assoc list of filename position font_size */
    rf = (const cst_val *)recent->data;

    if (val_length(rf) > (3*BARD_MAX_NUM_RECENT_FILES)/2)
    {
        for (v=rf; val_cdr(v); v=val_cdr(v));
        bard_delete_recent_file(recent,val_string(val_car(val_car(v))));
    }

    /* Remove it, if it was already there */
    bard_delete_recent_file(recent,text->textfile);
    rf = (const cst_val *)recent->data;

    /* Make new entry for recent file list */
    ne = cons_val(string_val(text->textfile),
            cons_val(int_val(text->sop_pos),
               cons_val(int_val(text->font->size),
                        cons_val(string_val(voicepath),
                                 cons_val(string_val(text->font->name),
                                          NULL)))));
    rf = cons_val(ne,rf);
    recent->data = (void *)rf;

    bard_window_recent_select(recent); /* update the screen */

    return;
}
