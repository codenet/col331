# Force Docker to use Intel x86 emulation (Crucial for M1/M2 Macs)
FROM --platform=linux/amd64 ubuntu:22.04

# Avoid prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Update and install tools
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-multilib \
    gdb \
    qemu-system-x86 \
    git \
    vim \
    libgomp1 \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /root/env

# docker build --platform=linux/amd64 -t os-lab .
# docker run --platform=linux/amd64 -it -v "$(pwd)":/root/env os-lab            
# echo 'alias oslab="docker run -it --rm --platform=linux/amd64 -v \"$(pwd)\":/root/env os-lab"' >> ~/.zshrc
# source ~/.zshrc