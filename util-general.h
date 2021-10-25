//-----------------------------------------------------------------*- c++ -*-
//                         INRIA-OLSRNet/OLSRBase
//             Cedric Adjih, projet Hipercom, INRIA Rocquencourt
//  Copyright 2003-2006 Institut National de Recherche en Informatique et
//  en Automatique.  All rights reserved.  Distributed only with permission.
//---------------------------------------------------------------------------
// [Oct2006] Comes from: OOLSR/include/general.h
// [Apr2007] Comes from: OLSRNet-initial/OLSRBase/common/util-general.h
//           (only parts, included)
//---------------------------------------------------------------------------

#ifndef _UTIL_GENERAL_H
#define _UTIL_GENERAL_H

//--------------------------------------------------

#include <assert.h>
#include <stdio.h>

#include <cstdlib>
#include <string>
#include <vector>
#include <list>
#include <map>

using std::string;
using std::vector;
using std::list;

#include <iostream>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <cfloat>

using std::cout;
using std::cerr;
using std::endl;
using std::ostringstream;
using std::ostream;

typedef double Time;


#ifdef SYSTEMlinux
#include <unistd.h>
#endif

//---------------------------------------------------------------------------

#define BeginMacro do {
#define EndMacro } while(0)

#define UNUSED(x) BeginMacro (x)=(x); EndMacro

//---------------------------------------------------------------------------
// Basic macros (from OOLSR/include/log.h)
//---------------------------------------------------------------------------

/// Fatal error
#define Fatal(x) \
   BeginMacro \
       ostringstream out; \
       out << "FATAL(function=" << __func__ << ", " \
           << __FILE__ <<":" << __LINE__  << "):"; \
       out <<  x; \
       out << endl; \
       cerr << out.str(); \
       abort(); \
       exit(EXIT_FAILURE); \
   EndMacro 

#define Exit(x) \
   BeginMacro \
       ostringstream out; \
       out << "Fatal error:"; \
       out <<  x; \
       out << endl; \
       cerr << out.str(); \
       exit(EXIT_FAILURE); \
   EndMacro 

#if 1 // XXX! for now
/// Warning
#define Warn(x) \
   BeginMacro \
       cerr << "WARNING(function=" << __func__ << ", " \
                 << __FILE__ <<":" << __LINE__  << "):"; \
       cerr << x; \
       cerr << endl; \
   EndMacro 
#else
#define Warn(x) BeginMacro EndMacro
#endif

//---------------------------------------------------------------------------
// The all-powerful macro Repr
//---------------------------------------------------------------------------

extern string getBufferAndDelete(ostringstream* out);

#define Repr(args) \
  (getBufferAndDelete(dynamic_cast<ostringstream*> \
                     (&  ((*(new ostringstream)) << args))  ))

#define REPR(args) Repr(args)

//---------------------------------------------------------------------------

#endif // _UTIL_GENERAL_H
