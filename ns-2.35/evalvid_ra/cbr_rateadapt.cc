/* -*-	Mode:C++; c-basic-offset:3; tab-width:3; indent-tabs-mode:t -*- */
#define RCP_MODE
/*
 * Copyright (c) Xerox Corporation 1997. All rights reserved.
 *  
 * License is granted to copy, to use, and to make and to use derivative
 * works for research and evaluation purposes, provided that Xerox is
 * acknowledged in all documentation pertaining to any such copy or derivative
 * work. Xerox grants no other licenses expressed or implied. The Xerox trade
 * name should not be used in any advertising without its written permission.
 *  
 * XEROX CORPORATION MAKES NO REPRESENTATIONS CONCERNING EITHER THE
 * MERCHANTABILITY OF THIS SOFTWARE OR THE SUITABILITY OF THIS SOFTWARE
 * FOR ANY PARTICULAR PURPOSE.  The software is provided "as is" without
 * express or implied warranty of any kind.
 *  
 * These notices must be retained in any copies of any part of this software.
 */

#include <stdlib.h>
 
#include "random.h"
#include "trafgen.h"
#include "ranvar.h"
#define MAX_NODE 100

/* 
 * Constant bit rate traffic source with rate adaptation based on ICMP SQ
 * cariied ECF feedback from P-AQM nodes.   
 * Parameterized by interval, (optional)
 * random noise in the interval, and packet size.  
 * Author: Arne Lie, NTNU Norway, 09.12.04.
 */

class CBR_RateAdapt : public TrafficGenerator {
public:
	CBR_RateAdapt();
	virtual double next_interval(int&);
	//HACK so that udp agent knows interpacket arrival time within a burst
	inline double interval() { return (interval_); }
protected:
	int command(int argc, const char*const* argv);
	virtual void start();
	virtual void stop();
	void init();
	double rate_;     /* send rate during on time (bps) */
	double interval_; /* packet inter-arrival time during burst (sec) */
	double random_;
	int seqno_;
	int maxpkts_;
	int running2_;
	double ECF_rateadapt[MAX_NODE];
	double ecf_new;
	double rate_inc_;
	double new_rate;
	int from_f;
	double ecf_store[MAX_NODE];  /* can handle 10 P-AQM nodes */
	double rate_store[MAX_NODE]; /* ---------  "  ----------- */
};


static class CBRRateAdaptClass : public TclClass {
public:
	CBRRateAdaptClass() : TclClass("Application/Traffic/CBR_RA") {}
	TclObject* create(int, const char*const*) {
		return (new CBR_RateAdapt());
	}
} class_cbr_rateadapt;

CBR_RateAdapt::CBR_RateAdapt() : seqno_(0)
{
	//printf("Yes, is in ::CBR_RateAdapt() constructor!\n");
	bind_bw("rate_", &rate_); 
	bind("random_", &random_);
	bind("packetSize_", &size_);
	bind("maxpkts_", &maxpkts_);
	bind("running_", &running2_);
	bind_bw("rate_increment_", &rate_inc_); 
	//printf("interval_=%5.4f, random=%d, rate=%5.4f, size=%d\n",interval_,random_,rate_,size_);
}

void CBR_RateAdapt::init()
{
	int i;
	// compute inter-packet interval 
	//printf("Yes, is in CBR_RA::init()!\n");
	//printf("interval_=%5.4f, size=%d, rate=%5.4f\n",interval_,size_,rate_);
	interval_ = (double)(size_ << 3)/(double)rate_;
	//printf("interval_=%5.4f, size=%d, rate=%5.4f\n",interval_,size_,rate_);
	for (i=0;i<MAX_NODE;i++) {
		ECF_rateadapt[i] = 1.0;
		rate_store[i] = rate_;
	}
	new_rate = rate_;
	ecf_new = 1.0;
	
	if (agent_)
		if (agent_->get_pkttype() != PT_TCP &&
 		    agent_->get_pkttype() != PT_TFRC)
			agent_->set_pkttype(PT_CBR);
}

void CBR_RateAdapt::start()
{
	//printf("Yes, is in CBR_RA::start()!\n");
	init();
	running_ = running2_ = 1;
	timeout();
}

void CBR_RateAdapt::stop()
{
	if (running_)
		timer_.cancel();
	running_ = running2_ = 0;
}
double CBR_RateAdapt::next_interval(int& size)
{
	// Recompute interval in case rate_ or size_ has changes
	
	
	// new_rate = rate_;
	
	//interval_ = (double)(size_ << 3)/(double)rate_;
	interval_ = (double)(size_ << 3)/(double)(new_rate+1.0); // add 1 to avoid div 0
	//printf("interval_=%10.9f\n",interval_);
	if (interval_ > 1.0) // This is added 25.02.05 to avoid that source becomes very slow!
		interval_ = 1.0;
	double t = interval_;
	if (random_)
		t += interval_ * Random::uniform(-0.5, 0.5);	
	size = size_;
	// printf("Yes, it is this!\n");
	if (t < 0)
		printf("t < 0\n");
	if (++seqno_ < maxpkts_)
		return(t);
	else
		return(-1); 
}

int CBR_RateAdapt::command(int argc, const char*const* argv)
{  
	if (argc == 5) {
		if (strcmp(argv[1], "rateadapt") == 0) {
			// This version might receive ECFs from multiple nodes
			// rate_ is the original rate (maximum rate)
			int i;
			double min_rate;
			const char* ecf_c = argv[2];
			const char* from_c = argv[3];
			
			ecf_new = (double) atof(ecf_c);
			from_f = (int) atoi(from_c);
			//printf("Is in CBR_RA command rateadapt, from_f = %d\n",from_f);
			ecf_store[from_f] = ecf_new;
			
			if (ecf_store[from_f] < 1.0) {
				ECF_rateadapt[from_f] *= ecf_store[from_f];
				rate_store[from_f] = rate_ * ECF_rateadapt[from_f];
			}
			else if (ecf_store[from_f] > 1.0) {
				//new_rate = new_rate + rate_inc_;
				/*
				 * This version is new (15.12.04)
				 * Will lead to faster adaption to full utilization
				 * so that the basic rate_inc can be smaller.
				 * Negative: possible slower settling to fair share
				 */
				//				rate_store[from_f] = rate_store[from_f] + rate_inc_*(ecf_store[from_f]-1.0);
				/*
				 * Even newer version, where ecf >= 1.0 is the excess bw for this source
				 * i.e. the whole increment!!
				 */
#ifdef RCP_MODE
				rate_store[from_f] = ecf_new;
				if (rate_store[from_f] > rate_) {
					rate_store[from_f] = rate_;
				}
#else
				rate_store[from_f] = rate_store[from_f] + ecf_store[from_f];
				ECF_rateadapt[from_f] = rate_store[from_f] / rate_;
				if (rate_store[from_f] > rate_) {
					rate_store[from_f] = rate_;
					ECF_rateadapt[from_f] = 1.0;
				}
#endif
			}
			// printf("rate_=%5.4f\n",rate_);
			// return TCL_OK, so the calling function knows that
			// the command has been processed
			
			/*
			 * The following processing selects the smallest rate
			 * Not sure if this is a good option (18.01.05 AL)
			 * It seems to be a good one!! (19.01.05 AL)
			 */
			min_rate = rate_;
			for (i=0;i<MAX_NODE;i++) {
				if (rate_store[i]<min_rate)
					min_rate = rate_store[i];
			}
			new_rate = min_rate;
			
			const char* whoami_c = argv[4];
			int whoami_i = (int) atoi(whoami_c);
			if (whoami_i == 113 && from_f == 0) {
				double t = Scheduler::instance().clock();
#ifdef RCP_MODE
				printf(">>PAQM_RCPM_CBR_RA: t=%2.2f, ecf_new=%2.1f, from_f=%d, rates: %2.1f %2.1f %2.1f %2.1f\n",
						 t,ecf_new,from_f,rate_store[0],rate_store[1],rate_store[2],rate_store[3]); 
#else
				printf(">>PAQM_AIMD_CBR_RA: t=%2.2f, ecf_new=%2.1f, from_f=%d, rates: %2.1f %2.1f %2.1f %2.1f\n",
						 t,ecf_new,from_f,rate_store[0],rate_store[1],rate_store[2],rate_store[3]); 
#endif
			} 
			// timeout(); // Force start of new rate (new 040205)
		}
		if (strcmp(argv[1], "TFRC_rateadapt") == 0) {
			const char* tfrc_rate_c = argv[2];	    
			double tfrc_rate = (double) atof(tfrc_rate_c);
			double old_rate;
			const char* whoami_c = argv[3];
			int whoami_i = (int) atoi(whoami_c);
			const char* backlog_c = argv[4]; /* In addition to rate, the RA engine also need to know the TFRC tx
															queue backlog, so that the rate information is scaled down to
															enforce queue draining */
			int backlog = (int) atoi(backlog_c);
			/* Convert from B/s to bits/s */
			tfrc_rate *= 8;
			//printf("Orig. tfrc_rate=%6.5f, whoami=%d\n",tfrc_rate,whoami_i);
			old_rate = new_rate;
			/* Modify the TFRC given rate downwars in order to drain the Tx queue is any */
			float d_f = 100.0; /* decay factor d_f as used in paper */
			new_rate = tfrc_rate = tfrc_rate*exp(-(double)backlog/d_f);
			if (new_rate > rate_)
				new_rate = tfrc_rate = rate_;
			if (whoami_i == 7) {
				double t = Scheduler::instance().clock();
				printf(">>TFRC_CBR_RA, t=%6.5f, new_rate=%6.5f, old_rate=%6.5f, tfrc_rate=%6.5f, backlog=%d\n"
						 ,t,new_rate,old_rate,tfrc_rate,backlog); 
			}
			//printf("Mod. tfrc_rate=%6.5f, whoami=%d\n",tfrc_rate,whoami_i);
			
		}
		return (TCL_OK);
	}
	return (Application::command(argc, argv));	
}
