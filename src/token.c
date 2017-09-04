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
/*  For non-TTS people, tokens are the white separated things you might  */
/*  casually call words, but to us words are the things the synthesizer  */
/*  says e.g "1984" is one token, "nineteen eighty four" is three tokens */
/*  each of which is one word                                            */
/*                                                                       */
/*  Bard Tokens are rectangles placed on a surface containing rendered   */
/*  text tokens                                                          */
/*                                                                       */
/*************************************************************************/
#include "bard_token.h"
#include "cst_alloc.h"
#include "cst_string.h"

char *bard_token_shorten(const char *name,int size)
{
    /* Shortens the given string to given size putting "..." in the middle */
    char *short_string;
    int sl;
    int i,m;

    sl = cst_strlen(name);
    
    if (sl <= size)
        return cst_strdup(name);
    if (size < 6)
        return cst_strdup("****");

    short_string = cst_alloc(char,size+1);

    m = (size-3)/2;
    for (i=0; i<m; i++)
        short_string[i] = name[i];
    short_string[i] = '.'; i++;
    short_string[i] = '.'; i++;
    short_string[i] = '.'; i++;
    for (   ; i<size; i++)
        short_string[i] = name[(sl-m)+(i-(m+3))];
    short_string[i] = '\0';

    return short_string;
}

bard_token *bard_token_new(const char *name)
{
    bard_token *token = cst_alloc(bard_token,1);

    token->token = cst_strdup(name);

    return token;
}

bard_token *bard_token_append(bard_token *t,bard_token *n)
{
    if (t) t->n = n;
    n->p = t;
    return n;
}

bard_token *bard_token_below(bard_token *t,bard_token *d)
{
    if (t) t->d = d;
    d->u = t;
    return d;
}

const bard_token *bard_token_first(const bard_token *t)
{
    const bard_token *x = t;
    while (x && x->p)
        x=x->p;
    return x;
}

const bard_token *bard_token_last(const bard_token *t)
{
    const bard_token *x = t;
    while (x && x->n)
        x=x->n;
    return x;
}

const bard_token *bard_token_top(const bard_token *t)
{
    const bard_token *x = t;
    while (x && x->u)
        x=x->u;
    return x;
}

const bard_token *bard_token_bottom(const bard_token *t)
{
    const bard_token *x = t;
    while (x && x->d)
        x=x->d;
    return x;
}

const bard_token *bard_token_up(const bard_token *t)
{
    const bard_token *x = bard_token_first(t);
    if (x && x->u)
        return x->u;
    else
        return x;
}

const bard_token *bard_token_down(const bard_token *t)
{
    const bard_token *x = bard_token_first(t);
    if (x && x->d)
        return x->d;
    else
        return x;
}

const bard_token *bard_token_prev(const bard_token *t)
{
    if (!t) return t;
    else if (t->p) return t->p;
    else if (t->u) return bard_token_last(t->u);
    else return t;
}

const bard_token *bard_token_next(const bard_token *t)
{
    const bard_token *x;
    if (!t) return t;
    else if (t->n) return t->n;
    else 
    {
        x = bard_token_first(t);
        if (x->d)
            return x->d;
        else
            return t;
    }
}

void bard_token_delete(bard_token *t)
{
    bard_token *n, *d;

    if (t == NULL) return;

    n = t->n; t->n = NULL;  /* just to be safe in case there are loops */
    bard_token_delete(n);
    d = t->d; t->d = NULL;  /* just to be safe in case there are loops */
    bard_token_delete(d);

    cst_free(t->token);
    cst_free(t->whitespace);
    cst_free(t->prepunctuation);
    cst_free(t->word);
    cst_free(t->postpunctuation);

    cst_free(t);

    return;
}

