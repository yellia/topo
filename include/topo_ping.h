/**************************************************
*Description:ping to check alive, not thread safe
*author: yellia
*DateTime: 2012-03-29
**************************************************/

#ifndef TOPO_PING_H
#define TOPO_PING_H

#include "topo_log.h"
#include "topo_network.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

class topo_ping
{
public:
	typedef struct { string ip; int flag;/*1 alive, -1 died*/ }ping_unit_t;
	typedef vector<ping_unit_t*> topo_ping_t;
public:
	/**************************************************************************
	*Description:
	*		check a server is alive		
	*Parameters:	 
	*		ip - hostip	
	*return value:
	*		true success,false failed		
	***************************************************************************/
	static bool ping(const string &ip);
public:
	/**************************************************************************
	*Description:
	*		check servers are alive		
	*Parameters:	 
	*		servers - check servers	
	***************************************************************************/
	static void ping(vector<topo_device*> &servers);
	/**************************************************************************
	*Description:
	*		check iprange are alive		
	*Parameters:	 
	*		servers - check servers	
	***************************************************************************/
	static void ping(topo_ping_t &servers);
	/**************************************************************************
	*Description:
	*		get ip range		
	*Parameters:	 
	*		first - first ip address in network
	*		last - last ip address in network
	*		broadcast - broadcast ip address in network
	*		network - network ip address in network
	***************************************************************************/
	static void get_iprange(const string &ip, unsigned int &first, unsigned int &last);
private:
	/**************************************************************************
	*Description:
	*		convert vector<topo_device*> to vector<ping_unit_t*>		
	*Parameters:	 
	*		servers - vector 
	*		v_servers - vector	
	***************************************************************************/
	static void convert_device2ping(vector<topo_device*> &servers, topo_ping_t &v_servers);
};

#endif
