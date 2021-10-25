//-----------------------------------------------------------------*- c++ -*-
//                          INRIA Rocquencourt
//  Copyright Institut National de Recherche en Informatique et
//  en Automatique.  All rights reserved.  Distributed only with permission.
//--------------------------------------------------------------------------

#include <list>
#include <iostream>
using std::list;
using std::cout;
using std::cerr;
using std::endl;

#include "gka-crypto.h"
#include "ecdsa-crypto.h"

int main(int argc, char** argv)
{
  GkaECCrypto crypto;

  CryptoNonce nonce = crypto.generateRandomNonce();
  cout << "Nonce: " << crypto.nonceToStr(nonce) << endl;
  CryptoSessionId sessionId = crypto.generateRandomSessionId();
  cout << "Session-id: " << crypto.sessionToStr(sessionId) << endl;

  CryptoExponent member1Secret = crypto.generateRandomSecret();
  CryptoExponent member2Secret = crypto.generateRandomSecret();
  CryptoExponent leaderSecret = crypto.generateRandomSecret();  
  cout << "Secret for member1: " << crypto.exponentToStr(member1Secret) << endl;
  cout << "Secret for member2: " << crypto.exponentToStr(member2Secret) << endl;
  cout << "Secret for leader: " << crypto.exponentToStr(leaderSecret) << endl;

  CryptoElem member1Contrib = crypto.computeBlindedSecret(member1Secret);
  CryptoElem member2Contrib = crypto.computeBlindedSecret(member2Secret);
  list<CryptoElem> memberContribList;
  memberContribList.push_back(member1Contrib);
  memberContribList.push_back(member2Contrib);
  cout << "Contrib for member1: " << crypto.elemToStr(member1Contrib) << endl;
  cout << "Contrib for member2: " << crypto.elemToStr(member2Contrib) << endl;

  CryptoElem leaderReply1 = crypto.computeBlindedResponse(member1Contrib, 
							 leaderSecret);
  CryptoElem leaderReply2 = crypto.computeBlindedResponse(member2Contrib, 
							  leaderSecret);
  list<CryptoElem> leaderResponseList;
  leaderResponseList.push_back(leaderReply1);
  leaderResponseList.push_back(leaderReply2);
  cout << "Resp. for member1: " << crypto.elemToStr(leaderReply1) << endl;
  cout << "Resp. for member2: " << crypto.elemToStr(leaderReply2) << endl;

  CryptoElem member1Key = crypto.memberComputeKey(member1Secret,leaderReply1,
						  leaderResponseList);
  CryptoElem member2Key = crypto.memberComputeKey(member2Secret,leaderReply2,
						  leaderResponseList);
  CryptoElem leaderKey = crypto.leaderComputeKey(leaderSecret,
  leaderResponseList);
			//			 memberContribList);
  cout << "Key for member1: " << crypto.elemToStr(member1Key) << endl;
  cout << "Key for member2: " << crypto.elemToStr(member2Key) << endl;
  cout << "Key for leader: " << crypto.elemToStr(leaderKey) << endl;


  //--------------------------------------------------

  ECDSAManager ecdsaManager;

  PubKey pubKey1, pubKey2;
  PrivKey privKey1, privKey2;

  ecdsaManager.generateKey(pubKey1, privKey1);
  ecdsaManager.generateKey(pubKey2, privKey2);

  string data = "12345678901234567890";

  Signature signature = ecdsaManager.sign(privKey1, data);
  cout << "ECDSA signature verif (key1): " 
       << ecdsaManager.verify(pubKey1, data, signature) << endl;
  cout << "ECDSA signature verif (key2): " 
       << ecdsaManager.verify(pubKey2, data, signature) << endl;
}

//---------------------------------------------------------------------------
