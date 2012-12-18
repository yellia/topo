=Description:

    This is a easy and small topo discovery program.

=Goal:

    we want to implement a small and fast system that could quickly 
    1. discovery devices of second layer and third layer in local network. 
    2. find out their relationship between each other.

    if all of above have reached, we will get a complete graph about the loca network,I think it could be interesting.

=Dependencies:

    Until now, snmp is our way to discovery. we use snmp++ lib which can be found in depend diectiory to get infomation of devices, so if devices don't support snmp, we could do nothing.

=Policy:

   1. init router ip when system starts manually from command line or system config file. we call it core router,that is to say it connect to Internet directly.  

   2. read core router's router table to find other routers, 
   
   and then find more other routers, finally we will get some networks,for example:192.168.1.1/24,192.168.2.1/24 and so on.
 and relationship among them.

   3. for each network and find second and third layer devices and their relationship according fdbtable.
   
   4. finish.

   !note: 
   router table: it record other routers connected to current.
   fdbtable: it record mac to ip.

=Difficultis

    1. how to distinguish router, second layer switch , third layer switch and gernal server.-->solved but not perfect.

    2. how to discover third layer devices (routers, third switches) relationship

    3. how to discover switch (second switches, gernal servers) relationship

    4. how to express all devices relationship, of course by graph, but how?
