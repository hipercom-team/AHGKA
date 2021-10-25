#--------------------------------------------------------------------------
#                          INRIA Rocquencourt
#  Copyright Institut National de Recherche en Informatique et
#  en Automatique.  All rights reserved.  Distributed only with permission.
#--------------------------------------------------------------------------

VERSION=1.0.1
NAME=AHGKA

#---------------------------------------------------------------------------

# Include here the actual path of openssl installation if needed in both
# SSLLIB and INCLUDE, for instance:
#INCLUDE=-I/<path>/openssl-XXX/include
#SSLLIB=-L/<path>/openssl-XXX/ -lssl -lcrypto

# Default: openssl is installed as system library, so no specific paths:
INCLUDE=
SSLLIB=-lssl -lcrypto


# for instance:
#SSLLIB=-L /home/rajesh/software/openssl-0.9.8d/ -lssl -lcrypto 
#INCLUDE=-I /home/rajesh/software/openssl-0.9.8d/include

#MORELIB=-L/home/adjih/openssl-0.9.8b -lssl -lcrypto
#MOREINC=-I/home/adjih/openssl-0.9.8b/include

#STATIC=-static

#--------------------------------------------------

all: tagdh protocol-test crypto-test

tagdh:	tagdh.c 
	$(CC) tagdh.c -g3 -o tagdh $(INCLUDE) $(SSLLIB) ${STATIC} \
                               ${MORELIB} ${MOREINC}

CXXFILES=protocol-test.cc \
         gka-protocol.h \
         gka-protocol.cc \
         scheduler-core.h \
         scheduler-core.cc \
         gka-crypto.h \
         api-scheduler.h \
         ecdsa-crypto.h

protocol-test: ${CXXFILES}
	g++ -g3 -o protocol-test protocol-test.cc $(INCLUDE) $(SSLLIB) -static\
             ${MORELIB} ${MOREINC}

crypto-test: ${CXXFILES} test-gka-crypto.cc gka-crypto.h
	g++ -Wall -g3 -o crypto-test test-gka-crypto.cc $(INCLUDE) \
                $(SSLLIB) -static ${MORELIB} ${MOREINC}

clean:
	/bin/rm  -f tagdh protocol-test crypto-test *~ ${NAME}-*.tar.gz \
                    output.tr

#---------------------------------------------------------------------------

release:
	make clean
	(N=${NAME}-${VERSION} && DIR=/tmp/$${N} && rm -rf $${DIR} \
         && mkdir $${DIR} && cp -av * $${DIR} && export N \
         && (cd /tmp &&  tar --exclude=site -cvf - $${N}) \
	 | gzip -9 -f > $${N}.tar.gz )

www:
	cd site && ./update-site

#---------------------------------------------------------------------------
