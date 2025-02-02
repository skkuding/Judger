FROM mcr.microsoft.com/devcontainers/base:alpine

# Install dependencies to build libseccomp
RUN apk update && apk add --no-cache \
build-base \
autoconf \
automake \
libtool \
gperf \
linux-headers \
cmake

# Clone libseccomp
RUN git clone https://github.com/seccomp/libseccomp.git /opt/libseccomp

# Change to libseccomp directory
WORKDIR /opt/libseccomp

# Build libseccomp
RUN ./autogen.sh
RUN ./configure --prefix=/usr --disable-shared --enable-static
RUN make
RUN make install
