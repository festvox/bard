/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                        Copyright (c) 2014                             */
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
/*               Date:  August 2014                                      */
/*************************************************************************/
/*                                                                       */
/*  Separate out the font and render functions so we can use more        */
/*  complex rendering libraries when they are available (harfbuzz).      */
/*  This is necessary for writing systems such as Devanagari and Arabic  */
/*                                                                       */
/*  There areas in this api: font, rendering and surface (display)       */
/*                                                                       */
/*  bard_font                                                            */
/*  bard_surface                                                         */
/*                                                                       */
/*  I got helpful example code from                                      */
/*      https://github.com/anoek/ex-sdl-cairo-freetype-harfbuzz          */
/*  Which wasn't actually copied, but did give me the framework within   */
/*  which I did this                                                     */
/*                                                                       */
/*************************************************************************/

#include "bard.h"
#include "bard_render.h"

int bard_font_init()
{
#ifdef BARD_SDL_TTF
    if (TTF_Init() == -1)
        return 0;
#endif
    
    return 1;
}

int bard_font_quit()
{
#ifdef BARD_SDL_TTF
    TTF_Quit();
#endif

    return 1;
}

bard_font *bard_font_new()
{
    bard_font *bf;

    bf = cst_alloc(bard_font,1);

    return bf;
}
int bard_font_delete(bard_font *bf)
{

    if (bf == NULL)
        return 0;

#ifdef BARD_SDL_TTF
    if (bf->ttf_font)
        TTF_CloseFont(bf->ttf_font);
#endif
#ifdef BARD_HARFBUZZ
    /* Probably need a guard on these in case they aren't set */
    if (bf->hb_ft_font)
        hb_font_destroy(bf->hb_ft_font);
    if (bf->cairo_ft_face)
        cairo_font_face_destroy(bf->cairo_ft_face);
    if (bf->ft_face)
        FT_Done_Face(bf->ft_face);
    if (bf->ft_library)
        FT_Done_FreeType(bf->ft_library);
#endif
    if (bf->name)
        cst_free(bf->name);
        
    cst_free(bf);

    return 1;
}

#ifdef BARD_SDL_TTF
int bard_font_set(bard_font **tbf,
                  const char *font_name,
                  int font_size,
                  int font_style)
{
    bard_font *font;
    TTF_Font *ttf_font;
    char *nbfn;

    font = *tbf;
    if (!font)
    {
        *tbf = bard_font_new();
    }

    if ((*tbf)->name &&
        cst_streq((*tbf)->name,font_name) &&
        ((*tbf)->size == font_size) &&
        ((*tbf)->style == font_style))
    {
        return 0;
    }

    if (!(*tbf)->name ||
        !cst_streq((*tbf)->name,font_name) ||
        ((*tbf)->size != font_size))
    {
        ttf_font = TTF_OpenFont(font_name,font_size);
        if (ttf_font == NULL)
        {   /* we failed to open the font, so give up */
            return 1;
        }
        if ((*tbf)->ttf_font)
            TTF_CloseFont((*tbf)->ttf_font);
        (*tbf)->ttf_font = ttf_font;
        nbfn = cst_strdup(font_name);
        if ((*tbf)->name)
            cst_free((*tbf)->name);
        (*tbf)->name = nbfn;
        (*tbf)->size = font_size;
    }

    if ((*tbf)->style != font_style)
    {
        TTF_SetFontStyle((*tbf)->ttf_font,font_style);    
        (*tbf)->style = font_style;
    }
    
    return 0;
}
#endif

#ifdef BARD_HARFBUZZ
int bard_font_set(bard_font **tbf,
                  const char *font_name,
                  int font_size,
                  int style)
{
    bard_font *bf,*font;
    char *actual_font_name;

    font = *tbf;  /* the existing value for the font */
    if (font &&
        cst_streq(font->name,font_name) &&
        (font->size == font_size) &&
        (font->style == style))
    {
        return 1;
    }
    else
    {
        /* We are going to reset the font */
        bf = bard_font_new();

        if (FT_Init_FreeType(&bf->ft_library) != 0)
        {
            bard_font_delete(bf);
            printf("awb_debug failed Init FreeType\n");
            return 0; 
        }
        /* Get the style (regaulr/bold) dependent name */
        actual_font_name = bard_font_get_font_name(font_name,style);
        bf->style = style; 

        if (FT_New_Face(bf->ft_library,actual_font_name,0,&bf->ft_face) != 0)
        {
            bard_font_delete(bf);
            printf("awb_debug font_face failed\n");
            return 0;
        }

        /* Note we always keep the general/regular font_name, and not */
        /* any bold/italic version of it                              */
        bf->name = cst_strdup(font_name);

        /* I have no idea why I need the 64, but I do */
        /* If I don't have the 64, then the first token might be wrong */
        FT_Set_Char_Size(bf->ft_face,0,font_size*64,72,72);
        bf->size = font_size;

        bf->cairo_ft_face = 
            cairo_ft_font_face_create_for_ft_face(bf->ft_face,0);
        bf->hb_ft_font = hb_ft_font_create(bf->ft_face,NULL);

        bard_font_set_lang_features(bf);
        cst_free(actual_font_name);

        /* We have a new font successfully opened */
        if (font)
            bard_font_delete(font);
        *tbf = bf;

#if 0
        /* This can't possibly be right, but if I do this I don't get a 
           very small render for the first call.
           adding the *64, above, seems to fix the need for this */
        SDL_Color ddd;
        ddd.r = 255; ddd.r = 255; ddd.r = 255;
        bard_surface_delete(bard_render_text(bf,"helloworld",ddd));
#endif

        return 1;
    }
}
#endif

int bard_font_size(bard_font *bf)
{
    if (bf)
        return bf->size;
    else
        return BARD_DEFAULT_FONT_SIZE;
}

const char *bard_font_name(bard_font *bf)
{
    if (bf)
        return bf->name;
    else
        return NULL;
}

int bard_font_style(bard_font *bf)
{
    if (bf)
        return bf->style;
    else
        return BARD_FONT_STYLE_NORMAL;
}

int bard_font_set_style(bard_font **bf, int style)
{
    if (*bf)
    {
#ifdef BARD_SDL_TTF
        if ((*bf)->ttf_font && (style != (*bf)->style))
            TTF_SetFontStyle((*bf)->ttf_font,style);
        (*bf)->style = style;
#endif
#ifdef BARD_HARFBUZZ
        if ((*bf)->style != style)
            bard_font_set(bf,(*bf)->name,(*bf)->size,style);
#endif
        return 1;
    }

    return 0;
}

#ifdef BARD_HARFBUZZ
static SDL_Surface *cairo_to_sdl(cairo_surface_t *cr)
{
    SDL_Surface *sdl_surface;

    sdl_surface = 
        SDL_CreateRGBSurfaceFrom((void *)cairo_image_surface_get_data(cr),
                                 cairo_image_surface_get_width(cr),
                                 cairo_image_surface_get_height(cr),
                                 32, /* should check that is okay */
                                 cairo_image_surface_get_width(cr)*4,/* pitch */
                                 0x00ff0000,  /* byte order dependent */
                                 0x0000ff00,
                                 0x000000ff,
                                 0xff000000);
    SDL_SetAlpha(sdl_surface,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);

    return sdl_surface;
}
#endif

bard_surface *bard_surface_new()
{
    bard_surface *bs;

    bs = cst_alloc(bard_surface,1);
    return bs;
}

int bard_surface_delete(bard_surface *bs)
{
    /* As we need both the SDL_Surface (and in the harfbuzz case) the
       cairo_surface live at the same time we save them to be deleted 
       together */
    
    SDL_FreeSurface(bs->sdl_surface);
#ifdef BARD_HARFBUZZ
    cairo_surface_destroy(bs->cairo_surface);
    cairo_destroy(bs->cr);
#endif
    cst_free(bs);

    return 1;
}

bard_surface *bard_render_text(bard_font *bf,char *text,SDL_Color c)
{
    /* Render text as a (sdl) displayable surface doing any shaping/kerning */
    /* as required.  As the harfbuzz case uses cairo, we keep the cairo     */
    /* surface around until we delete the sdl surface.  In the non-hardbuff */
    /* case there is only an sdl surface -- but in both cases they're held  */
    /* within a bard_srface structure                                       */
    bard_surface *bs = bard_surface_new();
#if BARD_HARFBUZZ
    hb_buffer_t *b;
    unsigned int count;
    hb_glyph_info_t *glyph_info;
    hb_glyph_position_t *glyph_pos;
    cairo_glyph_t *cairo_glyphs;
    int i,x,y;
    int width, height;

#if 0
    printf("awb_debug render %s with %s %d %d\n",
           text,bf->name,bf->size,bf->style);
#endif

    /* Set up the harfbuzz buffer for the characters */
    b = hb_buffer_create();
    /* Its going to be unicode */
    hb_buffer_set_unicode_funcs(b,hb_icu_get_unicode_funcs());
    /* or -- which might avoid icu4c dependency */
    /* hb_buffer_set_unicode_funcs(b,hb_glib_get_unicode_funcs()); */
    /* Copy in the font/language parameters */
    hb_buffer_set_direction(b,bf->text_direction);
    hb_buffer_set_script(b,bf->script);
    hb_buffer_set_language(b,
          hb_language_from_string(bf->language,cst_strlen(bf->language)));
    /* Put the utf8 text in the buffer */
    hb_buffer_add_utf8(b,text,cst_strlen(text),0,cst_strlen(text));
    /* Do the shaping */
    hb_shape(bf->hb_ft_font,b,NULL,0); 

    /* Get the glyph details */
    glyph_info = hb_buffer_get_glyph_infos(b,&count);
    glyph_pos = hb_buffer_get_glyph_positions(b,&count);
    cairo_glyphs = cst_alloc(cairo_glyph_t,count);

    /* Find the ultimate width/height of the surface for these characters */
    width = 0; height = 0;
    for (i=0; i<count; i++)
        width += glyph_pos[i].x_advance/64;
    height = bf->size + bf->size/3;
#if 0
    printf("awb_debug render_text %d %d\n",height,width);
#endif

    x = 0;
    /* What should the relationship between height and y be? */
    y = bf->size; /* ?? */
    /* Get the individual glyph positions for the cairo glyphs */
    for (i=0; i<count; i++)
    {
        cairo_glyphs[i].index = glyph_info[i].codepoint;
        cairo_glyphs[i].x = x + glyph_pos[i].x_offset/64;
        cairo_glyphs[i].y = y - glyph_pos[i].y_offset/64;
        x += glyph_pos[i].x_advance/64;
        y -= glyph_pos[i].y_advance/64;
    }
    
    /* Now we need to make this into a cairo surface */
    bs->cairo_surface = 
        cairo_image_surface_create(CAIRO_FORMAT_RGB24,width,height);
    bs->cr = cairo_create(bs->cairo_surface);
    cairo_set_source_rgba(bs->cr,c.r/255.0,c.g/255.0,c.b/255.0,1.0); /* color */
    cairo_set_font_face(bs->cr,bf->cairo_ft_face);
    cairo_set_font_size(bs->cr,bf->size);
    
    /* Render the cairo glyphs into the cairo surface */
    cairo_show_glyphs(bs->cr,cairo_glyphs,count);

    /* Convert cairo surface to sdl surface */
    bs->sdl_surface = cairo_to_sdl(bs->cairo_surface);

    /* Tidy up structures */
    cst_free(cairo_glyphs);
    hb_buffer_destroy(b);

#endif
#ifdef BARD_SDL_TTF 
    bs->sdl_surface = TTF_RenderUTF8_Blended(bf->ttf_font,text,c);
    if (bs->sdl_surface == NULL)
    {   /* Not sure this can happen, but let's be safe */
        /* You do sometimes get wild unicode characters, because we've jumped
           back into them -- so we do this a little more safely */
        bs->sdl_surface = TTF_RenderUTF8_Blended(bf->ttf_font," ",c);
    }
#endif

    return bs;
}

