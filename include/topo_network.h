#ifndef TOPO_NETWORK_H
#define TOPO_NETWORK_H

#include "topo_switch.h"
#include "topo_router.h"

class topo_network
{
public:
	topo_network():m_network("192.168.1.0/24"), m_corerouter_ip("192.168.1.1"){add_router(m_corerouter_ip);}
	topo_network(const string &name, const string &savepath, const string &corerouter);
	virtual ~topo_network();
public:
	typedef vector<topo_device*> topo_master_slave_t;/*vector[0] master vector[1] slave*/
	typedef struct{ string machinename; topo_master_slave_t ms; string floatip;}topo_unit_t;		

	typedef map<string,topo_unit_t> topo_server_t;
	typedef map<string,topo_device*> topo_switch_t;
	typedef map<string,topo_device*> topo_lay3switch_t;
	typedef map<string,topo_device*> topo_router_t;

	typedef map<string,string> topo_mac2ip_t;
	typedef map<string,string> topo_ip2mac_t;
	typedef map<string,topo_device*> topo_device_t;
	typedef map<string,topo_master_slave_t> topo_heartbeat_t;
private:
	string m_network;/*network name ex:192.168.1.0/24*/
	string m_savepath;/*network save path*/
	string m_corerouter_ip;/*core router ip*/
	topo_server_t m_servers;/*all servers in current network*/	
	topo_switch_t m_switches;/*all switches in current network*/	
	topo_lay3switch_t m_lay3switches;/*all switches in current network*/	
	topo_router_t m_routers;/*all routers in current network*/	

	topo_mac2ip_t m_mac2ip;/*update when network create , restore and add new ip*/
	topo_ip2mac_t m_ip2mac;/*update when network create , restore and add new ip*/
	topo_device_t m_devices;/*store all devices of current network <mac,device*>, update when network create ,restore, add new ip*/

	topo_heartbeat_t m_heartbeats;/*store floatip to devices*/
private:
	bool savepath_exist();/*check if network is saved befored*/
	void check_alive();/*check servers is alive by ping m_serveres*/
	void check_fdbtable();/*check switch fdb using in update network */
	void reset_device_status();/*reset device status died*/
	void discovery_ping();/*find new device in a newly created network*/
	void create_connection();/*create connection of switches,routers,servers*/
	void get_fdbip_switch(topo_device* s, topo_mac2ip_t &fdbip);/*get a switch fdbtable*/
	bool find_mac2ip(const string &mac, string &ip);/*find mac to ip using m_mac2ip golbal*/
	bool find_ip2mac(const string &ip, string &mac);/*find ip to mac using m_ip2mac golbal*/
	void update_mac2ip(topo_router *router);/*update mac2ip and ip2mac*/	
	void global_check_update(topo_switch *s, const string &ma, const string &ip);/*check mac ip global variables m_devices*/
	void update_server_unit(const string &float_ip, topo_device *d);/*write float_ip of device d*/
	void ping_iprange(unsigned int first, unsigned last);/*ping from first to last ,classify and add to devices*/
public:
	void set_network(const string &network){this->m_network = network;}
	const string &get_network()const{return this->m_network;}
	bool persistence();/*save network to file*/	
	bool restore();/*restore network from file*/
	void update();/*discover devices of network*/
	void print_devices();/*print out devices of network*/
	void print_connection();/*print out connection of current network*/
	void print_heartbeat();/*print floatip to devices*/
	string save_connection();/*save connection*/
	topo_device* add_device(const string&ip);/*add device to current network*/
	topo_device* add_switch(const string&ip);/*add switch to current network*/
	topo_device* add_router(const string&ip);/*add router to current network*/
	topo_device* add_server(const string&ip);/*add server to current network*/
};

#endif
