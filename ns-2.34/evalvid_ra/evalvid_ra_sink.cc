/***********************************************************
evalvid_ra_sink.cc, author (modifier): A. Lie, NTNU Norway
This file contains classes for enabling the ns-2 udp receiver to make
trace files storing:
time (packet tx), packet id, type (udp), size, 

The modification compared to smallko is only cosmetic (names)
plus one bug fix.

************************************************************/

#include <stdio.h> 
#include <stdlib.h>
#include "evalvid_ra_sink.h"
#include "ip.h"
#include "udp.h"
#include "rtp.h"

static class eraUdpSinkClass : public TclClass {
public:
        eraUdpSinkClass() : TclClass("Agent/eraUdpSink") {}
        TclObject* create(int, const char*const*) {
                return (new eraUdpSink);
        }
} class_eraudpsink;

void eraUdpSink::recv(Packet* pkt, Handler*)
{
    //hdr_ip* iph=hdr_ip::access(pkt);
    hdr_cmn* hdr=hdr_cmn::access(pkt);
    //hdr_rtp* rtp = hdr_rtp::access(pkt);
    
    //seqno_ = rtp->seqno();
    seqno_ = hdr->frame_pkt_id_;
    
    if (expected_ >= 0 && opensinkfile==1 ) {
	int loss = seqno_ - expected_;
	if (loss > 0) {
	    while (loss>0){
		fprintf(tFile,"%-16f id %-16d lost %-16d\n", Scheduler::instance().clock(), id++, 9999);	
		loss-=1;
	    }
	    
	} // else {
	if (id != hdr->frame_pkt_id_)
	    printf("Something is not correct, Rx/Tx non-synch, rx_id=%d, tx_id=%d\n", id, hdr->frame_pkt_id_);
	fprintf(tFile,"%-16f id %-16d udp %-16d\n", Scheduler::instance().clock(), id++, hdr->size()-28);
	pkt_received+=1;	
	// }
    }

    expected_ = seqno_ + 1;
    if (app_)
	app_->recv(hdr_cmn::access(pkt)->size());
    
    Packet::free(pkt);
}

int eraUdpSink::command(int argc, const char*const* argv)
{
	Tcl& tcl = Tcl::instance();
	
	if (strcmp(argv[1], "set_trace_filename") == 0) {
		strcpy(tbuf, argv[2]);
		tFile = fopen(tbuf, "w");
		opensinkfile = 1;
		return (TCL_OK);
	}  
	
	if (strcmp(argv[1], "closefile") == 0) {	
		fclose(tFile);
		return (TCL_OK);
	}
	
	if(strcmp(argv[1],"printstatus")==0) {
		print_status();
		return (TCL_OK);
	}
	
	return (Agent::command(argc, argv));
}

void eraUdpSink::print_status()
{
	printf("eraUdpSink: Total packets received:%ld\n", pkt_received);
}
