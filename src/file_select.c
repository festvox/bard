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

/* Allow space and tab in file names */
const char * const bard_filename_whitespacesymbols = "\n\r";
const char * const bard_filename_singlecharsymbols = "";
const char * const bard_filename_prepunctuationsymbols = "/";
const char * const bard_filename_postpunctuationsymbols = "/";

void bard_window_file_select_select(bard_window *w)
{
    char *dircontents;

    if (w->ts) ts_close(w->ts);
    dircontents = bard_file_select_dircontents(w->textfile);
    if (cst_streq(dircontents,"cannot open dir\n"))  /* can't open directory */
    {
        cst_free(w->textfile);
        w->textfile = cst_strdup(".");
        dircontents = bard_file_select_dircontents(w->textfile);
        if (cst_streq(dircontents,"cannot open dir\n"))
        {
            cst_free(w->textfile);
            w->textfile = cst_strdup("/");
            dircontents = bard_file_select_dircontents(w->textfile);
        }
        /* If that fails there isn't much we can do ... */
    }
    w->ts = ts_open_string(dircontents,
                     bard_filename_whitespacesymbols,
                     bard_filename_singlecharsymbols,
                     bard_filename_prepunctuationsymbols,
                     bard_filename_postpunctuationsymbols);
    w->ts_mode = "literal";

    bard_window_display_from_pos(w,w->sop_pos);

    cst_free(dircontents);
}

static void bard_window_file_select_update(bard_window *w)
{
    bard_window_draw_border(w,1,w->foreground_color);

    return;
}

void bard_window_select_file(bard_reader *br)
{
    /* User selected current token */
    char *fullpathname;
    char *r;
    bard_window *fsw, *tw;
    cst_tokenstream *ts;
    int ptti;
    const char *ptt;

    fsw = br->file_select;
    tw = br->text;

    if (fsw->current_token->file_pos == 0) /* ignore current directory title */
        return;
    else if (cst_strchr(fsw->current_token->token,'/'))
    {   /* Its a directory -- so update this page */
        /* fsw->textfile already has a / at the end */
        if (cst_streq("../",fsw->current_token->token) &&
            (!cst_streq(".",fsw->textfile)) &&
            (cst_strlen(fsw->textfile) > 2) &&
            (!cst_streq("/..",&fsw->textfile[cst_strlen(fsw->textfile)-3])))
        {   /* remove the last dirname, rather than append "../" */
            fullpathname = cst_strdup(fsw->textfile);
            r = cst_strrchr(fullpathname,'/');
            if (r != NULL)
               *r = '\0';
        }
        else 
        {   /* append selected dirname */
            fullpathname = 
                cst_alloc(char,cst_strlen(fsw->textfile)+1+
                          cst_strlen(fsw->current_token->token)+1);
            if (cst_streq("/",fsw->textfile))
                cst_sprintf(fullpathname,"%s%s",
                            fsw->textfile,fsw->current_token->token);
            else
            {
                /* Because we can get spurious inital spaces (on page breaks) */
                /* skip them -- thus filenames can't start with space -- but  */
                /* that's sort of a good idea any way */
                ptt = fsw->current_token->token;
                for (ptti=0; ptt[ptti] != '\0'; ptti++)
                    if (ptt[ptti] != ' ')
                    {
                        ptt = &ptt[ptti];
                        break;
                    }
                cst_sprintf(fullpathname,"%s/%s",fsw->textfile,ptt);
            }
            /* remove the final slash */
            fullpathname[cst_strlen(fullpathname)-1] = '\0';
        }
        if (cst_strlen(fullpathname) == 0)
            cst_sprintf(fullpathname,"/");
        /* We're going to assume its openable */
        cst_free(fsw->textfile);
        fsw->textfile = fullpathname;
        fsw->sop_pos = 0;
        bard_window_file_select_select(fsw);
    }
    else  
    {   /* Its a file -- so update the text page and the recents */
        fullpathname = 
            cst_alloc(char,cst_strlen(fsw->textfile)+1+
                      cst_strlen(fsw->current_token->token)+1);
        /* Because we can get spurious inital spaces (on page breaks) */
        /* skip them -- thus filenames can't start with space -- but  */
        /* that's sort of a good idea any way */
        ptt = fsw->current_token->token;
        for (ptti=0; ptt[ptti] != '\0'; ptti++)
            if (ptt[ptti] != ' ')
            {
                ptt = &ptt[ptti];
                break;
            }
        cst_sprintf(fullpathname,"%s/%s",fsw->textfile,ptt);
        ts = bard_open_textfile(fullpathname);
        if (ts == NULL)
        {   /* file not openable */
            cst_free(fullpathname);
            return;
        }
        /* Add current file to recent list */
        bard_add_recent_file(br->recent,br->text,
                             br->speech->default_voice_pathname);
        /* Update text window to newly selected file */
        if (tw->ts)
        {
            cst_free(tw->textfile);
            ts_close(tw->ts);
        }
        delete_val((cst_val *)(void *)tw->previous_pages);
        tw->previous_pages = NULL;

        tw->textfile = fullpathname;
        tw->ts = ts;
        tw->sop_pos = 0;
        /* Reset to font NORMAL style */
        bard_font_set(&tw->font,tw->font->name,tw->font->size,BARD_FONT_STYLE_NORMAL);
        br->speak = 0;
        br->display->current = tw;
        bard_window_display_from_pos(tw,tw->sop_pos);
    }

    return;
}

bard_window *bard_make_file_select_window(bard_reader *br)
{
    bard_window *fsw;
    int indent = 10;
    const char *font_name;
    int font_size;

    fsw = bard_window_new("File Select",
                         br->display->screen_width-(2*indent),
                         br->display->screen_height-(2*indent),
                         br->display->screen->format);
    fsw->x_offset = indent;
    fsw->y_offset = indent;
    font_name = get_param_string(br->config,"-font",BARD_DEFAULT_FONT);
    font_size = 
        get_param_int(br->config,"-file_select_font_size",
                      get_param_int(br->config,"-font_size",
                                    BARD_DEFAULT_FONT_SIZE));
    bard_font_set(&fsw->font,font_name,font_size,BARD_FONT_STYLE_NORMAL);

    fsw->background_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-file_select_background_color","black"));
    fsw->foreground_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-file_select_foreground_color","white"));
    fsw->highlight_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-file_select_highlight_color","blue"));

    fsw->tm=indent;
    fsw->bm=indent;
    fsw->tlm=indent;
    fsw->trm=indent;

    fsw->textfile = cst_strdup(get_param_string(br->config,"-textdir",
                                                bard_getwd()));
    fsw->update = bard_window_file_select_update;

    /* Put something on the screen */
    bard_window_file_select_select(fsw);

    return fsw;
}

    
    

