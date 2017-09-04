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
#include "bard_display.h"
#include "bard_system.h"

int bard_debug=0;

bard_reader *bard_open(cst_features *config)
{
    bard_reader *br;

    br = cst_alloc(bard_reader,1);
    br->name = "Bard";

    br->config = config;

    br->display = bard_display_open(br->config);
    if (br->display == NULL)
    {
        bard_close(br);
        return NULL;
    }

    br->speech = bard_speech_open(br->config);
    br->speech->asi->userdata = (void *)br;

    br->colors = bard_color_setup();
    /* These aren't customizable -- they only clear the whole display */
    /* which then gets completely covered with another window */
    br->display->background_color = bard_color_get(br->colors,"white");
    br->display->foreground_color = bard_color_get(br->colors,"black");

    /* wait 100ms if no key press happens: varies to affect scroll speed */
    br->no_key_delay = 100;  
    br->scroll_delay = get_param_int(config,"-scroll_delay",br->no_key_delay);
    br->screen_blank_idle_time =
        get_param_float(config,"-screen_blank_idle_time",30.0);
    br->quiet_time = SDL_GetTicks(); /* millisecond time of last keystroke */

    bard_epub_init(br);  /* for epub support, if we have it */

    return br;
}

void bard_close(bard_reader *br)
{

    if (br == NULL)
        return;

    bard_epub_end(br);

    if (br->config)
        delete_features(br->config);

    /* The different windows */
    bard_window_delete(br->text);
    bard_window_delete(br->file_select);
    bard_window_delete(br->info);
    bard_window_delete(br->help);
    bard_window_delete(br->recent);
    bard_window_delete(br->menu);
    if (br->voice_select)
        bard_window_delete(br->voice_select);
    if (br->font_select)
        bard_window_delete(br->font_select);

    /* The structural stuff */
    bard_color_delete(br->colors);
    bard_speech_close(br->speech);
    bard_display_close(br->display);

    cst_free(br);
    return;

}

cst_tokenstream *bard_open_textfile(const char *filename)
{
    size_t lnth = cst_strlen(filename);

    if ((lnth > 5) &&
        ((cst_streq(".epub",&filename[lnth-5])) ||
         (cst_streq(".EPUB",&filename[lnth-5]))))
        /* If we failed to open it, no point in trying standard open */
        /* as if it is a .epub file it is not simple text */
        return ts_bard_open_epub_file(filename);
    else if (((lnth > 5) &&
              ((cst_streq(".html",&filename[lnth-5])) ||
               (cst_streq(".HTML",&filename[lnth-5])))) ||
             ((lnth > 4) &&
              ((cst_streq(".htm",&filename[lnth-4])) ||
               (cst_streq(".HTM",&filename[lnth-4])))))
        return ts_bard_open_xhtml_file(filename);
    else
        return ts_open(filename,
                       cst_ts_default_whitespacesymbols,
                       "",
                       cst_ts_default_prepunctuationsymbols,
                       cst_ts_default_postpunctuationsymbols);
}

cst_features *bard_read_config(cst_features *args)
{
    /* Read config file from $HOME/.bard_config or as specified from */
    /* command line.  Values will be secondary to any values specified */
    /* the command line */
    const char *home;
    char *config_path = NULL;;
    const char *given_config_path = NULL;
    const char *config_file = BARD_DEFAULT_CONFIG_FILE;
    cst_features *config;
    cst_tokenstream *ts;
    char *feat, *val;
    const char *xfeat;

    config = new_features();
    given_config_path = get_param_string(args,"-config_file",NULL);
    if (given_config_path)
    {
        ts = ts_open(given_config_path,
                     cst_ts_default_whitespacesymbols,"","","");
    }
    else
    {
        home = bard_config_dir();
        if (home == NULL)
            home = ".";
        config_path = 
            cst_alloc(char,cst_strlen(home)+1+cst_strlen(config_file)+1);
        cst_sprintf(config_path,"%s/%s",home,config_file);
        ts = ts_open(config_path,
                     cst_ts_default_whitespacesymbols,"","","");
        cst_free(config_path);
    }

    /* Read in each feature with its value */
    /* Although config feats are of the form -featname "value", not all */
    /* feature names can also be command line options */
    while (ts && !ts_eof(ts))
    {
        feat = cst_strdup(ts_get(ts));
        /* Values may be quoted */
        val = cst_strdup(ts_get_quoted_token(ts,'"','\\'));
        /* We do this so the feat name can be gc'd later */
        xfeat = feat_own_string(config,feat);
        /* All features are treat as strings even though they might be */
        /* ints/floats -- they will be converted automatically when accessed */
        feat_set_string(config,xfeat,val);
        cst_free(feat);
        cst_free(val);
    }
    feat_copy_into(args,config);  /* overrides vals from config file */

    if (ts) ts_close(ts);
    return config;
}

static void write_feat_string(cst_file fd, const char *feat, const char *val)
{
    const char *space = " ";
    const char *nl = "\n";
    const char *quote = "\"";
    const char *escape = "\\";
    int i;

    cst_fwrite(fd,feat,sizeof(char),cst_strlen(feat));
    cst_fwrite(fd,space,sizeof(char),cst_strlen(space));
    /* Write it as a quoted string as filenames can have spaces in them */
    cst_fwrite(fd,quote,sizeof(char),1);
    for (i=0; val && val[i]; i++)
    {
        if (val[i] == '"')
            cst_fwrite(fd,escape,sizeof(char),1);
        cst_fwrite(fd,&val[i],sizeof(char),1);
    }
    cst_fwrite(fd,quote,sizeof(char),1);
    cst_fwrite(fd,nl,sizeof(char),cst_strlen(nl));

    return;
}

static void write_feat_int(cst_file fd, const char *feat, int val)
{
    const char *space = " ";
    const char *nl = "\n";
    char num[20];

    cst_fwrite(fd,feat,sizeof(char),cst_strlen(feat));
    cst_fwrite(fd,space,sizeof(char),cst_strlen(space));
    cst_sprintf(num,"%d",val);
    cst_fwrite(fd,num,sizeof(char),cst_strlen(num));
    cst_fwrite(fd,nl,sizeof(char),cst_strlen(nl));

    return;
}

static void write_feat_float(cst_file fd, const char *feat, float val)
{
    const char *space = " ";
    const char *nl = "\n";
    char num[20];

    cst_fwrite(fd,feat,sizeof(char),cst_strlen(feat));
    cst_fwrite(fd,space,sizeof(char),cst_strlen(space));
    cst_sprintf(num,"%f",val);
    cst_fwrite(fd,num,sizeof(char),cst_strlen(num));
    cst_fwrite(fd,nl,sizeof(char),cst_strlen(nl));

    return;
}

void bard_write_config(bard_reader *br)
{
    /* Write config to $HOME/.bard_config for later readering */
    cst_file fd;
    const char *home;
    char *config_path = NULL;;
    const char *given_config_path = NULL;
    const char *config_file = BARD_DEFAULT_CONFIG_FILE;
    char featname[20]; /* max featname plus two digits */
    const cst_val *rf;
    const char *fn;
    int i;

    given_config_path = get_param_string(br->config,"-config_file",NULL);
    if (given_config_path)
        fd = cst_fopen(given_config_path,CST_OPEN_WRITE);
    else
    {
        home = bard_config_dir();
        if (home == NULL)
            home = ".";
        config_path = 
            cst_alloc(char,cst_strlen(home)+1+cst_strlen(config_file)+1);
        cst_sprintf(config_path,"%s/%s",home,config_file);
        fd = cst_fopen(config_path,CST_OPEN_WRITE);
        cst_free(config_path);
    }
    if (fd == NULL)
        return;

    /* Save as simple features -- but values should be quoted strings */
    /* current text file info: name, position, font_size */
    /* list of recent text files (for selection from recent window) */
    write_feat_string(fd,"-text",br->text->textfile); 
    write_feat_int(fd,"-text_pos",br->text->sop_pos);
    write_feat_string(fd,"-font",bard_font_name(br->text->font));
    write_feat_int(fd,"-font_size",bard_font_size(br->text->font));
    write_feat_string(fd,"-text_font",bard_font_name(br->text->font));

    write_feat_string(fd,"-textdir",br->file_select->textfile); 
    write_feat_int(fd,"-screen_height",br->display->screen_height);
    write_feat_int(fd,"-screen_width",br->display->screen_width);
    write_feat_string(fd,"-voice",br->speech->default_voice_pathname); 

    /* voice, volume, speed */
    write_feat_float(fd,"-gain",br->speech->gain); 
    write_feat_float(fd,"-speed",br->speech->speed); 

    write_feat_int(fd,"-scroll_delay",br->scroll_delay); 
    write_feat_float(fd,"-screen_blank_idle_time",br->screen_blank_idle_time); 

    /* Each other window can have its own font size */
    write_feat_int(fd,"-info_font_size",bard_font_size(br->info->font)); 
    write_feat_int(fd,"-file_select_font_size",bard_font_size(br->file_select->font)); 
    write_feat_int(fd,"-recent_font_size",bard_font_size(br->recent->font)); 
    write_feat_int(fd,"-help_font_size",bard_font_size(br->help->font)); 
    write_feat_int(fd,"-menu_font_size",bard_font_size(br->menu->font)); 
    if (br->voice_select)
    {
        write_feat_int(fd,"-voice_select_font_size",bard_font_size(br->voice_select->font)); 
      write_feat_string(fd,"-voices_dir",br->voice_select->textfile);
    }
    if (br->font_select)
    {
        write_feat_int(fd,"-font_select_font_size",bard_font_size(br->voice_select->font)); 
      write_feat_string(fd,"-font_dir",br->font_select->textfile);
    }

    /* colors */
    /* main window */
    write_feat_string(fd,"-text_background_color",
                     bard_color_name(br->colors,br->text->background_color)); 
    write_feat_string(fd,"-text_foreground_color",
                     bard_color_name(br->colors,br->text->foreground_color)); 
    write_feat_string(fd,"-text_highlight_color",
                     bard_color_name(br->colors,br->text->highlight_color)); 
    /* (general) info window */
    write_feat_string(fd,"-info_background_color",
                     bard_color_name(br->colors,br->info->background_color)); 
    write_feat_string(fd,"-info_foreground_color",
                     bard_color_name(br->colors,br->info->foreground_color)); 
    write_feat_string(fd,"-info_highlight_color",
                     bard_color_name(br->colors,br->info->highlight_color)); 
    /* help window */
    write_feat_string(fd,"-help_background_color",
                     bard_color_name(br->colors,br->help->background_color)); 
    write_feat_string(fd,"-help_foreground_color",
                     bard_color_name(br->colors,br->help->foreground_color)); 
    write_feat_string(fd,"-help_highlight_color",
                     bard_color_name(br->colors,br->help->highlight_color)); 
    /* file_select window */
    write_feat_string(fd,"-file_select_background_color",
             bard_color_name(br->colors,br->file_select->background_color)); 
    write_feat_string(fd,"-file_select_foreground_color",
             bard_color_name(br->colors,br->file_select->foreground_color)); 
    write_feat_string(fd,"-file_select_highlight_color",
             bard_color_name(br->colors,br->file_select->highlight_color)); 
    /* recent files window */
    write_feat_string(fd,"-recent_background_color",
                    bard_color_name(br->colors,br->recent->background_color)); 
    write_feat_string(fd,"-recent_foreground_color",
                    bard_color_name(br->colors,br->recent->foreground_color)); 
    write_feat_string(fd,"-recent_highlight_color",
                    bard_color_name(br->colors,br->recent->highlight_color)); 
    /* menu window */
    write_feat_string(fd,"-menu_background_color",
                    bard_color_name(br->colors,br->menu->background_color)); 
    write_feat_string(fd,"-menu_foreground_color",
                    bard_color_name(br->colors,br->menu->foreground_color)); 
    write_feat_string(fd,"-menu_highlight_color",
                    bard_color_name(br->colors,br->menu->highlight_color)); 
    /* voices window (if -voices_dir has been specified) */
    if (br->voice_select)
    {
        write_feat_string(fd,"-voice_select_background_color",
              bard_color_name(br->colors,br->voice_select->background_color)); 
        write_feat_string(fd,"-voice_select_foreground_color",
              bard_color_name(br->colors,br->voice_select->foreground_color)); 
        write_feat_string(fd,"-voice_select_highlight_color",
              bard_color_name(br->colors,br->voice_select->highlight_color)); 
    }
    if (br->font_select)
    {
        write_feat_string(fd,"-font_select_background_color",
              bard_color_name(br->colors,br->font_select->background_color)); 
        write_feat_string(fd,"-font_select_foreground_color",
              bard_color_name(br->colors,br->font_select->foreground_color)); 
        write_feat_string(fd,"-font_select_highlight_color",
              bard_color_name(br->colors,br->font_select->highlight_color)); 
    }

    write_feat_string(fd,"-battery_script",
                    get_param_string(br->config,"-battery_script","battery"));

    rf = (const cst_val *)(void *)br->recent->data;
    for (i=0; rf && i<BARD_MAX_NUM_RECENT_FILES; i++,rf=val_cdr(rf))
    {
        cst_sprintf(featname,"-rf%d_text",i);
        write_feat_string(fd,featname,val_string(val_car(val_car(rf))));
        cst_sprintf(featname,"-rf%d_text_pos",i);
        write_feat_int(fd,featname,
                       val_int(val_car(val_cdr(val_car(rf)))));
        cst_sprintf(featname,"-rf%d_font_size",i);
        write_feat_int(fd,featname,
                       val_int(val_car(val_cdr(val_cdr(val_car(rf))))));
        cst_sprintf(featname,"-rf%d_voice_name",i);
        write_feat_string(fd,featname,
               val_string(val_car(val_cdr(val_cdr(val_cdr(val_car(rf)))))));
        cst_sprintf(featname,"-rf%d_font_name",i);
        if (val_cdr(val_cdr(val_cdr(val_cdr(val_car(rf))))))
            fn = val_string(val_car(val_cdr(val_cdr(val_cdr(val_cdr(val_car(rf)))))));
        else /* while the .bard_config is still old */
            fn = BARD_DEFAULT_FONT;
        write_feat_string(fd,featname,fn);
    }

    cst_fclose(fd);

    return;
}


