FROM ubuntu AS base

WORKDIR build

# get build tools
RUN apt-get update && apt-get -y install build-essential git autoconf

#
# build htslib
RUN git clone https://github.com/samtools/htslib.git --branch 1.9 --single-branch

# get htslib prerequisites
RUN apt-get -y install zlib1g-dev libbz2-dev liblzma-dev libcurl4-openssl-dev libssl-dev

# make
RUN cd htslib && autoheader && autoconf && \
  ./configure --enable-libcurl --enable-gcs --enable-s3 && \
  make && make install

#
# build seqlib
RUN git clone --recurse-submodules https://github.com/julianhess/SeqLib.git
RUN cd /build/SeqLib && ./configure LDFLAGS="-lcurl -lcrypto" && \
  make CXXFLAGS='-std=c++11'

#
# build tokenizer

# copy in tokenizer
RUN mkdir -p tokenizer/walker
COPY *.?pp Makefile /build/tokenizer/
COPY walker /build/tokenizer/walker/

RUN cd /build/tokenizer && \
  make INC=-I/build/SeqLib LDFLAGS=-L/build/SeqLib/src \
    LDLIBS="-l:libhts.a -l:libseqlib.a -lcurl -lcrypto -lz -lpthread -llzma -lbz2"

# package into minimal image

FROM frolvlad/alpine-glibc

WORKDIR run

RUN apk --no-cache add libcurl openssl zlib xz-libs libbz2 libstdc++ && \
  ln -s /usr/lib/libbz2.so.1.0.8 /usr/lib/libbz2.so.1.0

COPY --from=base /build/tokenizer/get_all_calls .
