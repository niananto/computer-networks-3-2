/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 1990, 2001 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Lawrence Berkeley Laboratory,
 * Berkeley, CA.  The name of the University may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// TCPW-NR - TCPW AReno with NewReno Features
//
// tcp-areno.cc - v1.3 - 2002/06/21 - mvalla
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>

#include "packet.h"
#include "ip.h"
#include "tcp.h"
#include "flags.h"
#include "address.h"

#include "tcp-areno.h"
#define DEBUG 0
#define PDEBUG 0

static class ARenoTcpClass : public TclClass
{
public:
  ARenoTcpClass() : TclClass("Agent/TCP/AReno") {}
  TclObject *create(int, const char *const *)
  {
    return (new ARenoTcpAgent());
  }
} class_areno;

ARenoTcpAgent::ARenoTcpAgent() : RenoTcpAgent(),
                                 // these where originally in TcpAgent()
                                 current_bwe_(0), last_bwe_sample_(0), unaccounted_(0),
                                 fr_a_(0), min_rtt_estimate(5.0), myseqno_(1),

                                 lastackrx_(0.0), fr_alpha_(0.9), filter_type_(1), tau_(1.0)
{
  // check if it has been called
  if (DEBUG)
    printf("ARenoTcpAgent() called!\n");

  // (these where originally in TcpAgent() )
  // Read defaults variables from ns-defaults.tcl
  // bind("mss_", &mss_);
  bind("current_bwe_", &current_bwe_);
  bind("last_bwe_sample_", &last_bwe_sample_);
  bind("unaccounted_", &unaccounted_);
  bind("fr_a_", &fr_a_);
  bind("min_rtt_estimate", &min_rtt_estimate);

  bind("fr_alpha_", &fr_alpha_);
  bind("filter_type_", &filter_type_);
  bind("tau_", &tau_);

  bind("rtt_cong", &rtt_cong);
  bind("last_rtt_cong", &last_rtt_cong);
  bind("rtt_factor", &rtt_factor);

  bind("cwnd_base", &cwnd_base);
  bind("cwnd_probe", &cwnd_probe);
  bind("cwnd_inc", &cwnd_inc);

  bind("max_seg_size", &max_seg_size);
}

void ARenoTcpAgent::dupack_action()
{
  // check if it has been called
  if (DEBUG)
    printf("dupack_action() called!\n");

  // update the rtt cong
  double rtt_estimate = t_rtt_ * tcp_tick_;
  rtt_cong = (1 - rtt_factor) * last_rtt_cong + rtt_factor * rtt_estimate;

  /* New adaptive BWE/a strategy */
  if (ssthresh_ > cwnd_)
  {
    /* 3dupacks were received while I was in slowstart */
    fr_a_ += 0.25;
    if (fr_a_ > 4)
      fr_a_ = 4;
  }
  else
  {
    /* 3dupacks were received while I was in congestion avoidance */
    fr_a_ = 1;
  }

  //
  //  ssthresh_ = (int)((current_bwe_/size_/8) * min_rtt_estimate)/fr_a_;
  //

  // WARNING!!!'a' is no longer being used
  ssthresh_ = (int)((current_bwe_ / size_ / 8) * min_rtt_estimate);

  // Safety Check: ssthresh should not be < 2
  // added 2002-05-20 by MV
  if (ssthresh_ < 2)
  {
    ssthresh_ = 2;
  }

  /* our algorithm dictates that CWIN=ssthresh after a 3DUPACK, but */
  /* we should not forcefully increase CWIN if it is smaller than   */
  /* ssthresh                                                       */

  if (cwnd_ > ssthresh_)
  {
    cwnd_ = ssthresh_;
  }

  recover_ = maxseq_;                       // used by ECN
  last_cwnd_action_ = CWND_ACTION_DUPACK;   // used by ECN
  reset_rtx_timer(1, 0);                    // reset RTO as in Reno
  output(last_ack_ + 1, TCP_REASON_DUPACK); // resend missing packet
  return;
}

void ARenoTcpAgent::timeout(int tno)
{
  // check if it has been called
  if (DEBUG)
    printf("timeout() called!\n");

  /* retransmit timer */
  if (tno == TCP_TIMER_RTX)
  {
    // These three lines catch the RenoTcpAgent::timeout() behavior
    dupwnd_ = 0;
    dupacks_ = 0;
    if (bug_fix_)
      recover_ = maxseq_;

    // There has been a timeout - will trace this event
    trace_event("TIMEOUT");

    if (cwnd_ < 1)
      cwnd_ = 1;
    if (highest_ack_ == maxseq_ && !slow_start_restart_)
    {
      /*
       * TCP option:
       * If no outstanding data, then don't do anything.
       */
      // Should this return be here?
      // What if CWND_ACTION_ECN and cwnd < 1?
      // return;
    }
    else
    {
      recover_ = maxseq_;
      if (highest_ack_ == -1 && wnd_init_option_ == 2)
        /*
         * First packet dropped, so don't use larger
         * initial windows.
         */
        wnd_init_option_ = 1;
      if (highest_ack_ == maxseq_ && restart_bugfix_)
        /*
         * if there is no outstanding data, don't cut
         * down ssthresh_.
         */
        slowdown(CLOSE_CWND_ONE);
      else if (highest_ack_ < recover_ &&
               last_cwnd_action_ == CWND_ACTION_ECN)
      {
        /*
         * if we are in recovery from a recent ECN,
         * don't cut down ssthresh_.
         */
        slowdown(CLOSE_CWND_ONE);
      }
      else
      {
        ++nrexmit_;
        last_cwnd_action_ = CWND_ACTION_TIMEOUT;
        // slowdown(CLOSE_SSTHRESH_HALF|CLOSE_CWND_RESTART);
        slowdown(CLOSE_FASTER); // TCPW action
      }
    }
    /* if there is no outstanding data, don't back off rtx timer */
    if (highest_ack_ == maxseq_ && restart_bugfix_)
    {
      reset_rtx_timer(0, 0);
    }
    else
    {
      reset_rtx_timer(0, 1);
    }
    last_cwnd_action_ = CWND_ACTION_TIMEOUT;
    send_much(0, TCP_REASON_TIMEOUT, maxburst_);
  }
  else
  {
    timeout_nonrtx(tno);
  }
}

int time_to_print = 0;
void ARenoTcpAgent::recv(Packet *pkt, Handler *)
{

  // check if it has been called
  if (DEBUG)
    printf("recv() called!\n");

  if (PDEBUG)
    printf("1\n");
  hdr_tcp *tcph = hdr_tcp::access(pkt);
  ts_peer_ = tcph->ts();

  double fr_now = Scheduler::instance().clock();
  printf("time %f bwe*min_rtt %f\n", fr_now, current_bwe_ * min_rtt_estimate/1000000);

  if (PDEBUG)
    printf("2\n");
  // last_ack_ indicates the ack no. of the ack received _before_
  // the current one

  // START BWE COMPUTATION
  // Idea: cumulative ACKs acking more than 2 packets count for 1 packet
  //   since DUPACKs have already been accounted for
  int cumul_ack = tcph->seqno_ - last_ack_;
  myseqno_ = tcph->seqno_;
  if (PDEBUG)
    printf("3\n");
  if (cumul_ack > 1)
  {

    /* check if current ACK ACKs fewer or same number of segments than */
    /* expected: if so, the missing ones were already accounted for by */
    /* DUPACKs, and current ACK only counts as 1 */
    if (unaccounted_ >= cumul_ack)
    {
      unaccounted_ = unaccounted_ - cumul_ack + 1;
      cumul_ack = 1;
    }
    else
      /* check if current ACK ACKs more segments than expected: if so,   */
      /* part of them were already accounted for by DUPACKs; the rest    */
      /* are cumulatively ACKed by present ACK. Make present ACK count   */
      /* as the unacknowledged ACKs in excess*/
      if (unaccounted_ < cumul_ack)
      {
        cumul_ack -= unaccounted_;
        unaccounted_ = 0;
      }
  }
  if (PDEBUG)
    printf("4\n");
  /* if cumul_ack=0, the current ACK is clearly a DUPACK and should */
  /* count 1 */
  if (cumul_ack == 0)
  {
    unaccounted_++;
    cumul_ack = 1;
  }

  /* safety check; if the previous steps are followed exactly,      */
  /* cumul_ack should not be >2 unless some strage events occur     */
  /* (e.g., an ACK is dropped on the way back and the following one */
  /* appears to ACK more than its due)                              */
  if (PDEBUG)
    printf("5\n");
  if (cumul_ack > 2)
  {
    cumul_ack = 2;
  }

  double rtt_estimate = t_rtt_ * tcp_tick_;

  if ((rtt_estimate < min_rtt_estimate) && (rtt_estimate > 0))
  {
    min_rtt_estimate = rtt_estimate;
  }

  nackpack_ += cumul_ack;

  int acked_size = size_ * 8 * cumul_ack;
  double ack_interv = fr_now - lastackrx_;
  double sample_bwe;

  if (PDEBUG)
    printf("6 %d %d %d %d\n", acked_size, ack_interv, tau_ * 2);

  if (ack_interv <= 0) printf("negative ack_interv\n");
  sample_bwe = acked_size / ack_interv;
  current_bwe_ = current_bwe_ * .9047 +
                 (sample_bwe + last_bwe_sample_) * .0476;

  if (PDEBUG)
    printf("7\n");
#ifdef MYDEBUG
  hdr_ip *iph = hdr_ip::access(pkt);
  char *src_portaddr = Address::instance().print_portaddr(iph->sport());
  printf("sc%s: ack. no. %d at time %f, bwe=%f, cwnd = %d, ssthresh_ = %d\n",
         src_portaddr, tcph->seqno_, fr_now, current_bwe_ / 1000000,
         (int)cwnd_, (int)ssthresh_);
  printf("sc%s: now = %f, acked_size = %d, rxdiff = %f, last_ack_ = %d\n",
         src_portaddr, fr_now, acked_size, (fr_now - lastackrx_), last_ack_);
  printf("sc%s: unaccounted_ = %d, fr_a_= %f, min_rtt_estimate = %f\n",
         src_portaddr, unaccounted_, fr_a_, min_rtt_estimate);
#endif
#ifdef MYDEBUG_RTT
  double f = t_rtt_ * tcp_tick_;
  printf("source %s: %f cwnd=%d	      bwe=%f	  rtt=%f\n",
         src_portaddr, fr_now, (int)cwnd_, current_bwe_ / 1000000, f);
#endif
#ifdef MYREPORT
  hdr_ip *iph = hdr_ip::access(pkt);
  char *src_portaddr = Address::instance().print_portaddr(iph->src());
  printf("%s    %f      %d      %f      %d\n",
         src_portaddr, fr_now, (int)cwnd_, current_bwe_ / 1000000,
         (int)ssthresh_);
#endif

  last_bwe_sample_ = sample_bwe;
  lastackrx_ = fr_now;

  /* grow cwnd and check if the connection is done */
  if (tcph->seqno() > last_ack_)
  {
    recv_newack_helper(pkt);
    if (last_ack_ == 0 && delay_growth_)
    {
      cwnd_ = initial_window();
    }
  }
  else if (tcph->seqno() == last_ack_)
  {
    if (hdr_flags::access(pkt)->eln_ && eln_)
    {
      tcp_eln(pkt);
      return;
    }
    if (++dupacks_ == NUMDUPACKS)
    {
      dupack_action();
    }
  }
  Packet::free(pkt);
  /*
   * Try to send more data.
   */
  send_much(0, 0, maxburst_);

  // print the bandwidth delay product wrt time

  // if (fr_now > time_to_print)
  // {
  //   printf("BDP: time %f bwe*minrtt %f cwnd %f\n", fr_now, current_bwe_ * min_rtt_estimate, (double)cwnd_);
  //   time_to_print += 1;
  // }

}

/*
 * open up the congestion window
 */
void ARenoTcpAgent::opencwnd()
{
  double increment;
  if (cwnd_ < ssthresh_)
  {
    /* slow-start (exponential) */
    cwnd_ += 1;
  }
  else
  {

    cwnd_ = open_congestion_window();
    return;

    /* linear */
    double f;
    switch (wnd_option_)
    {
    case 1:
      /* This is the standard algorithm. */
      increment = increase_num_ / cwnd_;
      if ((last_cwnd_action_ == 0 ||
           last_cwnd_action_ == CWND_ACTION_TIMEOUT) &&
          max_ssthresh_ > 0)
      {
        increment = limited_slow_start(cwnd_,
                                       max_ssthresh_, increment);
      }
      cwnd_ += increment;

    default:
#ifdef notdef
      /*XXX*/
      error("illegal window option %d", wnd_option_);
#endif
      abort();
    }
  }

  // if maxcwnd_ is set (nonzero), make it the cwnd limit
  if (((int)cwnd_ > maxcwnd_))
  {
    cwnd_ = maxcwnd_;
  }

  return;
}

/////////////////// Added by MV
// these where originally in TcpAgent()

void ARenoTcpAgent::slowdown(int how)
{
  // check if it is called
  if (DEBUG)
    printf("slowdown called\n");

  double decrease; /* added for highspeed - sylvia */
  double win, halfwin, decreasewin;
  int slowstart = 0;
  ++ncwndcuts_;
  if (!(how & TCP_IDLE) && !(how & NO_OUTSTANDING_DATA))
  {
    ++ncwndcuts1_;
  }

  // we are in slowstart for sure if cwnd < ssthresh
  if (cwnd_ < ssthresh_)
    slowstart = 1;

  cwnd_ = close_congestion_window();

  // if (precision_reduce_)
  // {
  //   halfwin = windowd() / 2;
  //   if (wnd_option_ == 6)
  //   {
  //     /* binomial controls */
  //     decreasewin = windowd() - (1.0 - decrease_num_) * pow(windowd(), l_parameter_);
  //   }
  //   else if (wnd_option_ == 8 && (cwnd_ > low_window_))
  //   {
  //     /* experimental highspeed TCP */
  //     decrease = decrease_param();
  //     // if (decrease < 0.1)
  //     //	decrease = 0.1;
  //     decrease_num_ = decrease;
  //     decreasewin = windowd() - (decrease * windowd());
  //   }
  //   else
  //   {
  //     decreasewin = decrease_num_ * windowd();
  //   }
  //   win = windowd();
  // }
  // else
  // {
  //   int temp;
  //   temp = (int)(window() / 2);
  //   halfwin = (double)temp;
  //   if (wnd_option_ == 6)
  //   {
  //     /* binomial controls */
  //     temp = (int)(window() - (1.0 - decrease_num_) * pow(window(), l_parameter_));
  //   }
  //   else if ((wnd_option_ == 8) && (cwnd_ > low_window_))
  //   {
  //     /* experimental highspeed TCP */
  //     decrease = decrease_param();
  //     // if (decrease < 0.1)
  //     //        decrease = 0.1;
  //     decrease_num_ = decrease;
  //     temp = (int)(windowd() - (decrease * windowd()));
  //   }
  //   else
  //   {
  //     temp = (int)(decrease_num_ * window());
  //   }
  //   decreasewin = (double)temp;
  //   win = (double)window();
  // }

  // // ssthresh_
  // if (how & CLOSE_SSTHRESH_HALF)
  //   // For the first decrease, decrease by half
  //   // even for non-standard values of decrease_num_.
  //   if (first_decrease_ == 1 || slowstart ||
  //       last_cwnd_action_ == CWND_ACTION_TIMEOUT)
  //   {
  //     // Do we really want halfwin instead of decreasewin
  //     // after a timeout?
  //     ssthresh_ = (int)halfwin;
  //   }
  //   else
  //   {
  //     ssthresh_ = (int)decreasewin;
  //   }
  // else if (how & THREE_QUARTER_SSTHRESH)
  //   if (ssthresh_ < 3 * cwnd_ / 4)
  //     ssthresh_ = (int)(3 * cwnd_ / 4);

  // // cwnd_
  // if (how & CLOSE_CWND_HALF)
  //   // For the first decrease, decrease by half
  //   // even for non-standard values of decrease_num_.
  //   if (first_decrease_ == 1 || slowstart || decrease_num_ == 0.5)
  //   {
  //     cwnd_ = halfwin;
  //   }
  //   else
  //   {
  //     cwnd_ = decreasewin;
  //   }
  // else if (how & CWND_HALF_WITH_MIN)
  // {
  //   // We have not thought about how non-standard TCPs, with
  //   // non-standard values of decrease_num_, should respond
  //   // after quiescent periods.
  //   cwnd_ = decreasewin;
  //   if (cwnd_ < 1)
  //     cwnd_ = 1;
  // }
  // ///
  // else if (how & CLOSE_FASTER)
  // {
  //   // TCP AReno
  //   // this might be critical what with the coarseness of the timer;
  //   // keep in mind that TCP computes the timeout as
  //   //              (#of ticks) * (tick_duration)
  //   // We need to do away with the coarseness...

  //   double rtt_estimate = t_rtt_ * tcp_tick_;

  //   if ((rtt_estimate < min_rtt_estimate) && (rtt_estimate > 0))
  //   {
  //     min_rtt_estimate = rtt_estimate;
  //   }
  //   /* New adaptive BWE/a strategy */
  //   if (ssthresh_ > cwnd_)
  //   {
  //     /* loss has occurred while I was in slowstart */
  //     fr_a_++;
  //     if (fr_a_ > 4)
  //       fr_a_ = 4;
  //   }
  //   else
  //   {
  //     /* loss has occurred while I was in congestion avoidance */
  //     fr_a_ = 1;
  //   }
  //   ssthresh_ = (int)(((current_bwe_ / size_ / 8) * min_rtt_estimate) / fr_a_);
  //   cwnd_ = 1;
  // }
  // ///
  // else if (how & CLOSE_CWND_RESTART)
  //   cwnd_ = int(wnd_restart_);
  // else if (how & CLOSE_CWND_INIT)
  //   cwnd_ = int(wnd_init_);
  // else if (how & CLOSE_CWND_ONE)
  //   cwnd_ = 1;
  // else if (how & CLOSE_CWND_HALF_WAY)
  // {
  //   // cwnd_ = win - (win - W_used)/2 ;
  //   cwnd_ = W_used + decrease_num_ * (win - W_used);
  //   if (cwnd_ < 1)
  //     cwnd_ = 1;
  // }
  // ///
  // ///
  // if (ssthresh_ < 2)
  //   ssthresh_ = 2;
  // if (cwnd_ < 1)
  //   cwnd_ = 1;
  // if (how & (CLOSE_CWND_HALF | CLOSE_CWND_RESTART | CLOSE_CWND_INIT | CLOSE_CWND_ONE))
  //   cong_action_ = TRUE;

  fcnt_ = count_ = 0;
  if (first_decrease_ == 1)
    first_decrease_ = 0;
  // for event tracing slow start
  if (cwnd_ == 1 || slowstart)
    // Not sure if this is best way to capture slow_start
    // This is probably tracing a superset of slowdowns of
    // which all may not be slow_start's --Padma, 07/'01.
    trace_event("SLOW_START");
}

/*
 * Process a packet that acks previously unacknowleged data.
 */
void ARenoTcpAgent::newack(Packet *pkt)
{
  hdr_tcp *tcph = hdr_tcp::access(pkt);
  myseqno_ = tcph->seqno_;
  // call parent newack
  RenoTcpAgent::newack(pkt);
}

// AReno binds
int ARenoTcpAgent::delay_bind_dispatch(const char *varName, const char *localName, TclObject *tracer)
{

  if (delay_bind(varName, localName, "lastackno_", &lastackno_, tracer))
    return TCL_OK;
  if (delay_bind(varName, localName, "lastackrx_", &lastackrx_, tracer))
    return TCL_OK;
  if (delay_bind(varName, localName, "fr_alpha_", &fr_alpha_, tracer))
    return TCL_OK;
  if (delay_bind(varName, localName, "filter_type_", &filter_type_, tracer))
    return TCL_OK;
  if (delay_bind(varName, localName, "tau_", &tau_, tracer))
    return TCL_OK;
  if (delay_bind(varName, localName, "mss_", &mss_, tracer))
    return TCL_OK;
  if (delay_bind(varName, localName, "current_bwe_", &current_bwe_, tracer))
    return TCL_OK;
  if (delay_bind(varName, localName, "last_bwe_sample_", &last_bwe_sample_, tracer))
    return TCL_OK;
  if (delay_bind(varName, localName, "unaccounted_", &unaccounted_, tracer))
    return TCL_OK;
  if (delay_bind(varName, localName, "fr_a_", &fr_a_, tracer))
    return TCL_OK;
  if (delay_bind(varName, localName, "min_rtt_estimate", &min_rtt_estimate, tracer))
    return TCL_OK;
  if (delay_bind(varName, localName, "myseqno_", &myseqno_, tracer))
    return TCL_OK;

  return RenoTcpAgent::delay_bind_dispatch(varName, localName, tracer);
}

/* tickoff is the time since the clock last ticked when
 *  the packet we are using to compute the RTT was sent
 */

/* t_rtt_ is the number of ticks that have occurred so far,
 * starting from the tick BEFORE the packet was sent
 */

double ARenoTcpAgent::estimate_cong_level()
{
  double rtt_estimate = t_rtt_ * tcp_tick_;

  // update the congestion level
  if ((rtt_cong - min_rtt_estimate <= 0))
  {
    return 0;
  }
  double probable_cong_level = (rtt_estimate - min_rtt_estimate) / (rtt_cong - min_rtt_estimate);
  probable_cong_level = (probable_cong_level < 0) ? 0 : probable_cong_level;
  double cong_level = (probable_cong_level < 1) ? probable_cong_level : 1;
  // printf("time %f cong_level %f\n", fr_now, cong_level);
  last_rtt_cong = rtt_cong;

  return cong_level;
}

int ARenoTcpAgent::open_congestion_window()
{
  double cong_level = estimate_cong_level();
  int factor_m = 10; // 10 mbps in paper

  double max_seg_size = 1460;
  double cwnd_inc_max = current_bwe_ / factor_m * max_seg_size;

  double alpha = 10;
  double beta = 2.0 * cwnd_inc_max * ((1.0 / alpha) - ((1.0 / alpha + 1.0) / (pow(2.71, alpha))));
  double gamma = 1.0 - (2.0 * cwnd_inc_max * ((1.0 / alpha) - ((1.0 / alpha + 0.5) / (pow(2.71, alpha)))));

  cwnd_inc = (int)((cwnd_inc_max / pow(2.71, alpha * cong_level)) + (beta * cong_level) + gamma);
  cwnd_inc = (cwnd_inc > cwnd_inc_max) ? cwnd_inc_max : cwnd_inc;

  // base_window = use new renos base window
  double adder;
  if (cwnd_ > 0) {
    adder = max_seg_size / cwnd_;
    adder = (adder < 1) ? 1 : adder;
  } else {
    adder = 1;
  }
  cwnd_base += (int)adder;

  // change probe window
  if (cwnd_ > 0) {
    cwnd_probe = (int)(cwnd_probe + cwnd_inc / cwnd_);
    cwnd_probe = (cwnd_probe < 0) ? 0 : cwnd_probe;
  } else {
    cwnd_probe = 0;
  }

  // change cwnd
  return cwnd_base + cwnd_probe;
}

int ARenoTcpAgent::close_congestion_window()
{
  double max_seg_size = 1460;
  double cong_level = estimate_cong_level();
  int ssthresh_ = (int)(cwnd_ / (1.0 + cong_level));
  ssthresh_ = (2 * max_seg_size > ssthresh_) ? 2 * max_seg_size : ssthresh_;
  ssthresh_ = (ssthresh_ > (int)cwnd_) ? (int)cwnd_ : ssthresh_;

  // reset calculations
  cwnd_base = ssthresh_;
  cwnd_probe = 0;

  return cwnd_base + cwnd_probe;
}