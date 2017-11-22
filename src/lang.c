/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2014                            */
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
/*               Date:  July 2014                                        */
/*************************************************************************/
/*                                                                       */
/*  Some functions that deal with language specific features             */
/*                                                                       */
/*************************************************************************/

#include "bard.h"
#include "bard_render.h"

#ifdef BARD_HARFBUZZ

struct bard_lang_font_struct {
    const char *dirname;
    const char *regular_name;
    const char *bold_name;
    const char *language;   /* textual description of language */
    int text_direction;
    hb_script_t script;
};
typedef struct bard_lang_font_struct bard_lang_font;
static bard_lang_font font_descriptions[] = {
    { NULL, "NotoSansDevanagari-Regular.ttf", "NotoSansDevanagari-Bold.ttf",
      "hi", BARD_FONT_DIRECTION_LTR, BARD_SCRIPT_DEVANAGARI },
    { NULL, "NotoSansTamil-Regular.ttf", "NotoSansTamil-Bold.ttf",
      "tamil", BARD_FONT_DIRECTION_LTR, BARD_SCRIPT_TAMIL },
    { NULL, "NotoSansTelugu-Regular.ttf", "NotoSansTelugu-Bold.ttf",
      "telugu", BARD_FONT_DIRECTION_LTR, BARD_SCRIPT_TELUGU },
    { NULL, "NotoSansKannada-Regular.ttf", "NotoSansKannada-Bold.ttf",
      "kannada", BARD_FONT_DIRECTION_LTR, BARD_SCRIPT_KANNADA },
    { NULL, "NotoSansHans-Regular.otf", "NotoSansHans-Bold.otf",
      "ch", BARD_FONT_DIRECTION_LTR, BARD_SCRIPT_HAN },
    { NULL, "NotoSansHant-Regular.otf", "NotoSansHant-Bold.otf",
      "ch", BARD_FONT_DIRECTION_LTR, BARD_SCRIPT_HAN },
    { NULL, "NotoSansJP-Regular.otf", "NotoSansJP-Bold.otf",
      "jp", BARD_FONT_DIRECTION_LTR, BARD_SCRIPT_HAN },
    { NULL, "NotoSerif-Regular.ttf", "NotoSerif-Bold.ttf",
      "en", BARD_FONT_DIRECTION_LTR, BARD_SCRIPT_LATIN },
    { NULL, NULL, NULL, NULL, BARD_FONT_DIRECTION_LTR, BARD_SCRIPT_LATIN }
};

static const char *bard_base_name(const char *pathname)
{
    const char *r;

    r = cst_strrchr(pathname,'/');

    if (r)
        return &r[1];
    else
        return NULL;
}

char *bard_font_get_font_name(const char *font_name,int style)
{
    /* Note the font_name should always be the the "regular" name */
    const char *fn;
    char *r;
    char *nfn;
    int i;

    fn = bard_base_name(font_name); /* just the font file name */

    for (i=0; font_descriptions[i].regular_name; i++)
    {
        if (cst_streq(fn,font_descriptions[i].regular_name))
        {
            if (style == BARD_FONT_STYLE_BOLD)
            {
                /* Longer than it needs to be */
                nfn = cst_alloc(char,cst_strlen(font_name)+
                                cst_strlen(font_descriptions[i].bold_name));
                cst_sprintf(nfn,"%s",font_name);
                r = cst_strrchr(nfn,'/');
                cst_sprintf(&r[1],"%s",font_descriptions[i].bold_name);
                return nfn;
            }
            if (style != BARD_FONT_STYLE_BOLD)
            {
                /* Longer than it needs to be */
                nfn = cst_alloc(char,cst_strlen(font_name)+
                                cst_strlen(font_descriptions[i].regular_name));
                cst_sprintf(nfn,"%s",font_name);
                r = cst_strrchr(nfn,'/');
                cst_sprintf(&r[1],"%s",font_descriptions[i].regular_name);
                return nfn;
            }
        }
    }

    /* Couldn't find an appropriate pathname current font style */
    return cst_strdup(font_name);
}

int bard_font_set_lang_features(bard_font *bf)
{
    /* Somehow need to set this depending on the font/language */
    const char *fn;
    int i;

    fn = bard_base_name(bf->name); /* just the font file name */

    for (i=0; font_descriptions[i].regular_name; i++)
    {
        if (cst_streq(fn,font_descriptions[i].regular_name))
        {
            bf->text_direction = font_descriptions[i].text_direction;
            bf->language = font_descriptions[i].language;
            bf->script = font_descriptions[i].script;
            return 0;
        }
    }

    bf->text_direction = BARD_FONT_DIRECTION_LTR;
    bf->language = "en";
    bf->script = BARD_SCRIPT_LATIN;

    return 0;
}
#endif

int bard_utf8_sequence_length(const char c0)
{
    // Get the expected length of UTF8 sequence given its most
    // significant byte
    return (( 0xE5000000 >> (( c0 >> 3 ) & 0x1E )) & 3 ) + 1;
}

static void bard_debug_dump_string(const char *token)
{
    cst_val *utflets, *ords;
    const cst_val *v, *tmpv;

    utflets = cst_utf8_explode(token);
    ords = NULL;

    /* chars to ord */
    for (v=utflets; v; v=val_cdr(v)) {
        tmpv = cst_utf8_ord(val_car(v));
        /* printf("awb_debug %s 0x%x\n",
               val_string(val_car(v)),
               val_int(tmpv)); */
        ords = cons_val(tmpv, ords);
    }
    ords = val_reverse(ords);

    return;
}

int bard_lang_breakable_language(const char *token)
{
    int char_len, i;
    int char_ord;
    char char_utf8[6];
    int j,k;
    /* Return true if this token is in a language that (probably) can */
    /* can be analyzed character by character and doesn't have word breaks */
    /* i.e. Chinese, Japanese or Thai */

    /* Scan the string, for the unicode class */
    /* map the class to language */
    /* But if its a mixed token with lots of languages ? */
    /* CJK: 4E00–9FFF -- but we want to distinguish Korean from CJ */
    /* CJ can have arbitrary breaks, Korean has spaces */
    for (i=0; token[i]; i++)
    {
        char_len = bard_utf8_sequence_length(token[i]);
        if (char_len > 2)
        {
            /* Will be true for Chinese and Japanese, and not Hindi */
            if (1==0)
                bard_debug_dump_string(token);
            return TRUE;  /* naively true so far */
        }

        if (1==0)
        {
            /* The rest of this is sort of debug code at present */
            /* The length thing is right -- but doesn't get the right ord val */
            for (char_ord=0,k=0,j=char_len; j>0; j--,k++)
            {
                char_ord = (char_ord * 256) + (unsigned char)token[i+k];
                if (k < 5) char_utf8[k] = token[i+k];
            }
            if (k < 6) char_utf8[k] = '\0';
            /* printf("awb_debug %d 0x%x >%s<\n",char_len,char_ord,char_utf8); */
        }
        i+=(char_len-1);
    }

    return FALSE;
}
    

