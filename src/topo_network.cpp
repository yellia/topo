#include "topo_network.h"
#include "topo_log.h"
#include "topo_router.h"
#include "topo_switch.h"
#include "topo_ping.h"

topo_network::topo_network(const string &name, const string &savepath, const string &corerouter)
	: m_network(name), m_savepath(savepath), m_corerouter_ip(corerouter)
{
	add_router(m_corerouter_ip);

	if(savepath_exist())
	{
		if(restore())
		{
			duker_log("restore %s network from %s success", m_network.c_str(), m_savepath.c_str());

			/*check alive all devices in this network*/
			check_alive();	
		}
	}
	else/* a net network*/
	{
		/*first ping all device in current network using ping*/
		discovery_ping();
	}
}

void topo_network::discovery_ping()
{
	/*foreach all network segment of corerouter from m_network*/
	/*default is m_network=192.168.1.0/24;192.168.3.0/24*/
	string::size_type pos,start = 0;
	while((pos = m_network.find(";", start)) != string::npos || start != string::npos)
	{
		string network = m_network.substr(start, pos);

		/*1, get ip range*/
		unsigned int first,last;
		topo_ping::get_iprange(network, first, last);

		duker_log("now begin to ping %u.%u.%u.%u - %u.%u.%u.%u", 
		(first & 0xff000000 ) >> 24, (first & 0x00ff0000) >> 16,
		(first & 0x0000ff00 ) >> 8, first & 0x000000ff,
		(last & 0xff000000 ) >> 24, (last & 0x00ff0000) >> 16,
		(last & 0x0000ff00 ) >> 8, last & 0x000000ff);
		
		/*2, ping each ip and classify of them and add to devices*/
		ping_iprange(first, last);

		start = pos != string::npos ? pos + 1 : string::npos;
	}
}

topo_network::~topo_network()
{
	persistence();

	/*free switches*/
	topo_device_t::iterator siter = m_devices.begin();
	while(siter != m_devices.end())
	{
		topo_device *&s = siter->second;	
		delete s;	
		s = NULL;

		++siter;
	}
	//duker_log("topo_network::~topo_network()");
}

bool topo_network::persistence()/*save network to file*/	
{
	duker_log("begin to persitence network object to file %s", m_savepath.c_str());
	/*not write died stauts to file*/

	return true;
}

bool topo_network::restore()/*restore network from file*/
{
	duker_log("begin to restore network object from file %s ", m_savepath.c_str()); 
	return true;
}

void topo_network::reset_device_status()
{
	duker_log("now begin to reset all device status to died");

	/*reset switches status*/
	topo_switch_t::iterator siter = m_switches.begin();
	while(siter != m_switches.end())
	{
		topo_switch* s = (topo_switch*)siter->second;
		s->set_device_status(topo_device::status_died);
		duker_log("reset switch =%15s status = %5s success", s->get_hostip().c_str(), s->get_status_string().c_str());
		++siter;
	}

	/*reset routeres status*/
	topo_router_t::iterator riter = m_routers.begin();
	while(riter != m_routers.end())
	{
		topo_router* s = (topo_router*)riter->second;
		s->set_device_status(topo_device::status_died);
		duker_log("reset router =%15s status = %5s success", s->get_hostip().c_str(), s->get_status_string().c_str());
		++riter;
	}

	/*reset servers status*/
	topo_server_t::iterator iter = m_servers.begin();
	while(iter != m_servers.end())
	{
		topo_unit_t unit = iter->second;	
		topo_master_slave_t ms = unit.ms;
		
		for(size_t j=0; j < ms.size(); ++j)
		{
			topo_device *d = ms[j];
			d->set_device_status(topo_device::status_died);
			duker_log("reset server =%15s status = %5s success", d->get_hostip().c_str(), d->get_status_string().c_str());
		}
		++iter;
	}
}

void topo_network::update()/*update status of all devices of network*/
{
	/*check status of devices already in this current network  if some device is died then we can find them*/	
	check_alive();

	/*compare fdbtable of all switches with devices in current network, if not exist then add to device: router,switch or server*/	
	check_fdbtable();

	//check connection with switches
	

	/*print out all device in current network*/
	print_devices();

	/*print connection of current network*/
	print_connection();

	/*print heart beat float ip*/
	print_heartbeat();
}

void topo_network::get_fdbip_switch(topo_device* d, topo_mac2ip_t &fdbip)
{
	topo_switch* s =(topo_switch*)d;
	vector<string> macs;

	s->get_fdb_table(macs);	

	for(size_t i=0; i < macs.size(); ++i)
	{
		string &mac = macs[i];
		string ip = "";

		if(find_mac2ip(mac,ip))/*find ma to ip in m_mac2ip*/
		{
			fdbip.insert(pair<string,string>(mac, ip));
		}	
		else
		{
		//	duker_log("mac %s !!", mac.c_str());
		}
	}
}

bool topo_network::find_mac2ip(const string &mac, string &ip)
{
	topo_mac2ip_t::iterator ret  = m_mac2ip.find(mac);
	bool flag = false;
	ip = "";

	if(ret != m_mac2ip.end()) 
	{
		ip = ret->second;	
		flag = true;
	} 	
	
	return flag;
}
bool topo_network::find_ip2mac(const string &ip, string &mac)
{
	topo_ip2mac_t::iterator ret  = m_ip2mac.find(ip);
	bool flag = false;
	mac = "";

	if(ret != m_ip2mac.end()) 
	{
		mac = ret->second;	
		flag = true;
	} 	
	else
	{
		duker_log("can't find ip %s", ip.c_str());
	}
	
	return flag;
}
/*condition:
*1, switch is find
*2, router is find
*/
void topo_network::check_fdbtable()
{
	topo_mac2ip_t fdbip;/*hold all ip connected to switches*/
	topo_switch_t::iterator siter = m_switches.begin();
	while(siter != m_switches.end())
	{
		topo_switch* s = (topo_switch*)siter->second;
		fdbip.clear();
		/*1, get mac to ip of  switch*/
		 get_fdbip_switch(s, fdbip); 	

		/*compare with device in current network, if not exist then add to current switch connection*/	
		topo_mac2ip_t::iterator iter = fdbip.begin();
		while(iter != fdbip.end())
		{
			string mac = iter->first;
			string ip = iter->second;	

			global_check_update(s, mac,ip);	

			++iter;
		}
		++siter;
	}
}
void topo_network::global_check_update(topo_switch *s, const string &mac, const string &ip)
{
	/*1, check if mac exist*/
	topo_device_t::iterator ret = m_devices.find(mac);	
	string::size_type pos = ip.find(";");
	topo_device *d = NULL;

	if(ret == m_devices.end())
	{
		string real_ip = (pos != string::npos) ? ip.substr(0,pos) : ip;

		d = add_device(real_ip);

	}
	else
	{
		d = ret->second;
		/*check d->type, if is server then wo can check next ,or do nothing*/
		if(d->get_device_type() == topo_device::type_server && pos != string::npos)
		{
			/*1, wo can write float to m_servers*/
			string rip = ip.substr(0, pos);
			string fip = ip.substr(pos + 1, ip.size()-1);
			string float_ip = d->get_hostip() == rip ? fip : rip;

			update_server_unit(float_ip, d);
				
			/*2, do not update m_devices*/	
			duker_log("device mac %s ip %s exist", mac.c_str(), ip.c_str());
		}	
		else if(pos == string::npos)/*one ip*/
		{
			//device ip is changed on line
			duker_log("device mac =%s,ip is change from %s to %s", d->get_mac().c_str(), d->get_hostip().c_str(), ip.c_str()); 
			d->set_hostip(ip);	
		}
		else
		{
			duker_log("device mac =%s,ip =%s is %s , so it can has more than one ip", d->get_mac().c_str(), ip.c_str(), d->get_device_type_string().c_str());
		}
	}

	/*3, add to switch connection*/
	if(d != NULL)
	{
		s->check_update(d);
	}
	else
	{
		duker_log("can't check the type of device mac =%s, ip=%s ,error : unknown device", mac.c_str(), ip.c_str());
	}
}

void topo_network::check_alive()
{
	/*first set all device status is died*/
//	reset_device_status();	

	vector<topo_device*> pings;	

	topo_device_t::iterator iter = m_devices.begin();
	while(iter != m_devices.end())
	{
		pings.push_back(iter->second);

		++iter;
	}

	topo_ping::ping(pings);	
}

bool topo_network::savepath_exist()
{
	return false;
}

void topo_network::print_devices()
{
	duker_log("==============network %s===================", m_network.c_str());	

	duker_log(">>switch size = %d :", m_switches.size());
	topo_switch_t::iterator siter = m_switches.begin();
	while(siter != m_switches.end())
	{
		topo_switch* s =(topo_switch*)siter->second;
		duker_log("\tswitch ip :%15s, hostname:%12s, status:%5s", s->get_hostip().c_str(), s->get_hostname().c_str(), s->get_status_string().c_str());
		++siter;
	}

	duker_log(">>routers size = %d :", m_routers.size());
	topo_router_t::iterator riter = m_routers.begin();
	while(riter != m_routers.end())
	{
		topo_router* s =(topo_router*)riter->second;
		duker_log("\trouter ip :%15s, hostname:%12s, status:%5s", s->get_hostip().c_str(), s->get_hostname().c_str(), s->get_status_string().c_str());
		++riter;
	}

	duker_log(">>servers size = %d :", m_servers.size());
	topo_server_t::iterator iter = m_servers.begin();
	while(iter != m_servers.end())
	{
		topo_unit_t unit = iter->second;	
		topo_master_slave_t ms = unit.ms;

		duker_log("<> machinename %s", unit.machinename.c_str());
		
		for(size_t j=0; j < ms.size(); ++j)
		{
			topo_device *d = ms[j];
		
			if(d->get_device_status() == topo_device::status_died)
			{

				duker_log("\t----------------server ip:%15s, hostname:%12s status:%5s", d->get_hostip().c_str(), d->get_hostname().c_str(), d->get_status_string().c_str()); 
			}
			else if(d->get_device_status() == topo_device::status_new)
			{
				duker_log("\t+++++++++++++++++server ip:%15s, hostname:%12s status:%5s", d->get_hostip().c_str(), d->get_hostname().c_str(), d->get_status_string().c_str()); 

			}
			else
			{
				duker_log("\t==================server ip:%15s, hostname:%10s status:%5s", d->get_hostip().c_str(), d->get_hostname().c_str(), d->get_status_string().c_str()); 
			}
		}
		++iter;
	}
}

void topo_network::update_mac2ip(topo_router *router)
{
	router->get_iptable(m_mac2ip, m_ip2mac);	
}

void topo_network::update_server_unit(const string &float_ip, topo_device *d)/*write float_ip of device d*/
{
	topo_master_slave_t &ms = m_heartbeats[float_ip];

	topo_master_slave_t::iterator iter = find(ms.begin(), ms.end(), d);

	if(iter == ms.end())
	{
		ms.push_back(d);
		duker_log("device ip = %s to floatip = %s", d->get_hostip().c_str(), float_ip.c_str());
	}
}

/*********************************************************
*Description:
*	add device to current network according to its type, we use snmp to check its type 	
*	current support type is: type_switch, type_router, type_server
**********************************************************/
topo_device* topo_network::add_device(const string &ip)
{
	topo_device *d = NULL;
	/* get type of current device, defalut is type_server*/	
	topo_device::device_type device_type = topo_device::check_device_type(ip);	

	switch(device_type)
	{
	case topo_device::type_switch:
		{	
			d = add_switch(ip);	
		}
		break;
	case topo_device::type_router:
		{
			d = add_router(ip);	
		}
		break;
	case topo_device::type_lay3switch:
		{
		//	d = new topo_lay3switch(real_ip);
		//	m_lay3switch.push_back(d);
		}
		break;
	case topo_device::type_server:
		{
			d = add_server(ip);	
		}
		break;
	case topo_device::type_unknown:
		duker_log("%s unkown device type", ip.c_str());
		break;
	default:
		break;
	}

	if(d != NULL)
	{
		//add new device network global variables	
		duker_log("find a new device ip %s, hostname %s, mac %s, type %s, stauts %s", d->get_hostip().c_str(), d->get_hostname().c_str(), 
		d->get_mac().c_str(), d->get_device_type_string().c_str(), d->get_status_string().c_str());
	}

	return d;
}
/*******************************************************
*Description:
*		add switch to current network
*		if not exist before then added, the parent is default corerouter, it will be updated in next call  network.update function
*		else do nothing
*********************************************************/
topo_device* topo_network::add_switch(const string &ip)/*add switch to current network*/
{
	string mac = ""; 
	topo_device *d = NULL;

	if(find_ip2mac(ip, mac))
	{
		topo_switch_t::iterator iter = m_switches.find(mac);
		if(iter == m_switches.end())
		{
			topo_switch *s = new topo_switch(ip);
			s->set_mac(mac);
			/*set switch parent, defalut is core router*/
			s->set_parent(m_routers.begin()->second);	
			d = s;

			m_switches.insert(pair<string,topo_device*>(mac, s));
			m_devices.insert(pair<string, topo_device*>(mac, s));
		}
		else
		{
			duker_log("switch ip = %s has exist", ip.c_str());
		}
	}
	return d;
}
/*same to add_switch except that corerouter's parent is NULL*/
topo_device* topo_network::add_router(const string&ip)/*add router to current network*/
{
	topo_device * d= NULL;
	if(m_mac2ip.size() == 0)/*set core router ,one network only one core router*/
	{
		topo_router *r = new topo_router(ip);
		r->set_parent(NULL);

		/*update m_mac2ip and m_ip2mac*/
		update_mac2ip(r);	
		
		/*find mac of router*/
		if(find_ip2mac(ip, r->get_mac()))
		{
			m_routers.insert(pair<string,topo_device*>(r->get_mac(), r));

			m_devices.insert(pair<string,topo_device*>(r->get_mac(), r));	
		}
		d = (topo_device*)r;	
	}
	else
	{
		string mac = "";
		if(find_ip2mac(ip, mac))
		{
			topo_router_t::iterator iter = m_routers.find(mac);
			if(iter == m_routers.end())
			{
				topo_router *r = new topo_router(ip);
				r->set_mac(mac);
				r->set_parent(m_routers.begin()->second);

				d = (topo_device*)r;	

				m_routers.insert(pair<string,topo_device*>(mac, r));
				m_devices.insert(pair<string, topo_device*>(mac, r));
			}
			else
			{
				duker_log("router ip = %s has exist", ip.c_str());
			}
		}
	}

	return d;
}
/*************************************************************
*Description:
*	same to add_switch	
**************************************************************/
topo_device* topo_network::add_server(const string&ip)/*add server to current network*/
{
	string mac = "";	
	topo_device *d = NULL;

	if(find_ip2mac(ip, mac))
	{
		topo_device_t::iterator iter = m_devices.find(mac);
		
		if(iter == m_devices.end())
		{	
			string hostname, key;

			topo_device::set_hostname(ip, hostname);

			if(hostname.size() != 0)
			{
				key = hostname.substr(0,hostname.size()-2);  
			}
			else
			{
				key = mac;
			}

			topo_unit_t &unit = m_servers[key];		

			d = new topo_device(hostname, ip, topo_device::type_server, mac, topo_device::status_new);

			d->set_parent(m_routers.begin()->second);
			unit.machinename = d->get_hostname();
			unit.ms.push_back(d);	
			
			m_devices.insert(pair<string, topo_device*>(mac, d));	
		}
		else
		{
			duker_log("server ip %s exist", ip.c_str());
		}
	}

	return d;
}

void topo_network::print_connection()/*print out connection of current network*/
{
	duker_log("=============switch connection================");
	/*default is one switch*/
	topo_switch_t::iterator iter = m_switches.begin();
	while(iter != m_switches.end())
	{
		topo_switch *d = (topo_switch*)iter->second;
		d->print_connection();
		++iter;
	}
}

string topo_network::save_connection()/*save connection*/
{
	/*default is one switch*/
	string conn = "";
	topo_device_t::iterator iter = m_devices.begin();
	while(iter != m_devices.end())
	{
		topo_device *d = iter->second;
		topo_device *p = d->get_parent();
		string type,parent;

		switch(d->get_device_type())
		{
		case topo_device::type_server:
			{
				type = "linux";
				break;
			}
		case topo_device::type_router:
			{
				type = "router";
				if(d == m_routers.begin()->second) 
				{
					parent = ""; 
				}
				break;
			}
		case topo_device::type_switch:
			{
				type = "switch";
				break;
			}
		default:
			break;
		}

		parent = p != NULL ? d->get_parent()->get_hostip() : "";	

		conn += type; 
		conn += "\t";
		conn += d->get_hostip();
		conn += "\t";
		conn += parent+"\n";

		++iter;
	}
	return conn;
}
void topo_network::ping_iprange(unsigned int first, unsigned int last)
{
	char buf[16];
	unsigned char j[4];
	topo_ping::topo_ping_t pings;

	for(unsigned int i=first; i <= last; ++i)
	{
		memcpy(j, &i, 4);	
		sprintf(buf, "%u.%u.%u.%u", j[3], j[2], j[1], j[0]);	
			
		topo_ping::ping_unit_t *punit = new topo_ping::ping_unit_t(); 
		punit->ip = buf;
		punit->flag = -1;

		pings.push_back(punit);
	}

	//ping
	topo_ping::ping(pings);

	for(size_t i=0; i < pings.size(); ++i)
	{
		topo_ping::ping_unit_t *punit = pings[i];

		if(punit->flag == 1)
		{
			add_device(pings[i]->ip);
		}

		if(punit != NULL)
		{
			delete punit;
		}
	}	
	
}

void topo_network::print_heartbeat()/*print floatip to devices*/
{
	char buf[MAX_BUF_LEN];
	duker_log("===================heart beat=======================");
	topo_heartbeat_t::iterator iter = m_heartbeats.begin();
	while(iter != m_heartbeats.end())
	{
		string floatip = iter->first;
		topo_master_slave_t ms = iter->second;
		int len = sprintf(buf, "float ip %15s:[", floatip.c_str());
		
		for(size_t i=0; i < ms.size(); ++i)
		{
			len += sprintf(buf + len ,"%15s, ", ms[i]->get_hostip().c_str());	
		}
		sprintf(buf + len, "]");
		duker_log(">>%s", buf);
		++iter;
	}
}
