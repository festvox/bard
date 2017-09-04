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
#include "bard_window.h"
#include "bard_color.h"

void bard_window_clear(bard_window *w)
{
    SDL_FillRect(w->surface, NULL, 
                 SDL_MapRGB(w->surface->format,
                            w->background_color->r,
                            w->background_color->g,
                            w->background_color->b));
    return;
}

void bard_window_display_from_pos(bard_window *w, int pos)
{
    /* Update surface and screen tokens from pos */

    if (pos == 0)
    {
        delete_val((cst_val *)(void *)w->previous_pages);
        w->previous_pages = NULL;
    }
    else if (pos < 0) 
        /* negative pos denotes jump back -- don't put on previous list */
        pos = -1 * pos;    
    else if (pos-1 > w->sop_pos)
    {
        /* Set this to NULL if we are speaking or scrolling */
        /* w->previous_pages = cons_val(int_val(w->sop_pos),w->previous_pages); */
        pos-=1; /* so we get all of the html tag */
    }

    bard_font_set_style(&w->font,BARD_FONT_STYLE_NORMAL);
    w->sop_pos = pos;
    ts_set_stream_pos(w->ts, w->sop_pos);
    bard_window_fill_text(w);

    return;
}

static int process_word_tags(bard_window *w)
{   /* Interpret XHTML tags or text layout for paragraph breaks */

    cst_features *tags;
    int rval = 0;
    char *r1, *r2;

    /* If its an epub book or html with xhtml tags ... */
    tags = w->ts->tags;
    if (!tags)
    {   /* Just a text file para_breaks and blank lines */
        r1=cst_strchr(w->ts->whitespace,'\n');
        if (r1 == NULL) return 0;
        r2=cst_strchr(&r1[1],'\n');
        if (r2)
            return 1;
        else
            return 0;
    }
    else
    {
        if (((cst_streq("p",get_param_string(tags,"tag",""))) ||
             (cst_streq("hr",get_param_string(tags,"tag","")))) &&
            (cst_streq("start",get_param_string(tags,"_type",""))))
        {
            bard_font_set_style(&w->font,BARD_FONT_STYLE_NORMAL);
            feat_set_string(tags,"tag","done");
            if (rval != 1) rval = 1;
        }
        else if (((cst_streq("br",get_param_string(tags,"tag",""))) ||
                  (cst_streq("li",get_param_string(tags,"tag","")))) &&
                 ((cst_streq("startend",get_param_string(tags,"_type",""))) ||
                  (cst_streq("start",get_param_string(tags,"_type","")))))
        {
            bard_font_set_style(&w->font,BARD_FONT_STYLE_NORMAL);
            feat_set_string(tags,"tag","done");
            if (rval == 0) rval = 2; /* means just a new line/not new para */
        }
        else if (((cst_streq("em",get_param_string(tags,"tag",""))) ||
                  (cst_streq("i",get_param_string(tags,"tag","")))) &&
                 (cst_streq("start",get_param_string(tags,"_type",""))))
        {   
            bard_font_set_style(&w->font,BARD_FONT_STYLE_ITALIC);
            feat_set_string(tags,"tag","done");
        }
        else if (((cst_streq("strong",get_param_string(tags,"tag",""))) ||
                  (cst_streq("b",get_param_string(tags,"tag","")))) &&
                 (cst_streq("start",get_param_string(tags,"_type",""))))
        {
            bard_font_set_style(&w->font,BARD_FONT_STYLE_BOLD);
            feat_set_string(tags,"tag","done");
            if (rval != 1) rval = 0;
        }
        else if (((cst_streq("title",get_param_string(tags,"tag",""))) ||
                  (cst_streq("h1",get_param_string(tags,"tag",""))) ||
                  (cst_streq("h2",get_param_string(tags,"tag",""))) ||
                  (cst_streq("h3",get_param_string(tags,"tag",""))) ||
                  (cst_streq("h4",get_param_string(tags,"tag","")))) &&
                 (cst_streq("start",get_param_string(tags,"_type",""))))
        {
            bard_font_set_style(&w->font,BARD_FONT_STYLE_BOLD);
            feat_set_string(tags,"tag","done");
            if (rval != 1) rval = 1;
        }
        else if (((cst_streq("em",get_param_string(tags,"tag",""))) ||
             (cst_streq("i",get_param_string(tags,"tag","")))) &&
            (cst_streq("end",get_param_string(tags,"_type",""))))
        {
            bard_font_set_style(&w->font,BARD_FONT_STYLE_NORMAL);
            feat_set_string(tags,"tag","done");
        }
        else if ((cst_streq("p",get_param_string(tags,"tag",""))) &&
            (cst_streq("end",get_param_string(tags,"_type",""))))
        {
            bard_font_set_style(&w->font,BARD_FONT_STYLE_NORMAL);
            feat_set_string(tags,"tag","done");
            if (rval != 1) rval = 1;
        }
        /* The ending tags */
        else if (((cst_streq("title",get_param_string(tags,"tag",""))) ||
                  (cst_streq("h1",get_param_string(tags,"tag",""))) ||
                  (cst_streq("h2",get_param_string(tags,"tag",""))) ||
                  (cst_streq("h3",get_param_string(tags,"tag",""))) ||
                  (cst_streq("h4",get_param_string(tags,"tag","")))) &&
                 (cst_streq("end",get_param_string(tags,"_type",""))))
        {
            bard_font_set_style(&w->font,BARD_FONT_STYLE_NORMAL);
            feat_set_string(tags,"tag","done");
            if (rval != 1) rval = 1;
        }
        else if (((cst_streq("b",get_param_string(tags,"tag",""))) ||
                  (cst_streq("strong",get_param_string(tags,"tag","")))) &&
                 (cst_streq("end",get_param_string(tags,"_type",""))))
        {
            bard_font_set_style(&w->font,BARD_FONT_STYLE_NORMAL);
            feat_set_string(tags,"tag","done");
            if (rval != 0) rval = 0;
        }

        /* Don't recognized the tag, do delete it */
        delete_features(w->ts->tags);
        w->ts->tags = NULL;
    }
    return rval;
}

int bard_window_fill_text(bard_window *w)
{  
    /* fill the display with text from the tokenstream at the set font */
    /* size */
    int x, y, ts_pos=0, token_size, n, rp;
    int line_number = 0;
    int word_number = 0;
    int word_x=0, sl;
    SDL_Rect offset;
    bard_surface *wordbox;
    SDL_Surface *imagebox, *full_image;
    char *r;
    const char *token;
    const char *whitespace;
    char *image;
    char *word, *sword;
    bard_token *current = NULL;
    bard_token *fol = NULL; /* first in line */
    bard_token *t;
    int current_style;
    int isize, didword=0;

    bard_window_clear(w);
    if (w->text_screen) bard_token_delete(w->text_screen);
    w->text_screen = NULL;
    ts_pos = ts_get_stream_pos(w->ts); /* for un get */
    token = ts_get(w->ts);
    current_style = w->font->style;
    bard_font_set_style(&w->font,w->font->style); /* reset it */
    wordbox = bard_render_text(w->font," ",*w->foreground_color); 
    w->tlh = wordbox->sdl_surface->h+w->tls;
    bard_surface_delete(wordbox);

    for (y=w->tls+w->tm; y+w->tlh < w->height-w->bm; y+=w->tlh)
    {
        word_number = 0;
        for (x=w->tlm; x < w->width; x+=word_x)
        {
            if (ts_eof(w->ts)) break;
            if ((didword==1) &&
                (image=bard_epub_grab_image(w->ts,&isize)) != NULL)
            {   /* create a surface with the image */
                full_image = bard_image_render(w,image,isize);
                /* Make it smaller than the screen */
                imagebox = bard_image_fit_window(w,full_image);
                if (imagebox)
                {
                    offset.x = (w->width-imagebox->w)/2;
                    if (word_number == 0)
                        offset.y = y;
                    else
                        offset.y = y+w->tlh;
                    SDL_BlitSurface(imagebox,NULL,w->surface,&offset);
                    y+=imagebox->h;
                    if (y > w->height-w->tlh)
                        didword=0;
                    cst_free(image);
                    SDL_FreeSurface(imagebox);
                    SDL_FreeSurface(full_image);
                    break;
                }
                else
                    SDL_FreeSurface(full_image);
                cst_free(image);
            }

            /* Newline/Paragraph break and other fancy stuff */
            if ((word_number > 0) &&
                w->ts_mode && cst_streq("literal",w->ts_mode))
            {
                n=0;
                r = w->ts->whitespace;
                while ((r=cst_strchr(r,'\n')) != NULL)
                {
                    n++;
                    r = &r[1];
                }
                if (n > 0)
                {
                    line_number+=n;
                    y += (n-1)*w->tlh/2;
                    break;
                }
            }
            else if (
                     /* this might change formating info too */
                     ((rp=process_word_tags(w)) > 0) && 
                     /* only do newline if we aren't already at pos 0 */
                     (word_number > 0))
            {   /* rp==1 para_break, rp==2 just break the line */
                line_number++;
                if (rp == 1)
                    y+=w->tlh/2;     /* only make it half a line height */
                break;
            }

            if (cst_strlen(token) == 0)
            {   /* a null width token (probably tags) */
                x -= word_x; /* so don't increment word separator */
                ts_pos = ts_get_stream_pos(w->ts); /* for un get */
                token = ts_get(w->ts);  /* OK, move to next token */
                continue;
            }

            /* Whitespace ? */
            if (x==w->tlm) /* first word in line */
                whitespace = "";
            else if ((w->ts_mode && cst_streq("literal",w->ts_mode)) ||
                     (w->ts->utf8_explode_mode))
                whitespace = w->ts->whitespace;
            else           /* not first word in line */
                whitespace = " ";

            token_size = 
                cst_strlen(whitespace)+
                cst_strlen(w->ts->prepunctuation)+
                cst_strlen(token)+
                cst_strlen(w->ts->postpunctuation)+
                1;
            word = cst_alloc(char,token_size);
            cst_sprintf(word,"%s%s%s%s", whitespace,
                        w->ts->prepunctuation,token,
                        w->ts->postpunctuation);
            if (w->font->style != current_style)
                bard_font_set_style(&w->font,w->font->style);
            current_style = w->font->style;
            /* Have to work out the right font for this word */
            wordbox = bard_render_text(w->font,word,*w->foreground_color); 
            word_x = wordbox->sdl_surface->w;
            sl = cst_strlen(word);
            if (x==w->tlm) /* first word in line - crush it so it fits */
            {              /* this is the most space we'll have for it */
                /* Is it too big to fit && is in a language we can/should */
                /* do character by character */
                if ((x+word_x+w->trm >= w->width) &&
                    /* && Chinese, Japanese, or Thai */
                    (bard_lang_breakable_language(token)))
                {
                    w->ts->utf8_explode_mode = 1;
                    /* I wonder if we can flip this off again later */
                    /* Jump back for re-analysis */
                    bard_surface_delete(wordbox);
                    cst_free(word);
                    ts_set_stream_pos(w->ts,ts_pos);
                    token = ts_get(w->ts);
                    continue;  
                }
                /* else w->ts->utf8_explode_mode = 0; */

                while (x+word_x+w->trm >= w->width)
                {
                    sl--;
                    if (sl < 2)
                        break;  /* never going to fit */
                    sword = bard_token_shorten(word,sl);
                    bard_surface_delete(wordbox);
                    wordbox = bard_render_text(w->font,sword,
                                               *w->foreground_color);
                    cst_free(sword);
                    word_x = wordbox->sdl_surface->w;
                }
            }
            if (x+word_x+w->trm < w->width)
            {
                word_number++;
                offset.x = x;
                offset.y = y;
                SDL_BlitSurface(wordbox->sdl_surface,NULL,w->surface,&offset);
                t = bard_token_new(word);
                bard_surface_delete(wordbox);
                t->x = x; t->y = y; 
                t->word = cst_strdup(token);
                t->line_number = line_number; t->token_number = word_number;
                t->file_pos = ts_pos; t->l = sl;
                t->font_style = w->font->style;
                didword=1;
                if (w->text_screen == NULL) w->text_screen = t;
                if (x == w->tlm) /* first in line */
                {
                    current = bard_token_below(fol,t);
                    fol = current;
                }
                else
                    current = bard_token_append(current,t);
                ts_pos = ts_get_stream_pos(w->ts); /* for un get */
                token = ts_get(w->ts);  /* OK, move to next token */
            }
            else 
                bard_surface_delete(wordbox);
           cst_free(word);
        }
        line_number++;
        if (ts_eof(w->ts)) break;
    }
    /* Move back to before last word */
    ts_set_stream_pos(w->ts,ts_pos);
    w->current_token = w->text_screen;

    return 0;
}

void bard_window_page_up(bard_window *cw)
{
    /* This is hard, in fact sometimes there might not be a previous page */
    /* that ends just before this page -- or there may be many possible */
    /* previous pages -- but you don't care about that you just want it */
    /* to work, irrespective of restarts, font size changes etc */
    const cst_val *o;
    const bard_token *home, *last;
    int end_target, jump_back, t1;
    int x;

    /* If we have a list of previous page starts -- use it */
    if (cw->previous_pages)
    {
        o = cw->previous_pages; /* current page position */
        cw->previous_pages = val_cdr(o);
        if (bard_debug)
            printf("bard_debug previous page at %d\n",
                   val_int(val_car(o)));
        bard_window_display_from_pos(cw,0-val_int(val_car(o)));
        set_cdr((cst_val *)(void *)o,NULL);
        delete_val((cst_val *)(void *)o);
        return;
    }
    
    /* When we don't have a previous page, we need to discover it */
    home = bard_token_top(bard_token_first(cw->current_token));
    last = bard_token_last(bard_token_bottom(home));
    /* We want to display a screen so that file_pos is that of the 
       current top left word on the screen now */
    if (!home || !last)
    {   /* Just not sure -- maybe got past the end of the file */
        bard_window_display_from_pos(cw,0); /* not right */
        return;
    }
    end_target = home->file_pos;
    jump_back = last->file_pos - end_target;
    jump_back += 50;  /* a bit more */
    printf("awb_debug page cw %s %d up %s %d %s %d\n",
           cw->current_token->word, cw->current_token->file_pos,
           home->word,home->file_pos,
           last->word,last->file_pos);
    t1 = end_target-jump_back;
    if (t1 < 0) 
    {
        bard_window_display_from_pos(cw,0);
        return;
    }
    ts_set_stream_pos(cw->ts,t1);  /* jump back an appropriate distance */
    ts_get(cw->ts);  /* get aligned to a token */
    if (cw->ts->utf8_explode_mode == 1)
    {   /* we might have jumped back into the middle of a character */
        /* skip a few characters (tokens) so we might more likely align */
        /* This still isn't right but is better than not skipping */
        ts_get(cw->ts); ts_get(cw->ts);
    }
    t1 = ts_get_stream_pos(cw->ts);
    bard_window_display_from_pos(cw,0-t1);
    x = 0;
    /* Try to find a display that ends just before the previous home word */
    while (ts_get_stream_pos(cw->ts)<end_target)
    {
        if (cw->current_token && cw->current_token->d &&
            (end_target-ts_get_stream_pos(cw->ts)) > 
            (cw->current_token->d->file_pos - cw->current_token->file_pos))
            /* move forward one line */
            t1 = cw->current_token->d->file_pos;
        else
        {
            ts_set_stream_pos(cw->ts,t1);
            ts_get(cw->ts);  /* move forward one token */
            t1 = ts_get_stream_pos(cw->ts);
        }
        bard_window_display_from_pos(cw,0-t1);
        /* Might also be in the middle of a utf8 char */
        x++;
    }
    if (bard_debug)
        printf("bard_debug jumped_back %d skipped %d\n",jump_back,x);

    return;
}

void bard_window_page_down(bard_window *cw)
{
    /* The token stream will already be positioned at the appropriate place */
    int p;
    const char *token;

    p = ts_get_stream_pos(cw->ts);
    /* Lets see if there really is anything following */
    token = ts_get(cw->ts);
    if ((cst_strlen(token) == 0) && ts_eof(cw->ts))
        return;

    /* Save current position so page up will work */
    cw->previous_pages = cons_val(int_val(cw->sop_pos),cw->previous_pages);

    bard_window_display_from_pos(cw,p);

    return;
}

static void write_current_text_color(bard_window *w, const SDL_Color *c)
{
    SDL_Rect offset;
    bard_surface *wordbox;
    char *sw;
    int old_style;

    if (!w->current_token) return;
    /* This isn't quite right, you should clear the surface before */
    /* This sort of leaves a "highlight" footprint like it has been */
    /* visited, maybe people will think its deliberate -- its not */
    sw = bard_token_shorten(w->current_token->token,w->current_token->l);
    old_style = w->font->style;
    bard_font_set_style(&w->font,w->current_token->font_style);
    wordbox = bard_render_text(w->font,sw,*c);
    bard_font_set_style(&w->font,old_style);
    cst_free(sw);
    offset.x = w->current_token->x; offset.y = w->current_token->y;
    SDL_BlitSurface(wordbox->sdl_surface,NULL,w->surface,&offset);
    bard_surface_delete(wordbox);

    return;
}

void bard_window_token_highlight_move(bard_window *w,int direction)
{
    const bard_token *l;
    int osop_pos;

    if (!w || !w->current_token)
        return;

    l = w->current_token;
    osop_pos = w->sop_pos;

    write_current_text_color(w,w->foreground_color);
    if (direction == SDLK_UP)
    {
        w->current_token = bard_token_up(w->current_token);
        if (w->current_token == l)
        {
            bard_window_page_up(w);
            if (osop_pos != w->sop_pos)
                w->current_token = bard_token_bottom(w->current_token);
        }
    }
    else if (direction == SDLK_DOWN)
    {
        w->current_token = bard_token_down(w->current_token);
        if (w->current_token == l)
            bard_window_page_down(w);
    }
    else if (direction == SDLK_RIGHT)
    {
        w->current_token = bard_token_next(w->current_token);
        if (w->current_token == l)
        {
            if (w->current_token->d == NULL)
                bard_window_page_down(w);
            else
                bard_window_display_from_pos(w,ts_get_stream_pos(w->ts));
        }
    }
    else if (direction == SDLK_LEFT)
    {
        w->current_token = bard_token_prev(w->current_token);
        if (w->current_token == l)
        {
            bard_window_page_up(w);
            if (osop_pos != w->sop_pos)
                w->current_token = 
                    bard_token_last(bard_token_bottom(w->current_token));
        }
    }
    write_current_text_color(w,w->highlight_color);

    return;
}

int bard_window_draw_border(bard_window *w, int thickness, 
                            const SDL_Color *color)
{
    /* Draw a border on the window's surface */
    SDL_Rect r;

    /* Left border */
    r.x=0; r.y=0; r.w=thickness; r.h=w->surface->h;
    SDL_FillRect(w->surface, &r,
                 SDL_MapRGB(w->surface->format, color->r,color->g,color->b));
    /* top border */
    r.x=0; r.y=0; r.w=w->surface->w; r.h=thickness;
    SDL_FillRect(w->surface, &r,
                 SDL_MapRGB(w->surface->format, color->r,color->g,color->b));
    /* right border */
    r.x=w->surface->w-thickness; r.y=0; r.w=thickness; r.h=w->surface->h;
    SDL_FillRect(w->surface, &r,
                 SDL_MapRGB(w->surface->format, color->r,color->g,color->b));
    /* bottom border */
    r.x=0; r.y=w->surface->h-thickness; r.w=w->surface->w; r.h=thickness;
    SDL_FillRect(w->surface, &r,
                 SDL_MapRGB(w->surface->format, color->r,color->g,color->b));

    return 0;
}

bard_window *bard_window_new(const char *name,
                             int width, int height, 
                             const SDL_PixelFormat *format)
{
    bard_window *w;

    w = cst_alloc(bard_window,1);

    w->name = name;
    w->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 
                                      format->BitsPerPixel,
                                      format->Rmask,
                                      format->Gmask,
                                      format->Bmask,
                                      format->Amask);
    w->width = w->surface->w;
    w->height = w->surface->h;

    /* This should probably be conditioned on the font size */
    /* These are fine for little devices, but if you have a screen size */
    /* 1920x1200 and 65 point size fonts ... */
    w->tm = 0;   /* top margin */
    w->bm = 0;   /* bottom margin */
    w->tlm = 2;  /* text left margin */
    w->trm = 2;  /* text right margin */
    w->tls = 1;  /* text line separator */

    bard_font_set_style(&w->font,BARD_FONT_STYLE_NORMAL);

    return w;
}

void bard_window_token_goto(bard_window *w,int line_number,int token_number)
{
    /* Move the current token to a specified position */
    if (!w) return;

    while (w->current_token && (w->current_token->line_number < line_number))
    {
        w->current_token = bard_token_down(w->current_token);
        if (! w->current_token)
            break;  /* shouldn't happen */
    }

    while (w->current_token &&
           (w->current_token->token_number < token_number))
        w->current_token = bard_token_next(w->current_token);

    return;
}

void bard_window_delete(bard_window *w)
{
    if (!w) return;

    if (w->surface)
        SDL_FreeSurface(w->surface);
    if (w->font)
        bard_font_delete(w->font);
    if (w->textfile)
        cst_free(w->textfile);
    if (w->ts)
        ts_close(w->ts);
    if (w->data)
        (w->data_delete)(w);
    if (w->text_screen)
        bard_token_delete(w->text_screen);
    if (w->previous_pages)
        delete_val((cst_val *)(void *)w->previous_pages);

    cst_free(w);
    return;
}

