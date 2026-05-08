FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    curl \
    gnupg \
    lsb-release \
    software-properties-common \
    ca-certificates \
    gpg \
    wget \
    git \
    ninja-build \
    ccache \
    nasm \
    && rm -rf /var/lib/apt/lists/*

# LLVM 19
RUN curl -fsSL https://apt.llvm.org/llvm.sh -o /tmp/llvm.sh \
    && chmod +x /tmp/llvm.sh \
    && /tmp/llvm.sh 19 all \
    && apt-get update \
    && apt-get install -y \
        llvm-19-dev \
        clang-19 \
        lld-19 \
    && ln -sf /usr/bin/llvm-config-19 /usr/local/bin/llvm-config \
    && rm -rf /var/lib/apt/lists/* /tmp/llvm.sh

# Latest CMake from Kitware APT
RUN apt-get update \
    && apt-get remove --purge -y cmake || true \
    && wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc \
        2>/dev/null | gpg --dearmor \
        > /usr/share/keyrings/kitware-archive-keyring.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main" \
        > /etc/apt/sources.list.d/kitware.list \
    && apt-get update \
    && apt-get install -y cmake \
    && rm -rf /var/lib/apt/lists/*

ENV CC=clang-19
ENV CXX=clang++-19
ENV LD=ld.lld-19

WORKDIR /workspace

CMD ["bash"]
