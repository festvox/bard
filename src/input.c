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
/*                                                                       */
/*  Key mappings can be platform customized through defining variables.  */
/*  Specifically                                                         */
/*                                                                       */
/*  -DPANDORA  Openpandora (game console keys and keyboard)              */
/*  -DGCW0     GCW-Zero (game console keys only)                         */
/*  none       Ben Nanonote                                              */
/*  none       Standard desktop (with standard keyboard)                 */
/*                                                                       */
/*  The big switch statement is getting complicated, different machines  */
/*  have different, conflicting, keynames, so this will have to be       */
/*  better structured at some point                                      */
/*                                                                       */
/*************************************************************************/

#include "bard.h"
#include "bard_speech.h"
#include "bard_system.h"

int input_process_events(bard_reader *br)
{
    /* Returns 1 if an event was processed, 0 if there are no events */
    SDL_Event event;
    bard_window *cw;
    int unblanked_screen = 0;

    cw = br->display->current;

    if (SDL_PollEvent(&event))
    {
        /* Any event resets the quiet time and switches the screen on */
        br->quiet_time = SDL_GetTicks(); /* An event, reset quiet time */
        if (br->blank == 1)
        {
            br->blank = 0;
            bard_screen_on(NULL);        /* Switch screen on again */
            unblanked_screen = 1;
        }
        switch (event.type)
        {
        case SDL_QUIT:
            br->quit=1;
            break;
        case SDL_MOUSEMOTION:              /* we dont support the mouse (yet)*/
            SDL_ShowCursor(SDL_ENABLE);    /* but display it if you move it */
            br->mouse_quiet_time = SDL_GetTicks();
            break;
        case SDL_KEYDOWN:
	  /*	  printf("bard_debug_X %s key was pressed\n",
		  SDL_GetKeyName(event.key.keysym.sym)); */
            if ((event.key.keysym.sym != SDLK_SPACE) &&
                (event.key.keysym.sym != SDLK_END) &&
#ifndef GCW0
                (event.key.keysym.sym != SDLK_RETURN) &&
#endif
                (event.key.keysym.sym != SDLK_HOME) &&
                (event.key.keysym.sym != SDLK_TAB) &&
                (event.key.keysym.sym != SDLK_s) &&
                (event.key.keysym.sym != SDLK_i) &&  /* kb based speed/vol */
                (event.key.keysym.sym != SDLK_o) &&
#ifdef PANDORA
                (event.key.keysym.sym != SDLK_RSHIFT) &&
                (event.key.keysym.sym != SDLK_RCTRL) &&
#endif
#ifdef GCW0
                (event.key.keysym.sym != SDLK_BACKSPACE) &&
                (event.key.keysym.sym != SDLK_LSHIFT) &&
                (event.key.keysym.sym != SDLK_LCTRL) &&
#endif		
                (event.key.keysym.sym != SDLK_F11) &&
                (event.key.keysym.sym != SDLK_F12))
            {
                br->scroll = 0;            /* anykey stops scrolling */
                br->speak = 0;             /* anykey stops speaking */
            }
            switch (event.key.keysym.sym)
            {
#ifndef GCW0
            case SDLK_ESCAPE:
#endif
            case SDLK_q:
                br->quit=1;
                break;
            case SDLK_p:
                br->pause ^= 1;          /* flip pause state */
                break;
#ifdef GCW0
            case SDLK_LSHIFT:            /* A on GCW0 */
#else
            case SDLK_TAB:
            case SDLK_HOME:              /* SQUARE on Pandora */
#endif
            case SDLK_s:
                if (br->display->current == br->text)
                {
                    br->scroll = 0;
                    br->speak ^= 1;      /* flip speak state */
                }
                else
                    br->speak = 0;
                break;
#ifndef GCW0
            case SDLK_SPACE:         
                if (br->display->current == br->text)
                {
                    br->speak = 0;
                    br->scroll ^= 1;     /* flip scroll state */
                }
                break;
#endif
            case SDLK_F4:
#ifdef PANDORA
            case SDLK_LALT:              /* START on Pandora */
#endif
#ifdef GCW0
            case SDLK_RETURN:            /* START on GCW0 */
#else
            case SDLK_t:                 /* display text screen */
#endif
                if (SDLK_F4 && 
                    (event.key.keysym.mod == KMOD_LALT || 
                     event.key.keysym.mod == KMOD_RALT))
                    br->quit=1;
                else if ((br->display->current == br->text) &&
                         (unblanked_screen == 0))
                {   /* If already displaying text screen flip to info screen */
                    bard_window_update_info_string(br);
                    br->display->current = br->info;
                }
                else
                    br->display->current = br->text;
                break;
            case SDLK_F3:
            case SDLK_f:             /* display file select screen */
                if (br->display->current != br->file_select)
                    br->display->current = br->file_select;
                else
                    br->display->current = br->text;
                break;
            case SDLK_F2:
            case SDLK_g:             /* display general info screen */
                if (br->display->current != br->info)
                {
                    bard_window_update_info_string(br);
                    br->display->current = br->info;
                }
                else
                    br->display->current = br->text;
                break;
            case SDLK_F1:
            case SDLK_h:             /* display help screen */
                if (br->display->current != br->help)
                    br->display->current = br->help;
                else
                    br->display->current = br->text;
                break;
            case SDLK_F5:
            case SDLK_r:             /* display recent files screen */
                if (br->display->current != br->recent)
                    br->display->current = br->recent;
                else
                    br->display->current = br->text;
                break;
            case SDLK_F6:
#ifdef PANDORA
            case SDLK_LCTRL:         /* MENU key on Pandora */
#endif
#ifdef GCW0
            case SDLK_ESCAPE:        /* SELECT key on GCW0 */
#else
            case SDLK_m:             /* display menu screen */
#endif
                if (br->display->current != br->menu)
                    br->display->current = br->menu;
                else
                    br->display->current = br->text;
                break;
            case SDLK_F7:
            case SDLK_v:             /* select voice */
                if ((br->voice_select != NULL) &&
                    (br->display->current != br->voice_select))
                    br->display->current = br->voice_select;
                else
                    br->display->current = br->text;
                break;
            case SDLK_c:             /* select font */
                if ((br->font_select != NULL) &&
                    (br->display->current != br->font_select))
                    br->display->current = br->font_select;
                else
                    br->display->current = br->text;
                break;
            case SDLK_0:             /* display recent files screen */
                if (event.key.keysym.mod == KMOD_LCTRL || event.key.keysym.mod == KMOD_RCTRL)
                {   
                   if (br->display->current != br->recent)
                       br->display->current = br->recent;
                   else
                       br->display->current = br->text;
                }   
                break;
#ifdef GCW0
            case SDLK_LCTRL:         /* X on GCW0 */
#else
            case SDLK_END:           /* CIRCLE on Pandora */
            case SDLK_RETURN:
#endif
                if (br->display->current == br->file_select)
                    bard_window_select_file(br);
                else if (br->display->current == br->text)
                {   
                    br->speak = 0;
                    br->scroll ^= 1;     /* flip scroll state */
                }
                else if (br->display->current == br->recent)
                    bard_window_recent_select_file(br);
                else if (br->display->current == br->menu)
                    bard_window_menu_select_window(br);
                else if (br->display->current == br->voice_select)
                    bard_window_select_voice(br);
                else if (br->display->current == br->font_select)
                    bard_window_choose_font(br);
                else if (br->display->current == br->info)
                    bard_window_info_params(br);
                break;
            case SDLK_PLUS:          /* increase font size */
            case SDLK_KP_PLUS:
#ifdef PANDORA
            case SDLK_RSHIFT:        /* L1 on Pandora */
#endif
#ifdef GCW0
            case SDLK_TAB:           /* L1 on GCW0 */
#endif	      
            case SDLK_i:
                /* Increase scroll speed, volume, font size */
                if ((br->display->current == br->text) && (br->scroll))
                    bard_text_scroll_speed_up(br);
                else if ((br->display->current == br->text) && (br->speak))
                    bard_speech_volume_up(br->speech);
                else
                {
                    bard_font_set(&cw->font,cw->font->name,
                                  cw->font->size+1,
                                  cw->font->style);
                    /* redisplay with new font */
                    bard_window_display_from_pos(cw,cw->sop_pos);
                }
                break;
            case SDLK_MINUS:         /* decrease font size */
            case SDLK_KP_MINUS:
#ifdef PANDORA
            case SDLK_RCTRL:             /* R1 on Pandora */
#endif
#ifdef GCW0
            case SDLK_BACKSPACE:         /* R1 on GCW0 */
#endif
            case SDLK_o:
                /* Decrease scroll speed, volume, font size */
                if ((br->display->current == br->text) && (br->scroll))
                    bard_text_scroll_speed_down(br);
                else if ((br->display->current == br->text) && (br->speak))
                    bard_speech_volume_down(br->speech);
                else 
                {
                    if (cw->font->size > 2)  /* not too wee */
                        bard_font_set(&cw->font,cw->font->name,
                                      cw->font->size-1,
                                      cw->font->style);
                    /* redisplay with new font */
                    bard_window_display_from_pos(cw,cw->sop_pos);
                }
                break;
            case SDLK_DOWN:          /* traverse tokens on screen */
            case SDLK_UP:
            case SDLK_LEFT:
            case SDLK_RIGHT:
                bard_window_token_highlight_move(cw, event.key.keysym.sym);
                break;
            case SDLK_F11:           /* Nanonote volume up */
                if ((br->display->current == br->text) && (br->scroll))
                    bard_text_scroll_speed_up(br);
                else if ((br->display->current == br->text) && (br->speak))
                    bard_speech_volume_up(br->speech);
                else                 /* page up if just displaying */
                    bard_window_page_up(cw);
                break;
            case SDLK_F12:           /* Nanonote volume down */
                if ((br->display->current == br->text) && (br->scroll))
                    bard_text_scroll_speed_down(br);
                else if ((br->display->current == br->text) && (br->speak))
                    bard_speech_volume_down(br->speech);
                else                 /* page down if just displaying */
                    bard_window_page_down(cw);
                break;
            case SDLK_KP7:
                if (event.key.keysym.mod != KMOD_NUM)
                   bard_window_display_from_pos(cw,0);
                break;
#ifdef GCW0
            case SDLK_SPACE:         /* Y on GCW0 */
#else
            case SDLK_PAGEUP:        /* X on Pandora */
#endif
                bard_window_page_up(cw);
                break;
            case SDLK_KP9:
                if (event.key.keysym.mod != KMOD_NUM)
                   bard_window_page_up(cw);
                break;
#ifdef GCW0
            case SDLK_LALT:          /* B on GCW0 */
#else
            case SDLK_PAGEDOWN:      /* Triangle on Pandora */
#endif
                bard_window_page_down(cw);
                break;
            case SDLK_KP3:
                if (event.key.keysym.mod != KMOD_NUM)
                   bard_window_page_down(cw);
                break;
            default:
                if (bard_debug)
                    printf("bard_debug %s key was unprocessed\n",
                           SDL_GetKeyName(event.key.keysym.sym));
                break;
            }
            /* We need to know if we unblanked this cycle so we have */
            /* a secondary variable that gets delayed update */
            if (br->blank == 0)
                unblanked_screen = 0;
            bard_display_update(br->display); /* Update screen */
            break;
        }
        return 1;  /* There was a keyboard event */
    }
    else
        return 0;  /* There wasn't a keyboard event */
}
