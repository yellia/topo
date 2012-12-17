#include "topo_device.h"

#define HOSTNAMEOID ".1.3.6.1.2.1.1.5.0" 

Snmp_manager topo_device::sm;

topo_device::topo_device()
	:m_hostname("localhost")
	,m_hostip("127.0.0.1")
	,m_mac("00000000000000000")
	,m_type(type_server)
	,m_status(status_new)
	,m_parent(NULL)
{}	

topo_device::topo_device(const string &hostip)
	:m_hostip(hostip)
	,m_mac("00000000000000000")
	,m_type(type_server)
	,m_status(status_new)
	,m_parent(NULL)
{
	set_hostname(m_hostip, m_hostname);	
}	

topo_device::topo_device(const string &hostip, device_type type)
	:m_hostip(hostip)
	,m_mac("0000000000000000")
	,m_type(type)
	,m_status(status_new)
	,m_parent(NULL)
{
	set_hostname(m_hostip, m_hostname);
}

topo_device::topo_device(const string &hostname, const string &hostip, const device_type type, const string &mac, const device_status status)
	:m_hostname(hostname)
	,m_hostip(hostip)
	,m_mac(mac)
	,m_type(type)
	,m_status(status)
	,m_parent(NULL)
{}	

topo_device::topo_device(const topo_device& device)
	:m_hostname(device.m_hostname)
	,m_hostip(device.m_hostip)
	,m_mac(device.m_mac)
	,m_type(device.m_type)
	,m_status(device.m_status)
	,m_parent(device.m_parent)
{}

topo_device& topo_device::operator=(const topo_device &device)
{
	if(this != &device)
	{
		set_hostname(device.m_hostname);
		set_hostip(device.m_hostip);
		set_mac(device.m_mac);
		set_device_type(device.m_type);
		set_device_status(device.m_status);
		set_parent(device.m_parent);
	}
	return *this;
}

/**************************************************************************
*Description:
*		set hostname by snmp	
*return value:
*		true success,false failed		
***************************************************************************/
bool topo_device::set_hostname(const string &hostip, string &hostname)
{
	Pdu pdu;
	Vb vb;	
	vb.set_oid(HOSTNAMEOID);
	pdu +=vb;
	
	snmp_column value;	
	int status;
	bool flag = true;

	assert(hostip.size() != 0);

//	duker_log("now begin to get device %s hostname", hostip.c_str());

	if(0 < hostip.size() && SNMP_CLASS_SUCCESS == (status = sm.get(pdu, hostip.c_str(), value)))
	{
		hostname = value[0].value;	
	}
	else
	{
		hostname = "unknown";
		duker_log("hostip %s snmp error: %s", hostip.c_str(), sm.error_msg(status));
	}

	return flag;
}
/**************************************************************************
*Description:
*		get device_status const this
*return value:
*		string - died,alive,new
***************************************************************************/
string topo_device::get_status_string()const 
{
	string ret;	

	switch(this->m_status)
	{
	case status_died:
		ret = "died";break;
	case status_alive:
		ret = "alive";break;
	case status_new:
		ret = "new";break;
	default:
		ret = "unknown";break;
	}
	
	return ret;
}

/**************************************************************************
*Description:
*		get snmp_manager	
*return value:
*		snmp_manager	
***************************************************************************/
Snmp_manager topo_device::get_sm()
{
	return sm;
}

/**************************************************************************
*Description:11
*		get device type	 const this
*return value:
*		string -unknown,server,switch,lay3switch,router
***************************************************************************/
string topo_device::get_device_type_string()const
{
	string ret;	
	switch(m_type)
	{
	case type_unknown:
		ret = "unkown";break;
	case type_server:
		ret = "server";break;
	case type_switch:
		ret = "switch";break;
	case type_lay3switch:
		ret = "lay3switch";break;
	case type_router:
		ret = "router";break;
	default:break;
	}	
	return ret;
}
/**************************************************************************
*Description:
*		get device_type using snmp	
*		default is type_server
*return value:
*		device_type - type_unknown,type_server,type_switch,type_lay3switch,type_router
***************************************************************************/
topo_device::device_type topo_device::check_device_type(const string &ip)
{
	device_type type = type_unknown;
	bool frouter = is_router(ip);
	bool fswitch = is_switch(ip);

	duker_log("begin to check device %s type", ip.c_str());

	if(frouter && fswitch)
	{
		type = type_lay3switch;	
	}		
	else if(frouter)
	{
		type = type_router;
	}
	else if(fswitch)
	{
		type = type_switch;
	}
	else
	{
		type = type_server;
	}

	return type;
}
/**************************************************************************
*Description:
*		check if router useing snmp	
*return value:
*		true is router,false not	
***************************************************************************/
bool topo_device::is_router(const string &ip)
{
	Pdu pdu;
	Vb vb;	
	vb.set_oid(HOSTNAMEOID);
	pdu +=vb;
	int status;
	bool flag = false;
	
	snmp_column value;	

	if(0 < ip.size() && SNMP_CLASS_SUCCESS == (status = sm.get(pdu, ip.c_str(), value)))
	{
		string ret = value[0].value;	
		
		int pos = ret.find("router");

		if(pos != -1)
		{
			flag = true;
		}	
	}
	else if(0 == ip.size())
	{
		duker_log("hostip is null");
	}
	else
	{
		duker_log("hostip %s snmp error: %s", ip.c_str(), sm.error_msg(status));
	}
	
	return flag;
}
/**************************************************************************
*Description:
*		check if switch useing snmp	
*return value:
*		true is switch,false not	
***************************************************************************/
bool topo_device::is_switch(const string &ip)
{
	Pdu pdu;
	Vb vb;	
	vb.set_oid(HOSTNAMEOID);
	pdu +=vb;
	int status;
	bool flag = false;
	
	snmp_column value;	

	if(0 < ip.size() && SNMP_CLASS_SUCCESS == (status = sm.get(pdu, ip.c_str(), value)))
	{
		string ret = value[0].value;	
		int pos = ret.find("switch");

		if(pos != -1)
		{
			flag = true;
		}	
		
	}
	else if(0 == ip.size())
	{
		duker_log("hostip is null");
	}
	else
	{
		duker_log("hostip %s snmp error: %s", ip.c_str(), sm.error_msg(status));
	}

	return flag;

}
/**************************************************************************
*Description:
*		check if support seing snmp	
*return value:
*		true if supprot snmp,false not	
***************************************************************************/
bool topo_device::is_snmp(const string &ip)
{
	Pdu pdu;
	Vb vb;	
	vb.set_oid(HOSTNAMEOID);
	pdu +=vb;
	int status;
	bool flag = false;
	
	snmp_column value;	

	if(0 < ip.size() && SNMP_CLASS_SUCCESS == (status = sm.get(pdu, ip.c_str(), value)))
	{
		string ret = value[0].value;	
		int pos = ret.find("server");
		int pos2 = ret.find("entry");

		if(pos != -1 || pos2 != -1)
		{
			flag = true;
		}	
	}
	else if(0 == ip.size())
	{
		duker_log("hostip is null");
	}
	else
	{
		duker_log("hostip %s snmp error: %s", ip.c_str(), sm.error_msg(status));
	}

	return flag;

}

/**************************************************************************
*Description:
*		print out device information  
***************************************************************************/
void topo_device::print_device()
{
	duker_log("topo_device hostname:%10s,hostip:%15s,mac:%17s,type:%6s,stauts:%6s", this->get_hostname().c_str(), this->get_hostip().c_str(), 
	this->get_mac().c_str(), this->get_device_type_string().c_str(), this->get_status_string().c_str());	
}

