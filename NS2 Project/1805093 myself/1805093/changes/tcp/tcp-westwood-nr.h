/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 1991-2001 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Computer Systems
 *	Engineering Group at Lawrence Berkeley Laboratory.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

// TCPW-NR - TCPW Westwood with NewReno Features
//
// tcp-westwood-nr.h - v1.1 - 2002/05/20 - mvalla
//

#ifndef tcp_w_nr_h
#define tcp_w_nr_h

#include "tcp.h"

/* these are bits for how to change the cwnd and ssthresh values */
/* used by TCP-W						 */
#define CLOSE_FASTER	0x00000400

/* TCP Westwood New Reno */

class WestwoodNRTcpAgent : public virtual NewRenoTcpAgent {
public:
	WestwoodNRTcpAgent();
	virtual void recv(Packet *pkt, Handler*);
	virtual void dupack_action();
	virtual void timeout (int tno);
	
	/* these where originally in class TcpAgent (file: tcp.h) */
	virtual void slowdown(int how);
	virtual void newack(Packet* pkt);
	virtual int  delay_bind_dispatch(const char *varName, const char *localName, TclObject *tracer);

protected:
	double lastackno_;  /* Last ACK number */
	double lastackrx_;  /* Time last ACK was received */ 
	double fr_alpha_;   /* exponential averaging coefficient */
	int filter_type_;   /* exponential filter type */
	double tau_;        /* time constant used in filter 3 */
	
	/* these where originally in class TcpAgent (file: tcp.h) */
 	int mss_;       	/* Maximum Segment Size - MGM+CC 31/08/2000 */
	double current_bwe_;    /* Current Bandwidth estimation */
        double last_bwe_sample_;/* Last sample used to compute BWE */
        int unaccounted_;       /* unaccounted ACKs already received */
        double fr_a_;           /* bandwidth reduction factor */
        double min_rtt_estimate;/* smaller recorded RTT estimate */
        TracedInt myseqno_;     /* my own exportable copy of seqno */
        
        virtual void bwe_computation(Packet *pkt);
	
};

#endif /* tcp_w_nr_h */
