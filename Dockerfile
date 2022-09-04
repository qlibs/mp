FROM debian:bullseye

RUN apt-get -qq update; \
    apt-get install -qqy --no-install-recommends \
        gnupg2 wget ca-certificates \
        autoconf automake cmake dpkg-dev file make patch libc6-dev

RUN wget -nv -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

RUN wget -P /home https://www.circle-lang.org/linux/build_170.tgz && \
    tar xvf /home/build_170.tgz -C /usr/bin && chmod +x /usr/bin/circle

RUN echo "deb http://apt.llvm.org/bullseye/ llvm-toolchain-bullseye main" \
        > /etc/apt/sources.list.d/llvm.list; \
    apt-get -qq update && \
    apt-get install -qqy -t llvm-toolchain-bullseye \
        git vim ruby clang clang-format llvm libc++-16-dev libc++abi-16-dev libboost1.74-dev libelf-dev && \
    rm -rf /var/lib/apt/lists/*
