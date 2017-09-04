###########################################################################
##                                                                       ##
##                  Language Technologies Institute                      ##
##                     Carnegie Mellon University                        ##
##                         Copyright (c) 2013                            ##
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
##  Open Pandora pnd support                                             ##
##  http://pandorawiki.org/                                              ##
##                                                                       ## 
##  The core files for making a Bard.pnd, and commands to build it       ##
##  These are very specific to my Pandora setup, so might not work on    ##
##  others' machines                                                     ##
##                                                                       ## 
###########################################################################
TOP=..
DIRNAME=pandora
BUILD_DIRS = 
ALL_DIRS = 
OTHERS = PXML.xml bard.sh Bard_Help.html
FILES = Makefile $(OTHERS)

ALL = pnd
PND_MAKE=/usr/pandora/scripts/pnd_make.sh

include $(TOP)/config/common_make_rules

pnd: 
	rm -rf Bard
	mkdir -p Bard/lib Bard/scripts Bard/voices
#       The following 2 lines are specific to awb's Pandora set up
	cp -pr /usr/local/lib/libzip* Bard/lib
	cp -pr ${FLITEDIR}/voices/*.flitevox Bard/voices
#       Create a fake file to identify the kal voice
	touch Bard/voices/kal.flitevox
	cp -pr bard.sh Bard/scripts
	cp -pr PXML.xml Bard/PXML.xml
	cp -pr Bard_Help.html Bard/Bard_Help.html
	cp -pr ${TOP}/bin/bard Bard/bard
	cp -pr ${TOP}/COPYING Bard/COPYING
	cp -pr ${TOP}/README Bard/readme.txt
	cp -pr ${TOP}/Bard32.png Bard/Bard.png
	${PND_MAKE} -d Bard -p Bard.pnd -i Bard/Bard.png -x Bard/PXML.xml -c

