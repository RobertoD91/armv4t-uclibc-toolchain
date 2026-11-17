# ARMv4T + uClibc-ng cross toolchain (ARM7TDMI / ARM720T / ARM920T). Build ~17 min.
#   docker build -t armv4t-uclibc .
#   docker run --rm -v "$PWD":/work armv4t-uclibc \
#       arm-linux-gcc -O2 -march=armv4t -static -o /work/a.out /work/a.c
FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential bison flex libncurses-dev unzip rsync bc cpio file wget git \
      ca-certificates python3 \
    && rm -rf /var/lib/apt/lists/* \
    && useradd -m br
USER br
WORKDIR /home/br
RUN git clone --depth 1 https://gitlab.com/buildroot.org/buildroot.git br-src
COPY --chown=br:br armv4t_uclibc.defconfig /home/br/br-src/configs/armv4t_defconfig
WORKDIR /home/br/br-src
# arm920t = ARMv4T. "make toolchain" builds gcc + binutils + uClibc-ng only.
RUN make armv4t_defconfig && make olddefconfig && make toolchain
ENV PATH=/home/br/br-src/output/host/bin:$PATH
WORKDIR /work
CMD ["arm-linux-gcc", "--version"]
