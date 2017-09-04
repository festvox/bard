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
/*  Tokens and Screen Layout Structure                                   */
/*************************************************************************/
#ifndef _BARD_TOKEN_
#define _BARD_TOKEN_

struct bard_token_struct {
    char *token;
    char *whitespace;
    char *prepunctuation;
    char *word;
    char *postpunctuation;
    int l;  /* token might have to be shortened to fit screen */

    /* Surface position */
    int x;
    int y;

    int line_number;
    int token_number;
    int file_pos;

    int font_size;
    int font_style;
    char *font_name;

    /* pointers to next, previous, up and down words */
    struct bard_token_struct *n;
    struct bard_token_struct *p;
    struct bard_token_struct *u;
    struct bard_token_struct *d;

};
typedef struct bard_token_struct bard_token;

char *bard_token_shorten(const char *name,int size);
bard_token *bard_token_new(const char *name);
bard_token *bard_token_append(bard_token *t,bard_token *n);
bard_token *bard_token_below(bard_token *t,bard_token *d);
const bard_token *bard_token_first(const bard_token *t);
const bard_token *bard_token_bottom(const bard_token *t);
const bard_token *bard_token_last(const bard_token *t);
const bard_token *bard_token_top(const bard_token *t);
const bard_token *bard_token_up(const bard_token *t);
const bard_token *bard_token_down(const bard_token *t);
const bard_token *bard_token_prev(const bard_token *t);
const bard_token *bard_token_next(const bard_token *t);
void bard_token_delete(bard_token *t);
                           
#endif
