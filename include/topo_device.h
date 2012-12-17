#ifndef TOPO_DEVICE_H
#define TOPO_DEVICE_H

#include "topo_log.h"
#include "snmp_manager.h"

class topo_device
{
public:
	typedef enum {type_unknown,type_server,type_switch,type_lay3switch,type_router}device_type;
	typedef enum {status_died,status_alive,status_new}device_status;

	topo_device();
	topo_device(const string &hostip);
	topo_device(const string &hostname, const string &hostip, const device_type type, const string &mac, const device_status device_status);
	topo_device(const string &hostip, device_type type);
	virtual ~topo_device(){duker_log("topo_device::~topo_device()");}

	topo_device(const topo_device& device);
	topo_device& operator=(const topo_device &device);
	/**************************************************************************
	*Description:
	*		set hostname	
	*Parameters:	 
	*		hostname - hostname 
	*return value:
	*		true success,false failed		
	***************************************************************************/
	bool set_hostname(const string &hostname){this->m_hostname = hostname; return this->m_hostname.size() > 0 ? true : false; }	
	/**************************************************************************
	*Description:
	*		set hostname by snmp	
	*return value:
	*		true success,false failed		
	***************************************************************************/
	static bool set_hostname(const string &hostip, string &hostname);
	/**************************************************************************
	*Description:
	*		set hostip	
	*Parameters:	 
	*		hostip - hostip	
	*return value:
	*		true success,false failed	
	***************************************************************************/
	bool set_hostip(const string &hostip){this->m_hostip = hostip;return this->m_hostip.size() > 0 ? true : false;}
	/**************************************************************************
	*Description:
	*		set device type	
	*Parameters:	 
	*		type - device type	
	***************************************************************************/
	void set_device_type(device_type type){this->m_type = type;}
	/**************************************************************************
	*Description:
	*		set macadderss	
	*Parameters:	 
	*		mac address string	
	***************************************************************************/
	void set_mac(const string &mac){this->m_mac = mac;}
	/**************************************************************************
	*Description:
	*		set device is device_status or died
	*Parameters:	 
	*		device_status - status_died,stauts_alive,status_new	
	***************************************************************************/
	void set_device_status(const device_status status){this->m_status = status;}
	/**************************************************************************
	*Description:
	*		get device type	
	*return value:
	*		device type	
	***************************************************************************/
	device_type get_device_type(){return this->m_type;}
	/**************************************************************************
	*Description:
	*		get device type	 const this
	*return value:
	*		device type	
	***************************************************************************/
	device_type get_device_type()const{return this->m_type;}
	/**************************************************************************
	*Description:
	*		get device type	 const this
	*return value:
	*		string -unknown,server,switch,lay3switch,router
	***************************************************************************/
	string get_device_type_string()const;
	/**************************************************************************
	*Description:
	*		get hostname const this 
	*return value:
	*		hostname	
	***************************************************************************/
	const string &get_hostname()const{return this->m_hostname;}
	/**************************************************************************
	*Description:
	*		get hostname 
	*return value:
	*		hostname string	
	***************************************************************************/
	string &get_hostname(){return this->m_hostname;}
	/**************************************************************************
	*Description:
	*		get hostip const this
	*return value:
	*		hostname string	
	***************************************************************************/
	const string &get_hostip()const{return this->m_hostip;}
	/**************************************************************************
	*Description:
	*		get hostip  
	*return value:
	*		hostname string	
	***************************************************************************/
	string &get_hostip(){return this->m_hostip;}
	/**************************************************************************
	*Description:
	*		get macaddress const this  
	*return value:
	*		mac string	
	***************************************************************************/
	const string &get_mac()const{return this->m_mac;}
	/**************************************************************************
	*Description:
	*		get macaddress const this  
	*return value:
	*		mac string	
	***************************************************************************/
	string &get_mac(){return this->m_mac;}
	/**************************************************************************
	*Description:
	*		get device_status const this
	*return value:
	*		status_died,status_alive,status_new
	***************************************************************************/
	device_status get_device_status()const {return this->m_status;}
	/**************************************************************************
	*Description:
	*		get device_status const this
	*return value:
	*		string - died,alive,new
	***************************************************************************/
	string get_status_string()const ;

	/**************************************************************************
	*Description:
	*		get device_status 
	*return value:
	*		status_died,status_alive,status_new
	***************************************************************************/
	device_status get_device_status(){return this->m_status;}
	/**************************************************************************
	*Description:
	*		set device parent
	***************************************************************************/
	void  set_parent(topo_device *parent){this->m_parent = parent;}
	/**************************************************************************
	*Description:
	*		get device parent
	***************************************************************************/
	topo_device* get_parent(){return this->m_parent;}

	/**************************************************************************
	*Description:
	*		print out device information  
	***************************************************************************/
	virtual void print_device();
	/**************************************************************************
	*Description:
	*		get snmp_manager	
	*return value:
	*		static snmp_manager	
	***************************************************************************/
	static Snmp_manager get_sm();
	/**************************************************************************
	*Description:
	*		check if router useing snmp	
	*return value:
	*		true is router,false not	
	***************************************************************************/
	static bool is_router(const string &ip);
	/**************************************************************************
	*Description:
	*		check if switch useing snmp	
	*return value:
	*		true is switch,false not	
	***************************************************************************/
	static bool is_switch(const string &ip);
	/**************************************************************************
	*Description:
	*		check if support seing snmp	
	*return value:
	*		true if supprot snmp,false not	
	***************************************************************************/
	static bool is_snmp(const string &ip);
	/**************************************************************************
	*Description:
	*		get device_type using snmp	
	*return value:
	*		device_type - type_unknown,type_server,type_switch,type_lay3switch,type_router
	***************************************************************************/
	static topo_device::device_type check_device_type(const string &ip);
private:
	string m_hostname;/*hostname*/
	string m_hostip;/*hostip*/
	string m_mac;
	device_type m_type;
	device_status m_status;/*get device status*/
	topo_device *m_parent;
	
	static Snmp_manager sm;/*snmp*/
protected:
	//trim space of mac
	const string mac_trim(string &mac)
	{
		return mac.substr(2, 17);
	}
};
#endif
