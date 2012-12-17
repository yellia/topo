
#include "topo_network.h"
#include "file_manager.h"	
#include "topo_ping.h"
#include <sys/types.h>
#include <signal.h>

static string routerip = "192.168.1.1";
static string switchip = "192.168.5.100";
static string filepath = "/home/dev/topo_easy/v1/ip.txt";
static int timeval = 10; 
static topo_network net("192.168.1.1/24;192.168.3.1/24;192.168.5.1-55","/home/dev/topo_easy/v1/ip.txt","192.168.1.1");

void func(int signo)
{
	net.update();
	string conn = net.save_connection();

	if(file_save(filepath.c_str(), conn.c_str()) == 1)
	{
		duker_log("save file to %s success", filepath.c_str());
	}
	else
	{
		duker_log("save file to %s failed", filepath.c_str());
	}
}

int main(int argc, char *argv[])
{
	if(argc > 1 && argc != 5)
	{
		duker_log("USAGE:%s [OPTIONS]", argv[0]);
		duker_log("OPTIONS:");
		duker_log("	--routerip	ip address of router. default is 192.168.1.1");
		duker_log("	--switchip	ip address of switch. default is 192.168.5.100");
		duker_log("	--filepath	absolute path to save ip.txt");
		duker_log("	--timeval	timeval to discover again");
		
		exit(1);
	}
	else if(argc == 5)
	{
		routerip = argv[1];
		switchip = argv[2];	
		filepath = argv[3];
		timeval  = atoi(argv[4]);
	}

	duker_log("routerip = %s switchip = %s timeval = %d filepath = %s", routerip.c_str(), switchip.c_str(), timeval, filepath.c_str());

	struct sigaction act;
	union sigval tsval;

	act.sa_handler = func;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(50, &act, NULL);

	while(1)
	{
		sleep(timeval);
		sigqueue(getpid(), 50, tsval);
	}

	return 0;
}
