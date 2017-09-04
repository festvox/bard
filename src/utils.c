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
#include "bard_system.h"
#include <dirent.h>
#include <sys/stat.h>

static cst_val *insert_val(cst_val *d,const char *s)
{
    /* Insert in some (strcmp) sorted order */
    const cst_val *v, *l=NULL, *x;

    if (d == NULL)
        return cons_val(string_val(s),d);

    for (v=d; v; l=v,v=val_cdr(v))
    {
        if (strcmp(s,val_string(val_car(v))) < 0)
            break;
    }

    if (l)  
    {
        x = cons_val(string_val(s),val_cdr(l));
        set_cdr((cst_val *)(void *)l,x);
        return d;
    }
    else /* d empty or s to be first in list */
        return cons_val(string_val(s),d);

}

char *bard_file_select_dircontents(const char *dirname)
{
    /* Returns a nice string to display the contents of the directory */
    DIR *cwd;
    cst_val *dirs = NULL;
    int dirs_count = 0;
    int dirs_len = 0;
    cst_val *files = NULL;
    int files_count = 0;
    int files_len = 0;
    struct dirent *d;
    struct stat et;
    const char *s;
    const cst_val *v;
    int dpos;
    char *dirstring;
    const char *pwd;
#ifdef __MINGW32__
    struct stat sb;
#endif

    cwd = opendir(dirname);
    if (cwd == NULL)
    {
        /* I wonder if we could be more helpful here ... */
        return cst_strdup("cannot open dir\n");
    }

    /* We need to be in the directory to follow relative symlinks */
    pwd = bard_getwd();
    bard_chdir(dirname);
    /* Find the directory contents, sort them by dir/file */
    while ((d = readdir(cwd)) != NULL)
    {
        if (cst_streq(d->d_name,"."))
            continue;
#ifdef __MINGW32__
        stat (d->d_name, &sb);
        if (S_ISDIR(sb.st_mode))
#else        
        if (d->d_type == DT_DIR)
#endif           
        {
            dirs = insert_val(dirs,d->d_name);
            dirs_count += 1;
            dirs_len += cst_strlen(d->d_name);
        }
#ifdef __MINGW32__
        else if (S_ISREG(sb.st_mode))
#else        
        else if (d->d_type == DT_REG)
#endif           
        {
            files = insert_val(files,d->d_name);
            files_count += 1;
            files_len += cst_strlen(d->d_name);
        }
        else if ((cst_strlen(d->d_name) > 1) &&
                 (d->d_name[0] == '.') &&
                 (d->d_name[1] == '#'))  /* emacs symlinks that go nowhere */
            continue;
#ifndef __MINGW32__
        else if (d->d_type == DT_LNK)  /* a symlink */
        {
            stat(d->d_name,&et);
            if (S_ISDIR(et.st_mode))
            {
                dirs = insert_val(dirs,d->d_name);
                dirs_count += 1;
                dirs_len += cst_strlen(d->d_name);
            }
            else if (S_ISREG(et.st_mode))
            {
                files = insert_val(files,d->d_name);
                files_count += 1;
                files_len += cst_strlen(d->d_name);
            }
            /* ignore all other file types */
        }
#endif
        /* ignore all other file types */
    }

    bard_chdir(pwd);  /* change back again */

    /* Make the string that will be displayed in the window */
    dirstring = cst_alloc(char,cst_strlen(dirname)+2+
                          dirs_len+dirs_count+dirs_count+
                          files_len+files_count+
                          1);
    dpos = 0;
    strcpy(&dirstring[dpos],dirname); dpos+=cst_strlen(dirname);
    strcpy(&dirstring[dpos],"\n\n"); dpos+=2;
    
    for (v=dirs; v; v=val_cdr(v))
    {
        s = val_string(val_car(v));
        strcpy(&dirstring[dpos],s); dpos+=cst_strlen(s);
        strcpy(&dirstring[dpos],"/\n"); dpos+=2;
    }
    delete_val(dirs);

    for (v=files; v; v=val_cdr(v))
    {
        s = val_string(val_car(v));
        strcpy(&dirstring[dpos],s); dpos+=cst_strlen(s);
        strcpy(&dirstring[dpos],"\n"); dpos+=1;
    }
    /* It will have a null at the end from last strcpy */
    delete_val(files);

    closedir(cwd);

    return dirstring;
}
    
    

