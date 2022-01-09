FROM ubuntu:20.04

RUN apt-get update; \
    apt-get install -y openssl libuuid1 libcrypto++6 opencl-headers ocl-icd-opencl-dev xxd  libsqlite3-0; \
    mkdir /etc/kyouko
COPY upload/KyoukoMind /usr/bin/KyoukoMind
CMD KyoukoMind
