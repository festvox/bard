#!/bin/sh
export PATH=":${PATH:-"/usr/bin:/bin:/usr/local/bin"}"
export LD_LIBRARY_PATH="/mnt/utmp/Bard/lib:${LD_LIBRARY_PATH:-"/usr/lib:/lib"}"
export HOME="/mnt/utmp/Bard" XDG_CONFIG_HOME="/mnt/utmp/Bard"

if [ -d /mnt/utmp/Bard/share ];then
   export XDG_DATA_DIRS=/mnt/utmp/Bard/share:$XDG_DATA_DIRS:/usr/share
fi

BARD_ARGS=
if [ ! -f .bard_config ]
then
   # No existing bard_config file so start up, setting reasonable
   # starting values for the Pandora, and reading the Bard_Help file
   BARD_VOICES="-voices_dir $HOME/voices"
   # Fit on the screen, but base tray still visible
   BARD_SCREEN=" -screen_height 420 -screen_width 800"
   # Enough to display basic use commands on first startup
   BARD_FSIZE="-font_size 29"
   BARD_SDELAY="-scroll_delay 45"
   BARD_HTEXT="-text Bard_Help.html"
   BARD_ARGS="$BARD_VOICES $BARD_SCREEN $BARD_FSIZE $BARD_SDELAY $BARD_HTEXT"
fi

export SDL_AUDIODRIVER="alsa"
cd $HOME
[ -e "$HOME/scripts/pre_script.sh" ] && . $HOME/scripts/pre_script.sh
if [ -e "$HOME/scripts/post_script.sh" ];then
	./bard $BARD_ARGS $*
	. $HOME/scripts/post_script.sh
else
	exec ./bard $BARD_ARGS $*
fi
