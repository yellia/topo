#include "snmp_manager.h"

snmp_version Snmp_manager::m_version = version1;
string 	Snmp_manager::m_community = "public";
unsigned long Snmp_manager::m_retries = 1;
u_short Snmp_manager::m_port = 161;
int Snmp_manager::m_timeout = 100;/*msecond*/

Snmp_manager::Snmp_manager()
{
	DefaultLog::log()->set_filter(DEBUG_LOG, 3);
	m_snmp = new Snmp(m_status);
	assert(NULL != m_snmp || SNMP_CLASS_SUCCESS != m_status);
}

Snmp_manager::Snmp_manager(const snmp_version &version, const string &community, 
const u_short &port, const unsigned long &retries, const int &timeout)
{
	set_version(version);	
	set_community(community);
	set_retries(retries);
	set_timeout(timeout);
	set_port(port);

	m_snmp = new Snmp(m_status);
	assert(NULL != m_snmp || SNMP_CLASS_SUCCESS != m_status);
}
Snmp_manager::Snmp_manager(const Snmp_manager &snmp_manager)
{
	m_snmp = new Snmp(m_status);
	assert(NULL != m_snmp || SNMP_CLASS_SUCCESS != m_status);
}
Snmp_manager& Snmp_manager::operator=(const Snmp_manager &snmp_manager) 
{
	if(this != &snmp_manager)
	{
		this->~Snmp_manager();

		set_version(snmp_manager.m_version);
		set_community(snmp_manager.m_community);
		set_port(snmp_manager.m_port);
		set_retries(snmp_manager.m_retries);
		set_timeout(snmp_manager.m_timeout);	

		m_snmp = new Snmp(m_status);
		assert(NULL != m_snmp || SNMP_CLASS_SUCCESS != m_status);
	}

	return *this;
}
Snmp_manager::~Snmp_manager()
{
	if(NULL != m_snmp)
	{
		delete m_snmp;
		m_snmp = NULL;
	}
}

void Snmp_manager::set_version(const snmp_version &version){this->m_version = version;}
void Snmp_manager::set_community(const string &community){this->m_community = community;}
void Snmp_manager::set_retries(const unsigned long &retries){this->m_retries = retries;}
void Snmp_manager::set_timeout(const int &timeout){this->m_timeout = timeout;}
void Snmp_manager::set_port(const u_short &port){this->m_port = port;}

snmp_version Snmp_manager::get_version() const{return this->m_version;}
string& Snmp_manager::get_community() const{return this->m_community;}
unsigned long Snmp_manager::get_retries() const{return this->m_retries;}
int Snmp_manager::get_timeout() const{return this->m_timeout;}
u_short Snmp_manager::get_port() const{return this->m_port;}

void Snmp_manager::set_snmp_column(const Pdu &pdu, snmp_column &column)
{
	Vb vb;
	for(int i=0; i < pdu.get_vb_count(); ++i)
	{
		pdu.get_vb(vb, i);	

		set_snmp_column(vb, column);
	}
}	
void Snmp_manager::set_snmp_column(const Vb &vb, snmp_column &column)
{
	snmp_cell cell;

	//duker_log("oid:%s value:%s", vb.get_printable_oid(), vb.get_printable_value());
	cell.oid = vb.get_printable_oid();
	cell.value = vb.get_printable_value();	

	column.push_back(cell);
}		
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
int  Snmp_manager::get(Pdu &pdu, const SnmpTarget &target, snmp_column &value)
{
	m_status = m_snmp->get(pdu, target);
	if(SNMP_CLASS_SUCCESS == m_status)
	{
		set_snmp_column(pdu, value);
	}

	return m_status;

}
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
int  Snmp_manager::get(Pdu &pdu, const char *ipaddress, snmp_column &value)
{
	UdpAddress address(ipaddress);
	address.set_port( m_port);

	CTarget ctarget( address);             // make a target using the address
	ctarget.set_version( m_version);         // set the SNMP version SNMPV1 or V2
	ctarget.set_retry( m_retries);           // set the number of auto retries
	ctarget.set_timeout( m_timeout);         // set timeout
	ctarget.set_readcommunity( m_community.c_str()); // set the read community name

	return get(pdu,ctarget, value);

}
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
int  Snmp_manager::get_next(Pdu &pdu, const SnmpTarget &target, snmp_column &value)
{
	m_status = m_snmp->get_next(pdu, target);
	if(m_status == SNMP_CLASS_SUCCESS)
	{
		set_snmp_column(pdu, value);
	}

	return m_status;

}
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
int  Snmp_manager::get_next(Pdu &pdu, const char *ipaddress, snmp_column &value)
{
	UdpAddress address(ipaddress);
	address.set_port( m_port);

	CTarget ctarget( address);             // make a target using the address
	ctarget.set_version( m_version);         // set the SNMP version SNMPV1 or V2
	ctarget.set_retry( m_retries);           // set the number of auto retries
	ctarget.set_timeout( m_timeout);         // set timeout
	ctarget.set_readcommunity( m_community.c_str()); // set the read community name
	
	return get_next(pdu, ctarget, value);

}
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
int  Snmp_manager::get_table(Pdu &pdu, const SnmpTarget &target, const int &non_repeaters, const int &max_reps, snmp_table &value)
{
	bool flag=true;
	int rows,column;
	rows=column=0;	

	Vb vb;
	Oid oid,tmp,rowoid;

	if(0 == pdu.get_vb_count())
	{
		return  SNMP_CLASS_INVALID_PDU;	
	}

	pdu.get_vb(vb,0);
	vb.get_oid(oid);
	rowoid = oid + ".1.1";

	/*hold snmp-bulk request value*/
	snmp_column s_column;	

	/*clear return value*/
	value.clear();	

	while(flag && (m_status = m_snmp->get_bulk(pdu, target, non_repeaters, max_reps)) == SNMP_CLASS_SUCCESS)
	{
		for(int i=0;i<pdu.get_vb_count();++i)
		{
			pdu.get_vb(vb,i);	
			vb.get_oid(tmp);

			if(oid.nCompare(oid.len(), tmp) != 0)
			{
				flag = false;
				break;
			}
			/*cal rows*/
			if(rowoid.nCompare(rowoid.len(), tmp) == 0)
			{
				++rows;	
			}
			
			set_snmp_column(vb, s_column);
		}
		pdu.set_vblist(&vb,1);
	}

	if(m_status != SNMP_CLASS_SUCCESS)
	{
		duker_log("snmp error: %s", error_msg(m_status));
		return m_status;
	}

	if(s_column.size() == 0 || s_column.size() % rows != 0)
	{
		duker_log("get table failed, reasion : oid not a table oid");
		return SNMP_CLASS_INVALID_PDU;
	}
	else
	{
		column = s_column.size() / rows;

		/*cal the n,m of the table from the v */
//		duker_log("%s table has %d items(%d*%d) ", oid.get_printable(), s_column.size(), rows, column);	
	
		/* hold one row*/
		snmp_column s_row;

		for( int i=0; i < column; ++i)
		{
			s_row.clear();
			for( int j=0; j < rows; ++j)
			{
				snmp_cell cell = s_column[i*rows+j];	
				s_row.push_back(cell);	
			}	
			value.push_back(s_row);
		}	
	}	
	
	
	return m_status;

}
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
int  Snmp_manager::get_table(Pdu &pdu, const char *ipaddress, snmp_table &value)
{
	UdpAddress address(ipaddress);
	address.set_port( m_port);

	CTarget ctarget( address);             // make a target using the address
	ctarget.set_version( version2c);         // set the SNMP version SNMPV1 or V2
	ctarget.set_retry( m_retries);           // set the number of auto retries
	ctarget.set_timeout( m_timeout);         // set timeout
	ctarget.set_readcommunity( m_community.c_str()); // set the read community name

	return get_table(pdu, ctarget, 0, 10, value);

}
/*************************************************************************
*Description:
*		print human readable error mes
*Parameters:
*		status - return value
**************************************************************************/
const char* Snmp_manager::error_msg(const int &status)
{
	return m_snmp->error_msg(status);
}
/*************************************************************************
*Description:
*		get interface table
*Paramenters:
*	 	ipaddress - ip
*		value - return value  
*return value:
*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
**************************************************************************/
int Snmp_manager::get_interface_table(const char *ip, snmp_table &value)
{
	return SNMP_CLASS_SUCCESS;
}
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
int Snmp_manager::get_physical_memory(const char *ip, int &memtotal, float &usedpercent)
{
	Pdu pdu;
	Vb vb;	
	vb.set_oid(PHYSICAL_MEMORY);
	pdu +=vb;
	
	snmp_table value;	
	int status;

	if((status = get_table(pdu, ip, value)) != SNMP_CLASS_SUCCESS)
	{
		duker_log(error_msg(status));
		return status;
	}
	
	snmp_column storage_size,storage_used;	
	
	storage_size = (snmp_column)value[4];
	memtotal = atoi(((snmp_cell)storage_size[0]).value.c_str());

	storage_used = (snmp_column)value[5];
	usedpercent = atof(((snmp_cell)storage_used[0]).value.c_str())*100 / memtotal;
		
	return status;

}
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
int Snmp_manager::get_cpu(const char *ip, char &cpu , float &usedpercent)
{
	Pdu pdu;
	Vb vb;	
	vb.set_oid(CPU_USAGE);
	pdu +=vb;
	
	snmp_column value;	
	int status;

	if((status = get_next(pdu, ip, value)) != SNMP_CLASS_SUCCESS)
	{
		duker_log(error_msg(status));
		return status;
	}
	
	duker_log("get cpu usage of load 5 minute success .result is %s%",((snmp_cell)value[0]).value.c_str());

	cpu = 4;
	usedpercent = atof(((snmp_cell)value[0]).value.c_str())*100;	

	return status;

}
/*************************************************************************
*Description:
*		get connections 
*Paramenters:
*	 	ipaddress - ip
*		int conn - connection count	
*return value:
*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
**************************************************************************/
int Snmp_manager::get_conn(const char *ip, int &conn)
{
	Pdu pdu;
	Vb vb;	
	vb.set_oid(TCP_CONNECTION);
	pdu +=vb;
	
	snmp_table value;	
	int status;

	if((status = get_table(pdu, ip, value)) != SNMP_CLASS_SUCCESS)
	{
		duker_log(error_msg(status));
		return status;
	}

	conn = ((snmp_column)value[0]).size();

	return status;

}
/*************************************************************************
*Description:
*		get nbd is alive 
*Paramenters:
*	 	ipaddress - ip
*		int - 1 alive or -1 died
*return value:
*		SNMP_CLASS_SUCCESS or a negative code ,you can get a  human readable error string by call error_msg(status)
**************************************************************************/
int Snmp_manager::get_nbd_isalive(const char *ip, int &is_alive)
{
	//first, get hrSWRun table to get all processes in remote sever 
	snmp_table ret;
	Pdu pdu; 
	Vb vb;
	int status = 0;
	
	//default nbd is down
	is_alive = -1;
	
	vb.set_oid(HRSWRun);
	pdu += vb;
	
	if((status = get_table(pdu, ip, ret)) == SNMP_CLASS_SUCCESS)
	{
		if(ret.size() == 0)
		{
			is_alive = -1;

			return status;
		}

		snmp_column swrun_path = ret[3];

		for(size_t j=0; j < swrun_path.size(); ++j)
		{
			snmp_cell cell = swrun_path[j];	
			int pos = cell.value.find("nbd");
				
			if(pos != -1)
			{
				is_alive = 1;	
			}
		}
	}

	return status;

}
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
int Snmp_manager::register_trap(const int trap_port, snmp_callback callback, const void *callback_data)
{
	OidCollection oidc;
	TargetCollection targetc;

	duker_log("Trying to register for traps on port %d .", trap_port);

	m_snmp->notify_set_listen_port(trap_port);
	m_status = m_snmp->notify_register(oidc, targetc, callback, NULL);

	if(m_status == SNMP_CLASS_SUCCESS)
	{
		duker_log("register traps on port %d SUCCESS.", trap_port);		
	}	
	else
	{
		duker_log("register failed . reason : %s .", error_msg(m_status));	
	}

	return m_status;

}
/*************************************************************************
*Description:
*		start recieve traps from clients 
*return:
*		bool -true success,false failed	
**************************************************************************/
bool Snmp_manager::start_rtrap(int timeout)
{
	bool status;
	status = m_snmp->start_poll_thread(timeout);
	
	if(status)
	{
		duker_log("start trap SUCCESS.  Waiting for traps/informs...");
	}
	else
	{
		duker_log("start recieve_trap failed .");
	}
	
	return status;	

}
/*************************************************************************
*Description:
*		stop recieve traps from clients 
**************************************************************************/
void Snmp_manager::stop_rtrap()
{
	m_snmp->stop_poll_thread();
	duker_log("stop recieve trap .");
}
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
int Snmp_manager::send_trap(const char *ip, const int trap_port, Pdu &pdu)
{
	UdpAddress address(ip);
	address.set_port( trap_port);

	CTarget ctarget( address);             // make a target using the address
	ctarget.set_version( version1);         // set the SNMP version SNMPV1 or V2
	ctarget.set_readcommunity( m_community.c_str()); // set the read community name

	duker_log("begin trap to %s:%d useing SNMP %d ...", ip, trap_port, version1 + 1);

	SnmpTarget *snmpTarget;
	snmpTarget = &ctarget;	

	m_status = m_snmp->trap(pdu, *snmpTarget);
	
	if(m_status  == SNMP_CLASS_SUCCESS)
	{
		duker_log("send trap to %s:%d useing SNMP %d SUCCESS .", ip, trap_port, version1 + 1);
	}
	else
	{
		duker_log("send trap failed . reason : %s .", error_msg(m_status)); 
	}

	return m_status;

}

