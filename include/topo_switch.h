#ifndef TOPO_SWITCH_H
#define TOPO_SWITCH_H

#include "snmp_manager.h"
#include "topo_device.h"

class topo_switch:public topo_device
{
private:
	typedef map<string,topo_device*> switch_conn_t;/*device connected to switch <mac,deivce*>*/
private:
	snmp_table m_fdbtable;/*mac port status*/	
	switch_conn_t m_conn;/*device that is direct or redirect connected to current switch*/
public:
	topo_switch();
	topo_switch(const string &hostip);
	topo_switch(const string &hostname, const string &hostip, const string &mac, const topo_device::device_status &status, const snmp_table &fdbtable);
	topo_switch(const topo_switch &s);
	virtual ~topo_switch(){duker_log("topo_switch::~topo_switch()");}

	topo_switch& operator=(const topo_switch &s);

	/**************************************************************************
	*Description:
	*		get fdb table of switch	
	*return value:
	*		true success,false failed	
	***************************************************************************/
	bool set_fdb_table(); 	
	/**************************************************************************
	*Description:
	*		print out fdb table of switch	
	***************************************************************************/
	void print_fdb_table();
	/**************************************************************************
	*Description:
	*		get fdb table of switch	
	***************************************************************************/
	void get_fdb_table(vector<string> &fdbtable);
	/**************************************************************************
	*Description:
	*		update connection of switch  	
	*Parameters:
	*		d - topo_device
	*return value:
	*		1 not exist, 2  exist	
	***************************************************************************/
	int check_update(topo_device *d);
	/**************************************************************************
	*Description:
	*		  print out connection of switch	
	***************************************************************************/
	void  print_connection();
	/**************************************************************************
	*Description:
	*		  get connection of switch	
	***************************************************************************/
	void  get_connection(string &conn);
};

#endif
