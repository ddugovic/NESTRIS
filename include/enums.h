#ifndef ENUMS_H
#define ENUMS_H
#ifndef nes_uchar
#define nes_uchar unsigned char
#endif
#ifndef nes_schar
#define nes_schar char
#endif
#ifndef nes_ushort
#define nes_ushort unsigned short
#endif
#ifndef nes_sshort
#define nes_sshort short
#endif
#ifndef PLAYFIELDX
#define PLAYFIELDX 96
#endif // PLAYFIELDX
#ifndef PLAYFIELDY
#define PLAYFIELDY 40
#endif // PLAYFIELDY

enum BTN {
    LEFT,
    UP,
    RIGHT,
    DOWN,
    SELECT,
    START,
    B,
    A
};
#endif // ENUMS_H
