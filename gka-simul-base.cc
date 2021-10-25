//-----------------------------------------------------------------*- c++ -*-
//                          INRIA Rocquencourt
//  Copyright Institut National de Recherche en Informatique et
//  en Automatique.  All rights reserved.  Distributed only with permission.
//--------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Without NS2
//---------------------------------------------------------------------------
#include "gka-protocol.cc"

void GKASimul::evIGROUP(IGROUPMessage* message, int nodeId) {

  int IGROUPSender = nodeId;
  int coverageX1, coverageX2, coverageY1, coverageY2;

  IGReTxnodesList.clear();
   
  if (nodeQueue.at(IGROUPSender)->locX >= NODE_COVERAGE_AREA) {
  coverageX1 = nodeQueue.at(IGROUPSender)->locX - NODE_COVERAGE_AREA;
  } else {
  coverageX1 = 0;
  }

  if (nodeQueue.at(IGROUPSender)->locY >= NODE_COVERAGE_AREA) {
  coverageY1 = nodeQueue.at(IGROUPSender)->locY - NODE_COVERAGE_AREA;
  } else {
  coverageY1 = 0;
  }
  
  coverageX2 = nodeQueue.at(IGROUPSender)->locX + NODE_COVERAGE_AREA;
  coverageY2 = nodeQueue.at(IGROUPSender)->locY + NODE_COVERAGE_AREA;

  for (int node=0;node<nodeQueue.size();node++) {
   if (node != IGROUPSender) {   
    if (nodeQueue.at(node)->locX >= coverageX1 && nodeQueue.at(node)->locX <= coverageX2 && nodeQueue.at(node)->locY >= coverageY1 && nodeQueue.at(node)->locY <= coverageY2 && nodeQueue.at(node)->isActive == true) {
  nodeQueue.at(node)->processIGROUPMessage(message);
  IGReTxnodesList.push_back(node); 
   }
   }
  }

  nodeQueue.at(IGROUPSender)->RxIGnodes.push_back(message->leaderId);
  nodeQueue.at(IGROUPSender)->RxIGNumber.push_back(message->sequenceNumber);
  nodeQueue.at(IGROUPSender)->RxIGTime.push_back(scheduler->getTime());

  if (IGReTxnodesList.size() > 0) {

  ReTxIGROUPMessage(nodeId, message);   
  }
}

void GKASimul::evIREPLY(IREPLYMessage* message, int nodeId) {

  int IREPLYSender = nodeId;
  int leader_ID = message->leaderId;
  int coverageX1, coverageX2, coverageY1, coverageY2;

  IRReTxnodesList.clear();
  

  if (nodeQueue.at(IREPLYSender)->locX >= NODE_COVERAGE_AREA) {
  coverageX1 = nodeQueue.at(IREPLYSender)->locX - NODE_COVERAGE_AREA;
  } else {
  coverageX1 = 0;
  }

  if (nodeQueue.at(IREPLYSender)->locY >= NODE_COVERAGE_AREA) {
  coverageY1 = nodeQueue.at(IREPLYSender)->locY - NODE_COVERAGE_AREA;
  } else {
  coverageY1 = 0;
  }
  
  coverageX2 = nodeQueue.at(IREPLYSender)->locX + NODE_COVERAGE_AREA;
  coverageY2 = nodeQueue.at(IREPLYSender)->locY + NODE_COVERAGE_AREA;

  for (int node=0;node<nodeQueue.size();node++) {

   if (node != IREPLYSender) {   
    if (nodeQueue.at(node)->locX >= coverageX1 && nodeQueue.at(node)->locX <= coverageX2 && nodeQueue.at(node)->locY >= coverageY1 && nodeQueue.at(node)->locY <= coverageY2 && nodeQueue.at(node)->isActive == true) {
     // Algorithm 4:
     if (node == leader_ID) {
     nodeQueue.at(node)->processIREPLYMessage(message);
     } else {
     IRReTxnodesList.push_back(node);
     }
    }
   }
  }

  nodeQueue.at(IREPLYSender)->RxIRnodes.push_back(message->nodeId);
  nodeQueue.at(IREPLYSender)->RxIRNumber.push_back(message->sequenceNumber);
  nodeQueue.at(IREPLYSender)->RxIRTime.push_back(scheduler->getTime());

  if (IRReTxnodesList.size() > 0) {

  ReTxIREPLYMessage(nodeId, message);   
  }
}

void GKASimul::GKAInit(int numNodes)
{
  ecdsaManager.generateKey(pubkey, privkey);

  for (int i=0;i<numNodes;i++) {

  GKANode *nodePtr = new GKANode(scheduler, gkainst);

  if (config->numGroups == 0) {
  srand (time(NULL)+i);
  nodePtr->locX = rand() % 1000 + 1;
  nodePtr->locY = rand() % 1000 + 1;

//  printf(" %d X %d\n", nodePtr->locX, nodePtr->locY);

  } else {
   
  srand ( time(NULL)+i);

  int selectSector = rand() % config->numGroups + 1;

  srand ( time(NULL)+i);

  if (selectSector == 1) {
  nodePtr->locX = rand() % 300 + 1;
  nodePtr->locY = rand() % 300 + 1;
  } else if (selectSector == 2) {
  nodePtr->locX = rand() % 300 + 600;
  nodePtr->locY = rand() % 300 + 1;
  } else if (selectSector == 3) {
  nodePtr->locX = rand() % 300 + 600;
  nodePtr->locY = rand() % 300 + 600;
  } else if (selectSector == 4) {
  nodePtr->locX = rand() % 300 + 1;
  nodePtr->locY = rand() % 300 + 600;
  }
  }

  nodeQueue.push_back(nodePtr);
  nodeQueue.at(i)->makeInstance(i, 1, this);
  }
}

void GKASimul::ReTxIGROUPMessage(int sender, IGROUPMessage* message) {

  tempIGRxTxnodelist.clear();
  tempIGRxTxnodelist = IGReTxnodesList;
  IGReTxnodesList.clear();
  bool isReTx = false;
  bool processed = false;

  for (int i=0;i<tempIGRxTxnodelist.size();i++) {

   int IGROUPSender = tempIGRxTxnodelist.at(i);

   if (nodeQueue.at(IGROUPSender)->RxIGnodes.size() == 0) {

   isReTx = false;
   }

   int x=0;
   while (x<nodeQueue.at(IGROUPSender)->RxIGnodes.size()) {
    double tmptime = scheduler->getTime() - nodeQueue.at(IGROUPSender)->RxIGTime.at(x);
    if (tmptime > LEADER_LAST_TIME) {
    nodeQueue.at(IGROUPSender)->RxIGnodes.erase(nodeQueue.at(IGROUPSender)->RxIGnodes.begin() + x);
    nodeQueue.at(IGROUPSender)->RxIGNumber.erase(nodeQueue.at(IGROUPSender)->RxIGNumber.begin() + x);
    nodeQueue.at(IGROUPSender)->RxIGTime.erase(nodeQueue.at(IGROUPSender)->RxIGTime.begin() + x);
    }
   x++; 
   }

   for (int j=0;j<nodeQueue.at(IGROUPSender)->RxIGnodes.size();j++) {
    if (nodeQueue.at(IGROUPSender)->RxIGnodes.at(j) == message->leaderId && nodeQueue.at(IGROUPSender)->RxIGNumber.at(j) == message->sequenceNumber) {
    isReTx = true;
    } else {
    isReTx = false;
    }
   }

   if (isReTx == false) {
   nodeQueue.at(IGROUPSender)->RxIGnodes.push_back(message->leaderId);
   nodeQueue.at(IGROUPSender)->RxIGNumber.push_back(message->sequenceNumber);
   nodeQueue.at(IGROUPSender)->RxIGTime.push_back(scheduler->getTime());

   int coverageX1, coverageX2, coverageY1, coverageY2;

   if (nodeQueue.at(IGROUPSender)->locX >= NODE_COVERAGE_AREA) {
   coverageX1 = nodeQueue.at(IGROUPSender)->locX - NODE_COVERAGE_AREA;
   } else {
   coverageX1 = 0;
   }

   if (nodeQueue.at(IGROUPSender)->locY >= NODE_COVERAGE_AREA) {
   coverageY1 = nodeQueue.at(IGROUPSender)->locY - NODE_COVERAGE_AREA;
   } else {
   coverageY1 = 0;
   }
  
   coverageX2 = nodeQueue.at(IGROUPSender)->locX + NODE_COVERAGE_AREA;
   coverageY2 = nodeQueue.at(IGROUPSender)->locY + NODE_COVERAGE_AREA;
 
   for (int node=0;node<nodeQueue.size();node++) {
    if (node != IGROUPSender && node != message->leaderId) {   
     if (nodeQueue.at(node)->locX >= coverageX1 && nodeQueue.at(node)->locX <=  coverageX2 && nodeQueue.at(node)->locY >= coverageY1 && nodeQueue.at(node)->locY  <= coverageY2 && nodeQueue.at(node)->isActive == true) {
       for (int j=0;j<tempIGRxTxnodelist.size();j++) {
            if (node == tempIGRxTxnodelist.at(j)) {
                 processed = true;
            }
       }
     if (processed == false) {
     nodeQueue.at(node)->processIGROUPMessage(message);
     }
     IGReTxnodesList.push_back(node);
    }
   }
  }
  }
 }
  if (IGReTxnodesList.size() > 0) {

  ReTxIGROUPMessage(sender, message);   
  }
}

void GKASimul::ReTxIREPLYMessage(int sender, IREPLYMessage* message) {

  tempIRRxTxnodelist.clear();
  tempIRRxTxnodelist = IRReTxnodesList;
  IRReTxnodesList.clear();
  bool isReTx = false;

  for (int i=0;i<tempIRRxTxnodelist.size();i++) {

   int IREPLYSender = tempIRRxTxnodelist.at(i);
   if (nodeQueue.at(IREPLYSender)->RxIRnodes.size() == 0) {
    isReTx = false;
   }

   int x=0;
   while (x<nodeQueue.at(IREPLYSender)->RxIRnodes.size()) {
    double tmptime = scheduler->getTime() - nodeQueue.at(IREPLYSender)->RxIRTime.at(x);
    if (tmptime > IREPLY_EXPIRE_TIME) {
    nodeQueue.at(IREPLYSender)->RxIRnodes.erase(nodeQueue.at(IREPLYSender)->RxIRnodes.begin() + x);
    nodeQueue.at(IREPLYSender)->RxIRNumber.erase(nodeQueue.at(IREPLYSender)->RxIRNumber.begin() + x);
    nodeQueue.at(IREPLYSender)->RxIRTime.erase(nodeQueue.at(IREPLYSender)->RxIRTime.begin() + x);
    }
   x++; 
   }

   for (int j=0;j<nodeQueue.at(IREPLYSender)->RxIRnodes.size();j++) {

    if (nodeQueue.at(IREPLYSender)->RxIRnodes.at(j) == message->nodeId && nodeQueue.at(IREPLYSender)->RxIRNumber.at(j) == message->sequenceNumber) {
    isReTx = true;
    } else {
    isReTx = false;
    }
   }

   if (isReTx == false) {
   nodeQueue.at(IREPLYSender)->RxIRnodes.push_back(message->nodeId);
   nodeQueue.at(IREPLYSender)->RxIRNumber.push_back(message->sequenceNumber);
   nodeQueue.at(IREPLYSender)->RxIRTime.push_back(scheduler->getTime());

   int coverageX1, coverageX2, coverageY1, coverageY2;

   if (nodeQueue.at(IREPLYSender)->locX >= NODE_COVERAGE_AREA) {
   coverageX1 = nodeQueue.at(IREPLYSender)->locX - NODE_COVERAGE_AREA;
   } else {
   coverageX1 = 0;
   }

   if (nodeQueue.at(IREPLYSender)->locY >= NODE_COVERAGE_AREA) {
   coverageY1 = nodeQueue.at(IREPLYSender)->locY - NODE_COVERAGE_AREA;
   } else {
   coverageY1 = 0;
   }
  
   coverageX2 = nodeQueue.at(IREPLYSender)->locX + NODE_COVERAGE_AREA;
   coverageY2 = nodeQueue.at(IREPLYSender)->locY + NODE_COVERAGE_AREA;
 
   for (int node=0;node<nodeQueue.size();node++) {
    if (node != IREPLYSender) {   
     if (nodeQueue.at(node)->locX >= coverageX1 && nodeQueue.at(node)->locX <=  coverageX2 && nodeQueue.at(node)->locY >= coverageY1 && nodeQueue.at(node)->locY  <= coverageY2 && nodeQueue.at(node)->isActive == true) {
     if (node == message->leaderId) {
     nodeQueue.at(node)->processIREPLYMessage(message);
     } else {
     IRReTxnodesList.push_back(node);
     }
    }
   }
  }
  }
 }
  if (IRReTxnodesList.size() > 0) {

  ReTxIREPLYMessage(sender, message);   
  }
}

void GKASimul::GKAFinish(int numNodes, FILE *fptr)
{
  void *temp;

  for (int i=0;i<numNodes;i++) {
  nodeQueue.at(i)->createReport(temp, fptr);
  }

  displayResult(numNodes, fptr);
}

void GKASimul::displayResult(int numNodes, FILE *fptr) {


  printf("Avg. key computation time by the leader Vs # of Group members in (ms): \n");
  for (int i=0;i<numNodes;i++) {
  printf("-------");
  }
  printf("\n");
  for (int i=1;i<=numNodes;i++) {
  printf("  %-2d   ", i);
  }
  printf("\n");
  for (int i=1;i<=numNodes;i++) {
  printf("-------");
  }
  printf("\n");

  for (int i=0;i<numNodes;i++) {
    for (int j=0;j<numNodes;j++) {

    leaderAverageTime[i][j][0] =  leaderComputeTime[i][j][0] / leaderCounter[i][j][0];
    if (leaderCounter[i][j][0] == 0) {
     leaderAverageTime[i][j][0] = 0;
    }

   avgLeaderkeytime[j] += leaderComputeTime[i][j][0];
   leaderKeyCount[j] += leaderCounter[i][j][0];

     cout.precision(4); 
     cout.width(6);
     cout.fill(' ');
     cout << leaderAverageTime[i][j][0] << "|";
    }
  printf("\n");
  }

  for (int i=1;i<=numNodes;i++) {
  printf("-------");
  }
  printf("\n\n");

  double avgIGveriTime=0.0;
  double avgIRveriTime=0.0;
  double avgIGSignTime=0.0;
  double avgIRSignTime=0.0;



  if (IREPLYtotalVeriCount != 0) {
  
  avgIRveriTime = (IREPLYsignVeriTime / IREPLYtotalVeriCount);
  avgIGSignTime = (LeadersignTime / LeadersignCount);
  } else {

  avgIRveriTime = 0.0;
  avgIGSignTime = 0.0;
  }

  printf("Average IREPLY Signature Verification Time: %f (%d) \n", avgIRveriTime, IREPLYtotalVeriCount);

  printf("Average Leader Signature Time: %f (%d) \n", avgIGSignTime, LeadersignCount);

  fprintf(fptr,"IRAvg %f %d \n", IREPLYsignVeriTime, IREPLYtotalVeriCount);
  fprintf(fptr,"LeaderSign %f %d \n", LeadersignTime, LeadersignCount);

  fprintf(fptr,"Leader");

for (int j=0;j<numNodes;j++) {

  double avgTime;
  if (leaderKeyCount[j] > 0) {
  avgTime = avgLeaderkeytime[j] / leaderKeyCount[j];
  } else {
  avgTime = 0;
  }
  fprintf(fptr," %6.3f %d", avgLeaderkeytime[j], leaderKeyCount[j]);

  cout <<" "<< avgLeaderkeytime[j] <<" "<< leaderKeyCount[j];
}
  fprintf(fptr,"\n");
  
  printf("\n\n");

  printf("Avg. key computation time by the member Vs # of Group members in (ms): \n");

  for (int i=0;i<numNodes;i++) {
  printf("-------");
  }
  printf("\n");

  for (int i=1;i<=numNodes;i++) {
  printf("  %-2d   ", i);
  }
  printf("\n");

  for (int i=1;i<=numNodes;i++) {
  printf("-------");
  }
  printf("\n");

  for (int i=0;i<numNodes;i++) {
    for (int j=0;j<numNodes;j++) {

    memberAverageTime[i][j][0] =  memberComputeTime[i][j][0] / memberCounter[i][j][0];
    if (memberCounter[i][j][0] == 0 ) {
     memberAverageTime[i][j][0] = 0;
    }

    avgMemberkeytime[j] += memberComputeTime[i][j][0];
    memberKeyCount[j] += memberCounter[i][j][0];

     cout.precision(4); 
     cout.width(6);
     cout.fill(' ');
     cout << memberAverageTime[i][j][0] << "|";
    }
  printf("\n");
  }
  for (int i=0;i<numNodes;i++) {
  printf("-------");
  }
  printf("\n\n");


  if (IGROUPtotalVeriCount != 0) {
  
  avgIGveriTime = (IGROUPsignVeriTime / IGROUPtotalVeriCount);
  avgIRSignTime = (MembersignTime / MembersignCount);
  } else {

  avgIGveriTime = 0.0;
  avgIRSignTime = 0.0;
  }

  printf("Average IGROUP Signature Verification Time: %f (%d) \n", avgIGveriTime, IGROUPtotalVeriCount);

  printf("Average Member Signature Time: %f (%d) \n", avgIRSignTime, MembersignCount);

  fprintf(fptr,"IGAvg %f %d \n", IGROUPsignVeriTime, IGROUPtotalVeriCount);
  fprintf(fptr,"MemberSign %f %d \n", MembersignTime, MembersignCount);

  fprintf(fptr,"Member");

for (int j=0;j<numNodes;j++) {

  double avgTime;

  if (memberKeyCount[j] > 0) {
  avgTime = avgMemberkeytime[j] / memberKeyCount[j];
  } else {
  avgTime = 0;
  }

  fprintf(fptr," %6.3f %d", avgMemberkeytime[j], memberKeyCount[j]);

 cout <<" "<< avgMemberkeytime[j] <<" "<< memberKeyCount[j];
}

  fprintf(fptr,"\n");

  printf("\n\n");
}

//---------------------------------------------------------------------------

/*
void GKASimul::sendIGROUPMessage(int sender, IGROUPMessage* message)
{
  int IGROUPSender = sender;
  int coverageX1, coverageX2, coverageY1, coverageY2;

  IGReTxnodesList.clear();
   
  if (nodeQueue.at(IGROUPSender)->locX >= NODE_COVERAGE_AREA) {
  coverageX1 = nodeQueue.at(IGROUPSender)->locX - NODE_COVERAGE_AREA;
  } else {
  coverageX1 = 0;
  }

  if (nodeQueue.at(IGROUPSender)->locY >= NODE_COVERAGE_AREA) {
  coverageY1 = nodeQueue.at(IGROUPSender)->locY - NODE_COVERAGE_AREA;
  } else {
  coverageY1 = 0;
  }
  
  coverageX2 = nodeQueue.at(IGROUPSender)->locX + NODE_COVERAGE_AREA;
  coverageY2 = nodeQueue.at(IGROUPSender)->locY + NODE_COVERAGE_AREA;

  for (int node=0;node<nodeQueue.size();node++) {
   if (node != IGROUPSender) {   
    if (nodeQueue.at(node)->locX >= coverageX1 && nodeQueue.at(node)->locX <= coverageX2 && nodeQueue.at(node)->locY >= coverageY1 && nodeQueue.at(node)->locY <= coverageY2 && nodeQueue.at(node)->isActive == true) {
  nodeQueue.at(node)->processIGROUPMessage(message);
  IGReTxnodesList.push_back(node); 
   }
   }
  }

  nodeQueue.at(IGROUPSender)->RxIGnodes.push_back(message->leaderId);
  nodeQueue.at(IGROUPSender)->RxIGNumber.push_back(message->sequenceNumber);
  nodeQueue.at(IGROUPSender)->RxIGTime.push_back(scheduler->getTime());

  if (IGReTxnodesList.size() > 0) {

  ReTxIGROUPMessage(sender, message);   
  }
}

void GKASimul::sendIREPLYMessage(int sender, IREPLYMessage *message)
{
  int IREPLYSender = sender;
  int leader_ID = message->leaderId;
  int coverageX1, coverageX2, coverageY1, coverageY2;

  IRReTxnodesList.clear();
  

  if (nodeQueue.at(IREPLYSender)->locX >= NODE_COVERAGE_AREA) {
  coverageX1 = nodeQueue.at(IREPLYSender)->locX - NODE_COVERAGE_AREA;
  } else {
  coverageX1 = 0;
  }

  if (nodeQueue.at(IREPLYSender)->locY >= NODE_COVERAGE_AREA) {
  coverageY1 = nodeQueue.at(IREPLYSender)->locY - NODE_COVERAGE_AREA;
  } else {
  coverageY1 = 0;
  }
  
  coverageX2 = nodeQueue.at(IREPLYSender)->locX + NODE_COVERAGE_AREA;
  coverageY2 = nodeQueue.at(IREPLYSender)->locY + NODE_COVERAGE_AREA;

  for (int node=0;node<nodeQueue.size();node++) {

   if (node != IREPLYSender) {   
    if (nodeQueue.at(node)->locX >= coverageX1 && nodeQueue.at(node)->locX <= coverageX2 && nodeQueue.at(node)->locY >= coverageY1 && nodeQueue.at(node)->locY <= coverageY2 && nodeQueue.at(node)->isActive == true) {
     // Algorithm 4:
     if (node == leader_ID) {
     nodeQueue.at(node)->processIREPLYMessage(message);
     } else {
     IRReTxnodesList.push_back(node);
     }
    }
   }
  }

  nodeQueue.at(IREPLYSender)->RxIRnodes.push_back(message->nodeId);
  nodeQueue.at(IREPLYSender)->RxIRNumber.push_back(message->sequenceNumber);
  nodeQueue.at(IREPLYSender)->RxIRTime.push_back(scheduler->getTime());

  if (IRReTxnodesList.size() > 0) {

  ReTxIREPLYMessage(sender, message);   
  }
}
*/


