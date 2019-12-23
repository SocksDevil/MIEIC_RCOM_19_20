if ["$1" != ""]; then
    stand = $(echo $1 | head -c 1)
    ifconfig eth0 up
    ifconfig eth0 172.16.${stand}0.254/24
    ifconfig eth1 up
    ifconfig eth1 172.16.${stand}1.253/24
    route add default gw 172.16.${stand}1.254
else 
    echo "Missing tuxy computer!"
fi