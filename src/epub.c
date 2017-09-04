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
/*               Date:  February 2012                                     */
/*************************************************************************/
/*                                                                       */
/*  Interface to epub files through libzip to read the zip archive in    */
/*  place http://www.nih.at/libzip/                                      */
/*                                                                       */
/*  we were going to use libepub but its dependencies are too hard to    */
/*  compile on my target devices, so we replace that (and libxml2) with  */
/*  our own -- probably not-as-general code                              */
/*                                                                       */
/*************************************************************************/

#ifndef BARD_EPUB
/* No EPUB Support */
#include "bard.h"
#include "cst_tokenstream.h"

cst_tokenstream *ts_bard_open_epub_file(const char *filename)
{
    return NULL;
}

void bard_epub_init(bard_reader *br)
{
    return;
}

void bard_epub_end(bard_reader *br)
{
    return;
}
cst_features *bard_epub_get_tag(cst_tokenstream *ts)
{
    return NULL;
}

char *bard_epub_grab_image(cst_tokenstream *ts,int *size)
{
    *size = 0;
    return NULL;
}
#else
/* This is a more direct approach -- and not go through libepub, which is */
/* good, but its hard to get all of its dependencies compiled             */
/* So we'll do it ourselves and only depend on libzip to read the file    */
#include <zip.h>
#include "bard.h"
#include "cst_tokenstream.h"

struct bard_epub_part_struct {
    char *name;
    int idx;
    int size;
};
typedef struct bard_epub_part_struct bard_epub_part;

struct bard_epub_info_struct {
    cst_tokenstream *ps;
    struct zip *epub;
    bard_epub_part **part;       /* Pointers to important text files */
    bard_epub_part *entries;     /* All files */
    int current_part;
    char *part_string;           /* string of the current part */
    char *current_image;         /* bytes of current image */
    int image_size;
    int num_parts;
    int total_parts;
    int num_files;
    int file_pos;  /* file pos in global virtual file */
};
typedef struct bard_epub_info_struct bard_epub_info;

static int bard_epub_name_to_idx(bard_epub_info *be, const char *filename);
static char *bard_epub_read_file(bard_epub_info *be, const char *filename);

/* Wrappers for reading the whole epub (zip) archive */
static int bard_epub_open(cst_tokenstream *ts,const char *filename);
static void bard_epub_close(cst_tokenstream *ts);
static int bard_epub_eof(cst_tokenstream *ts);
static int bard_epub_seek(cst_tokenstream *ts, int pos);
static int bard_epub_tell(cst_tokenstream *ts);
static int bard_epub_size(cst_tokenstream *ts);
static int bard_epub_getc(cst_tokenstream *ts);

void bard_epub_init(bard_reader *br)
{
    return;
}

void bard_epub_end(bard_reader *br)
{
    return;
}

cst_tokenstream *ts_bard_open_epub_file(const char *filename)
{
    return ts_open_generic(filename,
                           cst_ts_default_whitespacesymbols,
                           xhtml_singlecharsymbols_general,
                           cst_ts_default_prepunctuationsymbols,
                           cst_ts_default_postpunctuationsymbols,
                           NULL,
                           bard_epub_open,
                           bard_epub_close,
                           bard_epub_eof,
                           bard_epub_seek,
                           bard_epub_tell,
                           bard_epub_size,
                           bard_epub_getc);

}

static int bard_epub_name_to_idx(bard_epub_info *be, const char *filename)
{
    /* Find index given filename */
    int i;

    for (i=0; i<be->num_files; i++)
    {
        if (cst_streq(filename,be->entries[i].name))
            return i;
    }
    return -1;
}

static char *bard_epub_read_file(bard_epub_info *be, const char *filename)
{
    struct zip_file *zf;
    int idx;
    char *whole;
    int size;
    int n,m;

    idx = bard_epub_name_to_idx(be,filename);
    if (idx < 0) return NULL;
    size = be->entries[idx].size;
    whole = cst_alloc(char,size+1);

    zf = zip_fopen_index(be->epub,idx,0);
    m = 0;
    while (m < size)
    {
        n = zip_fread(zf,&whole[m],8192);
        if (n == 0)
        {
            zip_fclose(zf);
            cst_free(whole);
            return NULL;
        }
        m += n;
    }
    whole[size]='\0';
    
    zip_fclose(zf);
    return whole;
}

static int bard_epub_find_rootfile_idx(bard_epub_info *be)
{
    /* Find the idx for the root file in META-INF/container.xml */
    char *cf;
    const char *rfn;
    const char *token;
    cst_features *t;
    int rf_idx=0;
    cst_tokenstream *ts;

    cf = bard_epub_read_file(be,"META-INF/container.xml");
    if (cf == NULL) return 0;
    ts = ts_open_string(cf,
                        cst_ts_default_whitespacesymbols,
                        xhtml_singlecharsymbols_all,
                        cst_ts_default_prepunctuationsymbols,
                        cst_ts_default_postpunctuationsymbols);
    t = NULL;
    while (!ts_eof(ts))
    {
        token = ts_get(ts);
        if (cst_streq("<",token))
        {
            t = bard_xhtml_read_tag_attributes_ts(ts);
            if (!t) continue;
            if (cst_streq("rootfile",get_param_string(t,"tag","")))
            {
                rfn = get_param_string(t,"full-path","");
                rf_idx = bard_epub_name_to_idx(be,rfn);
                break;
            }
            delete_features(t);
            t = NULL;
        }
    }

    if (t) delete_features(t);
    
    ts_close(ts);
    cst_free(cf);

    return rf_idx;

}

static int bard_epub_find_parts(bard_epub_info *be)
{
    char *mimetype;
    int rf_idx, l;
    char *dirname;
    char *r;
    cst_features *t;
    const char *token, *np;
    char *opf;
    cst_tokenstream *ts;
    char *fp;

    /* Check its an epub file */
    if (be->num_files < 4)
        return 0;
    if (cst_streq("mimetype",be->entries[0].name))
    {
        mimetype = bard_epub_read_file(be,"mimetype");
        l = cst_strlen(mimetype);
        if ((l > 1) && (mimetype[cst_strlen(mimetype)-1] == '\n'))
            mimetype[cst_strlen(mimetype)-1] = '\0';
        if (!cst_streq("application/epub+zip",mimetype))
        {
            cst_free(mimetype);
            return 0;
        }
        cst_free(mimetype);
    }
    else
        return 0;

    rf_idx = bard_epub_find_rootfile_idx(be);
    if (rf_idx < 1)
        return 0;

    dirname = cst_strdup(be->entries[rf_idx].name);
    r = cst_strrchr(dirname,'/');
    if (r) 
        r[1]='\0';  /* include '/' in the name */
    else
        dirname[0] = '\0';

    be->num_parts = 0;
    /* Can't be more that num_files */
    be->part = cst_alloc(bard_epub_part *,be->num_files);

    opf = bard_epub_read_file(be,be->entries[rf_idx].name);
    if (opf == NULL) return 0;
    ts = ts_open_string(opf,
                        cst_ts_default_whitespacesymbols,
                        xhtml_singlecharsymbols_all,
                        cst_ts_default_prepunctuationsymbols,
                        cst_ts_default_postpunctuationsymbols);
    t = NULL;
    while (!ts_eof(ts))
    {
        token = ts_get(ts);
        if (cst_streq("<",token))
        {
            t = bard_xhtml_read_tag_attributes_ts(ts);
            if (!t) continue;
            if ((cst_streq("item",get_param_string(t,"tag",""))) &&
                (cst_streq("application/xhtml+xml",
                           get_param_string(t,"media-type",""))))
            {
                np = get_param_string(t,"href","");
                fp = cst_alloc(char,cst_strlen(dirname)+cst_strlen(np)+2);
                cst_sprintf(fp,"%s%s",dirname,np);
                be->part[be->num_parts] =
                    &be->entries[bard_epub_name_to_idx(be,fp)];
                be->num_parts++;
                cst_free(fp);
            }
            delete_features(t);
            t = NULL;
        }
    }

    ts_close(ts);
    cst_free(opf);
    cst_free(dirname);

    return 1;
}

static int bard_epub_open(cst_tokenstream *ts,const char *filename)
{
    bard_epub_info *be;
    struct zip_stat zst;
    int i, err;

    be = cst_alloc(bard_epub_info,1);

    be->epub = zip_open(filename,0,&err);
    if (be->epub == NULL)
    {
        cst_free(be);
        return 0;
    }
    ts->streamtype_data = (void *)be;

    be->num_files = zip_get_num_files(be->epub);

    be->entries = cst_alloc(bard_epub_part,be->num_files);

    for (i=0; i<be->num_files; i++)
    {   /* get info about all files in the archive */
        zip_stat_index(be->epub,i,0,&zst);
        be->entries[i].name = cst_strdup(zst.name);
        be->entries[i].size = zst.size;
    }

    /* Now find the spine and the parts */
    if (bard_epub_find_parts(be) == 0)
    {
        bard_epub_close(ts);
        return 0;
    }

    /* Let's load in the first part */
    be->part_string = bard_epub_read_file(be,be->part[0]->name);
    be->ps = ts_open_string(be->part_string,
                            cst_ts_default_whitespacesymbols,
                            xhtml_singlecharsymbols_general,
                            cst_ts_default_prepunctuationsymbols,
                            cst_ts_default_postpunctuationsymbols);
    
    return 1; /* success */
}

static void bard_epub_close(cst_tokenstream *ts)
{
    bard_epub_info *be;
    be = (bard_epub_info *)ts->streamtype_data;
    int i;

    if (be->ps)
        ts_close(be->ps);
    cst_free(be->part_string);
    for (i=0; i< be->num_files; i++)
        cst_free(be->entries[i].name);
    cst_free(be->entries);
    cst_free(be->part);
    zip_close(be->epub);
    cst_free(be);

    return;
}

static int bard_epub_eof(cst_tokenstream *ts)
{
    bard_epub_info *be;
    be = (bard_epub_info *)ts->streamtype_data;

    if (ts_eof(be->ps) && (be->current_part+1 == be->num_parts))
        return 1;
    else
        return 0;
}

static int bard_epub_seek(cst_tokenstream *ts, int pos)
{
    bard_epub_info *be;
    be = (bard_epub_info *)ts->streamtype_data;
    int i;
    int q;

    q = pos;
    for (i=0; i<be->num_parts; i++)
    {
        if (q - be->part[i]->size > 0)
            q -= be->part[i]->size;
        else
            break;
    }

    if (be->current_part != i)
    {
        ts_close(be->ps);
        cst_free(be->part_string);
        be->current_part = i;
        be->part_string = bard_epub_read_file(be,be->part[be->current_part]->name);
        be->ps = ts_open_string(be->part_string,
                                cst_ts_default_whitespacesymbols,
                                xhtml_singlecharsymbols_general,
                                cst_ts_default_prepunctuationsymbols,
                                cst_ts_default_postpunctuationsymbols);
    }
    ts_set_stream_pos(be->ps,q);
    be->file_pos = pos;

    return pos;
}

static int bard_epub_tell(cst_tokenstream *ts)
{
    bard_epub_info *be;
    be = (bard_epub_info *)ts->streamtype_data;

    return be->file_pos;
}

static int bard_epub_size(cst_tokenstream *ts)
{
    bard_epub_info *be;
    be = (bard_epub_info *)ts->streamtype_data;
    int s=0, i;

    for (i=0; i<be->num_parts; i++)
        s+=be->part[i]->size;

    return s;
}

char *bard_epub_grab_image(cst_tokenstream *ts, int *size)
{
    /* Return an image if there is one */
    /* You can only do this once, and then its gone */
    bard_epub_info *be;
    be = (bard_epub_info *)ts->streamtype_data;
    char *image=NULL;

    if (ts->open == bard_epub_open)
    {
        image = be->current_image;
        *size = be->image_size;
        be->current_image = NULL;
    }
    return image;
}

static int bard_epub_get_image(bard_epub_info *be,cst_features *ta)
{
    /* Load the referenced image */
    char *dirname;
    char *full_path;
    char *r;
    const char *img_ref;
    size_t lnth;
    size_t i;

    dirname = cst_strdup(be->part[be->current_part]->name);
    r = cst_strrchr(dirname,'/');
    if (r) *r='\0';
    img_ref = get_param_string(ta,"src","");

    lnth = cst_strlen (img_ref);
    /* Check if image file has ../ in front and if so, remove it */
    /* and corresponding section from dirname */
    for (i = 0; i + 3 <= lnth; i += 3)
    {
        if (cst_streqn (&img_ref[i], "../", 3))
        {
            r = cst_strrchr(dirname,'/');
            if (r) 
                *r='\0';
            else
                break;
        }
        else
            break;        
    } 
    r = (i != 0) ? (char *) &img_ref[i] : (char *) img_ref;
    full_path = cst_alloc(char,cst_strlen(dirname)+lnth-i+2);
    cst_sprintf(full_path,"%s/%s",dirname,r);
    /*    printf("awb_debug processing image %s\n",full_path); */

    if (be->current_image) cst_free(be->current_image);
    be->current_image = bard_epub_read_file(be,full_path);
    be->image_size = be->entries[bard_epub_name_to_idx(be,full_path)].size;

    cst_free(dirname);
    cst_free(full_path);

    return 1;
}

static int bard_epub_getc(cst_tokenstream *ts)
{
    bard_epub_info *be;
    be = (bard_epub_info *)ts->streamtype_data;
    unsigned char c;
    char ampchars[7];
    int i, tag_start, tag_end;
    char *tag_attributes;
    cst_features *ta;

    if (ts->token && ts->token[0] == '&')
    {   /* entities */
        c = '&';
        i=0;
        while ((c != ';') && i<6)
        {
            ampchars[i] = c;
            c = private_ts_getc(be->ps);
            be->file_pos++;
            if (ts_eof(be->ps))
                return -1;
            i++;
        }
        ampchars[i] = '\0';
        if (cst_streq("&nbsp",ampchars))
            ts->token[0] = ' ';
        else if (cst_streq("&amp",ampchars))
            ts->token[0] = ' ';
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
    {   /* Parse the tag and attributes */
        ts->token[0] = '\0'; /* make token empty and set tags field */
        tag_start = ts_get_stream_pos(be->ps);
        do
        {   /* assume tag doesn't go across parts */
            c = private_ts_getc(be->ps);
            be->file_pos++;
            if (ts_eof(be->ps)) return -1;  /* just in case there's no end > */
        } while (c != '>');
        tag_end = ts_get_stream_pos(be->ps);
        /* Extract the tag and attributes */
        tag_attributes = cst_alloc(char,(tag_end-tag_start)+1);
        memcpy(tag_attributes,&be->part_string[tag_start],
               tag_end-tag_start);
        tag_attributes[tag_end-tag_start] = '\0';
        ta = bard_xhtml_read_tag_attributes(tag_attributes);
        cst_free(tag_attributes);

        delete_features(ts->tags);
        ts->tags = ta;

        if (cst_streq("img",get_param_string(ta,"tag","")))
        {   /* This doesn't seem the right place for this */
            bard_epub_get_image(be,ta);
            feat_set_string(ta,"tag","done");
        }

    }
    c = private_ts_getc(be->ps);
    if (ts_eof(be->ps))
    {
        if (be->current_part+1 == be->num_parts)
        {
            ts->eof_flag = TRUE;
            return -1; /* this is used by tokenstream for EOF */
        }
        ts_close(be->ps);
        be->current_part++;
        cst_free(be->part_string);
        be->part_string = 
            bard_epub_read_file(be,be->part[be->current_part]->name);
        be->ps = ts_open_string(be->part_string,
                                cst_ts_default_whitespacesymbols,
                                xhtml_singlecharsymbols_general,
                                cst_ts_default_prepunctuationsymbols,
                                cst_ts_default_postpunctuationsymbols);
        /* open already calls getc */
        c = be->ps->current_char;
    }
    be->file_pos++;
    ts->file_pos = be->file_pos;

    return ((int)c);
}

#endif
