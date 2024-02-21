FROM --platform=linux/amd64 debian:bullseye
LABEL maintainer="horizon2k38"

RUN apt-get update && \
    apt-get install -y \
        bash \
        llvm \
        clang \
        lld \
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

