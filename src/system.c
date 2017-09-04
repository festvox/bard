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
/*  These are the interactions with the host OS                          */
/*************************************************************************/

#include "bard.h"
#include "bard_system.h"
#include <unistd.h>
#include <time.h>
#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

int bard_chdir(const char *dir)
{
    return chdir(dir);
}

const char *bard_getwd(void)
{
    const char *cwd;
#ifdef _WIN32
    char pathwd[MAX_PATH];
    const char *ptr = bard_getenv("PWD");
    if (!ptr)
       ptr = getcwd (pathwd, MAX_PATH);
    return (ptr);
#else
    cwd = bard_getenv("PWD");
    if (!cwd) cwd = ".";
    return cwd;
#endif
}

const char *bard_config_dir(void)
{
    const char *home;

    home = bard_getenv("HOME");
    if (!home) home = ".";
    return home;
}

const char *bard_getenv(const char *var)
{
    return getenv(var);
}

char *bard_get_time(void)
{
    /* Return a string of the current time */
    time_t thetime;
    struct tm * thetimeinfo;
    static char mytime[20];
    time (&thetime);
    thetimeinfo = localtime (&thetime);
    strftime (mytime, 20 ,"%H:%M:%S", thetimeinfo);
   
    return cst_strdup(mytime);
}

char *bard_get_battery(const char *bs)
{
    /* Return a string of the current battery status */
    /* This is done by calling an external (user specified) program */
    char *battery = NULL;
#if !defined (_WIN32) && !defined (MSDOS)
    cst_tokenstream *ts=NULL;
    char *bbs;
    const char *bsformat = "%s >/tmp/bard_current_battery 2>/dev/null";

    /* It would be nice if this worked on more systems as it really is */
    /* the neatest solution -- but it doesn't work on many systems */
    if ((ts = ts_open("/sys/class/power_supply/battery/capacity",
                          cst_ts_default_whitespacesymbols,
                           "","","")) != NULL)
    {
        battery = cst_strdup(ts_get(ts));
        ts_close(ts);
        /* Maybe should check its a number? */
        if (cst_streq("",battery) || (cst_strlen(battery) > 4))
        {
            cst_free(battery);
            battery = cst_strdup("N/A");
        }
    }
    else if (bs && (bs[0] != '\0'))  /* user specified script */
    {
        /* This is set up for my battery script on my ben and laptop */
        /* $2 in the output is the pecentage charge */
        bbs = cst_alloc(char,strlen(bs)+strlen(bsformat)+1);
        cst_sprintf(bbs,bsformat,bs);
        /* ignore return value, as I check its consequences later */
        (void)system(bbs);
        cst_free(bbs);
       
        ts = ts_open("/tmp/bard_current_battery",
                     cst_ts_default_whitespacesymbols,
                     "","","");
        if (ts)
        {
            ts_get(ts);
            battery = cst_strdup(ts_get(ts));
            ts_close(ts);
            if (cst_streq("",battery))
            {
                cst_free(battery);
                battery = cst_strdup("N/A");
            }
        }
        (void)unlink("/tmp/bard_current_battery");
    }
    else
#endif       
#if defined (_WIN32)   
    static char batval[10];
    SYSTEM_POWER_STATUS syspwrstat;
    int batlife;
    if (GetSystemPowerStatus(&syspwrstat) != 0)
    {
       batlife = syspwrstat.BatteryLifePercent;
       if (batlife == 255)
          battery = cst_strdup("N/A");
       else 
       { 
          cst_sprintf (batval, "%d%%", batlife);
          battery = cst_strdup(batval);
       }
    }
    else
#endif    
        battery = cst_strdup("N/A");

    return battery;
}

void bard_screen_off(const char *backlight_off)
{
#if !defined (BSD) && !defined (_WIN32) && !defined (MSDOS)
    /* Switches off LCD */
    cst_file f;

    if (!bard_getenv("DISPLAY"))  /* Not under X11 -- X11 does its own dpms  */
    {
        /* This is right for Nanonote, Pandora, n900 and GCW0  */
        /* If it doesn't work, then blanking doesn't work (which is okay) */
        f = cst_fopen("/sys/class/graphics/fb0/blank",CST_OPEN_WRITE);
        if (f)
        {
            cst_fprintf(f,"1\n");
            cst_fclose(f);
        }
    }
#endif
    
    if (bard_debug)
        printf("bard_debug screen off\n");

    return;
}

void bard_screen_on(const char *backlight_on)
{
#if !defined (BSD) && !defined (_WIN32) && !defined (MSDOS)
    /* Switches on LCD */
    cst_file f;

    if (!bard_getenv("DISPLAY"))  /* Not under X11 -- X11 does its own dpms  */
    {
        /* This is right for Nanonote, Pandora, GCW0 and n900 */
        /* If it doesn't work, then blanking doesn't work (which is okay) */
        f = cst_fopen("/sys/class/graphics/fb0/blank",CST_OPEN_WRITE);
        if (f)
        {
            cst_fprintf(f,"0\n");
            cst_fclose(f);
        }
    }
#endif

    if (bard_debug)
        printf("bard_debug screen on\n");

    return;
}

