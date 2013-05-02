/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 ************************************************************************
 * THIS PROPORTIONAL AQM (p_aqm) IMPLEMENTATION IS BASED ON THE REM 
 * IMPLEMENTATION IN NS-2. IT HAS MAY SIMILARITIES IN THE ALGORTIHM, SO
 * THAT IS WHY IT WAS SMART TO START WITH REM AS A "TEMPLATE". HOWEVER,
 * REM AND P-AQM IS NOT THE SAME ALGORITHM.
 ************************************************************************
 * Author: Arne Lie
 * Dept. of Telematics
 * Norwegian Univeristy of Science and Technology -- NTNU
 * N-7491 Trondheim
 * NORWAY
 * Work started autumn 2004
 */

#ifndef ns_p_aqm_h
#define ns_p_aqm_h

#include "queue.h"
#include <stdlib.h>
#include "agent.h"
#include "template.h"

class LinkDelay;

class PaqmQueue;

/*
 * Paqm parameters, supplied by user
 */
struct p_aqmp {
	/*
	 * User supplied.
	 */
	double p_inw;	     /* queue weight given to cur q size sample */
	double p_Kgain;      /* the proportional gain */
	double p_KgainUDP;      /* the proportional gain */
	double p_phi;        /* unused */
	double p_delta;      /* unused */
	double p_pktsize;    /* assumed averaged packet size (?) */
	double p_updtime;    /* update time of the loop */
	double p_Nstar;      /* the equilibrium point */
	double p_ECF_period; /* how many P-AQM loop periods between each ECF */
	double p_TCP_period; /* how many P-AQM loop periods between each numTCP_ estimation */
	double p_numb_adapt_UDP; /* enable knowledge of which UDPs that do not adapt to ECF */
	double p_ping_period; /* how many P-AQM loop periods between each ping */
	/*
	 * Computed as a function of user supplied parameters.
	 */
	double p_ptc; /* max number of pkts per second which can be placed on link */

};

/*
 * Paqm variables, maintained by Paqm
 */
struct p_aqmv {
	TracedDouble v_pl;	/* link price */
	TracedDouble v_prob;	/* prob. of TCP packet marking. */
	double v_in;	        /* used in computing the input rate */
	double v_ave;
	double v_ave_udp; /* added 26.01.05 to filter the udp counting */
	double v_count;         /* count incoming packets (or bytes) during one interval */
	double v_pl1;
	double v_pl2;
	double v_in1;
	double v_in2;
	float v_pr_ave;
	int id_num;

	p_aqmv() : v_pl(0.0), v_prob(0.0), v_in(0.0), v_ave(0.0),	v_count(0.0){ }
};

class PaqmTimer : public TimerHandler {
	public:
		PaqmTimer (PaqmQueue *a) : TimerHandler() { a_ = a; }
	protected:
		virtual void expire (Event *e);
		PaqmQueue *a_;
};

class PaqmQueue : public Queue {
 public:
	PaqmQueue();
	void set_update_timer();
	void timeout();
	float pr_ave_;
 protected:
	int command(int argc, const char*const* argv);
	void enque(Packet* pkt);
	Packet* deque();
	void reset();
	void run_updaterule();

	LinkDelay* link_;	/* outgoing link */
	PacketQueue *q_; 	/* underlying (usually) FIFO queue */
	PacketQueue *qUDP_; 	/* NEW 02.02.05: another underlying (usually) FIFO queue */

	Tcl_Channel tchan_;		 /* Place to write trace records */
	TracedInt curq_;	/* current qlen seen by arrivals */
	TracedInt curqUDP_;	/* current qlenUDP seen by UDP/CBR arrivals, NEW 02.02.05 */
	TracedDouble avr_;      /* for tracing average prob. of drop/mark */
	TracedDouble prob_udp;	/* prob. of UDP packet dropping */
	void trace(TracedVar*);	/* routine to write trace records */

	PaqmTimer p_aqm_timer_;

	/*
	 * Static state.
	 */
	double pmark_;	 //number of packets being marked
	p_aqmp p_aqmp_;	/* early-drop params */

	/*
	 * Dynamic state.
	 */
	p_aqmv p_aqmv_;		/* early-drop variables */

	int markpkts_ ;
	int bcount_;
	int nqueuedMem; /* New 250106: try to filter queue length for ECF algorithm */
	int bcountUDP_; /* NEW 02.02.05 */
	int qib_;
	int ecf_on_;
	int numUDP_;    /* NEW 02.02.05 */
	int numTCP_;    /* NEW 02.02.05 */
	double avr_cnt_udp; /* New 28.07.05 */
	/*
	 * Added by Arne Lie in Dec. 2004
	 */
	int count_;
	int numFlow_count_;
	double eUDP;
	int maxStrike;
	int maxStrikeIndex;

	double count_scale_ ;
	double tau1, tau2; // used for outer loop control (ECF) timing constants
	int long_udp_count_; // used for calculating input rate for outer loop (and also inner loop?)
	int count_small_MD; // this one is used to increase fair rate adaptation speed
	int num_period;
	int ping_count_;
	int drop_count_;
	int sdrop_count_;
	int udp_taildrop_;
	int tcp_drop_count_;
	int mark_spacer_;
#define SPACER_VAL 0
	int udp_count_;
	double rtt_est_; // New in 22.01.05: make rtt estimation available for calc. of increment
	double rtt_max_; // New in 17.02.05: hold last maximum RTT
	int NstarUDP_; // New 27.01.05: enable a pre-dynamic setting on the equilibrium for UDP
	int queueToggle_ ;
#define MAX_FQ_TABLE_SIZE 1000
	int toggleFQ[MAX_FQ_TABLE_SIZE]; // New 050805: queue toggle shall be FQ
	int tFQ_cnt;                     // Used as pointer in toggleFQ
	int tFQ_size;                    // Will be set dynamically to the sum of numTCP_ + numUDP_
	double remember_time;
#define FAIR_TABLE_MAX 1600
	// New 03.07.05: first impl. of flow counter. NB req. one flow per node
	int flow_table_tcp[FAIR_TABLE_MAX]; 
	// New 04.07.05: first impl. of flow counter. NB req. one flow per node
	int flow_table_cnt_tcp[FAIR_TABLE_MAX]; // If zombie list version: used for Total Occurance
	// New 03.07.05: first impl. of flow counter. NB req. one flow per node
	int flow_table_udp[FAIR_TABLE_MAX]; 
	// New 04.07.05: first impl. of flow counter. NB req. one flow per node
	int flow_table_cnt_udp[FAIR_TABLE_MAX]; // If zombie list version: used for Total Occurance
	int save_flow_table_cnt_udp[FAIR_TABLE_MAX]; // If zombie list version: used for Total Occurance
	int strike_taken[FAIR_TABLE_MAX];
	int flow_ECF_tag[FAIR_TABLE_MAX];  // Store the tag of this flow (1=ECF enabled)
	int flow_table_cntAll_udp[FAIR_TABLE_MAX]; // Counts all packets including the StrikeDropped ones
	//int flow_table_cnt_HR_udp[FAIR_TABLE_MAX]; // Counts all packets belonging to HighRate flows
	//int flow_table_cnt_LR_udp[FAIR_TABLE_MAX]; // Counts all packets belonging to LowRate flows
	int flow_table_last_cntAll[FAIR_TABLE_MAX];
	int tcp_mice_count[FAIR_TABLE_MAX];
	int class_HR_or_LR[FAIR_TABLE_MAX];
	int drop_loop_size[FAIR_TABLE_MAX];
	int drop_loop_cnt[FAIR_TABLE_MAX];
	int drop_inverse[FAIR_TABLE_MAX];
	int drop_loop_cnt_tenth[FAIR_TABLE_MAX];
	int drop_loop_size_tenth[FAIR_TABLE_MAX];
	int drop_inner_cnt_tenth[FAIR_TABLE_MAX];
	int sameCnt;
	int maxCntIndex;
	int lastMaxCntIndex;
	int maxThisECF;
	double extra_ECF;
	int numTCP_store;
	int last_state_is_mice;
	int this_state_is_mice;
	double last_ratio;
	int careful_MD;
	int strike_table[FAIR_TABLE_MAX];  // New 050805: used to combat ill-behaving sources
	double drop_table[FAIR_TABLE_MAX]; // and increased adaptation speed.
	double last_drop_table[FAIR_TABLE_MAX]; // and increased adaptation speed.
	// New 030705: Used by the dequeue scheduler
	double flow_ratio; 
	double pktRatio;
	int highload_state;
	double Rt_RCP;  /* added 10.09.06 to modifiy to RCP ideas, store last time rate */
	double Nt;      /* added 11.09.06 to hold the number of estimated sources */
	double alpha_; /* added 17.02.07 */
	double beta_;  /* added 17.02.07 */

// New 290705: Zombie variables. See SRED paper of Ott et al.
// #define ZOMBIE
// #define TCP_ZOMBIE
#define TCP_LOOP_SIZE 5
	int long_tcp_count;
	int long_numTCP_;
	double filtered_numTCP_;
#ifdef TCP_ZOMBIE
	int zombie_state_tcp; // 0 at initial build-up of zombie list, 1 after. 
	int zombie_state_udp; // 0 at initial build-up of zombie list, 1 after. 
	int zombie_init_tcp_ptr; // used to sequentially fill the list at init phase
	int zombie_init_udp_ptr; // used to sequentially fill the list at init phase
#define ZOMBIE_SIZE_M 100 
	int zombie_list_tcp[ZOMBIE_SIZE_M][2]; // Each row have two fields: source (i.e. flow ID) and Count
	int zombie_list_udp[ZOMBIE_SIZE_M][2]; // Each row have two fields: source (i.e. flow ID) and Count
	double zombie_p; // The probability p of overwriting candidate in list if No Hit.
	double zombie_alpha; // The alpha factor in eq. (3.2)
	double zombie_Pt_tcp; // The variable P(t) eq. (3.2) which inverse gives est. of # of flows
	double zombie_Pt_udp; // The variable P(t) eq. (3.2) which inverse gives est. of # of flows
	int zombie_hit_tcp;
	int zombie_hit_udp;
	double zombie_numTCP_;
	double zombie_numUDP_;
#endif
	// class IcmpAgent *icmp_ptr_;

	void print_p_aqmp();	// for debugging
	void print_p_aqmv();	// for debugging
};


#endif
