#include <ctype.h>

/*
 * Tabela de 256 entradas para classificação ASCII.
 * Índice = valor do byte (0-255).
 * Cada entrada é uma bitmask das categorias do caractere.
 *
 * Gerada estaticamente — zero overhead em runtime.
 *
 * Legenda das colunas (bits):
 *   UP=maiúscula  LO=minúscula  DI=dígito  SP=whitespace
 *   PU=pontuação  CN=controle   HX=hex     BL=espaço(0x20)
 */

#define U  _CT_UP
#define L  _CT_LO
#define D  _CT_DI
#define S  _CT_SP
#define P  _CT_PU
#define C  _CT_CN
#define X  _CT_HX
#define B  _CT_BL

const unsigned char _ctype_table[256] = {
/*  0 NUL*/C,  /*  1 SOH*/C,  /*  2 STX*/C,  /*  3 ETX*/C,
/*  4 EOT*/C,  /*  5 ENQ*/C,  /*  6 ACK*/C,  /*  7 BEL*/C,
/*  8 BS */C,  /*  9 TAB*/C|S,/* 10 LF */C|S,/* 11 VT */C|S,
/* 12 FF */C|S,/* 13 CR */C|S,/* 14 SO */C,  /* 15 SI */C,
/* 16 DLE*/C,  /* 17 DC1*/C,  /* 18 DC2*/C,  /* 19 DC3*/C,
/* 20 DC4*/C,  /* 21 NAK*/C,  /* 22 SYN*/C,  /* 23 ETB*/C,
/* 24 CAN*/C,  /* 25 EM */C,  /* 26 SUB*/C,  /* 27 ESC*/C,
/* 28 FS */C,  /* 29 GS */C,  /* 30 RS */C,  /* 31 US */C,
/* 32 SPC*/B|S,/* 33 !  */P,  /* 34 "  */P,  /* 35 #  */P,
/* 36 $  */P,  /* 37 %  */P,  /* 38 &  */P,  /* 39 '  */P,
/* 40 (  */P,  /* 41 )  */P,  /* 42 *  */P,  /* 43 +  */P,
/* 44 ,  */P,  /* 45 -  */P,  /* 46 .  */P,  /* 47 /  */P,
/* 48 0  */D|X,/* 49 1  */D|X,/* 50 2  */D|X,/* 51 3  */D|X,
/* 52 4  */D|X,/* 53 5  */D|X,/* 54 6  */D|X,/* 55 7  */D|X,
/* 56 8  */D|X,/* 57 9  */D|X,/* 58 :  */P,  /* 59 ;  */P,
/* 60 <  */P,  /* 61 =  */P,  /* 62 >  */P,  /* 63 ?  */P,
/* 64 @  */P,  /* 65 A  */U|X,/* 66 B  */U|X,/* 67 C  */U|X,
/* 68 D  */U|X,/* 69 E  */U|X,/* 70 F  */U|X,/* 71 G  */U,
/* 72 H  */U,  /* 73 I  */U,  /* 74 J  */U,  /* 75 K  */U,
/* 76 L  */U,  /* 77 M  */U,  /* 78 N  */U,  /* 79 O  */U,
/* 80 P  */U,  /* 81 Q  */U,  /* 82 R  */U,  /* 83 S  */U,
/* 84 T  */U,  /* 85 U  */U,  /* 86 V  */U,  /* 87 W  */U,
/* 88 X  */U,  /* 89 Y  */U,  /* 90 Z  */U,  /* 91 [  */P,
/* 92 \  */P,  /* 93 ]  */P,  /* 94 ^  */P,  /* 95 _  */P,
/* 96 `  */P,  /* 97 a  */L|X,/* 98 b  */L|X,/* 99 c  */L|X,
/*100 d  */L|X,/*101 e  */L|X,/*102 f  */L|X,/*103 g  */L,
/*104 h  */L,  /*105 i  */L,  /*106 j  */L,  /*107 k  */L,
/*108 l  */L,  /*109 m  */L,  /*110 n  */L,  /*111 o  */L,
/*112 p  */L,  /*113 q  */L,  /*114 r  */L,  /*115 s  */L,
/*116 t  */L,  /*117 u  */L,  /*118 v  */L,  /*119 w  */L,
/*120 x  */L,  /*121 y  */L,  /*122 z  */L,  /*123 {  */P,
/*124 |  */P,  /*125 }  */P,  /*126 ~  */P,  /*127 DEL*/C,
/* 128-255: todos zero (não-ASCII) */
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
};

#undef U
#undef L
#undef D
#undef S
#undef P
#undef C
#undef X
#undef B
