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
#include "bard_speech.h"
#include "bard_system.h"
#include "flite.h"

#include <SDL_audio.h>

/* There should be a cleaner way to do this with flite */
/* These are neded here for language and voice list initialization */
void usenglish_init(cst_voice *v);
cst_lexicon *cmu_lex_init(void);
void cmu_indic_lang_init(cst_voice *v);
cst_lexicon *cmu_indic_lex_init(void);
cst_voice *register_cmu_us_kal(const char *voxdir);

static int bard_SDL_PauseAudio(int p);
static void bard_speech_device_close(bard_speech *bs);

/* SDL requires globals to post data to play thread */
/* These are only written to by one thread -- they are read by both */
static unsigned char *GLOBAL_sound = 0;
static int GLOBAL_read_pos = 0;   
static int GLOBAL_write_pos = 0;
/* These might not need to be global, but SDL audio playing is already */
/* not implicitly thread safe */
static int GLOBAL_pause_state = 0;
static SDL_AudioSpec GLOBAL_fmt;

static void bard_SDL_play_audio(void *unused, Uint8 *stream, int len)
{
    int amount;

    amount = GLOBAL_write_pos-GLOBAL_read_pos;
    if (amount < len)
    {
        /* Near end of utterance ??? */
        memcpy(stream,&GLOBAL_sound[GLOBAL_read_pos],amount);
        /* not sure if this is really necessary */
        memset (&stream[amount], '\0', len-amount);
        /* printf("awb_debug had to write %d zeros len %d amount %d\n",len-amount,len,amount); */
    }
    else
    {
        amount = len;
        memcpy(stream,&GLOBAL_sound[GLOBAL_read_pos],amount);
    }
    GLOBAL_read_pos += amount;

    /* I've no idea why this makes it work better -- but it does */
    SDL_Delay(1); /* on awbt60 this makes slt work ok */
    return;
}

static int bard_SDL_PauseAudio(int p)
{
    if (GLOBAL_pause_state != p)
    {
        SDL_PauseAudio(p);
        GLOBAL_pause_state = p;
    }
    return GLOBAL_pause_state;
}

static int bard_SDL_post_audio(bard_reader *br,
                               const cst_wave *w, int start_index, int size, 
                               int last, cst_audio_streaming_info *asi)
{
    if ((start_index == 0) || (br->speech->sdl_open_audio == 0))
    {
        /* Zero it, as we may have changed voices */
        memset(&GLOBAL_fmt,0,sizeof(SDL_AudioSpec));
        GLOBAL_fmt.freq = w->sample_rate;
        GLOBAL_fmt.format = AUDIO_S16SYS;
        GLOBAL_fmt.channels = 1;
        GLOBAL_fmt.samples = 1024;
        GLOBAL_fmt.callback = bard_SDL_play_audio;
        GLOBAL_fmt.userdata = NULL;
        if (br->speech->sdl_open_audio == 0)
        {
            if (SDL_OpenAudio(&GLOBAL_fmt, NULL) < 0)
            {
                if (bard_debug)
                    printf("bard_debug audio open failed\n");
                return CST_AUDIO_STREAM_STOP;
            }
            br->speech->sdl_open_audio = 1;
        }
	GLOBAL_sound = (unsigned char *)w->samples;
	GLOBAL_read_pos = 0;
	GLOBAL_write_pos = 0;
	GLOBAL_pause_state = 1;
    }

    /* SDL_LockAudio(); */
    GLOBAL_write_pos += size*2;
    /* SDL_UnlockAudio(); */

    if ((GLOBAL_write_pos-GLOBAL_read_pos < 1000) && (last != 1)) 
    {   /* not enough audio */
        bard_SDL_PauseAudio(1);
    }
    else
        bard_SDL_PauseAudio(0);

    if (last == 1)
    {
        bard_SDL_PauseAudio(0);
        while (GLOBAL_write_pos-GLOBAL_read_pos > 0)
        {
            SDL_Delay(1); /* to drain audio channel */
        }
        bard_SDL_PauseAudio(1);
    }
    return CST_AUDIO_STREAM_CONT;
}

static int bard_flite_post_audio(bard_reader *br,
                                 const cst_wave *w, int start_index, int size, 
                                 int last, cst_audio_streaming_info *asi)
{
    /* works on leith, but fails on second opening on Ben with alsa errs */
    if (br->speech->ad == NULL)
    {
        br->speech->ad = 
            audio_open(w->sample_rate,w->num_channels,CST_AUDIO_LINEAR16);
        if (br->speech->ad == NULL)
            return CST_AUDIO_STREAM_STOP;
    }

    audio_write(br->speech->ad,&w->samples[start_index],size*sizeof(short));

    if (last == 1)
    {
        audio_close(br->speech->ad);
        br->speech->ad = NULL;
    }

    return CST_AUDIO_STREAM_CONT;
}

static int bard_audio_stream_chunk(const cst_wave *w, int start_index, 
                                   int size, int last, 
                                   cst_audio_streaming_info *asi)
{
    /* Somewhat scary function that will recurse on input_process_events() */
    /* in case some event has happened that affects our current playing */
    bard_reader *br;
    cst_item *first_token;
    int rc, i;
    float wavepos;
    /* This should be a factor of the play thread's buffer size  */
    /* but that actual value seems to be very platform dependent */
    /* in spite of what we request in the SDL_AudioSpec above.   */
    /* So we guess and modify it once we have more information */
    int bs = 8192; /* seems right for ben, awbt60, leith ... */
    int togo;

    br = (bard_reader *)asi->userdata;

    /* Check for any key presses to stop/start, volume change etc */
    do {   
        input_process_events(br); 
        if ((br->speak == 0) || (br->quit == 1))
        {   /* Stop -- but tidy up first */
            bard_speech_device_close(br->speech);
            return CST_AUDIO_STREAM_STOP; 
        }
        /* GLOBAL_* used in SDL mode only, but will be 0 in flite mode */
        /* so the following wont do anything bad in flite audio mode */
        if ((br->pause == 1) || (GLOBAL_write_pos-GLOBAL_read_pos > bs))
        {
            togo = GLOBAL_write_pos-GLOBAL_read_pos;
            SDL_Delay(br->no_key_delay); /* wait 100ms, rather than burn cpu */
            /* In case the audio stream has stopped */
            if (togo == GLOBAL_write_pos-GLOBAL_read_pos)
                break;
        }
    } while ((br->pause == 1) || (GLOBAL_write_pos-GLOBAL_read_pos > bs));

#ifndef GCW0
    /* When screenblank happens on GCW0 you get grshh sounds, it's probably
       something more complex than just screenblank, so don't do the 
       screenblank on that platform */
    if ((br->screen_blank_idle_time > 0) &&
        (((SDL_GetTicks()-br->quiet_time)/1000.0) > br->screen_blank_idle_time))
    {
        if (br->blank == 0)
        {
            br->blank = 1;
            bard_screen_off(NULL);
        }
    }
#endif

    /* Update screen to start of utterance position */
    if (start_index == 0)
    {
        first_token = relation_head(utt_relation(asi->utt,"Token"));
        if (first_token)
        {
            /* Even without screen updates to start of utt, these two */
            /* lines are necessary for highlighting the current spoken word */
            br->speech->token = first_token;
            br->speech->tokenend = ffeature_float(br->speech->token,
         "R:Token.daughtern.R:SylStructure.daughtern.daughtern.R:Segment.end");
            
            /* This is cute, but sort of gimmicky, not sure this will stay */
            /* Redisplay the screen with the start of the utterance at     */
            /* the top-left of the screen.                                 */
            /* We need file_pos+1 here to make it display well */
            /* yes, that's somewhat magic, but it works        */
            bard_window_display_from_pos(br->text,
                      ffeature_int(first_token,"file_pos")+1);
            bard_display_update(br->display);
        }
    }

    /* Highlight word being spoken on the screen */
    if (br->speech->token)
    {   
        wavepos = ((float)start_index)/w->sample_rate;
        if ((wavepos > br->speech->tokenend) &&
            (item_next(br->speech->token)))
        {
            bard_window_token_highlight_move(br->text,SDLK_RIGHT);
            br->speech->token = item_next(br->speech->token);
            br->speech->tokenend = ffeature_float(br->speech->token,
         "R:Token.daughtern.R:SylStructure.daughtern.daughtern.R:Segment.end");
            bard_display_update(br->display);
        }
    }

    /* Apply user volume control */
    if (br->speech->gain != 1.0)
    {   
        for (i=0; i<=size; i++)
            w->samples[start_index+i] = 
                (short)((float)w->samples[start_index+i]*br->speech->gain);
    }

    /* If the utterance is one only segment (pau) then don't speak it  */
    /* audio books sometimes have lots of empty utterances (tags) that */
    /* don't need 200ms of silence to be spoken, so skip those utts    */
    if (asi->utt &&
        (item_next(relation_head(utt_relation(asi->utt,"Segment"))) == NULL))
        return CST_AUDIO_STREAM_CONT;

    /* Post the new audio to the playing thread */
    if (cst_streq(br->speech->method,"SDL"))
        rc = bard_SDL_post_audio(br,w,start_index,size,last,asi);
    else
        rc = bard_flite_post_audio(br,w,start_index,size,last,asi);

    return rc;
}

int bard_speak_text(bard_speech *bs,const char *textfile,int pos)
{
    /* Speak text in textfile from position pos */
    cst_tokenstream *ts;
    cst_audio_streaming_info *asi;

    /* Set up call back function (if not already done) */
    if (bs->default_voice == NULL)
        return 0;
    if (!feat_present(bs->default_voice->features,"streaming_info"))
    {
        /* Make a new asi and copy in relevant information -- so we can */
        /* change voices without any problems */
        asi = new_audio_streaming_info();
        asi->min_buffsize = bs->asi->min_buffsize;
        asi->asc = bard_audio_stream_chunk;
        asi->userdata = bs->asi->userdata;
        feat_set(bs->default_voice->features,
                 "streaming_info",audio_streaming_info_val(asi));
    }

    /* Where to start speaking from */
    feat_set_int(bs->default_voice->features,"file_start_position",pos);

    /* it might be an epub file, and require special epub open */
    ts = bard_open_textfile(textfile); 
    if (ts == NULL) return 0;

    flite_ts_to_speech(ts,bs->default_voice,"stream");
        
    return 1;
}

bard_speech *bard_speech_open(cst_features *config)
{
    bard_speech *bs;
    
    bs = cst_alloc(bard_speech,1);

    flite_init();
    flite_add_lang("eng",usenglish_init,cmu_lex_init);
    flite_add_lang("cmu_indic_lang",cmu_indic_lang_init,cmu_indic_lex_init);

    /* Always include the kal diphone voice, but can load cg voices */
    /* if named on command line or through the voice menu */
    if (flite_voice_list == NULL)
        flite_voice_list = 
            cons_val(voice_val(register_cmu_us_kal(NULL)),flite_voice_list);
    bs->voice_list = flite_voice_list;

    bard_voice_select(bs,get_param_string(config,"-voice","kal"));

    /* The relevant parts get copied into the voice when it speaks */
    bs->asi = new_audio_streaming_info();
    bs->asi->asc = bard_audio_stream_chunk;
    bs->asi->min_buffsize *= 
        get_param_int(config,"-audio_stream_buffer_factor",2);
    /* asi->userdata will be set above to the bard_reader struct */

    bs->method = get_param_string(config,"-audio_method","SDL");

    bs->gain = get_param_float(config,"-gain",1.0);
    bs->speed = get_param_float(config,"-speed",1.0);

    return bs;
}

static void bard_speech_device_close(bard_speech *bs)
{
    /* Also required at voice change time */
    if (bs->ad)
    {
        audio_close(bs->ad);
        GLOBAL_write_pos = 0;
        GLOBAL_read_pos = 0;
        bs->ad = NULL;
    }

    if (bs->sdl_open_audio == 1)
    {
        SDL_CloseAudio();
        bs->sdl_open_audio = 0;
    }

    return;
}

void bard_speech_close(bard_speech *bs)
{
    if (bs == NULL)
        return;

    bard_speech_device_close(bs);

    if (bs->asi)
    {   
        delete_audio_streaming_info(bs->asi);
        bs->asi = NULL;
    }

    delete_val(bs->voice_list);
    cst_free(bs->default_voice_pathname);
    cst_free(bs);

    return;
}

void bard_speech_volume_down(bard_speech *bs)
{
    if (bs->gain > 0.2)
        bs->gain /= 1.1;
    return;
}

void bard_speech_volume_up(bard_speech *bs)
{
    if (bs->gain < 5.0)
        bs->gain *= 1.1;
    return;
}

int bard_voice_select(bard_speech *bs,const char *voicepath)
{
    cst_voice *new_voice;

    new_voice = flite_voice_select(voicepath);

    if (new_voice)
    {
        if (bs->default_voice_pathname) 
            cst_free(bs->default_voice_pathname);
        bs->default_voice_pathname = cst_strdup(voicepath);
        bs->default_voice = new_voice;
        bard_speech_device_close(bs);
        return 1;
    }
    else
        return 0;
}

