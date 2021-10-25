//-----------------------------------------------------------------*- c++ -*-
//  Copyright Daniel Augot.  
//  All rights reserved.  Distributed only with permission.
//--------------------------------------------------------------------------

#include "openssl/ec.h"
#include "openssl/bn.h"
#include "openssl/objects.h"

//#include "openssl/ec_lcl.h"


void dump(unsigned char* content, int size)
{  
  int i;
  for (i=0; i<size; i++)
    printf("%02x", content[i]);
}

int main( int argc , char ** argv) {

  char *str;

  char out[160] ;

  int len = 160 ;


  /* bn_ctx is a misterious thing, that must be passed to many
     functions, such that these function can perform auxiliary
     internal operations "in place", using the storage allocated into bn_ctx */
  /* bn_ctx has no otherwise meaningful value, it can be reused all
     the time, as it seems.*/
  BN_CTX * bn_ctx = BN_CTX_new();

  BIGNUM * r0, * r1 ,*r2 , * ord , * inv_r1, *a , *b, * inv_a ;

  int nid = NID_secp160r1 ; /* from objects.h, which defined cryptographic constants, as a curve "nist name" */
 
  int cmp ;

  const EC_GROUP  *G;

  const EC_POINT * g;

  EC_POINT  *g1, *g2, *g3, *g4, * contrib , * member_secret, * leader_secret , * response , *secret;


  /* see ecdsatest.c*/
  EC_KEY		*eckey = NULL, *wrong_eckey = NULL, *pubkey = NULL;
  EC_GROUP	*group;
  unsigned char	digest[20], wrong_digest[20];

  unsigned char	*signature = NULL; 
  unsigned int	sig_len;
  //int		nid, ret =  0;

  unsigned char* pout = NULL;
  
  int poutSize = -1;

  /* fill digest values with some random data */
  if (!RAND_pseudo_bytes(digest, 20) ||
      !RAND_pseudo_bytes(wrong_digest, 20))
    {
      abort();
    }

  /* create a key */
  
  

  /* allocate for the big integers */

  a = BN_new();
  b = BN_new();
  r0 = BN_new() ;
  r1 = BN_new() ;
  r2 = BN_new() ;
  ord = BN_new() ;
  inv_a = BN_new();
  inv_r1= BN_new();

  /* get the curve from her "name" */

//  nid = NID_secp160r1 ;
  
  nid = NID_sect163k1;

  G = EC_GROUP_new_by_curve_name(nid);		


  /*--------------------------------------------------*/

  /* create a key */
  
  if ((eckey = EC_KEY_new()) == NULL)
    abort();

  group = G;

  if (EC_KEY_set_group(eckey, group) == 0)
    abort();
  
  if (!EC_KEY_generate_key(eckey))
    abort();

  if (!EC_KEY_check_key(eckey))
    abort();


  poutSize = i2o_ECPublicKey(eckey, &pout);
  printf("EC_KEY, %d pout=",  poutSize, pout);
  dump(pout, poutSize);
  printf("\n");
  //pout = NULL;

  /* copy to public key */

  if ((pubkey = EC_KEY_new()) == NULL)
    abort();

  if (EC_KEY_set_group(pubkey, group) == 0)
    abort();

  if (o2i_ECPublicKey(&pubkey, &pout, poutSize) == NULL)
    abort();

  if (!EC_KEY_check_key(pubkey))
    abort();


  pout = NULL;
  poutSize = i2o_ECPublicKey(eckey, &pout);
  printf("EC_KEY, %d oldkey pout=",  poutSize, pout);
  dump(pout, poutSize);
  printf("\n");
  pout = NULL;

  poutSize = i2o_ECPublicKey(pubkey, &pout);
  printf("EC_KEY, %d pubkey, pout=",  poutSize, pout);
  dump(pout, poutSize);
  printf("\n");  
  pout = NULL;

  poutSize = i2o_ECPublicKey(eckey, &pout);
  printf("EC_KEY, %d oldkey pout=",  poutSize, pout);
  dump(pout, poutSize);
  printf("\n");
  pout = NULL;

  // ... and signature 
  sig_len = ECDSA_size(eckey);
  if ((signature = OPENSSL_malloc(sig_len)) == NULL)
    abort();

  if (!ECDSA_sign(0, digest, 20, signature, &sig_len, eckey))
    abort();

  // eckey->priv_key = NULL;

  if (ECDSA_verify(0, digest, 20, signature, sig_len, eckey) == 1)
    printf("ECDSA signature with old key ok\n");
  else abort();


  if (ECDSA_verify(0, digest, 20, signature, sig_len, pubkey) == 1)
    printf("ECDSA signature with pub key ok\n");
  else abort();

  /*--------------------------------------------------*/


  /* allocate for points */

  g1= EC_POINT_new(G);
  g2= EC_POINT_new(G);
  g3= EC_POINT_new(G);
  g4= EC_POINT_new(G);

  leader_secret = EC_POINT_new(G);
  member_secret = EC_POINT_new(G);
  response = EC_POINT_new(G);
  secret = EC_POINT_new(G);

  /* get the generator of the curve, no need to allocate for g */

  g = EC_GROUP_get0_generator(G);

  /* get the order of the curve into ord */

  EC_GROUP_get_order(G, ord, bn_ctx);

  printf("Order of the curve : %s\n", BN_bn2hex(ord));

  /* get a random number less than ord into a */

  BN_rand_range(a,ord);

  /* multiply g by a, get the result into g1 */

  EC_POINT_mul ( G, g1, NULL, g, a , bn_ctx );

  /* compare the points g and g1, they should be different */
  cmp = EC_POINT_cmp(G,g,g1,bn_ctx);

  printf("standard multiplication : cmp g g1 =%d (should be !=0)\n",cmp);

  /* compute the inverse of a modulo ord */

  BN_mod_inverse ( inv_a, a, ord, bn_ctx) ;

  /* multiplication of g1 by the inverse of a, get the result into g2 */

  EC_POINT_mul ( G, g2, NULL, g1, inv_a , bn_ctx );

  /* we should have g2 = g */

  cmp = EC_POINT_cmp(G,g,g2,bn_ctx);

  printf("Multiplication by the inverse : cmp g1 g2 =%d (should be 0)\n",cmp);

  /* generate two random numbers less than ord, a and b */

  BN_rand_range(a,ord);
  BN_rand_range(b,ord);

  /* standard Diffie-Hellmann with a and b */
  
  /* first multiply by a, then by b */
  EC_POINT_mul ( G, g1 , NULL , g, a , bn_ctx);
  EC_POINT_mul ( G, g2 , NULL , g1, b , bn_ctx);

  /* first multiply by b, then by a */
  EC_POINT_mul ( G , g3, NULL , g , b , bn_ctx) ;
  EC_POINT_mul ( G , g4, NULL , g3 , a , bn_ctx) ;

  cmp = EC_POINT_cmp(G,g2,g4,bn_ctx);
  printf("Standard Diffie-Hellmann: cmp g2 g4 =%d (should be 0)\n",cmp);

  /* Hughes Diffie-Hellmann */

  /* Compute member's contribution */

  contrib= EC_POINT_new(G); /* allocate */
  BN_rand_range(r1,ord); /* random number*/
  EC_POINT_mul ( G, contrib, NULL , g, r1 , bn_ctx);/* multiply generator by r1, result into contrib */

  /* Compute leader's response */
  
  response = EC_POINT_new (G) ; /* allocate */
  BN_rand_range(r0,ord);       /* random number */
  EC_POINT_mul ( G, response , NULL , contrib, r0 , bn_ctx); /*multiply*/

  /* Compute leader's secret on leader's side */

  EC_POINT_mul ( G, leader_secret , NULL , g, r0 , bn_ctx);

  /* Compute leader's secret on member's side */

  /* 1 : compute r1^(-1) into inv_r1*/
  BN_mod_inverse ( inv_r1, r1, ord, bn_ctx) ;

  /* 2 : compute leader's response multiplied by r1^(-1) */

  EC_POINT_mul ( G , member_secret, NULL, response, inv_r1, bn_ctx) ;

  /* test */
  cmp = EC_POINT_cmp(G,leader_secret,member_secret,bn_ctx);
  printf("Hughes Diffie_Hellmann : cmp leader_secret member_secret =%d (should be 0)\n",cmp);

  /* output/input conversions */

  BN_rand_range( a , ord );

  EC_POINT_mul ( G, g1, NULL, g, a , bn_ctx );

  BN_rand_range( b , ord );

  EC_POINT_mul ( G, g2, NULL, g, b, bn_ctx );

  cmp = EC_POINT_cmp(G,g1,g2,bn_ctx);

  printf("Input output cmp distintcs g1 g2=%d (should : be !=0)\n",cmp);

  str = EC_POINT_point2hex(G,g1,POINT_CONVERSION_COMPRESSED, bn_ctx) ;

  printf("point g1 in octal=%s\n",str);

  /* read the octal string into g2 */
  EC_POINT_hex2point(G,str,g2,bn_ctx);

  cmp = EC_POINT_cmp(G,g1,g2,bn_ctx);
  printf("Reading output, cmp g1 g2=%d(should be 0)\n",cmp);

  str="0311658F1DC16350EF1210D364695CD615D9DE4B9A";
 
  printf("arbitrary octal string=%s\n",str);

  /* read the octal string into g2 */
  EC_POINT_hex2point(G,str,g2,bn_ctx);

  cmp = EC_POINT_cmp(G,g1,g2,bn_ctx);
  printf("Reading output cmp g1 g2=%d(should be !=0)\n",cmp);

  /* you really need to desallocate everything now */

  /* bignums */
  BN_free(ord); 
  BN_free(a);   /* and so on ...*/
 

  /* points */
  EC_POINT_free(g);
  EC_POINT_free(g1);
  EC_POINT_free(g2);/* and so on ...*/

  /* curve */
  /* I have a problem: I can not desallocate the curve G...*/
  /* but may be it is not necessary */
  /* EC_GROUP_free( G);*/ /* gives a segmentation fault */

  /* desallocate  the mysterious bn_ctx*/

  BN_CTX_free (bn_ctx);
}


