#!/bin/sh


PNAME=vdev
PID_FILE=$PNAME.pid
VBIN=./$PNAME
PARAM=' config=.'$PNAME'.conf'


case $1 in

	help)
		$VIRT_BIN$SPS--$1 &
		exit 0
	;;

	start)
		echo
		printf "Starting $PNAME : "
		if [ -r $PID_FILE ] ; then 
			echo "file $PID_FILE exists! -- maybe $PNAME already running!"
			exit 1
		else
			if [ ! -x "$VBIN" ] ; then
				echo "file $VBIN not found!"
				exit 1
			fi
			$VBIN$PARAM >>/dev/null &
			sleep 2
			if [ ! -s $PID_FILE ] ; then
				echo "  PID file $PID_FILE does not exist -- $PNAME start failed"
				exit 1
			fi
			echo "started with pid(`cat $PID_FILE`)"
		fi
		exit 0
	;;

	stop)
		if [ -r $PID_FILE ] ; then
		    printf "Stopping $PNAME : "
		    kill -15 `cat $PID_FILE`
		    echo "Wait all threads closed..."
		    i=0
		    while [ -r $PID_FILE ] ; 
		    do
			sleep 1
			i=$(($i+1))
			if [ "$i" -gt 10 ]; then
			    echo "Timeout error !"
			    kill -9 `cat $PID_FILE`
			    rm $PID_FILE
			    echo
			    break
			fi
		    done
		else
			exit 1
		fi
		exit 0
	;;

	restart)
		$0 stop
		sleep 3
		$0 start
		exit 0
	;;

	*)
	    echo "No action present. Use : ./$0 help|start|stop|restart"
	;;

esac

echo

