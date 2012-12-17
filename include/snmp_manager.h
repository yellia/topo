/*********************************************************
*des:
	only support snmp v1,v2,read community default public
*author:   yellia
*datetime: 2012-03-19
*********************************************************/
#ifndef SNMP_MANAGER_H
#define SNMP_MANAGER_H

#include <snmp_pp/snmp_pp.h>
#include "topo_log.h"

#ifdef SNMP_PP_NAMESPACE
using namespace Snmp_pp;
#endif

#define PHYSICAL_MEMORY	".1.3.6.1.2.1.25.2.3"
#define CPU_USAGE	".1.3.6.1.4.1.2021.10.1.3.2"
#define TCP_CONNECTION	".1.3.6.1.2.1.6.13"
#define HRSWRun	".1.3.6.1.2.1.25.4.2"

/**
*a return value of  SNMP-GET request
*/
typedef struct
{
	string oid;
	string value;	
}snmp_cell;
/**
*a return value of SNMP-BULK request
**/
typedef vector<snmp_cell> snmp_column;
/**
*a return value of table  using SNMP-BULK request
**/
typedef vector<snmp_column> snmp_table;

class Snmp_manager
{
    private:
	static snmp_version m_version; /*version of snmp*/
	static string m_community; /*read community, default is public*/
	static unsigned long m_retries; /*static retries*/
	static int m_timeout; /*static timeout */
   	static u_short m_port; /*port to snmp 0-65535*/
	Snmp *m_snmp; /*snmp object*/
	int m_status; /*status to hold snmp++ return value,can call error_msg(status) get error msg*/
    private:
	void set_snmp_column(const Pdu &pdu, snmp_column &column);		
	void set_snmp_column(const Vb &vb, snmp_column &column);		
	
    public:
	void set_version(const snmp_version &version);
	void set_community(const string &community);
	void set_retries(const unsigned long &retries);
	void set_timeout(const int &timeout);
	void set_port(const u_short &port);

	snmp_version get_version() const;
	string& get_community() const;
	unsigned long get_retries() const;
	int get_timeout() const;
	u_short get_port() const;

	Snmp_manager();
	Snmp_manager(const snmp_version &version, const string &communtiy, 
	const u_short &port, const unsigned long &retries, const int &timeout);
	Snmp_manager(const Snmp_manager &snmp_manager);
	Snmp_manager& operator=(const Snmp_manager &snmp_manager); 
	~Snmp_manager();
	/**************************************************************************
	*Description:
	*		send a blocking SNMP-GET request
	*Parameters:	 
	*		pdu -pdu to send	
	*		tartet - Target for get	
	*		value - hold return value
	*return value:
	*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
	***************************************************************************/
	int  get(Pdu &pdu, const SnmpTarget &target, snmp_column &value);
	/**************************************************************************
	*Description:
	*		send a blocking SNMP-GET request
	*Parameters:	 
	*		pdu -pdu to send	
	*		ipaddress -ipadress format 
	*		value - hold return value
	*return value:
	*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
	***************************************************************************/
	int  get(Pdu &pdu, const char *ipaddress, snmp_column &value);
	/**************************************************************************
	*Description:
	*		send a blocking SNMP-GETNEXT request
	*Parameters:	 
	*		pdu -pdu to send	
	*		tartet - Target for get	
	*		value - hold return value
	*return value:
	*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
	***************************************************************************/
	int  get_next(Pdu &pdu, const SnmpTarget &target, snmp_column &value);
	/**************************************************************************
	*Description:
	*		send a blocking SNMP-GETNEXT request
	*Parameters:	 
	*		pdu -pdu to send	
	*		ipaddress -ipadress format 
	*		value - hold return value
	*return value:
	*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
	***************************************************************************/
	int  get_next(Pdu &pdu, const char *ipaddress, snmp_column &value);
	/**************************************************************************
	*Description:
	*		send a blocking SNMP-GETBULK request
	*Parameters:	 
	*		pdu -pdu to ,notice!! send only one vb in pdu support	
	*		tartet - Target for get	
	*		non_repeaters	-number of non repeaters ,usefull when pdu's vb is not one 
	*		max_reps	-maximum of number of repetitions,usefull to control the count of SNMP-GETNEXT request of oid
	*		value - hold return value
	*return value:
	*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
	***************************************************************************/
	int  get_table(Pdu &pdu, const SnmpTarget &target, const int &non_repeaters, const int &max_reps, snmp_table &value);
	/**************************************************************************
	*Description:
	*		send a blocking SNMP-GETBULK request
	*Parameters:	 
	*		pdu -pdu to send	
	*		ipadress -ipadress 
	*		non_repeaters	-number of non repeaters ,usefull when pdu's vb is not one 
	*		max_reps	-maximum of number of repetitions,usefull to control the count of SNMP-GETNEXT request of oid
	*		value - hold return value
	*return value:
	*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
	***************************************************************************/
	int  get_table(Pdu &pdu, const char *ipaddress, snmp_table &value);
	/*************************************************************************
	*Description:
	*		print human readable error mes
	*Parameters:
	*		status - return value
	**************************************************************************/
	const char* error_msg(const int &status);
	/*************************************************************************
	*Description:
	*		get interface table
	*Paramenters:
	*	 	ipaddress - ip
	*		value - return value  
	*return value:
	*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
	**************************************************************************/
	int get_interface_table(const char *ip, snmp_table &value);
	/*************************************************************************
	*Description:
	*		get physical memory ,include total,used
	*Paramenters:
	*	 	ipaddress - ip
	*		memtotal - total mem
	*		char percent  
	*return value:
	*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
	**************************************************************************/
	int get_physical_memory(const char *ip, int &memtotal, float &usedpercent);
	/*************************************************************************
	*Description:
	*		get cpu ,include total,useage(5 minute load)
	*Paramenters:
	*	 	ipaddress - ip
	*		cpu - total cpu count
	*		char percent  
	*return value:
	*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
	**************************************************************************/
	int get_cpu(const char *ip, char &cpu , float &usedpercent);
	/*************************************************************************
	*Description:
	*		get connections 
	*Paramenters:
	*	 	ipaddress - ip
	*		int conn - connection count	
	*return value:
	*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
	**************************************************************************/
	int get_conn(const char *ip, int &conn);
	/*************************************************************************
	*Description:
	*		get nbd is alive 
	*Paramenters:
	*	 	ipaddress - ip
	*		int - 1 alive or -1 died
	*return value:
	*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
	**************************************************************************/
	int get_nbd_isalive(const char *ip, int &is_alive);
	/*************************************************************************
	*Description:
	*		recieve traps from clients 
	*Paramenters:
	*	 	port - port for listen	
	*		callback -User callback function to use	
	*return:
	*		SNMP_CLASS_SUCCESS or SNMP_CLASS_TL_FAILED or SNMP_CLASS_TL_IN_USE
	*notice:
	*		one thread only call recieve_trap once 
	**************************************************************************/
	int register_trap(const int trap_port, snmp_callback callback, const void *callback_data = NULL);
	/*************************************************************************
	*Description:
	*		start recieve traps from clients 
	*return:
	*		bool -true success,false failed	
	**************************************************************************/
	bool start_rtrap(int timeout);
	/*************************************************************************
	*Description:
	*		stop recieve traps from clients 
	**************************************************************************/
	void stop_rtrap();
	/*************************************************************************
	*Description:
	*		send traps to server 
	*Paramenters:
	*	 	ip - remote ip for listen	
	*	 	port - remote port for listen	
	*	 	pdu - pdu to send	
	*return value:
	*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
	**************************************************************************/
	int send_trap(const char *ip, const int trap_port, Pdu &pdu);
};
#endif
