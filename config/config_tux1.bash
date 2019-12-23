if [-z $1]; then
    stand = $(echo $1 | head -c 1)
    ifconfig eth0 up
    ifconfig eth0 172.16.${stand}0.1/24
    route add -net 172.16.${stand}1.0/24 gw 172.16.${stand}0.254
    route add default gw 172.16.${stand}0.254
    echo "search netlab.fe.up.pt \n nameserver 172.16.1.1" > /etc/resolv.conf
else 
    echo "Missing tuxy computer!"
fi
