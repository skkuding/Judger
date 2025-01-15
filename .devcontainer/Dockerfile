FROM mcr.microsoft.com/devcontainers/base:alpine

RUN apk update && apk add --no-cache \
build-base \
autoconf \
automake \
libtool \
git \
bash \
gperf \
linux-headers \
cmake

RUN git clone https://github.com/seccomp/libseccomp.git /libseccomp

WORKDIR /libseccomp

RUN ./autogen.sh
RUN ./configure --prefix=/usr --disable-shared --enable-static
RUN make
RUN make install
