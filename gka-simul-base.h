//-----------------------------------------------------------------*- c++ -*-
//                          INRIA Rocquencourt
//  Copyright Institut National de Recherche en Informatique et
//  en Automatique.  All rights reserved.  Distributed only with permission.
//--------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Without NS2
//---------------------------------------------------------------------------
#ifndef __gka_simul_base_h__
#define __gka_simul_base_h__

#include "gka-protocol.h"

#define NODE_COVERAGE_AREA 300

class GKAConfig
{
public:
  int numberofNodes;
  double simulationTime;
  int simScenario;
  int numGroups;
};

class GKASimul: public IpacketExchange 
{
public:

  GKASimul(IScheduler* aScheduler, GKAConfig* aGKAConfig, GKAInstance* aGKAInstance, GKANode* aGKANode):scheduler(aScheduler), config(aGKAConfig), gkainst(aGKAInstance), gkanode(aGKANode) {}

  void GKAInit(int numNodes);  
  void GKAFinish(int numNodes, FILE *fptr);
  void displayResult(int numNodes, FILE *fptr);
  void sendIGROUPMessage(IGROUPMessage* message);
  void sendIREPLYMessage(IREPLYMessage* message);

  void sendIGROUPMessage(int sender, IGROUPMessage* message);
  void sendIREPLYMessage(int sender, IREPLYMessage* message);
  void ReTxIGROUPMessage(int sender, IGROUPMessage* message);
  void ReTxIREPLYMessage(int sender, IREPLYMessage* message);
  void evIGROUP(IGROUPMessage* message, int nodeId);
  void evIREPLY(IREPLYMessage* message, int nodeId);
  vector <int> IGReTxnodesList;
  vector <int> IRReTxnodesList;
  vector <int> tempIGRxTxnodelist;
  vector <int> tempIRRxTxnodelist;
  vector <GKANode*> nodeQueue;
  GKAConfig* config;

protected:
  IScheduler* scheduler;
  GKANode* gkanode;
  GKAInstance* gkainst; 
  IpacketExchange* ipacket;
};
//---------------------------------------------------------------------------
#endif
