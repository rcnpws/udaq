#!/bin/csh
# DAQ control commands
#
#    set HOST="192.168.2.21"
    set HOST="aino-1.rcnp.osaka-u.ac.jp"
#    set HOST="asagi2.rcnp.osaka-u.ac.jp"

    switch($1)
    case 'blp'
    set ps = `ps -fu $USER | grep blpcollect | grep -v grep | awk '{print $2}'`
    if("$ps" == "") then
	    blpcollect  -Q -e $HOST $HOST > /dev/null  & 
    endif
    breaksw
    case 'gr'
    set ps = `ps -fu $USER | grep grcollect | grep -v grep | awk '{print $2}'`
    if("$ps" == "") then
#	    xterm -title "GR collector" -e grcollect -Q -q 128 
	    xterm -title "GR collector" -e grcollect -q 128 
#	    grcollect -Q -q 128 
    endif
    breaksw
    default:
    echo "daq ... daq control command"
    echo "daq $*"
    echo "Usage: daq  [blp:gr]"
    breaksw
    endsw
    
