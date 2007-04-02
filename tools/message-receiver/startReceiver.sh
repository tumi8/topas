#!/bin/sh
if [ $# -eq 2 ]
then java -cp .:$XMLBLASTER/lib/xmlBlaster.jar MessageReceiver -plugin/socket/hostname $1 -plugin/socket/port 7607 $2
elif [ $# -eq 1 ]
then java -cp .:$XMLBLASTER/lib/xmlBlaster.jar MessageReceiver $1
else echo "Usage: $0 [hostname] topicname"
fi
