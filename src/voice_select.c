/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                      Copyright (c) 2012-2013                          */
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
/*************************************************************************/

#include "bard.h"
#include "bard_system.h"

static void bard_window_voice_select_update(bard_window *w)
{
    bard_window_draw_border(w,1,w->foreground_color);

    return;
}

static void bard_window_voice_select_select(bard_window *w)
{
    char *dircontents;

    if (w->ts) ts_close(w->ts);
    /* Just use the file selection routines to get the file list */
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

void bard_window_select_voice(bard_reader *br)
{
    /* User selected current token */
    char *fullpathname;
    char *r;
    bard_window *vsw;
    int ptti;
    const char *ptt;

    vsw = br->voice_select;

    if (vsw->current_token->file_pos == 0) /* ignore current directory */
        return;
    else if (cst_strchr(vsw->current_token->token,'/'))
    {   /* Its a directory -- so update this page */
        /* vsw->textfile already has a / at the end */
        if (cst_streq("../",vsw->current_token->token) &&
            (!cst_streq(".",vsw->textfile)) &&
            (cst_strlen(vsw->textfile) > 2) &&
            (!cst_streq("/..",&vsw->textfile[cst_strlen(vsw->textfile)-3])))
        {   /* remove the last dirname, rather than append "../" */
            fullpathname = cst_strdup(vsw->textfile);
            r = cst_strrchr(fullpathname,'/');
            *r = '\0';
        }
        else 
        {   /* append selected dirname */
            fullpathname = 
                cst_alloc(char,cst_strlen(vsw->textfile)+1+
                          cst_strlen(vsw->current_token->token)+1);
            if (cst_streq("/",vsw->textfile))
                cst_sprintf(fullpathname,"%s%s",
                            vsw->textfile,vsw->current_token->token);
            else
            {
                /* Because we can get spurious inital spaces (on page breaks) */
                /* skip them -- thus filenames can't start with space -- but  */
                /* that's sort of a good idea any way */
                ptt = vsw->current_token->token;
                for (ptti=0; ptt[ptti] != '\0'; ptti++)
                    if (ptt[ptti] != ' ')
                    {
                        ptt = &ptt[ptti];
                        break;
                    }
                cst_sprintf(fullpathname,"%s/%s",vsw->textfile,ptt);
            }
            /* remove the final */
            fullpathname[cst_strlen(fullpathname)-1] = '\0';
        }
        if (cst_strlen(fullpathname) == 0)
            cst_sprintf(fullpathname,"/");
        /* We're going to assume its openable */
        cst_free(vsw->textfile);
        vsw->textfile = fullpathname;
        vsw->sop_pos = 0;
        bard_window_voice_select_select(vsw);
    }
    else  
    {   /* Its a (voice) file -- so update the default voice */
        fullpathname = 
            cst_alloc(char,cst_strlen(vsw->textfile)+1+
                      cst_strlen(vsw->current_token->token)+1);
        /* Because we can get spurious inital spaces (on page breaks) */
        /* skip them -- thus filenames can't start with space -- but  */
        /* that's sort of a good idea any way */
        ptt = vsw->current_token->token;
        for (ptti=0; ptt[ptti] != '\0'; ptti++)
            if (ptt[ptti] != ' ')
            {
                ptt = &ptt[ptti];
                break;
            }
        cst_sprintf(fullpathname,"%s/%s",vsw->textfile,ptt);

        if (cst_streq(ptt,"kal.flitevox"))
        {  /* In order to support kal_diphone we use a fake file name */
            cst_free(fullpathname);
            fullpathname = cst_strdup("kal.flitevox");
        }

        /* Load voice (old one doesn't need delete, its in the flite list*/
        if (bard_voice_select(br->speech,fullpathname) == FALSE)
        {
            printf("bard_debug: voice select failed to load %s\n",fullpathname);
            /* beep */
        }
        cst_free(fullpathname);

        br->display->current = br->text;
        bard_display_update(br->display);
    }

    return;
}

bard_window *bard_make_voice_select_window(bard_reader *br)
{
    bard_window *vsw;
    int indent = 10;
    const char *font_name;
    int font_size;

    vsw = bard_window_new("Voice Select",
                         br->display->screen_width-(2*indent),
                         br->display->screen_height-(2*indent),
                         br->display->screen->format);
    vsw->x_offset = indent;
    vsw->y_offset = indent;
    font_name = get_param_string(br->config,"-font",BARD_DEFAULT_FONT);
    font_size = get_param_int(br->config,"-voice_select_font_size",
                      get_param_int(br->config,"-font_size",
                                    BARD_DEFAULT_FONT_SIZE));
    bard_font_set(&vsw->font,font_name,font_size,BARD_FONT_STYLE_NORMAL);

    vsw->background_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-voice_select_background_color","black"));
    vsw->foreground_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-voice_select_foreground_color","white"));
    vsw->highlight_color = 
        bard_color_get(br->colors,
         get_param_string(br->config,"-voice_select_highlight_color","blue"));

    vsw->tm=indent;
    vsw->bm=indent;
    vsw->tlm=indent;
    vsw->trm=indent;

    vsw->textfile = cst_strdup(get_param_string(br->config,"-voices_dir",
                                                bard_getwd()));
    vsw->update = bard_window_voice_select_update;

    /* Put something on the screen */
    bard_window_voice_select_select(vsw);

    return vsw;
}

    
    

