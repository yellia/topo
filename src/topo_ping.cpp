#include "topo_ping.h"

#define servers_num 10/*servers in one thread to check*/ 


static topo_ping::topo_ping_t v_servers;

void *check_alive(void *n)
{
	size_t start,end,num;
	start=end=0;
	num = *(size_t*)n;

	end = num - 1;	

	if(num % servers_num != 0)
	{
		start = num / servers_num * servers_num; 
	}
	else
	{
		start = (num / servers_num - 1)* servers_num; 
	}

//	duker_log("start = %d end = %d", start, end);

	for(size_t i=start; i <= end; ++i)
	{
		topo_ping::ping_unit_t *u = v_servers[i];	

	//	duker_log("thread:%lu begin to ping %s ...", pthread_self(), u->ip.c_str());
		if(!topo_ping::ping(u->ip))
		{
			duker_log("ping %s is died...", u->ip.c_str());
			/*server is died*/	
			u->flag = -1;	
		}
		else
		{
			duker_log("ping %s is alive...",u->ip.c_str());
			u->flag = 1;
		}	
	}

	pthread_exit(NULL);
}
/**************************************************************************
*Description:
*		check a server is alive		
*Parameters:	 
*		ip - hostip	
*return value:
*		true success,false failed		
***************************************************************************/
bool topo_ping::ping(const string &ip)
{
	FILE   *rstream = NULL;
	char   buf[MAX_STRING_LEN];
	char   cmdline[255];
	bool flag = false;

	memset(buf, 0, sizeof(buf) );//初始化buf,以免后面写如乱码到文件中
	sprintf(cmdline,"ping -c 1 %s", ip.c_str());

	rstream = popen( cmdline, "r" ); //将“ls －l”命令的输出 通过管道读取（“r”参数）到FILE* stream
	if(rstream != NULL)
	{
		fread(buf, sizeof(char), sizeof(buf), rstream); //将刚刚FILE* stream的数据流读取到buf中

		pclose(rstream);

		string ret = buf;
		int pos = ret.find("100%");

		if(pos == -1)
		{
			flag =  true; 
		}
	}
	else
	{
		duker_log("open pipe failed");
	}

	return flag;
}
/**************************************************************************
*Description:
*		check servers are alive		
*Parameters:	 
*		servers - check servers	
***************************************************************************/
void topo_ping::ping(vector<topo_device*> &servers)
{
	topo_ping_t p_servers;
	convert_device2ping(servers, p_servers);	

	ping(p_servers);

	
	for(size_t i=0; i < servers.size(); ++i)
	{
		topo_device *d = servers[i];	
		ping_unit_t *punit = v_servers[i];
		if(punit->flag == 1)
		{
			d->set_device_status(topo_device::status_alive);
		}
		else
		{
			d->set_device_status(topo_device::status_died);
		}

		delete punit;
	}
}

void topo_ping::convert_device2ping(vector<topo_device*> &servers, topo_ping_t &p_servers)
{
	vector<topo_device*>::iterator iter = servers.begin();

	while(iter != servers.end())
	{
		topo_device *d = *iter;	
		ping_unit_t *punit = new ping_unit_t(); 
		punit->ip = d->get_hostip();
		punit->flag = -1;

		p_servers.push_back(punit);	

		++iter;
	}
}

/**************************************************************************
*Description:
*		get ip range		
*Parameters:	 
*		first - first ip address in network
*		last - last ip address in network
*		broadcast - broadcast ip address in network
*		network - network ip address in network
***************************************************************************/
void topo_ping::get_iprange(const string &iprange, unsigned int &first, unsigned int &last)
{
	unsigned int j[5],mask,broadcast,network;	

	first = last = broadcast = network = 0;
	string::size_type pos_slash = iprange.find("/");
	string::size_type pos_dash = iprange.find("-");

	/*if ip is 192.168.1.1/24*/
	if(string::npos != pos_slash)
	{
		string s = iprange.substr(pos_slash + 1, iprange.size() - pos_slash + 1);	
		string ip = iprange.substr(0, pos_slash); 
		j[4] = atoi(s.c_str());

		mask = j[4] == 32 ? 0xffffffff : ~(0xffffffff >> j[4]);	

		if(4 == sscanf(ip.c_str(), "%u.%u.%u.%u", &j[0], &j[1], &j[2], &j[3]))
		{
			first = (j[0] << 24) + (j[1] << 16) + (j[2] << 8) + j[3];	

			network = first & mask; 
			first = network + 1;
			broadcast = network + ~mask; 
			last = broadcast - 1;
		}
	}		
	/*if ip is 192.168.1.1-255*/
	else if(string::npos != pos_dash)
	{
		string s = iprange.substr(pos_dash +1 , iprange.size() - pos_dash + 1);
		string ip = iprange.substr(0, pos_dash);	
		j[4] = atoi(s.c_str());
		
		if(4 == sscanf(ip.c_str(), "%u.%u.%u.%u", &j[0], &j[1], &j[2], &j[3]))
		{
			first = (j[0] << 24) + (j[1] << 16) + (j[2] << 8) + j[3];	
			last = (j[0] << 24) + (j[1] << 16) + (j[2] << 8) + j[4];	
		}
	}
}

/**************************************************************************
*Description:
*		check iprange are alive		
*Parameters:	 
*		servers - check servers	
***************************************************************************/
void topo_ping::ping(topo_ping_t &servers)
{
	v_servers.clear();

	topo_ping_t::iterator iter = servers.begin();
	while(iter != servers.end())
	{
		v_servers.push_back(*iter);
		++iter;
	}	

	/*use multi-thread to check*/
	int thread_num = v_servers.size() / servers_num;
	int left_servers = v_servers.size() % servers_num;


//	duker_log("v_servers.size()=%d, servers_num=%d,thread_num=%d", v_servers.size(), servers_num, thread_num);
	if(left_servers != 0)
	{
		thread_num++;	
	}

	duker_log("now begin to check servers is alive in multi-thread %d...", thread_num);

	if(thread_num > 0)
	{
		size_t *pnum = (size_t*)malloc(sizeof(size_t) * thread_num); 

		pthread_t *pthread = (pthread_t*)malloc(sizeof(pthread_t) * thread_num);	
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
			
		for(int i = 0; i < thread_num; ++i)
		{
			*(pnum + i) = (i + 1) * servers_num;	
			if(*(pnum + i) > v_servers.size())
			{
				*(pnum + i) = v_servers.size();
			} 

			int rc = pthread_create(pthread+i, &attr, check_alive, (void*)(pnum+i)); 	
			if(rc != 0)
			{
				duker_log("create ping thread failed error code %d", rc);  
			}			
		}
		pthread_attr_destroy(&attr);
		/*wait for all thread return*/
		for(int i=0; i < thread_num; ++i)
		{
			int rc = pthread_join(*(pthread+i), NULL);
			if(rc != 0)
			{
				duker_log("error: return code from pthread_join() %d", rc);
			}
		}
		duker_log("check server alive finish");	

		free(pthread);
		free(pnum);
	}
}

//int main(int argc, char *argv[])
//{
//	unsigned int first,last;
//
//	topo_ping::get_iprange("192.168.3.2/24", first, last);
//
//
//	duker_log("%d", last - first);
//
//	unsigned char *p = (unsigned char*)&last;
//
//	cout << (int)*p++ << endl;
//	cout << (int)*p++ << endl;
//	cout << (int)*p++ << endl;
//	cout << (int)*p << endl;
//
//	return 0;
//}
