/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                      Copyright (c) 2012-2014                          */
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
/*               Date:  February 2012                                    */
/*************************************************************************/
/*                                                                       */
/*  For processing xhtml token streams                                   */
/*                                                                       */
/*************************************************************************/

#include "bard.h"
#include "cst_tokenstream.h"
#include "cst_features.h"

/* These aren't quite right, the "=" in the general case shouldn't have */
/* to be there, but it makes it work */
const char *xhtml_singlecharsymbols_general = "<>&/\"=";
const char *xhtml_singlecharsymbols_inattr = "=>;/\"";
const char *xhtml_singlecharsymbols_all = "<=>&;/\"!";

struct bard_xhtml_info_struct {
    cst_val *unused_tags; /* now this is moved into the standard token stream */
    /* But we keep an unused field here in case we need to add s'thing later */
};
typedef struct bard_xhtml_info_struct bard_xhtml_info;

/* Wrappers for reading xhtml files (and putting tags in tag */
static int bard_xhtml_open(cst_tokenstream *ts,const char *filename);
static void bard_xhtml_close(cst_tokenstream *ts);
static int bard_xhtml_eof(cst_tokenstream *ts);
static int bard_xhtml_seek(cst_tokenstream *ts, int pos);
static int bard_xhtml_tell(cst_tokenstream *ts);
static int bard_xhtml_size(cst_tokenstream *ts);

cst_tokenstream *ts_bard_open_xhtml_file(const char *filename)
{
    return ts_open_generic(filename,
                           cst_ts_default_whitespacesymbols,
                           xhtml_singlecharsymbols_general,
                           cst_ts_default_prepunctuationsymbols,
                           cst_ts_default_postpunctuationsymbols,
                           NULL,
                           bard_xhtml_open,
                           bard_xhtml_close,
                           bard_xhtml_eof,
                           bard_xhtml_seek,
                           bard_xhtml_tell,
                           bard_xhtml_size,
                           bard_xhtml_getc);

}

static int bard_xhtml_open(cst_tokenstream *ts,const char *filename)
{
    bard_xhtml_info *bx;

    bx = cst_alloc(bard_xhtml_info,1);
    ts->streamtype_data = (void *)bx;
    ts->fd = cst_fopen(filename,CST_OPEN_READ|CST_OPEN_BINARY);
    if (ts->fd)
        return 1;
    else
        return 0;
}

static void bard_xhtml_close(cst_tokenstream *ts)
{
    bard_xhtml_info *bx;
    bx = (bard_xhtml_info *)ts->streamtype_data;

    if (bx->unused_tags)
        delete_val(bx->unused_tags);
    cst_free(bx);
    return;
}

static int bard_xhtml_eof(cst_tokenstream *ts)
{
    return ts_eof(ts);
}

static int bard_xhtml_seek(cst_tokenstream *ts, int pos)
{
    int newpos;

    /* Because sub-calls to specifics are conditioned on ts->open */
    ts->open = NULL;
    newpos = ts_set_stream_pos(ts,pos);
    ts->open = bard_xhtml_open;
    return newpos;
}

static int bard_xhtml_tell(cst_tokenstream *ts)
{
    int pos;

    /* Because sub-calls to specifics are conditioned on ts->open */
    ts->open = NULL;
    pos = ts_get_stream_pos(ts);
    ts->open = bard_xhtml_open;
    return pos;
}

static int bard_xhtml_size(cst_tokenstream *ts)
{
    int size;

    /* Because sub-calls to specifics are conditioned on ts->open */
    ts->open = NULL;
    size = ts_get_stream_size(ts);
    ts->open = bard_xhtml_open;
    return size;
}

cst_features *bard_xhtml_read_tag_attributes(const char *ta)
{   /* Get tag and attributes in XML string */
    cst_tokenstream *ts;
    cst_features *f;

    ts = ts_open_string(ta,
                        cst_ts_default_whitespacesymbols,
                        xhtml_singlecharsymbols_inattr,
                        cst_ts_default_prepunctuationsymbols,
                        cst_ts_default_postpunctuationsymbols);

    f = bard_xhtml_read_tag_attributes_ts(ts);

    ts_close(ts);

    return f;
}

cst_features *bard_xhtml_read_tag_attributes_ts(cst_tokenstream *ts)
{
    cst_features *a = new_features();
    const char *name, *val;
    const char *tag;
    char *xname, *dtag;

    tag = ts_get(ts);
    if (cst_streq("/",tag))
    {
        tag = ts_get(ts);
        feat_set_string(a,"_type","end");
    }
    else if (cst_streq("!",tag))
    {
        tag = ts_get(ts);
        feat_set_string(a,"_type","emark");
    }
    else
        feat_set_string(a,"_type","start");

    dtag = cst_downcase(tag);
    feat_set_string(a,"tag",dtag);
    cst_free(dtag);

    name = ts_get(ts);
    while (!cst_streq(">",name))
    {
	if (cst_streq(name,"/"))
	    feat_set_string(a,"_type","startend");
	else if (cst_streq(name,"?"))
	    feat_set_string(a,"_type","qmark");
        else if (cst_streq("emark",get_param_string(a,"_type","")))
        {
	    feat_set_string(a,feat_own_string(a,name),"");
        }
	else
	{
            xname = cst_strdup(name);
	    if (cst_streq("=",ts_get(ts)))
                val = ts_get_quoted_token(ts,'"','\\');
            else
                val = "";
	    feat_set_string(a,feat_own_string(a,xname),val);
            cst_free(xname);
	}
	if (ts_eof(ts))
	{
	    delete_features(a);
	    return NULL;
	}
        name = ts_get(ts);
    }

    return a;
}

int bard_xhtml_getc(cst_tokenstream *ts)
{
    int c;
    char ampchars[7];
    cst_features *ta;
    int i,p;
    int attr_token_max = 100;
    char *attr_token = NULL;
    char *nat;

    if (ts_eof(ts))
        return -1;
    else if (ts->token && ts->token[0] == '&')
    {  /* an entity */
        c = '&';
        i=0;
        while ((c != ';') && i<6)
        {
            ampchars[i] = c;
            c = private_ts_getc(ts);
            if (ts_eof(ts))
                return -1;
            i++;
        }
        ampchars[i] = '\0';
        /* note the all happen to be one char long */
        if (cst_streq("&nbsp",ampchars))
            ts->token[0] = ' ';
        else if (cst_streq("&amp",ampchars))
            ts->token[0] = '&';
        else if (cst_streq("&mdash",ampchars))
            ts->token[0] = '-';
        else if (cst_streq("&lt",ampchars))
            ts->token[0] = '<';
        else if (cst_streq("&gt",ampchars))
            ts->token[0] = '>';
        else if (cst_streq("&#8211",ampchars))
            ts->token[0] = '-';
        else if (cst_streq("&#8212",ampchars))
            ts->token[0] = '-';
        else
        {
            if (bard_debug)
                printf("bard_debug unknown XML entity %s\n",ampchars);
            ts->token[0] = '?';
        }
    }
    else if (ts->token && ts->token[0] == '<')
    {   /* Read Tag and attributes */
        ts->token[0] = '\0'; /* make token empty and set tags field */
        c = private_ts_getc(ts); /* skip '<' */
        /* Read up to next '>' into a string buffer -- thus be safer */
        /* with mal-formed attribute lists */
        attr_token = cst_alloc(char,attr_token_max);
        for (p=0; ((!ts_eof(ts)) && (c != '>')); p++)
        {
            if (p+2 >= attr_token_max)
            {
                attr_token_max += attr_token_max/5;
                nat = cst_alloc(char,attr_token_max);
                cst_sprintf(nat,"%s",attr_token);
                cst_free(attr_token);
                attr_token = nat;
            }
            attr_token[p] = c;
            c = private_ts_getc(ts);
        }
        attr_token[p] = '>';
        attr_token[p+1] = '\0';
        ta = bard_xhtml_read_tag_attributes(attr_token);

        delete_features(ts->tags);
        ts->tags = ta;
        cst_free(attr_token); attr_token = NULL;
    }

    c = private_ts_getc(ts);

    return c;
}
