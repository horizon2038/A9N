FROM --platform=linux/amd64 debian:bullseye
LABEL maintainer="horizon2k38"

ENV TERM xterm-256color

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
        bash \
        llvm \
        clang-16 \
        lld-16 \
        libc++-16-dev \
        libc++abi-16-dev \
        nasm \
        make \
        cmake \
        build-essential \
        uuid-dev \
        iasl \
        git \
        python-is-python3 \
        ovmf \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /A9N
SHELL ["/bin/bash", "-c"]

