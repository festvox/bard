###########################################################################
##                                                                       ##
##                  Language Technologies Institute                      ##
##                     Carnegie Mellon University                        ##
##                         Copyright (c) 2012                            ##
##                        All Rights Reserved.                           ##
##                                                                       ##
##  Permission is hereby granted, free of charge, to use and distribute  ##
##  this software and its documentation without restriction, including   ##
##  without limitation the rights to use, copy, modify, merge, publish,  ##
##  distribute, sublicense, and/or sell copies of this work, and to      ##
##  permit persons to whom this work is furnished to do so, subject to   ##
##  the following conditions:                                            ##
##   1. The code must retain the above copyright notice, this list of    ##
##      conditions and the following disclaimer.                         ##
##   2. Any modifications must be clearly marked as such.                ##
##   3. Original authors' names are not deleted.                         ##
##   4. The authors' names are not used to endorse or promote products   ##
##      derived from this software without specific prior written        ##
##      permission.                                                      ##
##                                                                       ##
##  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         ##
##  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      ##
##  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   ##
##  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      ##
##  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    ##
##  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   ##
##  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          ##
##  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       ##
##  THIS SOFTWARE.                                                       ##
##                                                                       ##
###########################################################################
##                                                                       ##
##  A storytelling program                                               ##
##     ebook reader with speech                                          ##
##                                                                       ##
##       Authors:  Alan W Black (awb@cs.cmu.edu)                         ##
##          Date:  Jan 2012                                              ##
##                                                                       ## 
##  Supports various output devices: terminals, SDL, or nothing          ## 
##                                                                       ## 
###########################################################################
TOP=..
DIRNAME=main
BUILD_DIRS = 
ALL_DIRS = 
SRCS = bard_main.c
OBJS = $(SRCS:.c=.o)
FILES = Makefile $(SRCS)

ALL = $(BINDIR)/bard

include $(TOP)/config/common_make_rules

FLITELIBDIR=$(FLITEDIR)/build/$(TARGET_PLATFORM)/lib
FLITE_LANG=usenglish
FLITE_LEX=cmulex
LOCAL_INCLUDES += -I$(FLITEDIR)/include

LOCAL_LANGLEX_LIBS = -lflite_$(FLITE_LANG) -lflite_$(FLITE_LEX) -lflite_cmu_indic_lang -lflite_cmu_indic_lex
LOCAL_VOICE_LIBS = -lflite_cmu_us_kal
LOCAL_LIBS = -L$(FLITELIBDIR) $(LOCAL_LANGLEX_LIBS) $(LOCAL_VOICE_LIBS)

LIBS := -lbard $(SDLLIBS) $(LOCAL_LIBS) $(FLITELIBFLAGS) $(LDFLAGS) $(LIBS)

$(BINDIR)/bard: $(OBJS) $(TOP)/build/$(TARGET_PLATFORM)/lib/libbard.a
	$(CC) -o $(BINDIR)/bard $(OBJS) $(LIBS)

install: $(BINDIR)/bard
	$(INSTALL) -s $(BINDIR)/bard $(DESTDIR)$(INSTALLBINDIR)
