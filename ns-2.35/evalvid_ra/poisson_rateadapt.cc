#ifndef lint
static const char rcsid[] =
    "@(#) $Header: /n/fs054/ramon/src/ns/ns-2/RCS/poisson.cc,v 1.1 1998/06/09 17:32:38 ramon Exp ramon $";
#endif

#include <stdlib.h>

#include "random.h"
#include "trafgen.h"
#include "ranvar.h"
#define MAX_NODE 500

/*
 * Implement a Poisson source.
 * Parameters: constant packet size and mean interarrival time.
 *
 * Modification 25.01.05 by Arne Lie, NTNU:
 * make the poisson source rate adaptive.
 */

class Poisson_Source_RA : public TrafficGenerator {
 public:
        Poisson_Source_RA();
        virtual double next_interval(int&);
 protected:
        virtual void start();
	virtual void stop();
        void init();
        ExponentialRandomVariable interval_;
        int command(int argc, const char*const* argv);
	int running2_;
  double rate_;
  double interval2_ ;
	double ECF_rateadapt[MAX_NODE];
	double ecf_new;
	double rate_inc_;
	double new_rate;
	int from_f;
	double ecf_store[MAX_NODE];  /* can handle MAX_NODE P-AQM nodes */
	double rate_store[MAX_NODE]; /* ------------  "  -------------- */

};


static class PoissonRateAdaptClass : public TclClass {
 public:
        PoissonRateAdaptClass() : TclClass("Application/Traffic/Poisson_RA") {}
        TclObject* create(int, const char*const*) {
                return (new Poisson_Source_RA());
        }
} class_poisson_ra;

Poisson_Source_RA::Poisson_Source_RA() : interval_(0.0)
{
        bind("size", &size_);
        bind_time("interval", interval_.avgp());
	bind("running_", &running2_);
	bind_bw("rate_", &rate_); 
}


void Poisson_Source_RA::init()
{
  int i;

  for (i=0;i<MAX_NODE;i++) {
    ECF_rateadapt[i] = 1.0;
    rate_store[i] = rate_;
  }
  new_rate = rate_;
  ecf_new = 1.0;
  
  return;
}


void Poisson_Source_RA::start()
{
        init();
        running_ = running2_ = 1;
        timeout();
}

void Poisson_Source_RA::stop()
{
	if (running_)
		timer_.cancel();
	running_ = running2_ = 0;
}

double Poisson_Source_RA::next_interval(int& size)
{
        size = size_;
	interval2_ = (double)(size_ << 3)/(double)new_rate;
	interval_.setavg(interval2_);
	//	printf("interval2_=%7.6f, new_rate=%4.3f \n", interval2_, new_rate);
        return(interval_.value());
}

int Poisson_Source_RA::command(int argc, const char*const* argv)
{
  if(argc==3){
    if (strcmp(argv[1], "use-rng") == 0) {
      interval_.seed((char *)argv[2]);
      return (TCL_OK);
    }
  }
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
	rate_store[from_f] = rate_store[from_f] + ecf_store[from_f];
	ECF_rateadapt[from_f] = rate_store[from_f] / rate_;
	if (rate_store[from_f] > rate_) {
	  rate_store[from_f] = rate_;
	  ECF_rateadapt[from_f] = 1.0;
	}
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
      //printf(">>CBR_RA, from_f=%d, new_rate = %5.1f,r_s[0]=%5.1f,r_s[1]=%5.1f\n",from_f,new_rate,rate_store[0],rate_store[1]);
      timeout();
    }
    return (TCL_OK);
  }

  return Application::command(argc,argv);
}

