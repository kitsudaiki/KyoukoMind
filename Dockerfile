FROM ubuntu:20.04

RUN apt-get update; \
    apt-get install -y libboost-filesystem-dev openssl libssl-dev opencl-headers ocl-icd-opencl-dev xxd; \
    mkdir /etc/KyoukoMind
COPY upload/KyoukoMind /etc/KyoukoMind/KyoukoMind

