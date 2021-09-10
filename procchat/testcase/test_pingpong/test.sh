timeout 20 ./procchat > output_program &
cat testcase/test_pingpong/connection_bot1 > gevent
sleep 18
ps -ef | grep defunct | grep -v grep | wc -l
sleep 2
ls chatRoom1/ | wc -l

