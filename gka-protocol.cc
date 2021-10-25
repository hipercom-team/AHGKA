//-----------------------------------------------------------------*- c++ -*-
//                          INRIA Rocquencourt
//  Copyright Institut National de Recherche en Informatique et
//  en Automatique.  All rights reserved.  Distributed only with permission.
//--------------------------------------------------------------------------

//---------------------------------------------------------------------------
//                         Ad Hoc GKA
//---------------------------------------------------------------------------
#include "gka-protocol.h"
#include <ctime>

#define MIN_BACKOFF 5.0
#define MAX_BACKOFF 20.0

int static IGmessageCount = 1;
int static IRmessageCount = 1;

typedef enum {
  StateDetached = 0,
  StateLeader = 1,
  StateMember = 2
} StateType;

IGROUPMessage::IGROUPMessage(){}

IREPLYMessage::IREPLYMessage(){}

void GKANode::evTest(void* unused){}

#ifdef NS2_WITH_NS2
void GKANode::makeInstance(int node_ID, int num_sessions, NSGKANode* nsgka)
{
  // Algorithm 2:
  nodeId = node_ID;
  isActive = false;
  nsgkaptr = nsgka;

  for (int i=0; i<num_sessions; i++) {

  // Later add the session details of each node and the initialize them
  GKAInstance *sessionPtr = new GKAInstance(scheduler);
  sessionPtr->nodeState = StateDetached;
  sessionPtr->isbackoffExists = 0; 
  sessionPtr->lastIGROUPMsgTime = scheduler->getTime();
  sessionPtr->lastTimeSecret = scheduler->getTime();
  sessionPtr->pendingLeaderList.clear();
  sessionPtr->memberList.clear();
  sessionPtr->sessionId++;
  sessionPtr->nsgkaptr = nsgka;
  sessionQueue.push_back(sessionPtr);
  }

  // Every node select a random time to become active
  double randomTime = (MIN_BACKOFF + ((MAX_BACKOFF - MIN_BACKOFF) * Random::uniform())) + scheduler->getTime();

//  printf("Node %d Activate at time: %f \n", nodeId, randomTime);

  isInitSent = false; 

  scheduler->addEventAt(randomTime, makeCallback(this, &GKANode::nodePeriodicUpdate), NULL);
}

void GKANode::evIGROUP(IGROUPMessage* message, int nodeId) {

  bool isReTx = false;

  if (reTxIGPktNum.size() > 0) {
   for(unsigned int i=0;i<reTxIGPktNum.size();i++) {

    if (reTxIGPktNum.at(i) > 0 && message->sequenceNumber > 0) { 
     if (reTxIGPktNum.at(i) == message->sequenceNumber) {
      isReTx = true;         
     }   
    }
   }
  }

  if (nodeId != message->leaderId && isReTx == false) {
  processIGROUPMessage(message);
  }

  if (isReTx == false) {
  reTxIGPktNum.push_back(message->sequenceNumber);      
  nsgkaptr->ReTxIGROUP(message);
  }
}

void GKANode::evIREPLY(IREPLYMessage* message, int nodeId) {

  bool isReTx = false;

  if (reTxIRPktNum.size() > 0) {
   for(unsigned int i=0;i<reTxIRPktNum.size();i++) {

    if (reTxIRPktNum.at(i) > 0 && message->sequenceNumber > 0) { 
     if (reTxIRPktNum.at(i) == message->sequenceNumber && usrListIRReTxPkt.at(i) == message->nodeId) {
      isReTx = true;         
     }   
    }
   }
  }

  if (nodeId == message->leaderId && isReTx == false) {
  processIREPLYMessage(message);
  }

  if (isReTx == false) {
  usrListIRReTxPkt.push_back(message->nodeId);
  reTxIRPktNum.push_back(message->sequenceNumber);      
  nsgkaptr->ReTxIREPLY(message);
  }
}

void GKANode::evNodeInit() {
  cout << "From GKANode Node Init" << endl;
}

void GKAInstance::evIGROUP(IGROUPMessage* message, int nodeId) {
  cout << "From GKAInstance IGROUP" << endl;
}

void GKAInstance::evIREPLY(IREPLYMessage* message, int nodeId) {
  cout << "From GKAInstance IREPLY" << endl;
}

void GKAInstance::evNodeInit() {
  cout << "From GKAInstance Node Init" << endl;
}

void GKANode::nodeInit() {

  nsgkaptr->evNodeInit();
}
#else 

void GKANode::makeInstance(int node_ID, int num_sessions, GKASimul* nsgka)
{
  // Algorithm 2:
  nodeId = node_ID;
  isActive = false;
  nsgkaptr = nsgka;

  for (int i=0; i<num_sessions; i++) {

  // Later add the session details of each node and the initialize them
  GKAInstance *sessionPtr = new GKAInstance(scheduler);
  sessionPtr->nodeState = StateDetached;
  sessionPtr->isbackoffExists = 0; 
  sessionPtr->lastIGROUPMsgTime = scheduler->getTime();
  sessionPtr->lastTimeSecret = scheduler->getTime();
  sessionPtr->pendingLeaderList.clear();
  sessionPtr->memberList.clear();
  sessionPtr->sessionId++;
  sessionPtr->nsgkaptr = nsgka;
  sessionQueue.push_back(sessionPtr);
  }

  // Every node select a random time to become active
  double randomTime = (double)random()/(double)RAND_MAX*10;

  scheduler->addEventAt(randomTime, makeCallback(this, &GKANode::nodePeriodicUpdate), NULL);
}
#endif

void GKANode::evIGROUP(IGROUPMessage* message, int nodeId) {
cout << "GKANode IGROUP" << endl;
}

void GKANode::evIREPLY(IREPLYMessage* message, int nodeId) {
cout << "GKANode IREPLY"<< endl;
}

void GKAInstance::evIGROUP(IGROUPMessage* message, int nodeId) {
cout << "GKAInstance IGROUP" << endl;
}

void GKAInstance::evIREPLY(IREPLYMessage* message, int nodeId) {
cout << "GKAInstance IREPLY"<< endl;
}


void GKANode::nodePeriodicUpdate(void *temp)
{
  // Node is now active
  isActive = true; 

#ifdef NS2_WITH_NS2
  if (isInitSent == false ) {

  nodeInit();
  isInitSent = true;
  }
#endif

  for (unsigned int i=0;i<sessionQueue.size();i++) {
  sessionQueue.at(i)->updateState(nodeId,i);
  }

  // Every node should update with periodic intervals
  double nextUpdateTime = PERIOD_INTERVAL + scheduler->getTime();

#ifdef NS2_WITH_NS2

  scheduler->addEventAt(nextUpdateTime, makeCallback(this, &GKANode::nodePeriodicUpdate), NULL);

#else
  if (nsgkaptr->config->simulationTime >= nextUpdateTime) {

  scheduler->addEventAt(nextUpdateTime, makeCallback(this, &GKANode::nodePeriodicUpdate), NULL);
  }

  scheduler->addEventAt(nextUpdateTime, makeCallback(this, &GKANode::nodeMovementUpdate), NULL);
#endif
}

void GKANode::nodeMovementUpdate(void *temp)
{
  if (nsgkaptr->config->simScenario == 1 && nsgkaptr->config->numGroups == 0) {

   // Nodes are random movement with the boundary level
   srand((unsigned)time(0)); 
   int random = rand() % 8;
   int r1 = rand() % 50;
   int r2 = rand() % 50;
     if (random == 0) {
     
     locX += r1;
     locY += r2;
     } else if (random == 1) {
  
     locY += r1;
     } else if (random == 2) {

     locX -= r1;
     locY += r2;
     } else if (random == 3) {

     locX -= r2;
     } else if (random == 4) {

     locX -= r1;
     locY -= r2;
     } else if (random == 5) {

     locY -= r1;
     } else if (random == 6) {

     locX += r1;
     locY -= r2;
     } else if (random == 7) {

     locX += r2;
     }  
  } else if (nsgkaptr->config->simScenario == 2 && nsgkaptr->config->numGroups > 1) {

   // Nodes are random movement to merge at the center of the grid simulation area
   srand((unsigned)time(0)); 
   int r1 = rand() % 11;
   int r2 = rand() % 11;

   if (locX > 575) {
   
   locX -= r1;
   } else if (locX < 425) {

   locX += r1;
   }

   if (locY > 575) {

   locY -= r2;
   } else if (locY < 425) {

   locY += r2;
   }

   } else if (nsgkaptr->config->simScenario == 3 && nsgkaptr->config->numGroups > 1) {
     if (scheduler->getTime() >= (nsgkaptr->config->simulationTime / 2)) {
     locX = 500;
     locY = 500;
   }
  }
//printf("Node %d: %d X %d at time: %f\n", nodeId, locX, locY, scheduler->getTime());
}

void GKAInstance::updateState(int node_ID, int session_ID)
{
  // Cryptography Object Creation
  GkaECCrypto GKACrypto;
  list<CryptoElem> memberContribList;

#ifdef NS2_WITHOUT_NS2
  int a, b;
  double ComputeSec;
  double ComputeMic;
  double tempSec;
  double tempMic;
  double tempTime;
#endif

  double lastIG = scheduler->getTime() - lastIGROUPMsgTime;
  double lastSecret = scheduler->getTime() - lastTimeSecret;

  // Algorithm 10:
  if (nodeState == StateLeader) {
   unsigned int i = 0;

    while (i<memberList.size()) {
      double lastIR = scheduler->getTime() - memberList.at(i)->memberLastIREPLYTime; 
      if (lastIR >= IREPLY_EXPIRE_TIME) {
      memberList.erase(memberList.begin()+i);
      compGroupChange = true;
      }   
    i++;
    }
  }

  // Algorithm 3:
  if (nodeState != StateLeader && lastIG > LEADER_LAST_TIME && isbackoffExists != 1) {

#ifdef NS2_WITHOUT_NS2
  // Become Leader
  backoffTime = (((rand() % 5) + 1) * PERIOD_INTERVAL) + scheduler->getTime();
#else
  // Become Leader
  backoffTime = (MIN_BACKOFF + ((MAX_BACKOFF - MIN_BACKOFF) * Random::uniform())) + scheduler->getTime();
#endif

  nodeState = StateDetached; 
  isbackoffExists = 1;
  } 

  if (nodeState == StateDetached && isbackoffExists == 1 && backoffTime <= scheduler->getTime()) { 

  isbackoffExists = 0;
  // Session new random number Algo 3: 3.3
  nodeState = StateLeader;

  CryptoSessionId cryptosessionId = GKACrypto.generateRandomSessionId();
  sessionNext = cryptosessionId;
  memberList.clear();
  compGroupChange = true;
  }

  if (nodeState == StateLeader && isbackoffExists == 0) {  

   // IGROUP Generation
   if (compGroupChange == true || lastSecret > SECRET_EXPIRE_TIME) {
   
   // Generate node new secret
   CryptoExponent nodesecret = GKACrypto.generateRandomSecret();
   nodeSecret = nodesecret;   

   CryptoElem contrib = GKACrypto.computeBlindedSecret(nodesecret);
   nodeContrib = contrib;

   // Last secret generated time
   lastTimeSecret = scheduler->getTime();
   
   // Last nonce used by the node
   CryptoNonce lastnonce = GKACrypto.generateRandomNonce();
   lastNonce = lastnonce;

   sessionCurrent = sessionNext;

   CryptoSessionId cryptosession = GKACrypto.generateRandomSessionId();
   sessionNext = cryptosession;

   memberContribList.clear();

#ifdef NS2_WITHOUT_NS2

//  a = gettimeofday(&time, NULL);

  a = getrusage(RUSAGE_SELF, &time);
  if ( a == 0) {
//  tempSec = time.tv_sec;
//  tempMic = time.tv_usec;
//  tempSec = (double) time.ru_utime.tv_sec + (1.e-6 * (double) time.ru_utime.tv_usec);
//  tempMic = (double) time.ru_stime.tv_sec + (1.e-6 * (double) time.ru_stime.tv_usec);
  tempSec = (double) time.ru_utime.tv_sec;
  tempMic = (double) time.ru_utime.tv_usec;
  }
#endif 

   // Members List Update with Leaders response
   for (unsigned int i=0;i<memberList.size();i++) {

    memberList.at(i)->leaderReply = GKACrypto.computeBlindedResponse(memberList.at(i)->memberContrib, nodeSecret);
    memberContribList.push_back(memberList.at(i)->leaderReply);
   }
  
#ifdef NS2_WITHOUT_NS2

   CryptoElem leaderKey = GKACrypto.leaderComputeKey(nodeSecret, 
						 memberContribList);

   b = getrusage(RUSAGE_SELF, &time);
   if ( b == 0) {
   ComputeSec = (double) time.ru_utime.tv_sec - tempSec;
   ComputeMic = (double) time.ru_utime.tv_usec - tempMic;
   tempTime = (ComputeSec + (ComputeMic * 0.000001)) * 1000;
   }

  computeTime *computetime = new computeTime;
  computetime->nodeId = node_ID;
  computetime->status = "L";
  computetime->msgSeqNumber = IGmessageCount;
  computetime->numberofNodes = memberList.size();    
  computetime->timeTaken = tempTime;
  generateIGROUPTime.push_back(computetime);
#endif

  compGroupChange = false;
  } 

  // Generate IGROUP Messages
  IGROUPMessage *message = new IGROUPMessage;


  ComputeSec, ComputeMic, tempSec, tempMic, tempTime = 0.0;
  a, b = 0;

  // Signature Creation Time
  a = getrusage(RUSAGE_SELF, &time);
  if ( a == 0) {
  tempSec = (double) time.ru_utime.tv_sec;
  tempMic = (double) time.ru_utime.tv_usec;
  }

  Signature signature = ecdsaManager.sign(privkey, data);

  b = getrusage(RUSAGE_SELF, &time);
  if ( b == 0) {
   ComputeSec = (double) time.ru_utime.tv_sec - tempSec;
   ComputeMic = (double) time.ru_utime.tv_usec - tempMic;
  tempTime = (ComputeSec + (ComputeMic * 0.000001)) * 1000;
  }

  LeadersignTime += tempTime;
  LeadersignCount++;

  ComputeSec, ComputeMic, tempSec, tempMic, tempTime = 0.0;
  a, b = 0;

  message->leaderId = node_ID;
  message->sessionId = session_ID;
  message->leaderLastNonce = lastNonce;
  message->sessionCurrent = sessionCurrent;
  message->sessionNext = sessionNext;
  message->sequenceNumber = IGmessageCount;
  message->sign = signature;

  IGmessageCount++;

  if (memberList.size() > 0) {

   for (unsigned int i=0;i<memberList.size();i++) {
   memberInfo *memberinfo = new memberInfo;
   memberinfo->memberId = memberList.at(i)->memberId;
   memberinfo->memberLastNonce = memberList.at(i)->memberLastNonce;
   memberinfo->memberContrib = memberList.at(i)->memberContrib;
   memberinfo->leaderReply = memberList.at(i)->leaderReply;
   message->memberInfoList.push_back(memberinfo);\
   memberContribList.push_back(memberList.at(i)->leaderReply); //??
   }
  }

#ifdef NS2_WITH_NS2
  // Group Key identity checking (By using session's old and present values)
  if (sessionRef == GKACrypto.sessionToStr(message->sessionCurrent)) {
  nsgkaptr->isSessionSame = true; 
  } else {
  nsgkaptr->isSessionSame = false;
  sessionRef = GKACrypto.sessionToStr(message->sessionCurrent);
  }
#endif


  //Sending IGROUP message
  nsgkaptr->evIGROUP(message, node_ID);

  }

  // Algorithm 11:
  if(nodeState == StateMember && lastIG > LEADER_LAST_TIME){
  nodeState = StateDetached;
  }
  return;
}

void GKANode::processIGROUPMessage(IGROUPMessage* message)
{
  if (message->sessionId < sessionQueue.size()) {
  sessionQueue.at(message->sessionId)->processIGROUP(nodeId, message);
  }
}

void GKAInstance::processIGROUP(int node_ID, IGROUPMessage* message)
{ 

#ifdef NS2_WITHOUT_NS2
  int a, b;
  double ComputeSec, ComputeMic, tempSec, tempMic, tempTime;
  CryptoExponent memberlastsecret;

  a = getrusage(RUSAGE_SELF, &time);
  if ( a == 0) {
  tempSec = (double) time.ru_utime.tv_sec;
  tempMic = (double) time.ru_utime.tv_usec;
  }

  ecdsaManager.verify(pubkey, data, message->sign);

  b = getrusage(RUSAGE_SELF, &time);
  if ( b == 0) {
   ComputeSec = (double) time.ru_utime.tv_sec - tempSec;
   ComputeMic = (double) time.ru_utime.tv_usec - tempMic;
  tempTime = (ComputeSec + (ComputeMic * 0.000001)) * 1000;
  }

  IGROUPsignVeriTime += tempTime;
  IGROUPtotalVeriCount++; 

//  cout << "From Member: " << node_ID << " - " << ecdsaManager.verify(pubkey, data, message->sign) << " Time: "<< tempTime << endl;

  ComputeSec, ComputeMic, tempSec, tempMic, tempTime = 0.0;
  a, b = 0;

  a = getrusage(RUSAGE_SELF, &time);
  if ( a == 0) {
  tempSec = (double) time.ru_utime.tv_sec;
  tempMic = (double) time.ru_utime.tv_usec;
  }

  Signature signature = ecdsaManager.sign(privkey, data);

  b = getrusage(RUSAGE_SELF, &time);
  if ( b == 0) {
   ComputeSec = (double) time.ru_utime.tv_sec - tempSec;
   ComputeMic = (double) time.ru_utime.tv_usec - tempMic;
  tempTime = (ComputeSec + (ComputeMic * 0.000001)) * 1000;
  }

  MembersignTime += tempTime;
  MembersignCount++;

  ComputeSec, ComputeMic, tempSec, tempMic, tempTime = 0.0;
  a, b = 0;
#endif

  bool memberExists = false;
  bool isSameMember = false;
  bool isTrue = false;
  bool isKeyChanged = false;
  double lastSecret;
  CryptoElem leaderreply, contribution;
  CryptoNonce memberlastnonce;
  CryptoSessionId session; 

  list<CryptoElem> leaderResponseList;

  // Cryptography Object Creation
  GkaECCrypto GKACrypto;

  for (unsigned int i=0;i<message->memberInfoList.size();i++) 
  {
    if (message->memberInfoList.at(i)->memberId == node_ID)
    {
    memberExists = true;
    break;
    }
  }

  // Algorithm 5: Algo 5: 5.1
  if (nodeState == StateMember && message->leaderId == leaderId) {

  leaderResponseList.clear();
  for (unsigned int i=0;i<message->memberInfoList.size();i++) {

   if (message->memberInfoList.at(i)->memberId == node_ID) {

   // Values from received IGROUP Message
   leaderreply = message->memberInfoList.at(i)->leaderReply;
   contribution = message->memberInfoList.at(i)->memberContrib;
   memberlastnonce = message->memberInfoList.at(i)->memberLastNonce;
   }
  }

  // Algorihm 6:
  if (GKACrypto.sessionToStr(message->sessionCurrent) == GKACrypto.sessionToStr(sessionNext)) {
   sessionNext = message->sessionNext;
   sessionCurrent = message->sessionCurrent;
   leaderLastNonce = message->leaderLastNonce;   
   isKeyChanged = true;
  } else {

   isKeyChanged = false;
  }

  if (GKACrypto.sessionToStr(message->sessionCurrent) != GKACrypto.sessionToStr(sessionCurrent) || GKACrypto.nonceToStr(message->leaderLastNonce) != GKACrypto.nonceToStr(leaderLastNonce) || memberExists == false) 
  {
  // Stop processing..
  return;
  }

  if(GKACrypto.nonceToStr(memberlastnonce) != GKACrypto.nonceToStr(lastNonce) || GKACrypto.elemToStr(contribution) != GKACrypto.elemToStr(nodeContrib)) {

  // Stop processing..
  return;
  }

  // Key Compute...6.12

   if (memberList.size() > 0 && message->memberInfoList.size() > 0) {
   if (memberList.size() == message->memberInfoList.size()) {
   isSameMember = true;
   isTrue = true;
   } 
   }

  int x=0;
  if (isSameMember == true) {
   for (unsigned int j=0;j<memberList.size();j++) {
    for (unsigned int i=0;i<message->memberInfoList.size();i++) {
    if (memberList.at(j)->memberId == message->memberInfoList.at(i)->memberId) {
    x++;
    } 
    }
   }
    if ( int (memberList.size()) == x) {

    isTrue = true;
    } else {
    isTrue = false;
    }

  } else {

   memberList.clear();
   for (unsigned int i=0;i<message->memberInfoList.size();i++) {
   Member *memberPtr = new Member;
   memberPtr->memberId = message->memberInfoList.at(i)->memberId;
   memberList.push_back(memberPtr);
   }
   isSameMember = false;
  }

  if (isSameMember == false || isTrue == false || isKeyChanged == true) {
  leaderResponseList.clear();

#ifdef NS2_WITHOUT_NS2
  a = getrusage(RUSAGE_SELF, &time);
  if ( a == 0) {
  tempSec = (double) time.ru_utime.tv_sec;
  tempMic = (double) time.ru_utime.tv_usec;
  }
#endif

  for (unsigned int i=0;i<message->memberInfoList.size();i++) {

  leaderResponseList.push_back(message->memberInfoList.at(i)->leaderReply);
  }

#ifdef NS2_WITHOUT_NS2
  CryptoElem memberKey = GKACrypto.memberComputeKey(nodeSecret,leaderreply,
						  leaderResponseList);

  b = getrusage(RUSAGE_SELF, &time);
  if ( b == 0) {
   ComputeSec = (double) time.ru_utime.tv_sec - tempSec;
   ComputeMic = (double) time.ru_utime.tv_usec - tempMic;
  tempTime = (ComputeSec + (ComputeMic * 0.000001)) * 1000;
  }

  computeTime *computetime = new computeTime;
  computetime->nodeId = node_ID;
  computetime->status = "M";
  computetime->msgSeqNumber = message->sequenceNumber;
  computetime->leaderId = message->leaderId;
  computetime->numberofNodes = message->memberInfoList.size();    
  computetime->timeTaken = tempTime;
  processIGROUPTime.push_back(computetime);
#endif
  }

  leaderId = message->leaderId;
  lastIGROUPMsgTime = scheduler->getTime();
 
  if (lastSecret > SECRET_EXPIRE_TIME) {
   // Generate node new secret
   CryptoExponent nodesecret = GKACrypto.generateRandomSecret();
   nodeSecret = nodesecret;   

   lastTimeSecret = scheduler->getTime();

   // Last nonce used by the node
   CryptoNonce lastnonce = GKACrypto.generateRandomNonce();
   lastNonce = lastnonce;
   session = sessionNext; 

  } else if (isKeyChanged = true) {

   // Last nonce used by the node
   CryptoNonce lastnonce = GKACrypto.generateRandomNonce();
   lastNonce = lastnonce;
   session = sessionCurrent; 
  } else {

   session = sessionCurrent; 
  }
 
  // Algorithm 4:
  IREPLYMessage *messageIR = new IREPLYMessage;
  messageIR->leaderId = leaderId;
  messageIR->leaderLastNonce = leaderLastNonce;
  messageIR->nodeId = node_ID;
  messageIR->sessionId = message->sessionId;
  messageIR->nodeNonce = lastNonce;
  messageIR->nodeContrib = nodeContrib;
  messageIR->sessionCurrent = session;
  messageIR->sequenceNumber = IRmessageCount;

  messageIR->sign = signature;

  IRmessageCount++;

  //Sending IREPLY message to GKANode
  nsgkaptr->evIREPLY(messageIR, node_ID);
 } else if (memberExists == false) { 

  if((nodeState == StateMember && message->leaderId < leaderId) || (nodeState == StateLeader && message->leaderId < node_ID) || (nodeState == StateDetached)){

  // Algorithm 9:
  unsigned int i = 0;
  while (i < pendingLeaderList.size()) {
  
  double pendingleaderlastIG = scheduler->getTime() - pendingLeaderList.at(i)->pendingLeaderLastIGROUPTime;

  if (pendingleaderlastIG >= LEADER_LAST_TIME) {

  pendingLeaderList.erase(pendingLeaderList.begin() + i);
  }
  i++;
  }

  // Algorithm 7:
  CryptoExponent secret = GKACrypto.generateRandomSecret();

  // Generate Contribution
  CryptoElem contrib = GKACrypto.computeBlindedSecret(secret);

  CryptoNonce nonce = GKACrypto.generateRandomNonce();

  pendingLeader *pendingleader = new pendingLeader;
  pendingleader->pendingLeaderId = message->leaderId;
  pendingleader->pendingLeaderNonce = message->leaderLastNonce;
  pendingleader->pendingSessionNext = message->sessionCurrent;
  pendingleader->lastNonce = nonce;
  pendingleader->nodeSecret = secret;
  pendingleader->pendingLeaderLastIGROUPTime = scheduler->getTime();
  pendingLeaderList.push_back(pendingleader);

  // Algorithm 4:
  IREPLYMessage *messageIR = new IREPLYMessage;
  messageIR->leaderId = message->leaderId;
  messageIR->leaderLastNonce = message->leaderLastNonce;
  messageIR->nodeId = node_ID;
  messageIR->sessionId = message->sessionId;
  messageIR->nodeNonce = nonce;
  messageIR->nodeContrib = contrib;
  messageIR->sessionCurrent = message->sessionCurrent;
  messageIR->sequenceNumber = IRmessageCount;

  messageIR->sign = signature;

  IRmessageCount++;

  //Sending IREPLY message to GKANode
  nsgkaptr->evIREPLY(messageIR, node_ID);
  } else {

  // Stop the processing...
  return;
  }

  } else {

  // Algorithm 8:

  bool pendingLeaderExists = false;
  bool memberExists = false;
  CryptoNonce templeaderNonce, tempNonce;
  CryptoSessionId tempSession;
  CryptoElem tempContrib; 
  CryptoExponent tempSecret;
  double tempLastIGROUPTime;
  isSameMember = false;
  isTrue = false;

  leaderResponseList.clear();

  for (unsigned int i=0;i<message->memberInfoList.size();i++) {
   if (message->memberInfoList.at(i)->memberId == node_ID) {
   memberExists = true;
   leaderreply = message->memberInfoList.at(i)->leaderReply;
   contribution = message->memberInfoList.at(i)->memberContrib;
   memberlastnonce = message->memberInfoList.at(i)->memberLastNonce;
   }
  }

  for (unsigned int i=0;i<pendingLeaderList.size();i++){
   templeaderNonce = pendingLeaderList.at(i)->pendingLeaderNonce;
   tempNonce = pendingLeaderList.at(i)->lastNonce;
   tempSession = pendingLeaderList.at(i)->pendingSessionNext;
   tempSecret = pendingLeaderList.at(i)->nodeSecret;
   tempContrib = GKACrypto.computeBlindedSecret(tempSecret);
   tempLastIGROUPTime = pendingLeaderList.at(i)->pendingLeaderLastIGROUPTime;
   
   if (GKACrypto.nonceToStr(templeaderNonce) == GKACrypto.nonceToStr(message->leaderLastNonce) && GKACrypto.sessionToStr(tempSession) == GKACrypto.sessionToStr(message->sessionCurrent) && GKACrypto.nonceToStr(tempNonce) == GKACrypto.nonceToStr(memberlastnonce) && GKACrypto.elemToStr(tempContrib) == GKACrypto.elemToStr(contribution)) {

   return;
   } 
  } 

  unsigned int i=0;
  while (i<pendingLeaderList.size()) { 

   if(pendingLeaderList.at(i)->pendingLeaderId == message->leaderId) {
   pendingLeaderExists = true;
   templeaderNonce = pendingLeaderList.at(i)->pendingLeaderNonce;
   tempNonce = pendingLeaderList.at(i)->lastNonce;
   tempSession = pendingLeaderList.at(i)->pendingSessionNext;
   tempSecret = pendingLeaderList.at(i)->nodeSecret;
   tempContrib = GKACrypto.computeBlindedSecret(tempSecret);
   tempLastIGROUPTime = pendingLeaderList.at(i)->pendingLeaderLastIGROUPTime;
   pendingLeaderList.erase(pendingLeaderList.begin()+i);
   pendingLeaderExists = true;
   break;   
   }
   i++;
  }

  if (pendingLeaderExists == false) {

  return;
  }

  nodeState = StateMember;
  lastIGROUPMsgTime = scheduler->getTime();
  lastNonce = tempNonce;
  nodeSecret = tempSecret;
  nodeContrib = tempContrib;
  lastTimeSecret = tempLastIGROUPTime;
  leaderId = message->leaderId;
  leaderLastNonce = message->leaderLastNonce;
  sessionCurrent = message->sessionCurrent;
  sessionNext = message->sessionNext;  

  if (memberList.size() > 0 && message->memberInfoList.size() > 0) {
   if (memberList.size() == message->memberInfoList.size()) {
   isSameMember = true;
   isTrue = true;
   } 
  }

  unsigned int x=0;
  if (isSameMember == true) {
   for (unsigned int j=0;j<memberList.size();j++) {
    for (unsigned int i=0;i<message->memberInfoList.size();i++) {
    if (memberList.at(j)->memberId == message->memberInfoList.at(i)->memberId) {
    x++;
    } 
    }
   }

    if (memberList.size() == x) {
    isTrue = true;
    } else {
    isTrue = false;
    }

  } else {

   memberList.clear();

   for (unsigned int i=0;i<message->memberInfoList.size();i++) {
    Member *memberPtr = new Member;
    memberPtr->memberId = message->memberInfoList.at(i)->memberId;
    memberList.push_back(memberPtr);
   }
   isSameMember = false;
  }

  if (isSameMember == false || isTrue == false || isKeyChanged == true) {

   // Compute the Key..
   leaderResponseList.clear();  

#ifdef NS2_WITHOUT_NS2
   a = getrusage(RUSAGE_SELF, &time);
   if ( a == 0) {
  tempSec = (double) time.ru_utime.tv_sec;
  tempMic = (double) time.ru_utime.tv_usec;
   }
#endif

   for (unsigned int i=0;i<message->memberInfoList.size();i++) {
   leaderResponseList.push_back(message->memberInfoList.at(i)->leaderReply);
   }

#ifdef NS2_WITHOUT_NS2
   CryptoElem memberKey = GKACrypto.memberComputeKey(nodeSecret,leaderreply,
						  leaderResponseList);
 
   b = getrusage(RUSAGE_SELF, &time);
   if ( b == 0) {
   ComputeSec = (double) time.ru_utime.tv_sec - tempSec;
   ComputeMic = (double) time.ru_utime.tv_usec - tempMic;
   tempTime = (ComputeSec + (ComputeMic * 0.000001)) * 1000;
  }

   computeTime *computetime = new computeTime;
   computetime->nodeId = node_ID;
   computetime->status = "M";
   computetime->msgSeqNumber = message->sequenceNumber;
   computetime->leaderId = message->leaderId;
   computetime->numberofNodes = message->memberInfoList.size();    
   computetime->timeTaken = tempTime;
   processIGROUPTime.push_back(computetime);
#endif
  }

  // Algorithm 4:
  IREPLYMessage *messageIR = new IREPLYMessage;
  messageIR->leaderId = leaderId;
  messageIR->leaderLastNonce = leaderLastNonce;
  messageIR->nodeId = node_ID;
  messageIR->sessionId = sessionId;
  messageIR->nodeNonce = lastNonce;
  messageIR->nodeContrib = nodeContrib;
  messageIR->sessionCurrent = sessionCurrent;
  messageIR->sequenceNumber = IRmessageCount;

  messageIR->sign = signature;

  IRmessageCount++;

  //Sending IREPLY message to GKANode
  nsgkaptr->evIREPLY(messageIR, node_ID);
  }
  return;
}

void GKANode::processIREPLYMessage(IREPLYMessage* message)
{

  // Algorithm 4:
  if (message->sessionId < sessionQueue.size()) {
   if (sessionQueue.at(message->sessionId)->nodeState == StateLeader &&  message->leaderId == nodeId) {
 sessionQueue.at(message->sessionId)->processIREPLY(nodeId, message);
   } else {

  // Stop processing..
  return;
   }
  }
}

void GKAInstance::processIREPLY(int node_ID, IREPLYMessage* message)
{

#ifdef NS2_WITHOUT_NS2
  int a, b;
  double ComputeSec, ComputeMic, tempSec, tempMic, tempTime;
  CryptoExponent memberlastsecret;


  // Signature Verification Time
  a = getrusage(RUSAGE_SELF, &time);
  if ( a == 0) {
  tempSec = (double) time.ru_utime.tv_sec;
  tempMic = (double) time.ru_utime.tv_usec;
  }


  ecdsaManager.verify(pubkey, data, message->sign);


  b = getrusage(RUSAGE_SELF, &time);
  if ( b == 0) {
   ComputeSec = (double) time.ru_utime.tv_sec - tempSec;
   ComputeMic = (double) time.ru_utime.tv_usec - tempMic;
  tempTime = (ComputeSec + (ComputeMic * 0.000001)) * 1000;
  }

//  cout << "From Leader: " << node_ID << " - " << ecdsaManager.verify(pubkey, data, message->sign) << " Time: "<< tempTime << endl;

  IREPLYsignVeriTime += tempTime;
  IREPLYtotalVeriCount++; 

  ComputeSec, ComputeMic, tempSec, tempMic, tempTime = 0.0;
  a, b = 0;
#endif

  // Cryptography Object Creation
  GkaECCrypto GKACrypto;
  CryptoElem contrib;
  list<CryptoElem> memberContribList;
  bool sessionExists = false;
  
  if (GKACrypto.sessionToStr(message->sessionCurrent) == GKACrypto.sessionToStr(sessionCurrent) || GKACrypto.sessionToStr(message->sessionCurrent) == GKACrypto.sessionToStr(sessionNext)) {

  sessionExists = true;
  }
  
  if (GKACrypto.nonceToStr(message->leaderLastNonce) == GKACrypto.nonceToStr(lastNonce)  && sessionExists == true ) {

  bool memberExists = false;

  for (unsigned int i=0;i<memberList.size();i++) 
  {
   if (memberList.at(i)->memberId == message->nodeId)
   {
   memberExists = true;
   contrib = memberList.at(i)->memberContrib;
   break;
   }
  }
  
  if (memberExists == false)
  {
  Member *memberPtr = new Member;
  memberPtr->memberId = message->nodeId;
  memberPtr->memberLastIREPLYTime = scheduler->getTime();
  memberPtr->memberLastNonce = message->nodeNonce;
  memberPtr->memberContrib = message->nodeContrib;
  memberList.push_back(memberPtr);
  compGroupChange = true;

  } else if (memberExists == true && GKACrypto.elemToStr(contrib) != GKACrypto.elemToStr(message->nodeContrib)){

  compGroupChange = true;  
  }
  
   if (GKACrypto.sessionToStr(message->sessionCurrent) == GKACrypto.sessionToStr(sessionNext)) {

   compGroupChange = true; 
   }

   for (unsigned int i=0;i<memberList.size();i++) {
    if (message->nodeId == memberList.at(i)->memberId) {
    memberList.at(i)->memberLastNonce = message->nodeNonce;
    memberList.at(i)->memberContrib = message->nodeContrib;
    memberList.at(i)->memberLastIREPLYTime = scheduler->getTime();
    }
   }

 } else {

  // Stop processing..
  return;
 }
}

#ifdef NS2_WITHOUT_NS2

void GKANode::createReport(void *temp, FILE *fptr)
{
   
  for (unsigned int i=0;i<sessionQueue.size();i++) {
   for (unsigned int j=0;j<sessionQueue.at(i)->generateIGROUPTime.size();j++) {

     int IG_id = sessionQueue.at(i)->generateIGROUPTime.at(j)->nodeId;
     int IG_members = sessionQueue.at(i)->generateIGROUPTime.at(j)->numberofNodes;
     char* stateL = sessionQueue.at(i)->generateIGROUPTime.at(j)->status;
     double IG_keytime = sessionQueue.at(i)->generateIGROUPTime.at(j)->timeTaken;

   if (stateL == "L") {
  
   leaderComputeTime[IG_id][IG_members][0] += IG_keytime;
   leaderCounter[IG_id][IG_members][0]++;
   } 
   
   fprintf(fptr, "%d\t   %s\t\t%d  \t  %6.3f\t\t%d\n", sessionQueue.at(i)->generateIGROUPTime.at(j)->nodeId, sessionQueue.at(i)->generateIGROUPTime.at(j)->status, sessionQueue.at(i)->generateIGROUPTime.at(j)->msgSeqNumber, sessionQueue.at(i)->generateIGROUPTime.at(j)->timeTaken, sessionQueue.at(i)->generateIGROUPTime.at(j)->numberofNodes );
   }

   for (unsigned int j=0;j<sessionQueue.at(i)->processIGROUPTime.size();j++) {

     int IR_id = sessionQueue.at(i)->processIGROUPTime.at(j)->nodeId;
     int IR_members = sessionQueue.at(i)->processIGROUPTime.at(j)->numberofNodes;
     char* stateM = sessionQueue.at(i)->processIGROUPTime.at(j)->status; 
     double IR_keytime =sessionQueue.at(i)->processIGROUPTime.at(j)->timeTaken;

   if (stateM == "M") {

   memberComputeTime[IR_id][IR_members][0] += IR_keytime;
   memberCounter[IR_id][IR_members][0]++;
   }

   fprintf(fptr, "%d\t   %s\t\t%d\t  %6.3f\t\t%d\n", sessionQueue.at(i)->processIGROUPTime.at(j)->nodeId, sessionQueue.at(i)->processIGROUPTime.at(j)->status, sessionQueue.at(i)->processIGROUPTime.at(j)->msgSeqNumber,  sessionQueue.at(i)->processIGROUPTime.at(j)->timeTaken, sessionQueue.at(i)->processIGROUPTime.at(j)->numberofNodes );
   }
  }
}
#endif
