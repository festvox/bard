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
/*  Bard Storyteller                                                     */
/*************************************************************************/
#ifndef _BARD_
#define _BARD_

#include "cst_features.h"
#include "cst_tokenstream.h"
#include "bard_version.h"
#include "bard_display.h"
#include "bard_render.h"
#include "bard_window.h"
#include "bard_speech.h"

#define BARD_DEFAULT_CONFIG_FILE ".bard_config"
#ifndef BARD_DEFAULT_FONT
#define BARD_DEFAULT_FONT "/usr/share/fonts/liberation/LiberationSerif-Regular.ttf"
#endif
#define BARD_DEFAULT_FONT_SIZE 20
#ifdef GCW0
/* Some screens are too small, so its good to have a reasonable (device
   specific) size to the user can see the options */
#define BARD_DEFAULT_MENU_FONT_SIZE 16
#define BARD_DEFAULT_INFO_FONT_SIZE 19
#else
#define BARD_DEFAULT_MENU_FONT_SIZE BARD_DEFAULT_FONT_SIZE
#define BARD_DEFAULT_INFO_FONT_SIZE BARD_DEFAULT_FONT_SIZE
#endif
#define BARD_MAX_NUM_RECENT_FILES 20

extern int bard_debug;

struct bard_reader_struct {
    const char *name;

    cst_features *config;

    bard_display *display;
    bard_speech *speech;

    /* The main windows */
    bard_window *text;        /* The main text display window */
    bard_window *help;        /* Help window */
    bard_window *file_select; /* File select and filesystem navigation */
    bard_window *info;        /* General info about the current file */
    bard_window *recent;      /* List of recent files */

    bard_window *menu;         /* List of other windows */
    bard_window *voice_select; /* Select other voice */
    bard_window *font_select;  /* Select other font */

    bard_colors *colors;

    int no_key_delay;  /* How long to wait (ms) if there is no key press */
    int scroll_delay;  /* how long to wait when scrolling */

    int quit;
    int pause;
    int speak;
    int scroll;
    int blank;            /* 1 if we blanked */
    float screen_blank_idle_time; /* in seconds */
    int quiet_time;       /* ms since last keystroke */

    int mouse_quiet_time; /* ms since last mouse motion */

};
typedef struct bard_reader_struct bard_reader;

bard_reader *bard_open(cst_features *config);
void bard_close(bard_reader *br);
cst_tokenstream *bard_open_textfile(const char *filename);

int input_process_events(bard_reader *br);
bard_window *bard_make_text_window(bard_reader *br);
bard_window *bard_make_help_window(bard_reader *br);
bard_window *bard_make_file_select_window(bard_reader *br);
bard_window *bard_make_info_window(bard_reader *br);
bard_window *bard_make_recent_window(bard_reader *br);
bard_window *bard_make_menu_window(bard_reader *br);
bard_window *bard_make_voice_select_window(bard_reader *br);
bard_window *bard_make_font_select_window(bard_reader *br);

int bard_text_scroll(bard_window *tw);
void bard_text_scroll_speed_up(bard_reader *br);
void bard_text_scroll_speed_down(bard_reader *br);

void bard_window_select_file(bard_reader *br);
void bard_window_info_params(bard_reader *br);
void bard_window_update_info_string(bard_reader *br);
void bard_add_recent_file(bard_window *recent, bard_window *text,
                          const char *voicepath);
void bard_window_recent_select_file(bard_reader *br);
void bard_window_menu_select_window(bard_reader *br);
void bard_window_select_voice(bard_reader *br);
void bard_window_choose_font(bard_reader *br);

void bard_window_file_select_select(bard_window *w);

cst_features *bard_read_config(cst_features *args);
void bard_write_config(bard_reader *br);

/* For EPUB support -- skeleton functions are used if epub unsupported */
cst_tokenstream *ts_bard_open_epub_file(const char *filename);
void bard_epub_init(bard_reader *br);
void bard_epub_end(bard_reader *br);
cst_features *bard_epub_get_tags(cst_tokenstream *ts);
char *bard_epub_grab_image(cst_tokenstream *ts,int *size);

SDL_Surface *bard_image_render(bard_window *w,char *image,int size);
SDL_Surface *bard_image_fit_window(bard_window *w, SDL_Surface *image);

char *bard_file_select_dircontents(const char *dirname);

/* These might move into flite's cst_tokenstream at some point */
cst_tokenstream *ts_bard_open_xhtml_file(const char *filename);
cst_features *bard_xhtml_read_tag_attributes(const char *ta);
cst_features *bard_xhtml_read_tag_attributes_ts(cst_tokenstream *ts);
int bard_xhtml_getc(cst_tokenstream *ts);

cst_features *bard_xhtml_get_tags(cst_tokenstream *ts);

extern const char *xhtml_singlecharsymbols_general;
extern const char *xhtml_singlecharsymbols_inattr;
extern const char *xhtml_singlecharsymbols_all;

extern const char * const bard_filename_whitespacesymbols;
extern const char * const bard_filename_singlecharsymbols;
extern const char * const bard_filename_prepunctuationsymbols;
extern const char * const bard_filename_postpunctuationsymbols;

/* Language specific functions */
int bard_lang_breakable_language(const char *token);
int bard_font_set_lang_features(bard_font *bardfont);
char *bard_font_get_font_name(const char *font_name,int style);

#endif
