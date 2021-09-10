timeout 10 ./procchat > output_program &
cat testcase/test_sayRec_disconnect/connection_bot2 > gevent
cat testcase/test_sayRec_disconnect/connection_bot1 > gevent

cat testcase/test_sayRec_disconnect/saysomething > chatRoom1/bot1_WR

sleep 1

read line < chatRoom1/bot2_RD
echo $line

cat testcase/test_sayRec_disconnect/disconnect > chatRoom1/bot1_WR
cat testcase/test_sayRec_disconnect/disconnect > chatRoom1/bot2_WR

