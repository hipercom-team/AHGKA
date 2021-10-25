//-----------------------------------------------------------------*- c++ -*-
//                          INRIA Rocquencourt
//  Copyright Institut National de Recherche en Informatique et
//  en Automatique.  All rights reserved.  Distributed only with permission.
//--------------------------------------------------------------------------

#include <algorithm>

class Signature
{
public:
  string content;
};

class PubKey
{
public:
  EC_KEY* pubkey;
};

class PrivKey
{
public:
  EC_KEY* eckey;

  PrivKey() : eckey(NULL) { }

};

#define CONST_KEY_HACK

class ECDSAManager
{
public:

  //int nid = NID_secp160r1 ; 

  //EC_KEY		*eckey = NULL, *wrong_eckey = NULL, *pubkey = NULL;
  //EC_GROUP	*group;

#ifdef CONST_KEY_HACK
  bool initialized;
  EC_KEY* eckey;
#endif

  ECDSAManager() : initialized(false), eckey(NULL) { }

  void generateKey(PubKey& pubKey, PrivKey& privKey)
  {
    int nid = NID_sect163k1; /* from objects.h, which defined cryptographic constants, as a curve "nist name" */

    const EC_GROUP* group = EC_GROUP_new_by_curve_name(nid);
  
    /* create a key */
  
#ifdef CONST_KEY_HACK
    if (!initialized) {
      if ((eckey = EC_KEY_new()) == NULL)
	abort();
      
      if (EC_KEY_set_group(eckey, group) == 0)
	abort();
      
      if (!EC_KEY_generate_key(eckey))
	abort();
      
      if (!EC_KEY_check_key(eckey))
	abort();

      initialized = true;
    }
    
#else
#error not implemented
#endif


    assert(privKey.eckey == NULL);
    privKey.eckey = eckey;

    // extract public part

    unsigned char* rawContent = NULL;
    int rawContentSize = i2o_ECPublicKey(eckey, &rawContent);
    //printf("EC_KEY, %d rawContent=",  rawContentSize, rawContent);
    //dump(rawContent, rawContentSize);
    //printf("\n");
    //
    
    /* copy to public key */

    EC_KEY* pubkey;
    
    if ((pubkey = EC_KEY_new()) == NULL)
      abort();
    
    if (EC_KEY_set_group(pubkey, group) == 0)
      abort();
    
    if (o2i_ECPublicKey(&pubkey, 
			//const_cast<const unsigned char**>(&rawContent), 
			(const unsigned char**)(&rawContent), 
			rawContentSize) == NULL)
      abort();
    
    if (!EC_KEY_check_key(pubkey))
      abort();

    pubKey.pubkey = pubkey;
  }

  Signature sign(const PrivKey& privKey, string data)
  {

    unsigned char *signature = NULL;     
    unsigned int sig_len = ECDSA_size(privKey.eckey);
    if ((signature = (unsigned char*) OPENSSL_malloc(sig_len)) == NULL)
      abort();

#warning not hashing
    unsigned char digest[20];
    memset(digest, 0, 20);
    memcpy(digest, data.c_str(), std::min(data.size(), (size_t)20));

    if (!ECDSA_sign(0, digest, 20, signature, &sig_len, privKey.eckey))
      abort();

    Signature result;
    result.content = string((char*)signature, (size_t)sig_len);
    
    OPENSSL_free(signature);

    // eckey.priv_key = NULL;
    return result;
  }
  
  bool verify(const PubKey& pubKey, string data, const Signature& signature)
  {
#warning not hashing
    unsigned char digest[20];
    memset(digest, 0, 20);
    memcpy(digest, data.c_str(), std::min(data.size(), (size_t)20));    

    return ECDSA_verify(0, digest, 20, 
			(const unsigned char*) signature.content.c_str(), 
			signature.content.size(), pubKey.pubkey) == 1;
  }
};


//---------------------------------------------------------------------------

