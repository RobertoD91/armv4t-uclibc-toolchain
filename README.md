# armv4t-uclibc-toolchain

### ⚠️ Waring: this repo is AI generated ! ⚠️

A ready-to-use **cross-compilation toolchain for ARMv4T Linux with uClibc-ng**, shipped as a Docker
image. It targets the old ARMv4T cores — **ARM7TDMI, ARM720T, ARM920T/922T** — that modern prebuilt
toolchains skip.

> **Why this exists:** **bootlin** and **dockcross** both start at **armv5**. A binary built with
> their libc throws `Illegal instruction` (SIGILL) on an ARMv4T core, because armv5 libraries contain
> armv5-only instructions. This image builds a proper **armv4t** toolchain (uClibc-ng, EABI,
> soft-float, `-mcpu=arm920t`) with [buildroot](https://buildroot.org), so the libc itself is
> armv4t-clean.

## Use it

Pull the prebuilt image (built by CI — see below):
```bash
docker pull ghcr.io/robertod91/armv4t-uclibc-toolchain:latest
docker run --rm -v "$PWD":/work ghcr.io/robertod91/armv4t-uclibc-toolchain \
    arm-linux-gcc -O2 -march=armv4t -static -o /work/a.out /work/a.c
```

Or build it yourself (~17 min, one time — the image then stays in your local Docker cache):
```bash
docker build -t armv4t-uclibc .
```

Toolchain prefix `arm-linux-` (`arm-linux-gcc`, `arm-linux-strip`, …), uClibc-ng, EABI, soft-float.

## Verify on an actual ARMv4T CPU (qemu)

`qemu-arm -cpu ti925t` emulates an ARMv4T core, so it **rejects any armv5 instruction** — a strict test:
```bash
qemu-arm-static -cpu ti925t ./a.out     # runs → armv4t-clean ; SIGILL → it's armv5
```

## Examples (`examples/`)
- `hello.c` — libc hello (printf + uname)
- `hello_bare.c` — freestanding (no libc, raw Linux syscalls) → runs on *any* ARMv4T
- `fbvncserver.c` — a real program: serve a Linux framebuffer (`/dev/fb0`) over VNC via LibVNCServer
  (needs `BR2_PACKAGE_LIBVNCSERVER=y` + openssl/zlib/lzo in the defconfig)

```bash
./scripts/build.sh    examples/hello.c  hello
./scripts/run-qemu.sh hello
```

## CI (GitHub Actions → GHCR)
[`.github/workflows/docker.yml`](.github/workflows/docker.yml) builds the image and pushes it to the
GitHub Container Registry (`ghcr.io`) on every push to `main` and on tags. It uses the built-in
`GITHUB_TOKEN` — **no secrets to configure**. Public repositories get free Actions minutes.

## Licenses
- The files **in this repository** (Dockerfile, examples, scripts) are **MIT** — see [`LICENSE`](LICENSE).
- The **built toolchain** bundles GPL (gcc, binutils, buildroot) and LGPL (uClibc-ng) software,
  produced from public upstream sources via buildroot. Run `make legal-info` in buildroot for the full
  source/license manifest.
