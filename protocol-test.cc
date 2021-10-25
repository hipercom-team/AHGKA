//-----------------------------------------------------------------*- c++ -*-
//                          INRIA Rocquencourt
//  Copyright Institut National de Recherche en Informatique et
//  en Automatique.  All rights reserved.  Distributed only with permission.
//--------------------------------------------------------------------------

#include <stdlib.h>

//---------------------------------------------------------------------------
//                         Ad Hoc GKA
//---------------------------------------------------------------------------
// These files come from outside
#include "util-general.h"
#include "api-scheduler.h"
#include "scheduler-core.h"
#include "scheduler-core.cc"
//---------------------------------------------------------------------------
// Theses files come from the Ad Hoc GKA
#include "gka-crypto.h"
#include "gka-simul-base.h"
#include "gka-simul-base.cc"
//---------------------------------------------------------------------------
int main(int argc, char** argv)
{
  FILE *fptr;
  fptr = fopen("output.tr", "w");
//  fptr = fopen("/dev/null", "w");

  // The scheduler for all GKA Nodes in the simulation
  SimulationScheduler scheduler;
  GKAConfig config;

  GKAInstance gkainst(&scheduler);
  GKANode gkanode(&scheduler, &gkainst);
  GKASimul simul(&scheduler, &config, &gkainst, &gkanode);
     
   
  if (argc != 5) {
    printf("Syntax: %s <scenario> <nb of groups> <nb of nodes>"
           " <EC type>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
	
   
  // Configure the Nodes....
  config.simulationTime = 100.0;
  config.simScenario = atoi(argv[1]); 
  config.numGroups = atoi(argv[2]);
  config.numberofNodes = atoi(argv[3]);
  curveType = atoi(argv[4]);

  // Create two nodes
  simul.GKAInit(config.numberofNodes);

  // Run the scheduler
  scheduler.runUntilNoEvent();

  simul.GKAFinish(config.numberofNodes, fptr);
  
  fclose(fptr);
}
//---------------------------------------------------------------------------
