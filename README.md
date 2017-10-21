                      Bard Storyteller 0.12 Sep 2017
                         http://festvox.org/bard/
                      https://github.com/festvox/bard

Bard Storyteller is a text reader.  Bard not only allows a user to
read books, but can also read books to the user using text-to-speech.
It supports txt, epub and (x)html files.

Bard Storyteller is free software, distributed under a BSD-like license.

Bard depends on CMU Flite (cmuflite.org) for both some general C
utilities and for synthesis itself.  Bard is targeted at small
portable devices such as smart phones, not so smart phones and offline
devices, but also works on laptops and desktops.  The original target
platform was the open hardware Ben Nanonote from sharism.cc
(http://en.qi-hardware.com/wiki/Ben_NanoNote).  

In addition to source code support for standard Linux and Windows
support, binary distributions are available for Ben Nanonote, Open
Pandora and GCW Zero.

Bard uses libSDL (http://www.libsdl.org -- 1.2) to provide graphics, sound
and input, thus aiming at multiple platforms.  Bard is written in C.

Bard was written by Alan W Black (awb@cs.cmu.edu) as a better ebook reader
for him to use, and as a tool to improve CMU Flite's speech synthesis API.

Bard was somewhat inspired by various ebook reading apps I've used on
various PDAs and phones.  Most recently, after downloading Lecturer
http://code.google.com/p/lecturer/ by Ulrich Hecht, and adding Flite
support to Lecturer, I saw it might be easy to write a full text
reader myself from scratch, so I did.

I'd like this to be TTS engine neutral but its not.

At present the kal_diphone voice is built in, but any dumped Flite cg 
voice may be specified on command line with

    -voice file:///home/.../VOICE.flitevox
     
If initially started with -voices_dir PATH, voices may be loaded
within Bard and remembered for particular files that are being read.  At
present 16KHz (order 25) cg voices are too slow for the Ben Nanonote
but are fine on faster devices (e.g. Open Pandora, Rasberry Pi, GCW
Zero etc).

It can be run fully from the graphical page, using game console keys only, 
but also can use a standard keyboard.

From 0.11, Bard supports multiple fonts including Chinese (Traditional and
Simplified), Korean, Japanese, Devanagari, Tamil and Telugu (with proper
kerning when libharfbuzz is compiled in).

    Alan W Black                                email: awb@cs.cmu.edu
    Language Technologies Institute             http://www.cs.cmu.edu/~awb/
    Carnegie Mellon University                  tel: +1-412-268-6299  
    5000 Forbes Ave, Pittsburgh PA, 15213, USA. fax: +1-412-268-6298

DEPENDENCIES

    flite-2.1
    libSDL 1.2
    libSDL_ttf 2.0

Both are available from  http://www.libsdl.org

For epub (which is optional)

    libzip http://www.nih.at/libzip/

Note we need both zip.h and zipconf.h installed before configure will
enable epub support.
   
For epub images you also need

    libSDL_image
    libSDL_gfx

COMPILATION

    git clone https//github.com/festvox/flite
    cd flite
    ./configure && make && make voices
    export FLITEDIR=`pwd`
    cd ..

    git clone https//github.com/festvox/bard
    cd bard
    ./configure
    make
   ./bin/bard -voices_dir $FLITEDIR/voices

Bard requires access to a .ttf font, at configuration time it tries to find 
this in /usr/share/fonts, but it might not succeed in finding the right one.
You can edit config/config and add a (full) pathname to a .ttf font.
For development I used LiberationSerif-Regular.ttf (you probably want a serif
font).  You can get the Liberation fonts from

    https://www.redhat.com/promo/fonts/
   
Or DejaVu fonts from

    http://dejavu-fonts.org
    
If it can't automatically find one, you can specify a full path name to one

    ./bin/bard -font /usr/share/fonts/liberation/LiberationSerif-Regular.ttf

And it will remember that for future calls (it saves the font pathname in 
$HOME/.bard_config on exit)

From version 0.11, Bard supports multiple fonts, and relates them to
the current file.  Bard supports at least Chinese (Traditional and
Simplified), Korean, Japanese, Hindi (Devanagari), Tamil and Telugu
(the last three with proper kerning if used with libharfbuzz).  If
started with -font_dir PATH a font menu window will be available to
allow you select an appropriate font.  Google's Noto Fonts have an
appropriate lincense and have good coverage for most languages
https://www.google.com/get/noto/.  Speech Synthesis in these languages
is also support when we have a voice in that language.

A config file is created in $HOME/.bard_config.  You might need to
remove that if things fail to work out properly.  This stores
customized colors, fonts and also what your current file (and
position) is and up to 20 recent files and positions.  I am trying to
make that robust over version upgrades, and have succeeded so far.

BASIC COMMANDS ARE

    tab    Toggle speech mode
    space  Toggle scrolling
    f      display file selection window
           arrow keys, home, page down, and enter for selection
    g      display general parameters (macro navigation, volume, speed)
    h      display help
    r      recent files
    m      menu (of other pages)
    v      voice select
    c      font select
    i      Increase font/volume up/increase scroll speed
    o      Decrease font/volume down/decrease scroll speed
    vol up/down (F11/F12) control scroll speed, volume or page up/down

You can navigate a page with pagedown, home and cursor keys within a
page.  Page up is harder than I thought, it does something useful, but
may be not always exactly what you want.

The general parameters page ('g') displays file name, position, speech
volume, speed, battery status (if supported), and current time.  If
you use the arrow keys to position over "<<<" and ">>>", pressing
enter will modify the values.  Currently the battery status on the
my Nanonote is found by running a script called "battery" and taking $2
field in the result.  (This works for my battery script but probably
not yours).  You can make it run your battery script by specifying it
on the command line with -battery_script SCRIPT (or editing the
.bard_config file).

The volume keys on the Ben Nanonote (F11 F12) provide control for 
scroll speed, volume or page up/down depending the current state
(scrolling, speaking or nothing, respectively)  (Shoulder buttons
L1/R1 do the same thing on machines with shoulder buttons)

This version doesn't support a mouse (or touch screen) but later versions
will (as well as whatever buttons you have on your mobile device).

FONTS
-----

Fonts are harder than you think.  As of 0.11 Bard supports selecting
fonts with files, if started with the -font_dir DIR argument.  With
harfbuzz support on, Bard will properly display Chinese (Simplified
and Traditional), Japanese, Korean, and many Indian scripts (Devnagari,
Telugu, Kannada and Tamil) and probably more.  Rendering and proper
kerning happen under harfbuzz, without it rendering will not be correct
for languages that require it (e.g. halant in devnagari).  There is no
support for right to left languages yet (e.g. Arabic script).  

The system will guess the font when loading a file, from your previous
font selections and an internal list, but you can always override the
selection in the font menu.  

VOICES
------

In addition to fonts, Bard also supports synthesis for other
languages, if started with the -voices_dir DIR argument.  At present
we only distribute voices for Hindi, Tamil and Telugu (and English).
Voices however need to be explicitly selected for new files (it will
default to the previously selected voice).  The Hindi, Telugu and
Tamil voices will speak romanized words/characters as English which
seems to be a reasonable option.

COLORS
------

You can customize your own colors by specify them in the form of 0xFFFFFF
in the ~/.bard_config file.  It is best to run bard and quit it to get a 
basic file then change the various color specifications to what you want.
There are only a very few builtin (black. white, red, blue and green), but
you can specify and red green blue hex values.  e.g. if you want the main
text window background color to be antiquewhite change the line

    -text_background_color "white"

to

    -text_background_color "0xcdc0b0"

HISTORY
-------

0.11 August 2014

     harfbuzz/cairo font layout (optional)
     Non-latin language support (Hindi, Chinese, Japanese ...) 
     added font selection sub-menu
     ISSUES: 
     - flite 2.0 voices are slow to load -- they run fine but can take
       5-10 seconds to load from slow devices like sd card
     - It doesn't automatically select the font, if you load a Chinese book
       you need to then use the font sub menu to select a Chinese font to
       go with it.  It will remember that font for that book in future loads
       but you've got to do that initial selection yourself.
       
0.10 July 2014

     gcw-zero support (which only has game console buttons)
     much better (x)html support so it can properly interpret multiple tags
     
0.9  Dec 2013

     menu window (for machines that don't have keyboards)
     voice select, voice name saved with recent files
     "game" controller support (but really for OpenPandora key bindings)
     Open Pandora .pnd support
     
x.x  Nov 17 2013  LM

     changes to build on Windows with mingw and msys, handle relative image 
     locations with ../ in path, handle &lt; &gt; and better support for &amp;
     add support for plus on US keyboards (SDLK_Equals plus shift).
     
0.8  ??? (never released)

     (x)html fixes/support
     
0.7  Mar 8th 2012

     cursor hiding, epub (generic tokenstreams), some xhtml highlighting
     paragraph processing improved, bug fixes, epub images,
     fixed smooth scrolling, better audio streaming.
     
0.6  Jan 29th 2012

     page up (ish), keyboard rationalization
     discover screen size (works on n900), screen blank
     bug fixes, externally specified colors, smooth scroll
     
0.5  Jan 22nd 2012 (first release to nanonote list)

     info page: volume, position, speed, battery and time working
     make ipk, logo
     
0.4  Jan 21st 2012

     imported into CVS, paging forward and back (with arrow keys and speech), 
     recent files page, general info page (%pos, volume, speed -- well almost)
     .bard_config, make install, use installed fonts rather than include one
     
0.3  Jan 14th 2012

     SDL audio stable and now default.  Help window.  Tokens on screen can 
     be traversed with arrow keys, and highlighted.  File selection
     works (with dirent and highlighting).  Improved configuration, added
     COPYING and README, included free ttf font.  Technically functional
     
0.2  Jan 7th 2012

     display text (good ttf), speaks, auto pages forward
     support SDL and flite_direct audio.  
     Runs on ben, yentna, leith and awbt60
     
0.1  Jan 4th 2012

     displays loaded text, pages forward, can change font size
     runs on desktop and Ben Nanonote

AKNOWLEDGEMENTS
---------------

Thanks to LM for giving a whole set of fixes, and getting me to add some
new features

The Bard port to OpenPandora was encouraged by the Alive and Kicking
Coding Competition 2013/2014.

FUTURE GOALS
------------

We want this to be a platform for new (fast) high quality synthesis
voices in Flite and are working on getting them fast enough to be
practical on 336MHz processors.  We also want to support more ebook
formats.  Specific features already on the list are:

    Other ebook formats: prc, rtf, pdf
    More complete support for epub, (x)html
    Search/indexing
    Chapter Navigation (when chapters can be found in epub files)
    Support for more platforms (e.g. Android)
    (Cleaner) Support for cross compilation
    SDL 2 support
    Automatic font selection within books

But please recommend other features you'd like to see.

