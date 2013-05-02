/* A. Lie: This file is functionally unchanged compared to the addons of smallko for 
   Evalvid-ns-2 interface. For clarity, myUdp* is changed to eraUdp*  */
#ifndef ns_eraudp_h
#define ns_eraudp_h

#include "udp.h"

class eraUdpAgent : public UdpAgent {
public:
	eraUdpAgent();
	int Qrecv;
	virtual void sendmsg(int nbytes, AppData* data, const char *flags = 0);
	//virtual void sendmsg(int nbytes, AppData* data, const char *flags = 0, int source = 0);
	virtual int command(int argc, const char*const* argv);
protected:
	// originally added by smallko
	int id_;	
	char BWfile[100];
	FILE *BWFile;
	int openfile;
};

#endif
