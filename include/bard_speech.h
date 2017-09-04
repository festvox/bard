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
/*  Speech interface (flite)                                             */
/*************************************************************************/
#ifndef _BARD_SPEECH_
#define _BARD_SPEECH_

#include "flite.h"
#include "cst_features.h"
#include "cst_tokenstream.h"

struct bard_speech_struct
{
    cst_val *voice_list;
    char *default_voice_pathname;
    cst_voice *default_voice;
    cst_audio_streaming_info *asi;
    const char *method;

    float gain;
    float speed;

    cst_item *token; /* token we are currently speaking */
    float tokenend;  /* end time of current token */

    /* Flite (native) audio device */
    cst_audiodev *ad;
    /* SDL audio open flag */
    int sdl_open_audio;
};
typedef struct bard_speech_struct bard_speech;

bard_speech *bard_speech_open(cst_features *config);
void bard_speech_close(bard_speech *bs);
int bard_voice_select(bard_speech *bs,const char *voicepath);
int bard_speak_text(bard_speech *bs,const char *textfile,int pos);
void bard_speech_volume_up(bard_speech *bs);
void bard_speech_volume_down(bard_speech *bs);


#endif
