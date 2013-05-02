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
// $Header: /nfs/jade/vint/CVSROOT/ns-2/apps/ping.h,v 1.4 2003/09/02 22:07:20 sfloyd Exp $

/*
 * File: Header File for a new 'ICMP' Agent Class for the ns
 *       network simulator. Only Source Quench implemented.
 * Author: Arne Lie (arne.lie@item.ntnu.no), Dec. 2004
 * based on ping implementation of:
 * Author: Marc Greis (greis@cs.uni-bonn.de), May 1998
 *
 */


#ifndef ns_icmp_h
#define ns_icmp_h

#include "agent.h"
#include "tclcl.h"
#include "packet.h"
#include "address.h"
#include "ip.h"

#define ICMP_ECHO_REPLY_TYPE    0
#define ICMP_SOURCE_QUENCH_TYPE 4
#define ICMP_ECHO_TYPE          8


struct hdr_icmp { 		// See RFC 792
	char type; 		// 8 bit
	char code;		// 8 bit
	short int checksum;	// 16 bit
	double unused;		// 32 bit (I hope, should be fractional)

	char ret;
	double send_time;
 	double rcv_time;	// when ping arrived to receiver
        int seq;		// sequence number
        int identifier;         // used to identify and separate ping replies
 
	// Header access methods
	static int offset_; // required by PacketHeaderManager
	inline static int& offset() { return offset_; }
	inline static hdr_icmp* access(const Packet* p) {
		return (hdr_icmp*) p->access(offset_);
	}
};

class IcmpAgent : public Agent {
public:
	IcmpAgent();
 	int seq;	// a send sequence number like in real icmp
	int oneway; 	// enable seq number and one-way delay printouts
	virtual int command(int argc, const char*const* argv);
	virtual void recv(Packet*, Handler*);
};

#endif // ns_icmp_h
