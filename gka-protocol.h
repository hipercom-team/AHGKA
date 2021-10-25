//-----------------------------------------------------------------*- c++ -*-
//                          INRIA Rocquencourt
//  Copyright Institut National de Recherche en Informatique et
//  en Automatique.  All rights reserved.  Distributed only with permission.
//--------------------------------------------------------------------------

//---------------------------------------------------------------------------
//                         Ad Hoc GKA
//---------------------------------------------------------------------------
#ifndef __gka_protocol_h__
#define __gka_protocol_h__

#define NS2_WITHOUT_NS2

// These files comes from outside
#include "api-scheduler.h"
#include <vector>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include "ecdsa-crypto.h"

//---------------------------------------------------------------------------
#ifdef NS2_WITH_NS2
#include <random.h>
#include <classifier-port.h>
#include "gka-crypto.h"
#include "gka-simul-ns2.h"
#endif

#define PERIOD_INTERVAL 5.0
#define LEADER_LAST_TIME 20.0
#define IREPLY_EXPIRE_TIME 20.0
#define SECRET_EXPIRE_TIME 100.0

#ifdef NS2_WITHOUT_NS2
static double leaderComputeTime[100][100][1];
static int leaderCounter[100][100][1];
static double leaderAverageTime[100][100][1];
static double memberComputeTime[100][100][1];
static int memberCounter[100][100][1];
static double memberAverageTime[100][100][1];
double avgLeaderkeytime[100], avgMemberkeytime[100];
int leaderKeyCount[100], memberKeyCount[100];

static double LeadersignTime=0.0;
static int LeadersignCount=0;
static double MembersignTime=0.0;
static int MembersignCount=0;

static double IGROUPsignVeriTime=0.0;
static int IGROUPtotalVeriCount=0;

static double IREPLYsignVeriTime=0.0;
static int IREPLYtotalVeriCount=0;

ECDSAManager ecdsaManager;
PubKey pubkey;
PrivKey privkey;
string data = "aBcDeF";
#endif

class GKAInstance;
class GKANode;

#ifdef NS2_WITH_NS2
class NSGKANode;
#else
class GKASimul;
#endif


class memberInfo
{
public:
  /* Information of member blind secret in IGROUP message*/
  int memberId;
  CryptoNonce memberLastNonce;
  CryptoElem memberContrib, leaderReply;

protected:

};

class Member
{
public:
  int memberId;
  int membeToken;
  double memberLastIREPLYTime;
  CryptoNonce memberLastNonce;
  CryptoElem memberContrib, leaderReply;
    
protected:

};

class computeTime
{
public:
  int nodeId;
  char *status;
  int leaderId; 
  int msgSeqNumber;
  int numberofNodes;    
  double timeTaken;
  double secTaken;
  double usecTaken;

protected:

};

class pendingLeader
{
public:
  int pendingLeaderId;
  int pendingLeaderSessionId;
  int Pending_secret;
  double pendingLeaderLastIGROUPTime;
  CryptoNonce pendingLeaderNonce, lastNonce, PendingreplyNonce;
  CryptoSessionId pendingSessionNext;
  CryptoElem nodeContrib;
  CryptoExponent nodeSecret;

protected:

};

class IGROUPMessage 
{
public:
  IGROUPMessage(); 
  int leaderId;
  int sessionId;
  int sequenceNumber;
  CryptoNonce leaderLastNonce;
  CryptoSessionId sessionCurrent;
  CryptoSessionId sessionNext;
  vector <memberInfo*> memberInfoList;

  Signature sign;

protected:

};

class IREPLYMessage
{
public:
  IREPLYMessage();
  int nodeId;  
  int leaderId;
  int sessionId;
  int sequenceNumber;
  CryptoNonce leaderLastNonce;
  CryptoNonce nodeNonce;
  CryptoElem nodeContrib;
  CryptoSessionId sessionCurrent;

  Signature sign;

protected:

};

class IpacketExchange {

public:
  virtual void evIGROUP(IGROUPMessage* message, int nodeId) = 0;
  virtual void evIREPLY(IREPLYMessage* message, int nodeId) = 0;
#ifdef NS2_WITH_NS2
  virtual void evNodeInit() = 0;
#endif
};

class GKANode : public IpacketExchange {

public:

  GKANode(IScheduler* aScheduler, GKAInstance* aGKAInst):scheduler(aScheduler), gkainst(aGKAInst) {}
  int locX, locY;
#ifdef NS2_WITH_NS2
  void makeInstance(int node_ID, int num_sessions, NSGKANode* nsgka);
#else 
  void makeInstance(int node_ID, int num_sessions, GKASimul* nsgka);
#endif
  void createReport(void *temp, FILE *fptr);
  void evTest(void* unused); 
  void evIGROUP(IGROUPMessage* message, int nodeId);
  void evIREPLY(IREPLYMessage* message, int nodeId);
  void nodePeriodicUpdate(void* temp);
  void nodeMovementUpdate(void* temp);
  void processIGROUPMessage(IGROUPMessage* message);
  void processIREPLYMessage(IREPLYMessage* message);
  vector <GKAInstance *> sessionQueue;

  vector <int> RxIGnodes;
  vector <int> RxIGNumber;
  vector <double> RxIGTime;
  vector <int> RxIRnodes;
  vector <int> RxIRNumber;
  vector <double> RxIRTime;
  bool  isActive; // true-Active, false-Active

  // For Packet Retransmissions
  vector <int> usrListIGReTxPkt;
  vector <int> usrListIRReTxPkt;

  vector <int> reTxIGPktNum;
  vector <int> reTxIRPktNum;
  void evNodeInit();
  void nodeInit();
  bool isInitSent;

#ifdef NS2_WITH_NS2
  NSGKANode* nsgkaptr;
#else
  GKASimul* nsgkaptr;
#endif

  int nodeId;

protected:
  IScheduler* scheduler;
  GKAInstance* gkainst; 
};

class GKAInstance : public IpacketExchange {

public:
  GKAInstance(IScheduler* aScheduler):scheduler(aScheduler) {}
  void updateState(int node_ID, int session_ID);  
  void generateIGROUPMessage(int node_ID, int session_ID);
  void processIGROUP(int node_ID, IGROUPMessage* message);
  void generateIREPLYMessage(int node_ID, int session_ID, int leader_ID);
  void processIREPLY(int node_ID, IREPLYMessage* message);
  void evIGROUP(IGROUPMessage* message, int nodeId);
  void evIREPLY(IREPLYMessage* message, int nodeId);
  int nodeState; // 0-Detached, 1-Leader, 2-Member
  double backoffTime; 
  double lastTimeSecret;
  double lastIGROUPMsgTime;
  int isbackoffExists; // 0-not exists, 1-exists
  int leaderId;
  int sessionId;
  CryptoSessionId sessionCurrent;
  CryptoSessionId sessionNext;
  CryptoExponent nodeSecret;
  CryptoNonce lastNonce;
  CryptoNonce leaderLastNonce;
  CryptoElem nodeContrib;

  vector <Member *> memberList;
  vector <pendingLeader *> pendingLeaderList;
  vector <computeTime *> generateIGROUPTime;
  vector <computeTime *> processIGROUPTime;

#ifdef NS2_WITH_NS2
  void evNodeInit();
  NSGKANode* nsgkaptr;
#else
  GKASimul* nsgkaptr;
#endif
  bool compGroupChange;
  struct rusage time;
  string sessionRef;

protected:
  IScheduler* scheduler;
};

#endif
