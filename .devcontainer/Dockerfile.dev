FROM mcr.microsoft.com/devcontainers/base:dev-alpine3.21

# Install dependencies to build libseccomp
RUN apk update && apk add --no-cache \
  build-base \
  autoconf \
  automake \
  libtool \
  gperf \
  linux-headers \
  cmake \
  # For clang-format
  clang19 clang19-extra-tools \
  # For openjdk17
  openjdk17

# Clone libseccomp
RUN git clone https://github.com/seccomp/libseccomp.git /opt/libseccomp

# Change to libseccomp directory
WORKDIR /opt/libseccomp

# Build libseccomp
RUN ./autogen.sh
RUN ./configure --prefix=/usr --disable-shared --enable-static
RUN make -j$(nproc)
RUN make install
