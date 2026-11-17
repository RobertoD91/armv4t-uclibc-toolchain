/*
 * fbvncserver — serve il framebuffer /dev/fb0 via VNC (LibVNCServer).
 * Read-only (vedere lo schermo). Auto-rileva risoluzione/formato pixel via ioctl;
 * se /dev/fb0 non c'e' (es. test sotto qemu) usa un framebuffer sintetico.
 *
 * Build col toolchain in sdk/ (le lib sono nel sysroot buildroot):
 *   arm-linux-gcc -O2 -march=armv4t -marm -static -o fbvncserver fbvncserver.c \
 *       -lvncserver -lz -ljpeg -lpthread -lssl -lcrypto -latomic
 * Uso sul device:
 *   ./fbvncserver              # serve /dev/fb0 su :5900
 *   ./fbvncserver --selftest   # init ed esci (verifica sotto qemu-ARMv4T)
 *   ./fbvncserver --device /dev/fb0 --port 5900
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <rfb/rfb.h>

int main(int argc, char **argv)
{
    const char *fbdev = "/dev/fb0";
    int port = 5900, selftest = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--selftest")) selftest = 1;
        else if (!strcmp(argv[i], "--device") && i + 1 < argc) fbdev = argv[++i];
        else if (!strcmp(argv[i], "--port")   && i + 1 < argc) port  = atoi(argv[++i]);
    }

    /* default, sovrascritti da FBIOGET_* se /dev/fb0 e' presente */
    int w = 320, h = 240, bpp = 16;
    int rL = 5, gL = 6, bL = 5, rO = 11, gO = 5, bO = 0;
    unsigned char *fb = NULL;
    long fbsize = 0;

    int fd = open(fbdev, O_RDWR);
    if (fd >= 0) {
        struct fb_var_screeninfo vi;
        struct fb_fix_screeninfo fi;
        if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) == 0 &&
            ioctl(fd, FBIOGET_FSCREENINFO, &fi) == 0) {
            w = vi.xres; h = vi.yres; bpp = vi.bits_per_pixel;
            rL = vi.red.length;  gL = vi.green.length;  bL = vi.blue.length;
            rO = vi.red.offset;  gO = vi.green.offset;  bO = vi.blue.offset;
            fbsize = fi.smem_len ? fi.smem_len : (long)fi.line_length * vi.yres;
            fb = mmap(NULL, fbsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (fb == MAP_FAILED) fb = NULL;
            rfbLog("fb %s: %dx%d %dbpp  R%d@%d G%d@%d B%d@%d\n",
                   fbdev, w, h, bpp, rL, rO, gL, gO, bL, bO);
        }
    }
    if (!fb) {
        rfbLog("nessun %s (%s) -> framebuffer sintetico %dx%d %dbpp\n",
               fbdev, strerror(errno), w, h, bpp);
        fbsize = (long)w * h * (bpp / 8 ? bpp / 8 : 2);
        fb = calloc(1, fbsize);
        unsigned short *p = (unsigned short *)fb;      /* pattern RGB565 */
        for (int y = 0; y < h; y++)
            for (int x = 0; x < w; x++)
                p[y * w + x] = ((x >> 3) << 11) | ((y >> 2) << 5) | ((x ^ y) & 0x1f);
    }

    int bytespp = bpp / 8; if (bytespp < 1) bytespp = 2;
    rfbScreenInfoPtr s = rfbGetScreen(&argc, argv, w, h, (rL ? rL : 5), 3, bytespp);
    if (!s) { fprintf(stderr, "rfbGetScreen fallito\n"); return 1; }
    s->desktopName  = "Framebuffer";
    s->frameBuffer  = (char *)fb;
    s->port         = port;
    s->alwaysShared = TRUE;
    /* formato pixel reale del framebuffer del device */
    s->serverFormat.bitsPerPixel = bpp;
    s->serverFormat.depth        = rL + gL + bL;
    s->serverFormat.trueColour   = TRUE;
    s->serverFormat.redShift   = rO; s->serverFormat.greenShift = gO; s->serverFormat.blueShift = bO;
    s->serverFormat.redMax   = (1 << (rL ? rL : 5)) - 1;
    s->serverFormat.greenMax = (1 << (gL ? gL : 6)) - 1;
    s->serverFormat.blueMax  = (1 << (bL ? bL : 5)) - 1;

    rfbInitServer(s);
    rfbLog("VNC server 'Framebuffer' pronto su :%d (%dx%d %dbpp)\n", port, w, h, bpp);
    if (selftest) {
        rfbLog("selftest: init LibVNCServer completo su questa architettura -> OK\n");
        rfbShutdownServer(s, TRUE);
        return 0;
    }
    while (rfbIsActive(s)) {
        rfbMarkRectAsModified(s, 0, 0, w, h);   /* read-only: ridisegna l'intero schermo */
        rfbProcessEvents(s, 50000);             /* 50 ms */
    }
    rfbScreenCleanup(s);
    return 0;
}
