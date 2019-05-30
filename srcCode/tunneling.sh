#!/bin/bash

ifconfig beitunT0 192.168.4.4 pointopoint 192.168.5.1
ifconfig beitunS 192.168.5.1 pointopoint 192.168.4.4
ip route del 192.168.5.1 table local
ip route del 192.168.4.4 table local
ip route add local 192.168.5.1 dev beitunS table 5
ip route add local 192.168.4.4 dev beitunT0 table 4
ip rule add iif beitunS lookup 5
ip rule add iif beitunT0 lookup 4

