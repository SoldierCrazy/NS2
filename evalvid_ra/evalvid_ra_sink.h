/* A. Lie: This file is functionally unchanged compared to the addons of smallko for 
   Evalvid-ns-2 interface. For clarity, myUdp* is changed to eraUdp*  */
#ifndef eraudpsink_h
#define eraudpsink_h

#include <stdio.h>
#include "agent.h"

class eraUdpSink : public Agent
{
 public:
    eraUdpSink() : Agent(PT_UDP), pkt_received(0), id(0), expected_(0), opensinkfile(0) {} 		
	void recv(Packet*, Handler*);
	int command(int argc, const char*const* argv);
	void print_status();
 protected:
	char tbuf[100];
	FILE *tFile;
	unsigned long int  pkt_received;
	int seqno_;
	int expected_;
	int id;
	int opensinkfile;
};

#endif
