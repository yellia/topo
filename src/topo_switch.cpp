#include "topo_switch.h"

#define	FDBOID ".1.3.6.1.2.1.17.4.3"

topo_switch::topo_switch(){}

topo_switch::topo_switch(const string &hostip)
	:topo_device(hostip,type_switch)
{
	set_fdb_table();
}

topo_switch::topo_switch(const string &hostname, const string &hostip, const string &mac, const topo_device::device_status &status, const snmp_table &fdbtable)
	:topo_device(hostname, hostip, type_switch, mac, status),
	 m_fdbtable(fdbtable)	
{
}

topo_switch::topo_switch(const topo_switch &s)
	:topo_device(s.get_hostname(), s.get_hostip(), s.get_device_type(), s.get_mac(), s.get_device_status()),
	 m_fdbtable(s.m_fdbtable)
{
}

topo_switch& topo_switch::operator=(const topo_switch &s)
{
	if(this != &s)
	{
		this->~topo_switch();
		
		set_hostname(s.get_hostname());	
		set_hostip(s.get_hostip());
		m_fdbtable = s.m_fdbtable;
	}

	return *this;
}

/**************************************************************************
*Description:
*		get fdb table of switch	
*Parameters:	 
*		topo_switch - switch	
*return value:
*		true success,false failed	
***************************************************************************/
bool topo_switch::set_fdb_table()
{
	Pdu pdu;
	Vb vb(Oid(FDBOID));
	pdu += vb;
	int status;
	
	if((SNMP_CLASS_SUCCESS == (status = get_sm().get_table(pdu, this->get_hostip().c_str(), m_fdbtable))) && m_fdbtable.size() > 0)
	{
		snmp_column &macs = m_fdbtable[0];	
		for(size_t j=0; j< macs.size(); ++j)
		{
			snmp_cell &s_mac = macs[j];
			s_mac.value = mac_trim(s_mac.value);
		} 
		return true;
	}

	duker_log("get switch %s fdbtable falied,error %s", this->get_hostip().c_str(), get_sm().error_msg(status));
	return false;
}
/**************************************************************************
*Description:
*		print out fdb table of switch	
***************************************************************************/
void topo_switch::print_fdb_table()
{
	if(m_fdbtable.size() == 0 && !set_fdb_table())
	{
		duker_log("get switch %s fdbtable failed", get_hostip().c_str());	
		return;
	}

	duker_log(">>switch %s fdbtable size=%d items<<", get_hostip().c_str(), m_fdbtable[0].size());
	duker_log("|%17s|%4s|%6s","mac","port","status");
	
	snmp_column &macs = m_fdbtable[0];	
	snmp_column &ports = m_fdbtable[1];	
	snmp_column &status = m_fdbtable[2];	

	for(size_t j=0; j< macs.size(); ++j)
	{
		snmp_cell &s_mac = macs[j];
		snmp_cell &s_port = ports[j];
		snmp_cell &s_status = status[j];

		duker_log("|%17s|%4s|%6s", s_mac.value.c_str(), s_port.value.c_str(), s_status.value.c_str()); 		
		duker_log("-------------------------------");
	} 
}
/**************************************************************************
*Description:
*		get fdb table of switch	
***************************************************************************/
void topo_switch::get_fdb_table(vector<string> &fdbtable)
{
	if(m_fdbtable.size() == 0)
	{
		set_fdb_table();
	}

	fdbtable.clear();	
	snmp_column &macs = m_fdbtable[0];	
	for(size_t j=0; j< macs.size(); ++j)
	{
		snmp_cell &s_mac = macs[j];
		fdbtable.push_back(s_mac.value);
	}
}

/**************************************************************************
*Description:
*		check device is in current fdbtable  	
*Parameters:
*		mac - macaddress
*		ip - ipaddress not float ip
*return value:
*		1 not exist, 2  exist 
***************************************************************************/
int topo_switch::check_update(topo_device *d)
{
	int status = 1;
	
	if(this == d)
	{
		duker_log("switch is self");
	}
	else
	{
		switch_conn_t::iterator value = m_conn.find(d->get_mac());   	

		if(value != m_conn.end())
		{
			//arleady exist, update ipaddress
			status = 2;
			duker_log("update device type = %s mac = %s ipaddress %s to %s'connection success", d->get_device_type_string().c_str(), d->get_mac().c_str(), d->get_hostip().c_str(), this->get_hostip().c_str());
		}
		else
		{
			m_conn[d->get_mac()] = d;	
			d->set_parent(this);
			duker_log("add device type = %s mac = %s ipaddress %s to %s'connection success", d->get_device_type_string().c_str(), d->get_mac().c_str(), d->get_hostip().c_str(), this->get_hostip().c_str());
		}
	}
	return status;
}
/**************************************************************************
*Description:
*		  print out connection of switch	
***************************************************************************/
void  topo_switch::print_connection()
{
	switch_conn_t::iterator iter = m_conn.begin();
	duker_log(">>switch %s connection table", this->get_hostip().c_str());
	while(iter != m_conn.end())
	{
		topo_device *d = iter->second; 	
		d->print_device();
		++iter;
	}	
}

/**************************************************************************
*Description:
*		  get connection of switch	
***************************************************************************/
void  topo_switch::get_connection(string &conn)
{
	switch_conn_t::iterator iter = m_conn.begin();
	while(iter != m_conn.end())
	{
		topo_device *d = iter->second; 	
		string type = "linux";
		string parent = "";
		
		switch(d->get_device_type())
		{
		case topo_device::type_router:
			{
				type = "router";
				break;
			}
		case topo_device::type_switch:
		case topo_device::type_unknown:
		case topo_device::type_lay3switch:
			{
				type = "switch";	
				parent = this->get_hostip();
				break;
			}
		case topo_device::type_server:
			{
				type = "linux";
				parent = this->get_hostip();
				break;
			}	
		}

		conn += type; 
		conn += "\t";
		conn += d->get_hostip();
		conn += "\t";
		conn += parent+"\n";
		++iter;
	}
}

