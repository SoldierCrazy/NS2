/* -*-	Mode:C++; c-basic-offset:3; tab-width:3; indent-tabs-mode:t -*- */
#define CAP_LIMIT 0.95     /* For slow links < 100Mbit/s,  set to 0.95 */
                          /* This limits utilization a little, 
										but keeps the queue backlog low */
                          /* Only when 1.0, the real queue equilibrium will be obtainable */
#define RCP_MODE          /* Enable this define if wanting the RCP_MODE (P-AQM+ERF) */
#define MONITOR_NODE 100  /* Select which P-AQM node to monitor */
#define FORCE_EXT_MD_LMT 50000 /* Make it a large number, e.g. 5000, 
											 if no extra MD is wanted (AIMD/P-AQM+ECF mode only) */
#define EXT_MD_VAL 0.93    /* value of extra MD */
//#define STRIKE_DETECT    /* Enable this to include ill-behaving UDP flow punishment aka FRED strike */
/*
 *******************************************************************************
 * P-AQM:
 *
 * THIS PROPORTIONAL AQM (p_aqm) IMPLEMENTATION WAS INITIALLY BASED ON THE REM
 * IMPLEMENTATION IN NS-2. IT HAS SOME SIMILARITIES TO REM, SO
 * THAT IS WHY I USED REM AS A "TEMPLATE", TO INTERFACE TO THE NS-2 STUFF. 
 * HOWEVER, REM AND P-AQM IS FAR FROM THE SAME ALGORITHM!
 * Note that this implementation is by no means optimized on performance. It must
 * be regarded as pure experimental / research based software for evaluation of
 * proof of concept of P-AQM+ECF and ERF.
 *
 * Note:
 * AIMD-mode = P-AQM+ECF
 * RCP_MODE  = P-AQM+ERF
 *
 * Refs: 
 * [1] A. Lie, O. M. Aamo, L. A. Ronningen, "A performance comparison
 *     study of DCCP and a method with non-binary congestion
 *     metrics for streaming media rate control", 19th International
 *     Teletraffic Congress
 *     (ITC'19), Beijing China, 29 August to 2nd September, 2005.
 *
 * [2] A. Lie, "P-AQM: Low-Delay Max-Min Fairness Streaming of Scalable
       Real-Time CBR And VBR Media", pending publication fall 2007.
 * 
 * Date file first created: October 2004
 * Author: Arne Lie
 * Dept. of Telematics
 * Norwegian Univeristy of Science and Technology -- NTNU
 * N-7491 Trondheim
 * NORWAY
 */

#include <math.h>
#include <sys/types.h>
#include "config.h"
#include "template.h"
#include "random.h"
#include "flags.h"
#include "delay.h"
#include "p_aqm.h"
#include <iostream>

FILE *fout;
FILE *fout2;

static class PaqmClass : public TclClass {
public:
	PaqmClass() : TclClass("Queue/Paqm") {}
	TclObject* create(int, const char*const*) {
		return (new PaqmQueue);
	}
} class_p_aqm;


void PaqmQueue :: set_update_timer()
{
	p_aqm_timer_.resched(p_aqmp_.p_updtime);
}

void PaqmQueue :: timeout()
{
	//do P-AQM inner loop recalculation
	run_updaterule();
	// schedule next timeout for event simulation
	set_update_timer();
}


void PaqmTimer :: expire (Event *e) {
	a_->timeout();
}


PaqmQueue::PaqmQueue() : link_(NULL), tchan_(0), p_aqm_timer_(this)
{
	bind("Kgain_", &p_aqmp_.p_Kgain); /* K gain of TCP queue (see [1])*/
	bind("KgainUDP_", &p_aqmp_.p_KgainUDP); /* K gain of UDP queue */
	bind("phi_", &p_aqmp_.p_phi);   /* Obsolete */
	bind("inw_", &p_aqmp_.p_inw);   /* queue weight given to cur q size sample */
	bind("mean_pktsize_", &p_aqmp_.p_pktsize); /* if used in packet mode: byte mode is recommended */
	bind("pupdtime_", &p_aqmp_.p_updtime); /* (Inner) loop update period (s) */
	bind("Nstar_", &p_aqmp_.p_Nstar);    // This is the wanted TCP buffer occupancy equilibrium
	bind("ECF_period_", &p_aqmp_.p_ECF_period); // No. of loop periods between each ECF ICMP SQ 
	bind("TCP_period_", &p_aqmp_.p_TCP_period); // No. of loop periods between each numTCP_ estimation
	bind("numb_adapt_UDP_", &p_aqmp_.p_numb_adapt_UDP); // No. of loop periods between each numTCP_ estimation
	bind("ping_period_", &p_aqmp_.p_ping_period); // No. of loop periods between each ping 
	bind("prob_", &p_aqmv_.v_prob);	// TCP dropping or ECN marking probability
	bind("2probUDP_", &prob_udp);	   // UDP dropping probability
	bind("debug_", &p_aqmv_.v_pl);	// Use the REM link price variable to debug other
	bind("curq_", &curq_);		      // current TCP queue size
	bind("3curqUDP_", &curqUDP_);		// current UDP queue size
	bind("avr_", &avr_);		         // current ECF value
	bind("ave_", &p_aqmv_.v_ave);	   // average input bit rate
	bind("numUDP_", &numUDP_);	      // set num flows manually (just first period, kindof obsolete)
	bind("numTCP_", &numTCP_);	      // set num flows manually (just first period, kindof obsolete)
	bind("pmark_", &pmark_);         //number of packets being marked
	bind_bool("markpkts_", &markpkts_); /* Whether to mark or drop?  Default is drop */
	bind_bool("qib_", &qib_);        /* queue in bytes? */
	bind_bool("ecf_on_", &ecf_on_);  /* ECF/ERF feedback switched on? */
	bind("NstarUDP_", &NstarUDP_);   // This is the wanted UDP queue equilibrium (inner loop, outer loop is 10%)
	bind("alpha_", &alpha_);      	// New stabilizing factor [2]
	bind("beta_", &beta_);      	   // New stabilizing factor [2]
    
	q_ = new PacketQueue();	        // Instantiate underlying TCP queue
	qUDP_ = new PacketQueue();	     // NEW 02.02.05: underlying queue for UDP
	pq_ = q_;
	reset();
   
#ifdef notdef
	print_p_aqmp();
	print_p_aqmv();
#endif
   
}

void PaqmQueue::reset()
{
	int i;
	/*
	 * Compute the "packet time constant" if we know the
	 * link bandwidth.  The ptc is the max number of
	 * pkts per second which can be placed on the link.
	 * The link bw is given in bits/sec, so scale psize
	 * accordingly.
	 */
	
	if (link_)
		if (qib_) {
			p_aqmp_.p_ptc = link_->bandwidth() / (8.0); // capacity in Bytes per sec
		}
		else
			p_aqmp_.p_ptc = link_->bandwidth() / (8.0 * p_aqmp_.p_pktsize);
	
	p_aqmv_.v_pl = 0.0;
	p_aqmv_.v_prob = 0.0;
	prob_udp = 0.0;
	p_aqmv_.v_in = 0.0;
	p_aqmv_.v_ave = 0.0;
	p_aqmv_.v_ave_udp = 0.0;
	p_aqmv_.v_count = 0.0;
	
	p_aqmv_.v_pl1 = 0.0;
	p_aqmv_.v_pl2 = 0.0;
	p_aqmv_.v_in1 = 0.0;
	p_aqmv_.v_in2 = 0.0;
	pmark_ = 0.0;
	bcount_ = 0;
	bcountUDP_ = 0;
	
	p_aqmv_.v_pr_ave = 1.0;
	
	curq_ = 0;
	curqUDP_ = 0;
	avr_ = 0.0;
	
	count_ = 0;
	numFlow_count_ = 0;
	count_scale_ = 1.0;
	ping_count_ = 0;
	drop_count_ = 0;
	sdrop_count_ = 0;
	tcp_drop_count_ = 0;
	udp_taildrop_ = 0;
	mark_spacer_ = 0;
	udp_count_ = 0;
	rtt_est_ = p_aqmp_.p_updtime;
	rtt_max_ = 0.0;
	queueToggle_ = 0;
	for (i=0;i<MAX_FQ_TABLE_SIZE;i++) {
		toggleFQ[i++] = 0; // 0 means UDP queue
		toggleFQ[i] = 1; // 1 means TCP queue
	}
	tFQ_cnt = 0;
	tFQ_size = MAX_FQ_TABLE_SIZE;
	tau1 = 0.0;
	tau2 = p_aqmp_.p_ECF_period;
	long_udp_count_ = num_period = 0;
	count_small_MD = 0;
	remember_time = 0.0;
	numTCP_store = 0;
	last_state_is_mice = this_state_is_mice = 0;
	fout = fopen("paqm_iat.txt","w");
	fout2 = fopen("paqm_arr.txt","w");
	for (i=0;i<FAIR_TABLE_MAX;i++) {
		flow_table_tcp[i] = 0;
		flow_table_cnt_tcp[i] = 0;
		tcp_mice_count[i] = 0;
	}
	for (i=0;i<FAIR_TABLE_MAX;i++) {
		flow_table_udp[i] = 0;
		flow_table_cnt_udp[i] = 0;
		save_flow_table_cnt_udp[i] = 0;
		flow_ECF_tag[i] = 2;
		flow_table_cntAll_udp[i] = 0;
		flow_table_last_cntAll[i] = 0;
		//flow_table_cnt_HR_udp[i] = 0;
		//flow_table_cnt_LR_udp[i] = 0;
		strike_table[i] = 0;
		drop_table[i] = 0.0;
		last_drop_table[i] = 0.0;
		drop_loop_size[i] = 0;
		drop_loop_cnt[i] = 0;
		drop_loop_cnt_tenth[i] = 0;
		drop_inner_cnt_tenth[i] = 0;
		drop_loop_size_tenth[i] = 0;
		drop_inverse[i] = 0;
	}
	numTCP_ = numUDP_ = 1; // New 030705: force this old bind values to one: now flow counter impl
	// will modify these adaptively
	flow_ratio = 0.5; // init. to 50% TCP and 50% UDP
	last_ratio = 1.0;
	eUDP = 0.0;
	maxStrike = 0;
	maxStrikeIndex = 0;
	
	Rt_RCP = p_aqmp_.p_ptc * 8;  /* added 10.09.06 to modifiy to RCP ideas, init to link capacity */
	Nt = 1.0;      /* Start with an estimate of one source */


	pktRatio = 0.5;
	avr_cnt_udp = 10000;
	careful_MD = 0;
	sameCnt = 0;
	maxCntIndex = 0;
	lastMaxCntIndex = 0;
	maxThisECF = 0;
	highload_state = 0;
	long_tcp_count = 0; 
	long_numTCP_ = 0;
	filtered_numTCP_ = 0.0;
	nqueuedMem = 0;
	//Queue::reset();
	set_update_timer();
}

/*****************************************************************************************
 * This method is run each p_updtime (inner loop period [1])
 * Compute the average input rate, new inner loop drop/mark probabilities (both queues),
 * eventually strike ill-behaving flows, calculate the feedback value (outer-loop) for
 * ECF or ERF signaling. 
 * The method is by no means performance optimized, and has unneccesary many tests to improve
 * debug and monitor functionality.
 *****************************************************************************************/
void PaqmQueue::run_updaterule()
{
	int i;
	double in, in_udp, in_avg, in_avg_udp, nqueued, nqueuedUDP, pl, pr, pr_udp;
	
	// in is the number of bytes (if qib_ is true) or packets (otherwise)
	// arriving at the link (input rate) during one update time interval
	in = p_aqmv_.v_count;
	in_udp = udp_count_;
	
	// in_avg is the low pss filtered input rate
	// which is in bytes if qib_ is true and in packets otherwise.
	in_avg = p_aqmv_.v_ave;
	in_avg *= (1.0 - p_aqmp_.p_inw);
	
	in_avg_udp = p_aqmv_.v_ave_udp;
	in_avg_udp *= (1.0 - p_aqmp_.p_inw);
	
	if (qib_) {
		in_avg += p_aqmp_.p_inw*in;
		in_avg_udp += p_aqmp_.p_inw*in_udp;
		nqueued = bcount_;
		nqueuedUDP = bcountUDP_;
	}
	else {
		in_avg += p_aqmp_.p_inw*in;
		in_avg_udp += p_aqmp_.p_inw*in_udp;
		nqueued = q_ -> length();
		nqueuedUDP = qUDP_ -> length();
	}
	nqueuedMem = 0.95*nqueuedUDP + 0.05*nqueuedMem; /* New 250106 */
	
	// c measures the maximum number of packets (or #of bytes) that
	// could be sent during one update interval dT=p_updtime, since p_ptc is the
	// maximum packets (or #of bytes) per second that can be placed on link
	double c  = p_aqmp_.p_updtime*p_aqmp_.p_ptc; /* capacity (byte / dT) */
	//double c  = p_aqmp_.p_updtime*p_aqmp_.p_ptc * 0.9;
	if ((numTCP_ + numUDP_)>0)
		flow_ratio = (double) numTCP_ / (numTCP_ + numUDP_);
	else
		flow_ratio = 0.5;
	
	double ct  = c * flow_ratio; // These values should get real ratio!!! (see modification below)
	double cu  = c - ct;
	
	/*
	 * Proportional gain controlled AQM after Lie, Roenningen, Aamo (P-AQM inner loop) [1]
	 */
	/* 
	 * For TCP: 
	 */
	double e = p_aqmp_.p_Nstar - nqueued;
	if (in_avg >= ct) 
		pr = ct /(in_avg+0.000000001) + e*p_aqmp_.p_Kgain/(in_avg+0.000000001);
	else 
		pr = ct /(in_avg+0.000000001);
	
	/* 
	 * For UDP: 
	 */
	double eUDPouter = NstarUDP_*(1-flow_ratio) - nqueuedUDP;
	if (in_avg_udp >= cu) 
		pr_udp = cu /(in_avg_udp+0.000000001) + eUDPouter*p_aqmp_.p_KgainUDP/(in_avg_udp+0.000000001);
	else 
		pr_udp = cu /(in_avg_udp+0.000000001);
	
	
	// pr_ave is not in practice use anymore..., can be deleted probably
	pr_ave_ = p_aqmv_.v_pr_ave;
	pr_ave_ *= (1.0 - p_aqmp_.p_inw);
	pr_ave_ += p_aqmp_.p_inw * pr_udp;
	if (pr_ave_ < 0.0)
		pr_ave_ = 0.0;
	if (pr_ave_ > 1.0)
		pr_ave_ = 1.0;
	
	pr = 1.0 - pr;	
	if (pr < 0.0) 
		pr = 0.0;	
	if (pr > 1.0)
		pr = 1.0;
	
	
	pr_udp = 1.0 - pr_udp;	
	if (pr_udp < 0.0) 
		pr_udp = 0.0;
	
	if (pr_udp > 1.0)
		pr_udp = 1.0;
	
	/* 
	 * Update the couter for periodic ECF reports, 
	 * i.e. r(k) is estimated differently in the outer loop than in the inner loop
	 */
	count_++; 
	numFlow_count_++;
	if ((count_-1) > (rtt_max_ / p_aqmp_.p_updtime)) {
		/* 
		 * The if is to avoid monitoring input rate before it has changed due to
		 * last ECF
		 */
		long_udp_count_ += (int) in_udp;
		num_period ++;
		/*
		if (p_aqmv_.id_num == MONITOR_NODE) {
			printf("in_udp=%4.3f, count_=%d, rtt_max_=%4.3f, long_udp_count_=%d, num_period=%d\n",
					 in_udp,count_,rtt_max_,long_udp_count_, num_period);
		}
		*/
	}
	
	double this_time = Scheduler::instance().clock();
	fprintf(fout2,"%8.7f   %d\n",this_time, udp_count_ );
	
	double excess_BW;
	//    printf("Just before, numFlow_count_ =%d \n",numFlow_count_);
	/***************************************************************************************
	 **                                                                                   **
	 * Test if new estimation of number of flows and strikes and scheduler shall start now *
	 **                                                                                   **
	 ***************************************************************************************/ 
	if (numFlow_count_ >= (p_aqmp_.p_ECF_period*p_aqmp_.p_TCP_period*count_scale_)) {
		if (p_aqmv_.id_num == MONITOR_NODE) 
			printf("################# Monitor P-AQM router No-%d  #####################\n",p_aqmv_.id_num);
		/*
		 * Modified flow counter (040705), deselect short flows
		 * The total algorithm is changed several times.
		 * Consists of: all flow count, separate mice from elephants (both TCP and UDP), 
		 * strike misbehaving UDP sources and enable drop before enqueue, ECF-extra on possible
		 * adaptive UDP sources that sends too much (fasten up the AIMD convergence process).
		 */
		
		/************************************************************
		 * First loop: count ALL active flows
		 ************************************************************/
		double numTcpPkts = 0.0;
		double numUdpPkts = 0.0;
		double numAllUdpPkts = 0.0;
		int max_index = 0;
		numTCP_ = 0;
		numUDP_ = 0;
		for (i=0;i<FAIR_TABLE_MAX;i++) {
			numTcpPkts += flow_table_cnt_tcp[i];
			numUdpPkts += flow_table_cnt_udp[i];
			numAllUdpPkts += flow_table_cntAll_udp[i]; // Count UDP packets, also before strike drop
			numTCP_ += flow_table_tcp[i];
			numUDP_ += flow_table_udp[i];
			if ((flow_table_tcp[i]==1) || (flow_table_udp[i]==1) ||
				 (flow_table_cnt_tcp[i]>0) || (flow_table_cnt_udp[i]>0))
				max_index = i;
		}
		// printf("\nBefore just.: numTCP_ = %d, numUDP_ = %d\n",numTCP_,numUDP_);
		// limit set to a fraction of total # packets div. on # of flows
		double avrUdp = (double) (numUdpPkts / (numUDP_+0.0001));
		double avrAllUdp = (double) (numAllUdpPkts / (numUDP_+0.0001));
		
		/*************************************************************
		 * Second loop: FILTER OUT SIGNIFICANT FLOWS LOOP:
		 * ***********************************************************
		 * Calc. new flow counter values to be used the next whole ECFp period
		 * The following loop will identify high-rate flows and give each of them one 
		 * complete channel. It will also identify mice flows (TCP) and low-rate flows (UDP)
		 * and assign also these some capacity.
		 */
		// #of TCP packets per flow per ECF period:
		double bw_share_TCP = 
			numFlow_count_ * ct / (1500.0*numTCP_+0.0001); 
		// If zero TCP flows detected, calculate as if there was one:
		if (numTCP_ == 0) 
			bw_share_TCP = numFlow_count_ * ct / (1500.0*1);
		// If zero TCP capacity given, give it 20%
		else if (ct == 0) 
			bw_share_TCP = numFlow_count_ * 0.2 * c / (1500.0*numTCP_);
		//printf("bw_share_TCP = %4.3f\n",bw_share_TCP);
		numTCP_ = 0;
		numUDP_ = 0;
		int mice = 0;
		int miceUDP = 0;
		double avrUDP_hr = 0.0;
		int cnt_HR_packets = 0;
		int cnt_LR_packets = 0;
		
		for (i=0;i<FAIR_TABLE_MAX;i++) {
			/*
			 * TCP filtering first
			 */
			if (flow_table_tcp[i]==1) {
				mice++;
				tcp_mice_count[i]++;
				/* printf("Mice detected, flowID=%d, mice_cnt=%d!\n",
					i,tcp_mice_count[i]); */
				if (tcp_mice_count[i]>30) 
					tcp_mice_count[i] = 30;
				if (tcp_mice_count[i]>2) {
					numTCP_ ++;	
					/* printf("Long Mice convert to Elephant, flowID=%d, mice_cnt=%d!\n",
						i,tcp_mice_count[i]); */
				}
			}
			else { // Flow is not present this period, decrease its count (it CAN still be active)
				//tcp_mice_count[i] = (int)(0.2*tcp_mice_count[i]);	
				tcp_mice_count[i] = tcp_mice_count[i] - 10;	
				if (tcp_mice_count[i]>2) {
					numTCP_ ++;	
					/* printf("Flow not present, but still Long Mice convert to Elephant, flowID=%d, mice_cnt=%d!\n",
						i,tcp_mice_count[i]); */
				}
				if (tcp_mice_count[i]<0)
					tcp_mice_count[i]=0;
			}
			flow_table_tcp[i] = 0 ; // reset flow tables
			flow_table_cnt_tcp[i] = (int) (flow_table_cnt_tcp[i]*0.0); // forget not so fast...
			
			/*
			 * UDP filter now
			 */
			if (flow_table_cntAll_udp[i]>(0.75*avrUdp)) {
				/* 
				 * This is high-rate flow, typically CC high-rate or non-CC high rate.
				 */
				cnt_HR_packets += flow_table_cnt_udp[i]; // Update # of high rate packets
				avrUDP_hr = avrUDP_hr * numUDP_ + flow_table_cntAll_udp[i]; // Calc avr of HR only
				numUDP_ ++; // "Assign" one "channel" for this flow
				avrUDP_hr = avrUDP_hr / numUDP_; // Update avr finished
				class_HR_or_LR[i] = 1; /* Set to 1 to flag HE */
				/*	printf("UDP Flow %d counted! numUDP_=%d, numbPkts=%d > %4.3f, numbPktsAll=%d, avrAllUdp=%4.3f\n",
				  i,numUDP_,flow_table_cnt_udp[i],0.75*avrUdp,flow_table_cntAll_udp[i],avrAllUdp);*/
			}
			else if (flow_table_udp[i]==1) {
				if (p_aqmv_.id_num < 0) 
					printf("UDP low-rate detected. flowID=%d, numbPkts=%d\n",i,flow_table_cnt_udp[i]);
				miceUDP ++; // What to do with these? See "if (miceUDP > 0)" below
				cnt_LR_packets += flow_table_cnt_udp[i]; // Update # of low rate packets
				class_HR_or_LR[i] = 0;
			}
		}
		if (mice > 0 && numTCP_==0)
			numTCP_ ++; // Allocate one extra TCP channel for small flows aggregate
		
		if (miceUDP > 0) {
			// Allocate extra channels worth the amount of aggregate packets of the low-rate flows
			double tmp = (double) (numUDP_*cnt_LR_packets / (cnt_HR_packets + 0.0001));
			numUDP_ = numUDP_ + (int)ceil(tmp);
			//numUDP_ = numUDP_ + (int)floor(tmp);
		}
		
		/*
		 * Prevent shutoff:
		 */
		if ((nqueued > 0) && (numTCP_==0))
			numTCP_ = 1;
		if ((nqueuedUDP > 0) && (numUDP_==0))
			numUDP_ = 1;
		
		/*printf("After mice justification: mice_state=%d, numTCP_ = %d, filtered_numTCP=%4.3f, numUDP_ = %d\n",
		  this_state_is_mice,numTCP_,filtered_numTCP_, numUDP_);  */
		if (p_aqmv_.id_num < 0) 
			printf("After mice justification: TCP-mice? %d, numTCP_ = %d, UDP-mice? %d, numUDP_ = %d\n",
					 mice, numTCP_, miceUDP, numUDP_); 
		
#ifdef STRIKE_DETECT
		/************************************************************
		 * STRIKE-LOOP:
		 ************************************************************
		 * Find ill-behaving UDP sources and give them penalty with STRIKE...
		 * If too many strikes these flows will experience randomized dropping in enqueue
		 */
		eUDP = 0.1*NstarUDP_*(1-flow_ratio) - nqueuedUDP; // Difference in equilibrium and queue
		maxStrike = 0;
		maxStrikeIndex = 0;
		int numb_of_dropps = 0;
		double avrUDP_hr_mod = avrUDP_hr;
		careful_MD = 0; // Init to NOT do careful downscale
		// BW share divided on valid number of UDP sources
		double bw_share = numFlow_count_ * cu / (1500.0*numUDP_+0.001); 
		double udp_capacity_left = numFlow_count_ * cu / 1500.0 - numUdpPkts;
		double strike_capacity = 0.0;
		double strikeAll_capacity = 0.0;
		if (p_aqmv_.id_num < 0) 
			printf("bw_share = %4.3f, capacity_left=%4.3f, numFlow_count_=%d, max_index=%d\n",
					 bw_share,udp_capacity_left,numFlow_count_,max_index);
		maxCntIndex = 0; // Index of UDP flow with most arriving packets (before strike dropping)
		maxThisECF = 0;  // Number of packets from the most active UDP flow this period
		extra_ECF = 0.0;
		for (i=0;i<=max_index;i++) {
			if (flow_table_udp[i]==1 && flow_ECF_tag[i]==0) { 
				printf("==>OUPS Lost TAG for flowID=%d?\n",i);
				/* Then, test if this flow has unnormal many packets compared
				 * to the average for the high-rate flows OR bw_share (>20% more), OR
				 * if this ECF period has experienced more new packets from this 
				 * flow than MD*last period flow. Both indications are highly
				 * probable for a non-CC flow. 
				 * Added state of being HR class to be extra prob. dropped */
				if (( (flow_table_cntAll_udp[i] > (avrUDP_hr*1.2))||
						(flow_table_cntAll_udp[i] > (1.2*bw_share ))||
						((flow_table_cntAll_udp[i]> 1.1*last_ratio*flow_table_last_cntAll[i]))) &&
					 (class_HR_or_LR[i] ==1) ) {
					strike_table[i] ++; // Increment strike if YES.		    
					if (numUDP_ > 1) { // Calc. new average for the set without THIS non-CC flow
						avrUDP_hr_mod = 
							(avrUDP_hr * numUDP_ - flow_table_cntAll_udp[i])/(numUDP_ - 1);
						/* Drop so many that the flow is forced to fair rate!!: */
						drop_table[i] = 
							1 - (bw_share / flow_table_cntAll_udp[i]); 
						
						if (drop_table[i] < 0.0) 
							drop_table[i] = 0.0;
						if (drop_table[i] > 0.5) { // then this is severe, must execute now!
							strike_table[i] = strike_table[i] + 2;
							// careful_MD = 1;
						} 
						
						/* If only one (non-CC) flow is dominating, then there is no change
						 * in the average rate, and thus drop-table is calculated
						 * to zero. Force to 50% drop to make something happen! */
						if (drop_table[i]==0.0 && numUDP_ == 2) {
							careful_MD = 1;
						}
					}
					else if (eUDP < 0) {
						drop_table[i] = 1 - (bw_share / flow_table_cntAll_udp[i]); 
						if (drop_table[i] <= 0.0) {
							drop_table[i] = 0.0;
							strike_table[i] = strike_table[i] - 10; // Downcount faster than upward
						}
						avrUDP_hr_mod = avrUDP_hr;
					}
					if (strike_table[i] >= maxStrike) {
						/* This version is limitated to only one of the CC sources do get special
							feedback, typically extra MD is applied to speed up adaptation to 
							fair bandwidth share */
						/* This means that if a non-CC high rate is present, then this functionality
							will not improve the fairness, but strike dropping should continue! */
						if (drop_table[i] > drop_table[maxStrikeIndex]) {
							maxStrike = strike_table[i];
							maxStrikeIndex = i; // Store which flow has most strikes and most severe dropp rate
						}
					}
				}
				else {
					strike_table[i] = strike_table[i] - 10; // Downcount faster than upward
					drop_table[i] = 0.0;
				}
				if (drop_table[i]>0.0 && strike_table[i]>=3) {
					numb_of_dropps++;
					strike_capacity += flow_table_cnt_udp[i];
					strikeAll_capacity += flow_table_cntAll_udp[i];
				}
				
			}
			/* 
			 * This test is for ECF enabled flows only
			 * Search for the ECF flow with largest BW consumption
			 */
			
			if (flow_table_cntAll_udp[i]>maxThisECF && flow_ECF_tag[i]==1) {
				maxCntIndex = i;
				maxThisECF = flow_table_cntAll_udp[i];
				//extra_ECF = 0.025;
				extra_ECF = 0.1;
				// extra_ECF = 1 - (bw_share / flow_table_cntAll_udp[i]); 
				//extra_ECF = 0.1;
				if (extra_ECF < 0) extra_ECF = 0.0;
				
			}
			
			if (strike_table[i] < 0)
				strike_table[i] = 0;
			if (strike_table[i] > 0)
				printf("strike_table[%d] = %d, drop_table[same] = %4.3f, avrUDP_hr=%4.3f, avrUDP_hr_mod=%4.3f\n",
						 i,strike_table[i],drop_table[i],avrUDP_hr,avrUDP_hr_mod);
			
			// Reset counters
			flow_table_udp[i] = 0;
			save_flow_table_cnt_udp[i] = flow_table_cnt_udp[i];
			flow_table_cnt_udp[i] = (int) (flow_table_cnt_udp[i]*0.0);
			flow_table_last_cntAll[i] = flow_table_cntAll_udp[i];
			flow_table_cntAll_udp[i] = (int) (flow_table_cntAll_udp[i]*0.0);
			
			last_drop_table[i] = drop_table[i];
			strike_taken[i]=0; /* Prepare reset of flags for strike drop adjustment algorithm below */
			
		} /* end for(;;) loop for this flow (i.e. index i) */
		// printf("numb_of_dropps=%d\n", numb_of_dropps);
		
		/*****************************************************
		 * MAKE DROP_TABLE A TABLE-LOOKUP ROUND-ROBIN:
		 *****************************************************/
		for (i=0;i<=max_index;i++) {
			/*
			 * Create the parameters that makes the strike dropping of almost correct percentage
			 * but chained in a round-robin fashion, not randomized (to stabilize the queue size jitter)
			 * It basically transform the fractional number drop_table[i] to counter values for stready drop.
			 */
			drop_loop_size[i] = 0;
			drop_loop_cnt[i] = 0;
			drop_inverse[i] = 0;
			drop_loop_size_tenth[i] = 0;
			// Run only for those flows that shall be strike-dropped:
			if (drop_table[i]>0.0) {
				if (drop_table[i] <= 0.5) {
					drop_loop_size[i] = (int) (1.0/drop_table[i]);
					double tmp = 1.0/(drop_table[i]);
					double tmp2[1];
					tmp = modf(tmp,tmp2);
					//printf("modf=%8.7f, drop_table=%8.7f\n",tmp,drop_table[i]);
					drop_loop_size_tenth[i] = (int) (10.0*tmp);
					
					if (eUDP < 0) { // Buffer is too full, empty it by increasing the drop ratio!
						if (drop_loop_size_tenth[i] > 0)
							drop_loop_size_tenth[i] = drop_loop_size_tenth[i] - 1;
						else {
							drop_loop_size[i]--;
							drop_loop_size_tenth[i]=9;
						}
					}
					
					printf("size_tenth[%d] = %d\n",i,drop_loop_size_tenth[i]);
					drop_loop_cnt[i] = 0;
				}
				else {
					drop_loop_size[i] = (int) (1.0/(1.0 - drop_table[i]));
					drop_inverse[i] = 1;
					double tmp = 1.0/(1.0 - drop_table[i]);
					double tmp2[1];
					tmp = modf(tmp,tmp2);
					//printf("modf=%4.3f, drop_table=%4.3f\n",tmp,drop_table[i]);
					drop_loop_size_tenth[i] = (int) (10.0*tmp);
					
					if (eUDP < 0) { // Buffer is too full, empty it!
						if (drop_loop_size_tenth[i] < 9)
							drop_loop_size_tenth[i] = drop_loop_size_tenth[i] + 1;
						else {
							drop_loop_size[i]++;
							drop_loop_size_tenth[i]=0;
						}
					}
					
					printf("size_tenth[%d] = %d\n",i,drop_loop_size_tenth[i]);
					drop_loop_cnt[i] = 0;
				}
			}
		}
		if (p_aqmv_.id_num < 0) 
			printf("==...Most active flow is %d, and last Most active flow was %d\n",
					 maxCntIndex,lastMaxCntIndex);
		
		lastMaxCntIndex = maxCntIndex;

#endif

		/***************************************************************
		 * FAIR QUEUEING (FQ) ROUND ROBIN:
		 * This should have been rebuilt to become byte oriented, 
		 * but is still not 06.05.07. The work-around (in case of variable
		 * sized packets in VBR, such as Evalvid-RA), is to know (or guess)
		 * a-priori what the approximate average packet size of the VBR are.
		 ***************************************************************
		 * New 050805: calculate a new Fair Queueing (FQ) table numTCP_+numUDP_ long and with 0 for UDP
		 * and 1 for TCP. Dequeue will cycle through this table thus ensuring the capacity as  calculated.
		 */
		int minNum = (numTCP_ < numUDP_) ? 2*numTCP_ : 2*numUDP_ ;
		for (i=0;i<minNum;i++) {
			toggleFQ[i++] = 0; // Is UDP
			toggleFQ[i] = 1;   // Is TCP
		}
		int anyRest = (numTCP_ > numUDP_) ? 1 : 0 ;
		for (int ii=0;ii<abs(numTCP_-numUDP_);ii++) {
			toggleFQ[i] = anyRest;
			// printf("anyRest = %d, i = %d\n",anyRest,i);
			i++;
		}
		tFQ_size = numTCP_ + numUDP_;
		tFQ_cnt = 0;
		
		/*****************************************************************
		 * New 290705: update flow_ratio and cu and ct imediately
		 *****************************************************************/
		if ((numTCP_ + numUDP_)>0)
			flow_ratio = (double) numTCP_ / (numTCP_ + numUDP_);
		else
			flow_ratio = 0.5;
		
		ct  = c * flow_ratio; // These values should get real ratio!!! (see modification below)
		cu  = c - ct;
		
		numFlow_count_ = 0; /* reset the periodic counter */
	}
	
	/********************************************************
	 * Test if outer loop (ECF) shall be activated now 
	 ************************************************************/
	if (count_ >= (p_aqmp_.p_ECF_period*count_scale_)) {
		double t = Scheduler::instance().clock();
		if (p_aqmv_.id_num == MONITOR_NODE) {
			printf(">> t=%5.4f, qUDP=%4.3f, ru(k)=%4.3f, cu=%4.3f, pr_udp=%4.3f, taildrops=%d, probDrops=%d, strike_drop=%d\n",
					 t,nqueuedUDP,in_avg_udp,cu,pr_udp,udp_taildrop_,drop_count_,sdrop_count_);	
		}
		avr_ = pr_ave_;         // update for tracing
		
		if (ecf_on_) {
			char out[100];
			double next_r,delta,ratio,new_udp_rate;
		   double next_Rt_RCP;
			/** NEW 150306: to keep a lower queue size on average, set a lower capacity utilization! **/
			cu *= CAP_LIMIT; /* Must be set to e.g. 0.95 for VBR and poisson sources */
			/****************************************/
			eUDP = 0.1*NstarUDP_*(1-flow_ratio) - nqueuedUDP; // Difference in equilibrium and queue, no queue filter
			//eUDP = 0.1*NstarUDP_*(1-flow_ratio) - nqueuedMem; // Difference in equilibrium and queue, with queue filter
			tau1 = rtt_est_/(p_aqmp_.p_updtime+0.0000001); // e.g. 100ms / 40ms = 2.5 . This is tau1.
			tau2 = count_ - tau1; // e.g. 5 - 2.5 = 2.5. This is tau2.
			new_udp_rate = (double)long_udp_count_ / ((double)num_period+0.00000001);
		
			/*
			 * NEW 15.02.05: NEW r(k) estimator, just override old for the time being...
			 */
			if (num_period > 1) /* If the new estimator of r(k) has too short time window, 
										  then num_period <= 1, then use old estimation */
				in_avg_udp = new_udp_rate;

			if (p_aqmv_.id_num == MONITOR_NODE) {
				printf(">> t=%5.4f, eUDP=%4.3f, ru(k)=%4.3f, cu=%4.3f\n",t,eUDP,in_avg_udp,cu);
			}
		
			
			if (tau2 > 0.0) {
				double alpha = 0.4*3.1416*4/(2*2.2361*2.9); /* w_z = 0.5*w_c modified */
				double beta = (2.2361/4)*alpha*alpha; /* w_z = 0.5*w_c modified*/
				alpha = alpha_;
				beta =  beta_;
				if (p_aqmv_.id_num == MONITOR_NODE) {
					printf("alpha=%4.3f\n",alpha);
					printf("beta=%4.3f\n",beta);
				}
				/*
				 * The next line is the original (as of Mar. '07) P-AQM
				 */
				next_r = in_avg_udp + beta*eUDP/(tau1+tau2) + alpha*(cu-in_avg_udp);
				/*
				 * The next line is the original (as of Jan. '06, i.e. post ITC'19) P-AQM
				 */
				//next_r = in_avg_udp + beta*eUDP/tau2 + alpha*(tau1 + tau2)*(cu-in_avg_udp)/tau2;
				/*
				 * The next line is the ITC'19 original (as of Jan. '05) P-AQM, i.e. up to Jan. 06
				 */
				//next_r = (eUDP-(pr_ave_*in_avg_udp-cu)*tau1)/tau2 + cu;

				/*
				 * The next line is the RCP/ERF version of P-AQM (as of Oct. '06)
				 */
				next_Rt_RCP = Rt_RCP + 
					(8.0/p_aqmp_.p_updtime)*(beta*eUDP/(tau2+tau1) + alpha*(cu-in_avg_udp))/(Nt+0.0001);

				if (next_Rt_RCP < 10000.0) {
					printf("next_Rt_RCP below low limit, was %4.3f\n",next_Rt_RCP);
					next_Rt_RCP = 10000.0; // Minimum bit rate is 10kbit/s
				}
				if (Nt <=1.0)
					if (next_Rt_RCP > (cu*(8.0/(p_aqmp_.p_updtime+0.00001))))
						next_Rt_RCP = cu*(8.0/(p_aqmp_.p_updtime+0.000001));
				Rt_RCP = next_Rt_RCP;
				if (p_aqmv_.id_num == MONITOR_NODE) {
					printf("tau1 = %4.3f (# of dT), tau2 = %4.3f (# of dT)\n",tau1,tau2);
				}
			}
			else
				next_r = 1.0;
			
			delta = next_r - in_avg_udp;
			//			if (delta < 0.0) {
			int force_select_MD = 0;
			
			if (delta <= 0.0) { /* then do MD */
				highload_state = 1;
				ratio = next_r / (in_avg_udp + 0.00001);
				//ratio = (1.0+ratio)*0.5; // Smooth the MD (04.02.05)
				if (ratio < 0.5) ratio = 0.5; // Never downscale faster than 50%
				if ((careful_MD==1) && (ratio<0.90)) ratio = 0.90;
				if (ratio > EXT_MD_VAL)
					count_small_MD ++;
				else
					count_small_MD = 0;
				if (count_small_MD > FORCE_EXT_MD_LMT) {
					ratio = EXT_MD_VAL;
					force_select_MD = 1;
					count_small_MD = 0;
				}
				//force_select_MD = 1;
				last_ratio = ratio;
				/*
				  if (p_aqmv_.id_num == MONITOR_NODE) 
					printf(">>NEW ECF MD, ratio=%8.7f, c=%4.3f, ECF=%4.3f \n",
							 ratio,c,pr_ave_);
				*/
				if (force_select_MD==1 && highload_state >= 1) {
					/*if (maxStrike > 1) { */
					if (p_aqmv_.id_num == MONITOR_NODE) 
						printf(">>>>sq_gen %8.7f %d %8.7f %d\n", ratio, 
								 p_aqmv_.id_num,extra_ECF,maxCntIndex);
					sprintf(out, "%s sq_gen %8.7f %d %8.7f %d", name(), ratio, 
							  p_aqmv_.id_num,extra_ECF,maxCntIndex);
				}
				else {
#ifdef RCP_MODE
					if (p_aqmv_.id_num == MONITOR_NODE) 
						printf(">>>>RCP_MODE->Down: R(t+T)=%4.3f(bit/s) ID=%d N(t)=%4.3f\n", next_Rt_RCP, 
								 p_aqmv_.id_num,Nt);
					sprintf(out, "%s sq_gen %8.7f %d %8.7f %d", name(), next_Rt_RCP, 
							  p_aqmv_.id_num,0.0,0);
#else
					if (p_aqmv_.id_num == MONITOR_NODE) 
						printf(">>>>sq_gen %8.7f %d %8.7f %d\n", ratio, 
								 p_aqmv_.id_num,0.0,0);
					sprintf(out, "%s sq_gen %8.7f %d %8.7f %d", name(), ratio, 
							  p_aqmv_.id_num,0.0,0);
#endif
				}
				Tcl& tcl = Tcl::instance();
				tcl.eval(out);
			}
			else if (eUDP > 0){ // else AI (but only only when queue is lower than Nstar (new 020705)
				// HERE is the calculation of excess BW. possible scaled by some proc.
				
				if (qib_)
					//excess_BW = delta * (8.0) / p_aqmp_.p_updtime*0.5;
					excess_BW = delta * (8.0) / (p_aqmp_.p_updtime+0.000001);
				else
					excess_BW = delta * (8.0 * p_aqmp_.p_pktsize) / (p_aqmp_.p_updtime+0.000001)*0.5;
				if (excess_BW < 1.0) excess_BW = 1.0;
				/* if (highload_state >= 1)
					excess_BW *= 0.20;  */
				last_ratio = excess_BW;
				/*
				  if (p_aqmv_.id_num == MONITOR_NODE) 
					printf(">>NEW ECF AI, delta_bps=%8.7f, c=%4.3f, ECF=%4.3f\n",
							 excess_BW,c,pr_ave_);
				*/
				if (force_select_MD==1 && highload_state >= 1) {
					/* if (maxStrike > 1) { */
					if (p_aqmv_.id_num == MONITOR_NODE) 
						printf(">>>>sq_gen %8.7f %d %8.7f %d\n", excess_BW, 
								 p_aqmv_.id_num,extra_ECF,maxCntIndex);
					// Force MD here to punish the source with strike! Set 1.0 i.o. excess_BW below
					// Taken away again, because ill-behaving source will prevent adaptive source regain
					// This liitation is only due to this implementation where a singe call
					// is made to sq_gen with EITHER MD or AI for ALL sources. It should be possible
					// with individual ICMP SQ where low-rates are AI and striked once are MDed.
					sprintf(out, "%s sq_gen %8.7f %d %8.7f %d", name(),excess_BW, 
							  p_aqmv_.id_num,extra_ECF,maxCntIndex);
				}
				else {
#ifdef RCP_MODE
					if (p_aqmv_.id_num == MONITOR_NODE) 
						printf(">>>>RCP_MODE->Up: R(t+T)=%4.3f(bit/s) ID=%d N(t)=%4.3f\n", next_Rt_RCP, 
								 p_aqmv_.id_num,Nt);
					sprintf(out, "%s sq_gen %8.7f %d %8.7f %d", name(), next_Rt_RCP, 
							  p_aqmv_.id_num,0.0,0);
#else
					if (p_aqmv_.id_num == MONITOR_NODE) 
						printf(">>>>sq_gen %8.7f %d %8.7f %d\n", excess_BW, 
								 p_aqmv_.id_num,0.0,0);
					
					sprintf(out, "%s sq_gen %8.7f %d %8.7f %d", name(),excess_BW, 
							  p_aqmv_.id_num,0.0,0);
#endif
				}
				Tcl& tcl = Tcl::instance();
				tcl.eval(out);
			}
			else {
				last_ratio = 1.0; // Means that 0 is AI (or nothing), except if RCP_MODE
#ifdef RCP_MODE
				if (p_aqmv_.id_num == MONITOR_NODE) 
					printf(">>>>RCP_MODE->Stay: R(t+T)=%4.3f(bit/s) ID=%d N(t)=%4.3f\n", next_Rt_RCP, 
							 p_aqmv_.id_num,Nt);
				sprintf(out, "%s sq_gen %8.7f %d %8.7f %d", name(), next_Rt_RCP, 
						  p_aqmv_.id_num,0.0,0);
				Tcl& tcl = Tcl::instance();
				tcl.eval(out);
#else
				if (p_aqmv_.id_num == MONITOR_NODE) 
					printf(">>No NEW ICMP SQ: keep as is!\n");
#endif
			}
			if (p_aqmv_.id_num == MONITOR_NODE) 
				printf("\n");
			
		}
		Nt = (8.0/p_aqmp_.p_updtime) * cu / (Rt_RCP+0.00001);
		//Nt = 20; // for testing purposes 10.04.07!!
		if (Nt < 1.0)
			Nt = 1.0;
		long_udp_count_ = 0;
		num_period = 0;
		count_ = 0; /* reset the periodic counter */
	} /* End ECF update */
	
	ping_count_ ++; /* Update counetr for ping packets to probe for RTTs */
	if (ping_count_ >= p_aqmp_.p_ping_period) {
		char out[100];
		ping_count_ = 0;
		sprintf(out, "%s ping_gen %d", name(), p_aqmv_.id_num);
		Tcl& tcl = Tcl::instance(); 
		if (ecf_on_)
			tcl.eval(out);		
	}
	
	/*
	 * Debugging section
	 */
	/* printf("calculated input rate is: %f \n",in_avg);
		printf("calculated output capacity is: %f \n",c);
		printf("given equilibrium is: %f \n",p_aqmp_.p_Nstar);
		printf("packets in queue: %f \n",nqueued);
		printf("Difference in queue is: %f \n",e);
		printf("Drop probability calculated is: %f \n\n",pr); */
	
	/* Update variables before next update interval */
	p_aqmv_.v_prob = pr;    // Update probability of dropping/marking (used by enque/deque)
	p_aqmv_.v_count = 0.0;  // Reset the counting
	udp_count_ = 0; // Reset UDP counting
	p_aqmv_.v_ave = in_avg; // Update average for filtering of input rate average
	p_aqmv_.v_ave_udp = in_avg_udp; // Update average for filtering of UDP input rate average
	
	prob_udp = pr_udp;    // Update probability of dropping/marking (used by enque/deque)
	p_aqmv_.v_pr_ave = pr_ave_;    // Update probability averaging memory
	
}


/*
 * Return the next packet in the queue for transmission. It may be ECN marked
 * if ECN is enabled with true in markpkts_ Includes also the FQ round robin scheduler.
 */
Packet* PaqmQueue::deque()
{
	hdr_cmn* ch;
	
	//	queueToggle_ = 1;
	double q_rnd = Random::uniform();
	//	printf("flow_ratio = %4.3f\n",flow_ratio);
	//	if (queueToggle_ == 0) { // If yes, then UDP packet shall be transmitted
	if (toggleFQ[tFQ_cnt++] == 0) { // If yes, then UDP packet shall be transmitted
		if (tFQ_cnt >= tFQ_size) {
			tFQ_cnt = 0;
		}
		
		// if (q_rnd >= flow_ratio) {
		// printf("tFQ is 0, tFQ_cnt = %d, tFQ_size = %d\n",tFQ_cnt,tFQ_size);
		Packet *pUDP = qUDP_->deque();
		if (pUDP != 0) {
			ch = hdr_cmn::access(pUDP);
			bcountUDP_ -= ch->size();
			double qlen = qib_ ? bcountUDP_ : qUDP_->length();
			curqUDP_ = (int) qlen;
			queueToggle_ = 1;
			//printf("UDP packet size %d\n",ch->size());
		}
		else {
			// printf("Empty UDP Queue, try TCP \n");
			Packet *p = q_->deque();
			if (p != 0) {
				ch = hdr_cmn::access(p);
				bcount_ -= ch->size();
				
				if (markpkts_) {
					double u = Random::uniform();
					double pro = p_aqmv_.v_prob;
					if (qib_) { /* adjust probability with relative size of packet */
						// NEW 17.02.05: decide to take this away, all packets equal drop prob
						//pro = pro*ch->size()/p_aqmp_.p_pktsize;
					}
					if ( u <= pro ) { /* if yes, mark the CE bit... */
						hdr_flags* hf = hdr_flags::access(p);
						/* hm, ns-2 is using obsolete ECN flags (RFC 2481, see RFC 3168 p. 7) */
						//						if(hf->ect() == 1) {
						/* ---  Altered to support the codepoints in rfc3168 --- */
						if(hf->ect() != hf->ce() || // ECN capable
							hf->ect() == 1 && hf->ce() == 1) {  //marked
							hf->ce() = 1; // Set ce codepoint
							hf->ect() = 1;
							
							pmark_++;
						}
					}
				}
				double qlen = qib_ ? bcount_ : q_->length();
				curq_ = (int) qlen;
				queueToggle_ = 0; // I suppose already set to 0?
			}
			return (p);
		}
		return (pUDP);
	}
	else {
		if (tFQ_cnt >= tFQ_size)
			tFQ_cnt = 0;
		
		// printf("tFQ is 1, tFQ_cnt = %d, tFQ_size = %d\n",tFQ_cnt,tFQ_size);
		Packet *p = q_->deque();
		if (p != 0) {
			ch = hdr_cmn::access(p);
			bcount_ -= ch->size();
			
			if (markpkts_) {
				double u = Random::uniform();
				double pro = p_aqmv_.v_prob;
				if (qib_) { /* adjust probability with relative size of packet */
					//pro = pro*ch->size()/p_aqmp_.p_pktsize;
				}
				if ( u <= pro ) { /* if yes, mark the CE bit... */
					hdr_flags* hf = hdr_flags::access(p);
					/* hm, ns-2 is using obsolete ECN flags (RFC 2481, see RFC 3168 p. 7) */
					/* ---  Altered to support the codepoints in rfc3168 --- */ 
					if(hf->ect() != hf->ce() || // ECN capable
						hf->ect() == 1 && hf->ce() == 1) {  //marked
						hf->ce() = 1; // Set ce codepoint
						hf->ect() = 1;
						
						//					if(hf->ect() == 1) {
						//						hf->ce() = 1;
						pmark_++;
					}
				}
			}
			double qlen = qib_ ? bcount_ : q_->length();
			curq_ = (int) qlen;
			queueToggle_ = 0;
		}
		else {
			// printf("Empty TCP, try UDP instead\n");
			Packet *pUDP = qUDP_->deque();
			if (pUDP != 0) {
				ch = hdr_cmn::access(pUDP);
				bcountUDP_ -= ch->size();
				double qlen = qib_ ? bcountUDP_ : qUDP_->length();
				curqUDP_ = (int) qlen;
				queueToggle_ = 1;
			}
			return (pUDP);
		}
		return (p);
	}	
}

/*
 * Receive a new packet arriving at the queue.
 * The packet is dropped if the maximum queue size is exceeded.
 * Includes the type of packet divider (into two different queues)
 */
void PaqmQueue::enque(Packet* pkt)
{
	hdr_cmn* ch = hdr_cmn::access(pkt);
	double qlen;
	hdr_ip* myip = hdr_ip::access(pkt);
	int source = myip->saddr(); // This packet comes from src source (i.e. node)
	int tos = myip->prio();
	
	// printf("TOS field of this packet is %d\n",tos);
	double qlim = qib_ ? (qlim_*p_aqmp_.p_pktsize) : qlim_ ;
	/*
	 * NEW 02.02.05: two queues, one special for UDP!
	 */
	packet_t typo;
	typo = ch->ptype();
	if ((typo != PT_UDP) && (typo != PT_CBR) && (typo != PT_PARETO) && (typo != PT_ICMP) && (typo != PT_VIDEO) ) {
		if (source < FAIR_TABLE_MAX) {
			if (typo == PT_TCP) {
				flow_table_tcp[source] = 1; // Set flag in table to count this flow
				flow_table_cnt_tcp[source] += 1; // count numb of packets this flow
			}
		}
		else {
			printf("WARNING: too many sources, FAIR_TABLE_MAX should be expanded!\n");
		}
		// NOW we know that the packet is neither UDP nor CBR
		if (qib_) {
			p_aqmv_.v_count += ch->size(); /* increment by number of bytes */
		}
		else {
			++p_aqmv_.v_count;             /* increment by one more packet */
		}
		
		q_ -> enque(pkt);
		bcount_ += ch->size(); /* increment byte-count by size of incoming packet */
		
		qlen = qib_ ? bcount_ : q_->length();
		
		if (qlen >= qlim) {
			q_->remove(pkt);
			bcount_ -= ch->size();
			tcp_drop_count_ ++;
			drop(pkt); /* This is tail-dropping due to buffer overflow */
		}
		else { // TCP can be stoch. dropped if ECN marking is not enabled
			if (!markpkts_ ) {
				if (typo != PT_ICMP) { /* added this test to avoid dropping ICMP */
					double u = Random::uniform();
					double pro = p_aqmv_.v_prob;
					if (qib_) {
						// Adjust the probability by relative size of packet!!
						//pro = p_aqmv_.v_prob*ch->size()/p_aqmp_.p_pktsize;
					}
					if ( (u <= pro) ) {
						printf("TCP stoch. drop!\n");
						tcp_drop_count_ ++;
						q_->remove(pkt);
						bcount_ -= ch->size();
						drop(pkt);
					}
				}
			}
		}
		qlen = qib_ ? bcount_ : q_->length();
		curq_ = (int) qlen;
	}
	else { // it is CBR or UDP or ICMP...
		double delta_time = Scheduler::instance().clock() - remember_time;
		if (source<FAIR_TABLE_MAX) {
			if (typo != PT_ICMP) {
				flow_ECF_tag[source] = tos;
				flow_table_udp[source] = 1; // Set flag in table to count this flow
				flow_table_cnt_udp[source] += 1; // count numb of packets this flow. SHOULD BE in BYTES!
				flow_table_cntAll_udp[source] += 1; // count numb of packets this flow. SHOULD BE in BYTES!
			}
		}
		else {
			printf("WARNING: too many sources, FAIR_TABLE_MAX should be expanded!\n");
		}
		
		fprintf(fout,"%8.7f   %d\n",delta_time,ch->size());
		remember_time = Scheduler::instance().clock();
		if (qib_) {
			udp_count_ += ch->size(); /* increment by number of bytes */
			/* Note that this size is IP + UDP + payload length */
			//printf("packet size = %d\n",ch->size());
		}
		else {
			++udp_count_;             /* increment by one more packet */
		}
		
		qUDP_ -> enque(pkt);
		//printf("size of packet: %d\n",ch->size());
		bcountUDP_ += ch->size(); /* increment byte-count by size of incoming packet */
		qlen = qib_ ? bcountUDP_ : qUDP_->length();
		if (qlen >= qlim) {
			qUDP_->remove(pkt);
			bcountUDP_ -= ch->size();
			udp_taildrop_ ++;
			drop(pkt); /* This is tail-dropping due to buffer overflow */
		}
		else if (strike_table[source] > 2) {
			/* printf("Too many Strike for source %d, drop with prob=%4.3f\n",
				source,drop_table[source]); */
			double u = Random::uniform();
			double pro = drop_table[source];
			if (qib_) {
				// Adjust the probability by relative size of packet!!
				//pro = pro*ch->size()/p_aqmp_.p_pktsize;
			}
			if (drop_loop_size[source] > 0) {
				drop_loop_cnt[source]++;
				// printf("drop_loop_cnt=%d\n",drop_loop_cnt[source]);
				
				int do_drop = 0; // 0 = False
				if (drop_inverse[source]==0) {
					if ((drop_loop_cnt_tenth[source]>=10) && (drop_loop_size_tenth[source]>0)) {
						drop_inner_cnt_tenth[source]++;
						if (drop_inner_cnt_tenth[source] >= drop_loop_size_tenth[source]) {
							drop_inner_cnt_tenth[source] = 0;
							drop_loop_cnt_tenth[source] = 0;
							drop_loop_cnt[source] = 0;
							// if (source==2) printf("Here is loop-shift...\n");
						}
						
					}
					else if ((drop_loop_cnt[source] >= drop_loop_size[source])) {
						drop_loop_cnt_tenth[source]++;
						drop_loop_cnt[source] = 0;	
						do_drop = 1;
					}
				}
				else {
					do_drop = 1;
					if ((drop_loop_cnt_tenth[source]>=10) && (drop_loop_size_tenth[source]>0)) {
						drop_inner_cnt_tenth[source]++;
						if (drop_inner_cnt_tenth[source] >= drop_loop_size_tenth[source]) {
							drop_inner_cnt_tenth[source] = 0;
							drop_loop_cnt_tenth[source] = 0;
							drop_loop_cnt[source] = 0;
							// printf("Here is loop-shift...\n");
						}
						
					}
					else if ((drop_loop_cnt[source] >= drop_loop_size[source])) {
						drop_loop_cnt_tenth[source]++;
						drop_loop_cnt[source] = 0;	
						do_drop = 0;
					}
				}
				/* if (source==2)
					printf("do_drop=%d\n",do_drop);  */
				if (do_drop == 1) {
					// printf("Strike-Drop with prob=%4.3f\n",pro);
					
					sdrop_count_ ++;
					flow_table_cnt_udp[source] -= 1; // do not count this dropped packet in packet count!
					bcountUDP_ -= ch->size();
					/* Remove dropped packets from ru(t) counter since this dropping
						is supposed performed on non-adaptive sources! */
					if (qib_) {
						udp_count_ -= ch->size(); /* remove from packet (rate) count */
					}
					else {
						udp_count_--;             /* remove from packet (rate) count */
					}
					qUDP_->remove(pkt);
					drop(pkt);
				}
			}
		}
		else if (typo != PT_ICMP) {
			/* hm, ns-2 is using obsolete ECN flags (RFC 2481, see RFC 3168 p. 7) */
			double u = Random::uniform();
			double pro = prob_udp;
			if (qib_) {
				// Adjust the probability by relative size of packet!!
				//pro = pro*ch->size()/p_aqmp_.p_pktsize;
			}
			if ( (u <= pro) ) { 
				drop_count_ ++;
				flow_table_cnt_udp[source] -= 1; // do not count this dropped packet in packet count!
				qUDP_->remove(pkt);
				bcountUDP_ -= ch->size();
				drop(pkt);
			}
		}
		qlen = qib_ ? bcountUDP_ : qUDP_->length();
		curqUDP_ = (int) qlen;
		
	}    
}


int PaqmQueue::command(int argc, const char*const* argv)
{
	
	Tcl& tcl = Tcl::instance();
	if (argc == 2) {
		if (strcmp(argv[1], "reset") == 0) {
			reset();
			return (TCL_OK);
		}
	}
	else if (argc == 3) {
		// attach a file for variable tracing
		if (strcmp(argv[1], "attach") == 0) {
			int mode;
			const char* id = argv[2];
			tchan_ = Tcl_GetChannel(tcl.interp(), (char*)id, &mode);
			if (tchan_ == 0) {
				tcl.resultf("Paqm: trace: can't attach %s for writing", id);
				return (TCL_ERROR);
			}
			return (TCL_OK);
		}
		// tell Paqm about link stats
		if (strcmp(argv[1], "link") == 0) {
			LinkDelay* del = (LinkDelay*)TclObject::lookup(argv[2]);
			if (del == 0) {
				tcl.resultf("Paqm: no LinkDelay object %s",
								argv[2]);
				return(TCL_ERROR);
			}
			// set ptc now
			link_ = del;
			if (qib_) {
				p_aqmp_.p_ptc = link_->bandwidth() / (8.0); // capacity in Bytes per sec
			}
			else
				p_aqmp_.p_ptc = link_->bandwidth() / (8.0 * p_aqmp_.p_pktsize);
			
			
			// printf("link_->bandwidth()=%4.3f\n",link_->bandwidth());
			return (TCL_OK);
		}
		if (!strcmp(argv[1], "packetqueue-attach")) {
			delete q_;
			if (!(q_ = (PacketQueue*) TclObject::lookup(argv[2])))
				return (TCL_ERROR);
			else {
				pq_ = q_;
				return (TCL_OK);
			}
		}
		if (strcmp(argv[1], "set_id") == 0) {
			const char* id_c = argv[2];
			
			p_aqmv_.id_num = atoi(id_c);
			return (TCL_OK);
		}
	}
	else if (argc == 4) {
		if (strcmp(argv[1], "send_rttupd_aqm") == 0) {
			const char* rtt_c = argv[2];
			const char* rtt_maxc = argv[3];
			// float rtt_f;
			rtt_est_ = (double) atof(rtt_c);
			rtt_max_ = (double) atof(rtt_maxc);
			//float new_period = (float)(1.25*rtt_est_/p_aqmp_.p_updtime);
			// double new_period = (double)(rtt_est_/p_aqmp_.p_updtime) + 2;
			//doble new_period = (double)(2.5*rtt_est_/p_aqmp_.p_updtime);
			double new_period = (double)(3.0 + rtt_est_/p_aqmp_.p_updtime); // at least three periods with new rate
			if ((new_period - rtt_max_/p_aqmp_.p_updtime) < 3) {
				new_period = rtt_max_/p_aqmp_.p_updtime + 3;
				//printf("New new_period used, rtt_max=%6.5f!\n",rtt_max_);
			}
			/* the test below keeps count_scale_ >= 1 */
			//			if ( new_period > p_aqmp_.p_ECF_period) {
			count_scale_ = new_period / p_aqmp_.p_ECF_period;
			if (count_scale_ < 1.0) count_scale_ = 1.0;
			//printf("count_scale=%5.4f\n",count_scale_);
			//			}
			return (TCL_OK);
		}
		
	}
	return (Queue::command(argc, argv));
}

/*
 * Routine called by TracedVar facility when variables change values.
 * Currently used to trace values of avg queue size, drop probability,
 * and the instantaneous queue size seen by arriving packets.
 * Note that the tracing of each var must be enabled in tcl to work.
 */

void PaqmQueue::trace(TracedVar* v)
{
	char wrk[500];
	const char *p;
   
	if (((p = strstr(v->name(), "ave_")) == NULL) &&
		 ((p = strstr(v->name(), "avr_")) == NULL) &&
		 ((p = strstr(v->name(), "prob_")) == NULL) &&
		 ((p = strstr(v->name(), "2probUDP_")) == NULL) &&
		 ((p = strstr(v->name(), "3curqUDP_")) == NULL) &&
		 ((p = strstr(v->name(), "curq_")) == NULL)) {
		fprintf(stderr, "Paqm: unknown trace var %s\n",
				  v->name());
		return;
	}
	
	if (tchan_) {
		int n;
		double t = Scheduler::instance().clock();
		
		if (*p == 'c') {
			sprintf(wrk, "Q %g %d", t, int(*((TracedInt*) v)));
		} 
		else if (*p == '3') {
			sprintf(wrk, "3 %g %d", t, int(*((TracedInt*) v)));
		} 
		else {
			sprintf(wrk, "%c %g %g", *p, t,
					  double(*((TracedDouble*) v)));
		}
		n = strlen(wrk);
		wrk[n] = '\n';
		wrk[n+1] = 0;
		(void)Tcl_Write(tchan_, wrk, n+1);
	}
	return;
}

/* for debugging help */
void PaqmQueue::print_p_aqmp()
{
	printf("=========\n");
}

void PaqmQueue::print_p_aqmv()
{
	printf("=========\n");
}
