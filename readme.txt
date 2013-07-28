20000 150connect just insrv midsrv.test
20000 4000connect just insrv midsrv.test

use test: 1 srv block socket, 1 client no sleep, iptarf: 3000kbits/sec 
use insrv midsrv.test: 1 srv block socket, 1 client no sleep, iptarf: 700kbits/sec [because one socket handle one msg then usleep(10), if send two msg one time then 800kbits/sec] !!! [use ClientSR 1 then 1000kbits/sec, ClientSR 100 then 12000kbits/sec, ClientSR 200 then 13000kbits/sec but id has 50, 16000kbits/sec == 18000msg/sec, peak is 18000kbits/sec == 20000msg/sec id 32 but host id 3 ]

[insrv, midsrv, outsrv, db_insrv, db_midsrv, Client]
17,000kbits/sec, 13000msg/sec, id < 10, 151connect
when close iptraf: 20000msg/sec, id < 5, 201connect
500msg/sec, id > 80, 1connect, sleep 0
18,000msg/sec, id < 5, 2000connect, sleep 100

[insrv, midsrv, outsrv, db_insrv, db_midsrv, ClientSR]
100,000msg/sec, id < 5, 1connect, sleep 0, network is nearly 100%

===== 2013-07-27 Night =====
finish big endian and little endian
about 18000msg/sec is bottleneck
===== 2013-07-28 Day =====
add timeout check
about 18000msg/sec is bottleneck
4000connect, 500ms, 6000msg/sec is bottleneck, id < 5
if not timeout check:
7000conncet, 500ms, 11000msg/sec is bottleneck, id < 5
heap has problem, it doesnot work. quick sort is ok.