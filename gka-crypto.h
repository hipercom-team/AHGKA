//-----------------------------------------------------------------*- c++ -*-
//                          INRIA Rocquencourt
//  Copyright Institut National de Recherche en Informatique et
//  en Automatique.  All rights reserved.  Distributed only with permission.
//--------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Template methods for cryptography
//---------------------------------------------------------------------------
// Use documentation from IGkaCrypto
// Use the classe GkaDummyCrypto
//---------------------------------------------------------------------------

#include <assert.h>

#include <iostream>
#include <sstream>
#include <string>
#include "sys/time.h"
using std::string;


#include "openssl/ec.h"
#include "openssl/bn.h"
#include "openssl/objects.h"
#include "openssl/ecdsa.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
typedef EC_POINT* CryptoElem;
typedef BIGNUM* CryptoExponent;

class CryptoNonce
{
public: 
  string value;
};

typedef CryptoNonce CryptoSessionId;

int curveType;

static double leaderKeyComputeTime;
static int leaderKeyCounter;
static double leaderKeyAverageTime;

static double memberKeyComputeTime;
static int memberKeyCounter;
static double memberKeyAverageTime;

static double blindResponseComputeTime;
static int blindResponseCounter;
static double blindResponseAverageTime;

static double blindSecretComputeTime;
static int blindSecretCounter;
static double blindSecretAverageTime;

int a, b;
double ComputeSec;
double ComputeMic;
double tempSec;
double tempMic;
double tempTime;

class IGkaCrypto
{
public:

  // Generate a random secret
  virtual CryptoExponent generateRandomSecret() = 0;

  // Generate a new random nonce
  virtual CryptoNonce generateRandomNonce() = 0;

  // Generate a new session identifier
  virtual CryptoSessionId generateRandomSessionId() = 0;

  // [For member]: take an existing secret and blind it
  virtual CryptoElem computeBlindedSecret(CryptoExponent& randomSecret) = 0;

  // [For leader]: take a member blinded secret, the leader secret,
  //               and compute the blinded response
  virtual CryptoElem computeBlindedResponse(CryptoElem& memberBlindedSecret,
					    CryptoExponent& leaderRandomSecret) =0;

  // [For member]: 
  virtual CryptoElem memberComputeKey(CryptoExponent& memberRandomSecret,
				      CryptoElem& blindedResponseFromLeader,
				      list<CryptoElem>& blindedResponseList
				      ) = 0;

  // [For leader]:
  virtual CryptoElem leaderComputeKey(CryptoExponent& randomSecret,
				      list<CryptoElem>& blindedSecretList)=0;
  
  virtual ~IGkaCrypto() { }
  
  struct timeval time;

};

class GkaECCrypto : public IGkaCrypto
{
public:
	
  char * randomChar;
  int curveId;

  EC_GROUP  *G;
  const EC_POINT *g ;
  BIGNUM *ord ;

  static BN_CTX * bn_ctx ;

  void displayCryptoTime() {

  leaderKeyAverageTime = leaderKeyComputeTime / leaderKeyCounter;
  memberKeyAverageTime = memberKeyComputeTime / memberKeyCounter;
  blindResponseAverageTime = blindResponseComputeTime / blindResponseCounter;
  blindSecretAverageTime = blindSecretComputeTime / blindSecretCounter;

  cout << endl;
  cout << endl;
  cout << "Leader Average Time: " << leaderKeyAverageTime << " ("<< leaderKeyCounter << ")"<< endl;
  cout << "Member Average Time: " << memberKeyAverageTime << " ("<< memberKeyCounter << ")" << endl;
  cout << "Blind Response Average Time: " << blindResponseAverageTime << " ("<< blindResponseCounter << ")" << endl;
  cout << "Blind Secret Average Time: " <<  blindSecretAverageTime << " ("<< blindSecretCounter << ")" << endl;
  cout << endl;
  }

  GkaECCrypto() 
  { 

  switch (curveType){
    case 1:
	G = EC_GROUP_new_by_curve_name(NID_secp160r1);	
      break;
    case 2:
	G = EC_GROUP_new_by_curve_name(NID_secp224r1);	
      break;
    case 3:
	G = EC_GROUP_new_by_curve_name(NID_secp224k1);	
      break;
    case 4:
	G = EC_GROUP_new_by_curve_name(NID_secp128r1);	
      break;
    case 5:
	G = EC_GROUP_new_by_curve_name(NID_secp192k1);	
      break;
    case 6:
	G = EC_GROUP_new_by_curve_name(NID_secp256k1);	
      break;

      // char. p:
    case 10: G = EC_GROUP_new_by_curve_name(NID_secp160r1); break;
    case 11: G = EC_GROUP_new_by_curve_name(NID_secp160r2); break;
    case 12: G = EC_GROUP_new_by_curve_name(NID_secp160k1); break;
      // char 2.:
    case 20: G = EC_GROUP_new_by_curve_name(NID_sect163r1); break;
    case 21: G = EC_GROUP_new_by_curve_name(NID_sect163r2); break;
    case 22: G = EC_GROUP_new_by_curve_name(NID_sect163k1); break;


    default: 
	G = EC_GROUP_new_by_curve_name(NID_secp112r1);	
  }
        ord = BN_new();
	EC_GROUP_get_order(G, ord, bn_ctx);
        g = EC_GROUP_get0_generator(G);
  }

  virtual ~GkaECCrypto() {}

  virtual CryptoExponent generateRandomSecret()
  { 
    BIGNUM* a = BN_new();
    BN_rand_range(a,ord);
    return a; 
  }

  virtual CryptoNonce generateRandomNonce()
  { 
    randomChar = (char*) malloc (21);

    for (int n=0; n<21; n++) {
    randomChar[n]=rand()%26+'a';
    }
    randomChar[21]='\0';

    CryptoNonce result;
    result.value = randomChar; 
    free (randomChar);
    return result;
  }

  virtual CryptoSessionId generateRandomSessionId()
  {
    randomChar = (char*) malloc (21);

    for (int n=0; n<21; n++) {
    randomChar[n]=rand()%26+'a';
    }
    randomChar[21]='\0';

    CryptoNonce result;
    result.value = randomChar;
    free (randomChar);
    return result;
  }

  virtual CryptoElem computeBlindedSecret(CryptoExponent& randomSecret)
  { 

  a = gettimeofday(&time, NULL);
  if ( a == 0) {
  tempSec = time.tv_sec;
  tempMic = time.tv_usec;
  }

    EC_POINT * g1 = EC_POINT_new (G);
    // [expensive]:
    EC_POINT_mul ( G, g1, NULL, g, randomSecret , bn_ctx ); 

  b = gettimeofday(&time, NULL);
  if ( b == 0) {
  ComputeSec = time.tv_sec - tempSec;
  ComputeMic = time.tv_usec - tempMic;
  tempTime = (ComputeSec + (ComputeMic * 0.000001)) * 1000;
  }

  blindSecretComputeTime += tempTime;
  blindSecretCounter++;

    return g1; 
  }

  virtual CryptoElem computeBlindedResponse(CryptoElem& memberBlindedSecret,
					    CryptoExponent& leaderRandomSecret)
  {
    a = gettimeofday(&time, NULL);
    if ( a == 0) {
    tempSec = time.tv_sec;
    tempMic = time.tv_usec;
    }

    EC_POINT *g1  = EC_POINT_new (G);
    // [expensive]:
    EC_POINT_mul ( G, g1, NULL, memberBlindedSecret, leaderRandomSecret , bn_ctx );

    b = gettimeofday(&time, NULL);
    if ( b == 0) {
    ComputeSec = time.tv_sec - tempSec;
    ComputeMic = time.tv_usec - tempMic;
    tempTime = (ComputeSec + (ComputeMic * 0.000001)) * 1000;
    }

    blindResponseComputeTime += tempTime;
    blindResponseCounter++;

    return g1;
  }

  virtual CryptoElem memberComputeKey(CryptoExponent& randomSecret,
				      CryptoElem& blindedResponseForMember,
				      list<CryptoElem>& blindedResponseList
				      )
  {
    a = gettimeofday(&time, NULL);
    if ( a == 0) {
    tempSec = time.tv_sec;
    tempMic = time.tv_usec;
    }

    EC_POINT *g0 = EC_POINT_new (G);
    BIGNUM *inv = BN_new ();
    BN_mod_inverse ( inv, randomSecret, ord, bn_ctx) ; // [little expensive?]

    // [expensive]:
    EC_POINT_mul ( G, g0, NULL, blindedResponseForMember, inv , bn_ctx );
    //EC_POINT_copy(current, g0); 
    EC_POINT *current = EC_POINT_dup (g0, G);
    for (std::list<CryptoElem>::iterator it = blindedResponseList.begin();
	 it != blindedResponseList.end(); it++) {
      CryptoElem& currentResponse = *it;
      EC_POINT *tmp = EC_POINT_dup (current, G);
      EC_POINT_add ( G, current, tmp, currentResponse , bn_ctx );
    }

    b = gettimeofday(&time, NULL);
    if ( b == 0) {
    ComputeSec = time.tv_sec - tempSec;
    ComputeMic = time.tv_usec - tempMic;
    tempTime = (ComputeSec + (ComputeMic * 0.000001)) * 1000;
    }

    memberKeyComputeTime += tempTime;
    memberKeyCounter++;

    return current; 
  }

  virtual CryptoElem leaderComputeKey(CryptoExponent& randomSecret,
				      list<CryptoElem>& blindedResponseList)

  {
    a = gettimeofday(&time, NULL);
    if ( a == 0) {
    tempSec = time.tv_sec;
    tempMic = time.tv_usec;
    }

    EC_POINT *current = EC_POINT_new (G);
    // [expensive]:
    EC_POINT_mul ( G, current, NULL, g, randomSecret , bn_ctx );
    for (std::list<CryptoElem>::iterator it = blindedResponseList.begin();
	 it != blindedResponseList.end(); it++) {
      CryptoElem& currentResponse = *it;
      EC_POINT *tmp = EC_POINT_dup (current, G);
      EC_POINT_add ( G, current, tmp, currentResponse , bn_ctx );
    }

    b = gettimeofday(&time, NULL);
    if ( b == 0) {
    ComputeSec = time.tv_sec - tempSec;
    ComputeMic = time.tv_usec - tempMic;
    tempTime = (ComputeSec + (ComputeMic * 0.000001)) * 1000;
    }

    leaderKeyComputeTime += tempTime;
    leaderKeyCounter++;

    return current;
  }

  virtual string elemToStr(CryptoElem& elem)
  { 
   char* str = EC_POINT_point2hex(G,elem,POINT_CONVERSION_COMPRESSED, bn_ctx); 
   return str;
  }

 virtual string exponentToStr(CryptoExponent& elem)
  { 
   char* str = BN_bn2hex(elem);
   return str;
 }

  virtual string nonceToStr(CryptoNonce& nonce)
  { return nonce.value; }

  virtual string sessionToStr(CryptoSessionId& sessionId)
  { return sessionId.value; }

protected:
};

BN_CTX*  GkaECCrypto::bn_ctx = BN_CTX_new();

