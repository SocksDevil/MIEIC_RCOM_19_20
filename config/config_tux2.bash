if ["$1" != ""]; then
    stand = $(echo $1 | head -c 1)
    ifconfig eth0 up
    ifconfig eth0 172.16.${stand}1.1/24
    route add -net 172.16.${stand}0.0/24 gw 172.16.${stand}1.253
    route add default gw 172.16.${stand}1.254
    echo "search netlab.fe.up.pt \n nameserver 172.16.1.1" > /etc/resolv.conf
else 
    echo "Missing tuxy computer!"
fi