#include <time.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "../lib/e-Paper/EPD_IT8951.h"
#include "../lib/GUI/GUI_Paint.h"
#include "../lib/GUI/GUI_BMPfile.h"
#include "../lib/Config/Debug.h"

UBYTE *Refresh_Frame_Buf = NULL;

bool Four_Byte_Align = false;

extern int epd_mode;

static void Epd_Mode(int mode)
{
	if(mode == 1) {
		Paint_SetRotate(ROTATE_0);
		Paint_SetMirroring(MIRROR_HORIZONTAL);
	}else {
		Paint_SetRotate(ROTATE_0);
		Paint_SetMirroring(MIRROR_NONE);
	}
}



UBYTE Display_ColorPalette_Example(UWORD Panel_Width, UWORD Panel_Height, UDOUBLE Init_Target_Memory_Addr){
    UWORD In_4bp_Refresh_Area_Width;
    if(Four_Byte_Align == true){
        In_4bp_Refresh_Area_Width = Panel_Width - (Panel_Width % 32);
    }else{
        In_4bp_Refresh_Area_Width = Panel_Width;
    }
    UWORD In_4bp_Refresh_Area_Height = Panel_Height/16;

    UDOUBLE Imagesize;

    clock_t In_4bp_Refresh_Start, In_4bp_Refresh_Finish;
    double In_4bp_Refresh_Duration;

    Imagesize  = ((In_4bp_Refresh_Area_Width*4 % 8 == 0)? (In_4bp_Refresh_Area_Width*4 / 8 ): (In_4bp_Refresh_Area_Width*4 / 8 + 1)) * In_4bp_Refresh_Area_Height;

    if((Refresh_Frame_Buf = (UBYTE *)malloc(Imagesize)) == NULL) {
        Debug("Failed to apply for black memory...\r\n");
        return -1;
    }

    Debug("Start to demostrate 4bpp palette example\r\n");
    In_4bp_Refresh_Start = clock();

    UBYTE SixteenColorPattern[16] = {0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00};

    for(int i=0; i < 16; i++){
        memset(Refresh_Frame_Buf, SixteenColorPattern[i], Imagesize);
        EPD_IT8951_4bp_Refresh(Refresh_Frame_Buf, 0, i * In_4bp_Refresh_Area_Height, In_4bp_Refresh_Area_Width, In_4bp_Refresh_Area_Height, false, Init_Target_Memory_Addr, false);
    }

    In_4bp_Refresh_Finish = clock();
    In_4bp_Refresh_Duration = (double)(In_4bp_Refresh_Finish - In_4bp_Refresh_Start) / CLOCKS_PER_SEC;
    Debug( "Write and Show 4bp occupy %f second\n", In_4bp_Refresh_Duration );

    if(Refresh_Frame_Buf != NULL){
        free(Refresh_Frame_Buf);
        Refresh_Frame_Buf = NULL;
    }
    return 0;
}

UBYTE Display_BMP_Example(UWORD Panel_Width, UWORD Panel_Height, UDOUBLE Init_Target_Memory_Addr, UBYTE BitsPerPixel){
   
    UWORD WIDTH;
    if(Four_Byte_Align == true){
        WIDTH  = Panel_Width - (Panel_Width % 32);
    }else{
        WIDTH = Panel_Width;
    }
    UWORD HEIGHT = Panel_Height;

    UDOUBLE Imagesize;

    Imagesize = ((WIDTH * BitsPerPixel % 8 == 0)? (WIDTH * BitsPerPixel / 8 ): (WIDTH * BitsPerPixel / 8 + 1)) * HEIGHT;
    if((Refresh_Frame_Buf = (UBYTE *)malloc(Imagesize)) == NULL) {
        Debug("Failed to apply for black memory...\r\n");
        return -1;
    }
   

    Paint_NewImage(Refresh_Frame_Buf, WIDTH, HEIGHT, 0, BLACK);
    Paint_SelectImage(Refresh_Frame_Buf);
	Epd_Mode(epd_mode);
    Paint_SetBitsPerPixel(BitsPerPixel);
    Paint_Clear(WHITE);


   
    //1.5s
    char Path[30];
    sprintf(Path,"./pic/%dx%d_4.bmp", WIDTH, HEIGHT);

    GUI_ReadBmp(Path, 0, 0);

    //you can draw your character and pattern on the image, for color definition of all BitsPerPixel, you can refer to GUI_Paint.h, 
    //Paint_DrawRectangle(50, 50, WIDTH/2, HEIGHT/2, 0x30, DOT_PIXEL_3X3, DRAW_FILL_EMPTY);
    //Paint_DrawCircle(WIDTH*3/4, HEIGHT/4, 100, 0xF0, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
    //Paint_DrawNum(WIDTH/4, HEIGHT/5, 709, &Font20, 0x30, 0xB0);


    //1.1s 2.0s 4.0s
    switch(BitsPerPixel){
        case BitsPerPixel_8:{
           //Paint_DrawString_CN(10, 10, "你好微软", &Font24CN, 0xF0, 0x00);
            EPD_IT8951_8bp_Refresh(Refresh_Frame_Buf, 0, 0, WIDTH,  HEIGHT, false, Init_Target_Memory_Addr);
            break;
        }
        case BitsPerPixel_4:{
            //Paint_DrawString_CN(100, 100, "你好微软", &Font24CN, 0xF0, 0x00);
            EPD_IT8951_4bp_Refresh(Refresh_Frame_Buf, 0, 0, WIDTH,  HEIGHT, false, Init_Target_Memory_Addr,false);
            break;
        }
        case BitsPerPixel_2:{
           // Paint_DrawString_CN(10, 10, "你好微软", &Font24CN, 0xF0, 0x00);
            EPD_IT8951_2bp_Refresh(Refresh_Frame_Buf, 0, 0, WIDTH,  HEIGHT, false, Init_Target_Memory_Addr,false);
            break;
        }
        case BitsPerPixel_1:{
           //Paint_DrawString_CN(10, 10, "你好微软", &Font24CN, 0xF0, 0x00);
            EPD_IT8951_1bp_Refresh(Refresh_Frame_Buf, 0, 0, WIDTH,  HEIGHT, A2_Mode, Init_Target_Memory_Addr,false);
            break;
        }
    }
    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    Debug( "4 occupy %f second\n", duration );

    //start = clock();

    if(Refresh_Frame_Buf != NULL){
        free(Refresh_Frame_Buf);
        Refresh_Frame_Buf = NULL;
    }
/*
    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    Debug( "5 occupy %f second\n", duration );*/

    //DEV_Delay_ms(5000);

    return 0;
}


