// Copyright (c) 2000 by the University of Southern California
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software and its
// documentation in source and binary forms for non-commercial purposes
// and without fee is hereby granted, provided that the above copyright
// notice appear in all copies and that both the copyright notice and
// this permission notice appear in supporting documentation. and that
// any documentation, advertising materials, and other materials related
// to such distribution and use acknowledge that the software was
// developed by the University of Southern California, Information
// Sciences Institute.  The name of the University may not be used to
// endorse or promote products derived from this software without
// specific prior written permission.
//
// THE UNIVERSITY OF SOUTHERN CALIFORNIA makes no representations about
// the suitability of this software for any purpose.  THIS SOFTWARE IS
// PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Other copyrights might apply to parts of this software and are so
// noted when applicable.
//
// $Header: /nfs/jade/vint/CVSROOT/ns-2/apps/ping.cc,v 1.6 2003/09/02 22:07:20 sfloyd Exp $

/*
 * File: Code for a new 'Icmp' Agent Class for the ns
 *       network simulator
 * Author: Arne Lie, NTNU, Dec. 2004
 * Based on the Ping application agent made by:
 * Author: Marc Greis (greis@cs.uni-bonn.de), May 1998
 *
 */

#include "icmp.h"


int hdr_icmp::offset_;
static class IcmpHeaderClass : public PacketHeaderClass {
public:
	IcmpHeaderClass() : PacketHeaderClass("PacketHeader/Icmp",
					      sizeof(hdr_icmp)) {
		bind_offset(&hdr_icmp::offset_);
	}
} class_icmphdr;


static class IcmpClass : public TclClass {
public:
	IcmpClass() : TclClass("Agent/Icmp") {}
	TclObject* create(int, const char*const*) {
		return (new IcmpAgent());
	}
} class_icmp;


IcmpAgent::IcmpAgent() : Agent(PT_ICMP), seq(0), oneway(0)
{
	bind("packetSize_", &size_);
	//printf("In IcmpAgent constructor \n");
}

int IcmpAgent::command(int argc, const char*const* argv)
{
  if (argc == 3) {
    if (strcmp(argv[1], "send_ping") == 0) {
      // Create a new packet
      Packet* pkt = allocpkt();
      // Access the ICMP header for the new packet:
      hdr_icmp* hdr = hdr_icmp::access(pkt);
      // Set the 'ret' field to 0, so the receiving node
      // knows that it has to generate an echo packet
      hdr->type = ICMP_ECHO_TYPE;
      hdr->code = 0;
      hdr->ret = 0;  // This should be obsolete by now...
      hdr->seq = seq++;
      const char* ping_id_c = argv[2];
      int ping_id_i;
      ping_id_i = atoi(ping_id_c);
      hdr->identifier = ping_id_i;
      // Store the current time in the 'send_time' field
      hdr->send_time = Scheduler::instance().clock();
      // Send the packet
      send(pkt, 0);
      // return TCL_OK, so the calling function knows that
      // the command has been processed
      return (TCL_OK);
    
    }
    else if (strcmp(argv[1], "send_sq") == 0) {
      //printf("In IcmpAgent::command is SEND ECF, creating ICMP SQ \n");
      // Create a new packet
      Packet* pkt = allocpkt();
      // Access the Icmp header for the new packet:
      hdr_icmp* hdr = hdr_icmp::access(pkt);
      // Set the 'ret' field to 0, so the receiving node
      // knows that it has to generate an echo packet
      hdr->ret = 0; // This field is a "survivor" from PING app., not used
      hdr->seq = seq++; // This field is also a survivor, both could be deleted if deleted from recv()
      // Store the current time in the 'send_time' field
      hdr->send_time = Scheduler::instance().clock(); // Survivor too, but could be useful...
      /*
       *  Set the ICMP fields correctly for Source Quench
       */
      hdr->type = (char) ICMP_SOURCE_QUENCH_TYPE;
      hdr->code = (char) 0;
      const char* ecf_c = argv[2];
      double ecf_f;
      ecf_f = (double) atof(ecf_c);
      // printf("ECF is in char: %s; and in float: %5.4f\n",ecf_c,ecf_f);
      hdr->checksum = (short int) 0; // Sould have correct CRC if correct ICMP implementation
      hdr->unused =  ecf_f; // HERE is the P-AQM avr value to be put!! 
      // Send the packet
      send(pkt, 0);
      // return TCL_OK, so the calling function knows that
      // the command has been processed
      return (TCL_OK);

    }
  }
  // If the command hasn't been processed by IcmpAgent()::command,
  // call the command() function for the base class
  return (Agent::command(argc, argv));
}


void IcmpAgent::recv(Packet* pkt, Handler*)
{
  // Access the IP header for the received packet:
  // printf("In IcmpAgent::recv at top \n");
  hdr_ip* hdrip = hdr_ip::access(pkt);

  // Access the Icmp header for the received packet:
  hdr_icmp* hdr = hdr_icmp::access(pkt);

  if (hdr->type == ICMP_ECHO_TYPE) {
    // Send an 'echo'. First save the old packet's send_time
    double stime = hdr->send_time;
    int rcv_seq = hdr->seq;

    // Create a new packet
    Packet* pktret = allocpkt();
    // Access the Ping header for the new packet:
    hdr_icmp* hdrret = hdr_icmp::access(pktret);
    // Set the 'ret' field to 1, so the receiver won't send
    // another echo
    hdrret->type = ICMP_ECHO_REPLY_TYPE;
    hdrret->ret = 1; // Should be obsolete by now...
    // Set the send_time field to the correct value
    hdrret->send_time = stime;
    // Added by Andrei Gurtov for one-way delay measurement.
    hdrret->rcv_time = Scheduler::instance().clock();
    hdrret->seq = rcv_seq;
    hdrret->identifier = hdr->identifier;
    // Discard the old packet
    Packet::free(pkt);
    // Send the new packet
    send(pktret, 0);
  } 
  else if (hdr->type == ICMP_ECHO_REPLY_TYPE) {
    // A packet was received. Use tcl.eval to call the Tcl
    // interpreter with the ping results.
    // Note: In the Tcl code, a procedure
    // 'Agent/Ping recv {from rtt}' has to be defined which
    // allows the user to react to the ping result.
    char out[100];
    // Prepare the output to the Tcl interpreter. Calculate the
    // round trip time
    if (oneway) //AG
      	sprintf(out, "%s ping_recv %d %d %3.1f %3.1f", name(), 
	    hdrip->src_.addr_ >> Address::instance().NodeShift_[1],
	    hdr->seq, (hdr->rcv_time - hdr->send_time) * 1000,
	    (Scheduler::instance().clock()-hdr->rcv_time) * 1000);
    else sprintf(out, "%s ping_recv %d %3.1f %d", name(), 
	    hdrip->src_.addr_ >> Address::instance().NodeShift_[1],
		 (Scheduler::instance().clock()-hdr->send_time) * 1000,hdr->identifier);
    Tcl& tcl = Tcl::instance();
    tcl.eval(out);
    // Discard the packet
    Packet::free(pkt);
  }
  else if ((hdr->type==ICMP_SOURCE_QUENCH_TYPE) && (hdr->code == 0)) {  
    // A packet was received. Use tcl.eval to call the Tcl
    // interpreter with the icmp results.
    // Note: In the Tcl code, a procedure
    // 'Agent/Icmp recv {from rtt}' has to be defined which
    // allows the user to react to the icmp result.
    char out[100];
    
    // Prepare the output to the Tcl interpreter.
    
    sprintf(out, "%s sq_recv %d %3.1f %d %d %8.7f", name(),
	    hdrip->src_.addr_ >> Address::instance().NodeShift_[1],
	    (Scheduler::instance().clock()-hdr->send_time) * 1000,
	    hdr->type,hdr->code,hdr->unused);
    Tcl& tcl = Tcl::instance();
    tcl.eval(out);
    // Discard the packet
    Packet::free(pkt);
  }  
}


