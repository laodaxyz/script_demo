#! /bin/sh
num=$(netstat -anpl |grep sniffer_svr |wc -l)
#num=$(netstat -anpl |grep :4100 |wc -l)
if [ $num -gt 2 ]; then
    sniffer_pid=$(ps aux|grep ./sniffer_svr|grep -v grep|awk '{print $2}')
    echo $sniffer_pid
    kill $sniffer_pid
    cd /home/server1
    ./sniffer_svr
fi
