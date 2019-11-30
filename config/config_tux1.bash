ifconfig eth0 up
ifconfig eth0 172.16.50.1/24
route add -net 172.16.51.1/24 gw 172.16.50.254

