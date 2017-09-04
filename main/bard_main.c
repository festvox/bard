/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                       Copyright (c) 2012-2014                         */
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

#include <stdio.h>
#include <unistd.h>
#include "bard.h"
#include "bard_display.h"
#include "bard_system.h"
#include "flite.h"
#include "cst_args.h"

int main(int argc, char **argv)
{
    bard_reader *br;
    cst_features *args;
    cst_features *config;
    float idle_time;
    int spos;

    args = new_features();
    cst_args(argv,argc,
	     "usage: bard OPTIONS\n"
             "Bard Storyteller: "BARD_PROJECT_VERSION"-"BARD_PROJECT_STATE" "BARD_PROJECT_DATE"\n"
             "Most of these options have defaults in $HOME/.bard_config\n"
             "-text <string> Pathname to text file to read\n"
             "-text_pos <int> Starting position in text file\n"
             "-config_file <string> Override default config file ($HOME/.bard_config)\n"
             "-font <string> Pathname to default ttf font\n"
             "-font_size <int> Initial font size\n"
             "-text_font <string> Pathname to ttf font for text window\n"
             "-audio_method <string> SDL or flite\n"
             "-voice <string> Voice name (or pathname to dumped voice)\n"
             "-audio_stream_buffer_factor  <int> Bigger for slower processors\n"
             "-gain <float> Volume factor (1.0 is default)\n"
             "-speed <float> Duration stretch (inverse speed)\n"
             "-battery_script <string> Script to get battery charge state\n"
             "-screen_height <int> \n"
             "-screen_width <int> \n"
             "-screen_blank_idle_time <float> In secs (0 means no blanking)\n"
             "-scroll_delay <int> In ms\n"
             "-voices_dir <string> Default directory containing *.flitevox voices\n"
             "-font_dir <string> Default directory containing ttf fonts\n"
             "-debug <int> Print debug messages\n"
             "",
             args);

    /* If text specified and no position given, start from 0 */
    if (feat_present(args,"-text") && !feat_present(args,"-text_pos"))
        feat_set_int(args,"-text_pos",0);
    if (!feat_present(args,"-font"))
        feat_set_string(args,"-font",BARD_DEFAULT_FONT);
    bard_debug = get_param_int(args,"-debug",0);

    config = bard_read_config(args);
    
    cst_feat_print(stdout,config);

    br = bard_open(config);
    if (br == NULL) return -1;  /* something went really wrong */

    /* Make the windows */
    br->text = bard_make_text_window(br);
    br->file_select = bard_make_file_select_window(br);
    br->info = bard_make_info_window(br); /* general info/params */
    br->help = bard_make_help_window(br);
    br->recent = bard_make_recent_window(br);
    br->menu = bard_make_menu_window(br);
    if (feat_present(br->config,"-voices_dir"))
        br->voice_select = bard_make_voice_select_window(br);
    if (feat_present(br->config,"-font_dir"))
        br->font_select = bard_make_font_select_window(br);
    /* Select focus */
    br->display->current = br->text;

    bard_display_clear(br->display);
    bard_display_update(br->display);
    bard_screen_on(NULL); /* just in case it is currently off */

    while (br->quit == 0)
    {
        if (input_process_events(br) == 0)
        {
            if (br->scroll)
            {
                SDL_Delay(br->scroll_delay); /* no key press but scrolling */
                br->quiet_time = SDL_GetTicks(); /* no blank while scrolling */
            }
            else
                SDL_Delay(br->no_key_delay); /* no key press, so pause a bit */
        }

        if (br->speak == 1)
        {
            /* Within the audio callback function, we also call */
            /* input_process_events so we can stop if requested */
            br->pause = 0;
            if (br->display->current->current_token)
                spos = br->display->current->current_token->file_pos;
            else
                spos = br->display->current->sop_pos;
            /* We need spos-1 here to make it display well */
            bard_speak_text(br->speech,br->display->current->textfile,spos-1);
                            
            br->speak = 0;
            br->quiet_time = SDL_GetTicks();
        }
        else if (br->scroll)
        {   /* Speaking and smooth scrolling are mutually exclusive */
            if (bard_text_scroll(br->text) == 0)
                br->scroll = 0;  /* Reached end of file so stop scroll */
            bard_display_update(br->display); /* Update screen */
        }
        else
            br->text->scroll_offset = 0;

        /* Check if we should black/hide cursor */
        idle_time = (SDL_GetTicks()-br->quiet_time)/1000.0;
        if ((br->screen_blank_idle_time > 0) &&
            (idle_time > br->screen_blank_idle_time) && 
            (br->blank == 0))
        {
            br->blank = 1;
            bard_screen_off(NULL);
        }
        idle_time = (SDL_GetTicks()-br->mouse_quiet_time)/1000.0;
        if (idle_time > 5.0)
            SDL_ShowCursor(SDL_DISABLE);
    }

    /* Save context */
    bard_write_config(br);

    bard_close(br);
    delete_features(args);

    return 0;
}

