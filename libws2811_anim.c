#include "libws2811_anim.h"

uint32_t rgb[] = {3, 0xFF0000, 0x00FF00, 0x0000FF }; // 3


// Rainbow colors
uint32_t rainbow[] =
{
	7,
	0x9400D3,0x4B0082,0x0000FF,0x00FF00,0xFFFF00,0xFF7F00,0xFF0000
};


// Rainbow colors with alternating stripes of black
uint32_t rainbowStripe[] =
{
	16,
    0xFF0000, 0x000000, 0xAB5500, 0x000000, 0xABAB00, 0x000000, 0x00FF00, 0x000000,
    0x00AB55, 0x000000, 0x0000FF, 0x000000, 0x5500AB, 0x000000, 0xAB0055, 0x000000
};

// Blue purple ping red orange yellow (and back)
// Basically, everything but the greens.
// This palette is good for lighting at a club or party.
uint32_t party[] =
{
	16,
    0x5500AB, 0x84007C, 0xB5004B, 0xE5001B,
    0xE81700, 0xB84700, 0xAB7700, 0xABAB00,
    0xAB5500, 0xDD2200, 0xF2000E, 0xC2003E,
    0x8F0071, 0x5F00A1, 0x2F00D0, 0x0007F9
};

// Approximate "black body radiation" palette, akin to
// the FastLED 'Heatuint32_t' function.
// Recommend that you use values 0-240 rather than
// the usual 0-255, as the last 15 colors will be
// 'wrapping around' from the hot end to the cold end,
// which looks wrong.
uint32_t heat[] =
{
	4,
    0x000000, 0xFF0000, 0xFFFF00, 0xFFFFCC
};


uint32_t fire[] =
{
	6,
    0x000000, 0x220000,
    0x880000, 0xFF0000,
    0xFF6600, 0xFFCC00
};

uint32_t hun[]=
{
		4,
		0xff0000,0xffffff,0x00ff00,0x000000
};

uint32_t ro[]={
		4,
		0xff0000,0xffff00,0x0000ff,0x000000
};

#define PALETTES_LEN 8

uint32_t * palettes[PALETTES_LEN]={
	rgb,
	rainbow,
	rainbowStripe,
	party,
	heat,
	fire,
	hun,
	ro
};

#define ANIMS_LEN 3

void (*anims[ANIMS_LEN])(uint32_t,uint32_t*, uint32_t)={
		animSolid,
		animPattern,
		animSin
};


uint8_t sin_lut[1024]={
	128,129,130,130,131,132,133,133,134,135,136,137,137,138,139,140,
	140,141,142,143,144,144,145,146,147,147,148,149,150,151,151,152,
	153,154,154,155,156,157,157,158,159,160,160,161,162,163,163,164,
	165,166,166,167,168,169,169,170,171,172,172,173,174,174,175,176,
	177,177,178,179,180,180,181,182,182,183,184,184,185,186,187,187,
	188,189,189,190,191,191,192,193,193,194,195,195,196,197,197,198,
	199,199,200,201,201,202,202,203,204,204,205,206,206,207,207,208,
	209,209,210,210,211,212,212,213,213,214,215,215,216,216,217,217,
	218,218,219,220,220,221,221,222,222,223,223,224,224,225,225,226,
	226,227,227,228,228,229,229,230,230,231,231,231,232,232,233,233,
	234,234,235,235,235,236,236,237,237,237,238,238,239,239,239,240,
	240,240,241,241,242,242,242,243,243,243,244,244,244,244,245,245,
	245,246,246,246,247,247,247,247,248,248,248,248,249,249,249,249,
	250,250,250,250,250,251,251,251,251,251,252,252,252,252,252,252,
	253,253,253,253,253,253,253,254,254,254,254,254,254,254,254,254,
	254,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,
	254,254,254,254,254,254,254,254,254,253,253,253,253,253,253,253,
	253,252,252,252,252,252,252,251,251,251,251,251,250,250,250,250,
	249,249,249,249,249,248,248,248,247,247,247,247,246,246,246,246,
	245,245,245,244,244,244,243,243,243,242,242,242,241,241,241,240,
	240,240,239,239,238,238,238,237,237,236,236,236,235,235,234,234,
	233,233,233,232,232,231,231,230,230,229,229,228,228,227,227,226,
	226,225,225,224,224,223,223,222,222,221,221,220,220,219,219,218,
	218,217,216,216,215,215,214,214,213,212,212,211,211,210,210,209,
	208,208,207,207,206,205,205,204,203,203,202,202,201,200,200,199,
	198,198,197,196,196,195,194,194,193,192,192,191,190,190,189,188,
	188,187,186,186,185,184,183,183,182,181,181,180,179,178,178,177,
	176,176,175,174,173,173,172,171,170,170,169,168,168,167,166,165,
	165,164,163,162,162,161,160,159,159,158,157,156,155,155,154,153,
	152,152,151,150,149,149,148,147,146,145,145,144,143,142,142,141,
	140,139,139,138,137,136,135,135,134,133,132,132,131,130,129,128,
	128,127,126,125,124,124,123,122,121,121,120,119,118,117,117,116,
	115,114,114,113,112,111,111,110,109,108,107,107,106,105,104,104,
	103,102,101,101,100, 99, 98, 97, 97, 96, 95, 94, 94, 93, 92, 91,
	 91, 90, 89, 88, 88, 87, 86, 86, 85, 84, 83, 83, 82, 81, 80, 80,
	 79, 78, 78, 77, 76, 75, 75, 74, 73, 73, 72, 71, 70, 70, 69, 68,
	 68, 67, 66, 66, 65, 64, 64, 63, 62, 62, 61, 60, 60, 59, 58, 58,
	 57, 56, 56, 55, 54, 54, 53, 53, 52, 51, 51, 50, 49, 49, 48, 48,
	 47, 46, 46, 45, 45, 44, 44, 43, 42, 42, 41, 41, 40, 40, 39, 38,
	 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30,
	 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23, 23,
	 22, 22, 21, 21, 20, 20, 20, 19, 19, 18, 18, 18, 17, 17, 16, 16,
	 16, 15, 15, 15, 14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 11,
	 10, 10, 10, 10,  9,  9,  9,  9,  8,  8,  8,  7,  7,  7,  7,  7,
	  6,  6,  6,  6,  5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  3,
	  3,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,
	  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,
	  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,
	  7,  7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 11,
	 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 16, 16,
	 16, 17, 17, 17, 18, 18, 19, 19, 19, 20, 20, 21, 21, 21, 22, 22,
	 23, 23, 24, 24, 25, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30,
	 30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 38, 38,
	 39, 39, 40, 40, 41, 41, 42, 43, 43, 44, 44, 45, 46, 46, 47, 47,
	 48, 49, 49, 50, 50, 51, 52, 52, 53, 54, 54, 55, 55, 56, 57, 57,
	 58, 59, 59, 60, 61, 61, 62, 63, 63, 64, 65, 65, 66, 67, 67, 68,
	 69, 69, 70, 71, 72, 72, 73, 74, 74, 75, 76, 76, 77, 78, 79, 79,
	 80, 81, 82, 82, 83, 84, 84, 85, 86, 87, 87, 88, 89, 90, 90, 91,
	 92, 93, 93, 94, 95, 96, 96, 97, 98, 99, 99,100,101,102,102,103,
	104,105,105,106,107,108,109,109,110,111,112,112,113,114,115,116,
	116,117,118,119,119,120,121,122,123,123,124,125,126,126,127,128};

uint8_t curAnim=0;

uint32_t t=0;


void anim(uint8_t mod,uint32_t pal,uint32_t * col_ptr, uint32_t len_t){
	if(mod<=ANIMS_LEN)
		anims[mod](pal,col_ptr,len_t);
}

void animPattern(uint32_t pal,uint32_t * col_ptr, uint32_t len_t){
	pal=(pal>>16)&0xff;
	uint8_t l=palettes[pal][0];

	for(uint32_t i=0;i<len_t;i++){
		uint8_t r_t=gammaCor(palettes[pal][1+i%l]>>16);
		uint8_t g_t=gammaCor(palettes[pal][1+i%l]>>8);
		uint8_t b_t=gammaCor(palettes[pal][1+i%l]);

		col_ptr[i]=packColor(r_t,g_t,b_t);
	}

}

void animSolid(uint32_t pal,uint32_t * col_ptr, uint32_t len_t){
	for(uint32_t i=0;i<len_t;i++){
		uint8_t r_t=gammaCor(pal>>16);
		uint8_t g_t=gammaCor(pal>>8);
		uint8_t b_t=gammaCor(pal);

		col_ptr[i]=packColor(r_t,g_t,b_t);
	}
}



void animSin(uint32_t pal,uint32_t * col_ptr, uint32_t len_t){
	pal=(pal>>16)&0xff;
	uint8_t l=palettes[pal][0];

	for(uint32_t i=0;i<len_t;i++){
		uint8_t r_t=gammaCor(palettes[pal][1+i%l]>>16)*t/255;
		uint8_t g_t=gammaCor(palettes[pal][1+i%l]>>8)*t/255;
		uint8_t b_t=gammaCor(palettes[pal][1+i%l])*t/255;


		col_ptr[i]=packColor(r_t,g_t,b_t);
	}

	t++;

	if(t>=255)
		t=0;
}
















