#include "../lib/Config/DEV_Config.h"
#include "../lib/GUI/GUI_BMPfile.h"
#include "../lib/e-Paper/EPD_IT8951.h"
#include "../lib/GUI/GUI_Paint.h"
#include "../lib/Config/Debug.h"

#include <math.h>
#include <stdlib.h>     //exit()
#include <time.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

// 1 bit per pixel, which is 2 grayscale
#define BitsPerPixel_1 1
// 2 bit per pixel, which is 4 grayscale 
#define BitsPerPixel_2 2
// 4 bit per pixel, which is 8 grayscale
#define BitsPerPixel_4 4
// 8 bit per pixel, which is 256 grayscale, but will automatically reduce by hardware to 4bpp, which is 16 grayscale
#define BitsPerPixel_8 8

UWORD VCOM = 2510;

IT8951_Dev_Info Dev_Info;
UWORD Panel_Width;
UWORD Panel_Height;
UDOUBLE Init_Target_Memory_Addr;
int epd_mode = 0;	//0: no rotate, no mirror
					//1: no rotate, horizontal mirror


UBYTE *Refresh_Frame_Buf = NULL;

UBYTE *Panel_Frame_Buf = NULL;
UBYTE *Panel_Area_Frame_Buf = NULL;

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
    //UBYTE SixteenColorPattern[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    for(int i=0; i < 16; i++){
        memset(Refresh_Frame_Buf, SixteenColorPattern[i], Imagesize);
        //EPD_IT8951_1bp_Refresh(Refresh_Frame_Buf, 0, i * In_4bp_Refresh_Area_Height, In_4bp_Refresh_Area_Width, In_4bp_Refresh_Area_Height, A2_Mode, Init_Target_Memory_Addr,false);
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

UBYTE Display_BMP_Example(UWORD Panel_Width, UWORD Panel_Height, UDOUBLE Init_Target_Memory_Addr, UBYTE BitsPerPixel,int name){
    
    clock_t start, finish;
    double duration;

    start = clock();
    UWORD WIDTH=720,HEIGHT=720;
/*
    if(Four_Byte_Align == true){
        WIDTH  = Panel_Width - (Panel_Width % 32);
    }else{
        WIDTH = Panel_Width;
    }
    UWORD HEIGHT = Panel_Height;
*/
    UDOUBLE Imagesize;
    char Path[30];
    Imagesize = ((WIDTH * BitsPerPixel % 8 == 0)? (WIDTH * BitsPerPixel / 8 ): (WIDTH * BitsPerPixel / 8 + 1)) * HEIGHT;
    Refresh_Frame_Buf = (UBYTE *)malloc(Imagesize);
    Paint_NewImage(Refresh_Frame_Buf, WIDTH, HEIGHT, 0, BLACK);
    Paint_SelectImage(Refresh_Frame_Buf);
    Epd_Mode(epd_mode);
    Paint_SetBitsPerPixel(BitsPerPixel);
    Paint_Clear(WHITE);
    //Paint_DrawString_EN(0, 0, "Hello World", &Font24, 0x00, 0xFF);
    int na=name;
    for(name=0;name<10;name++)
    {
    sprintf(Path,"./pic/1872x1404_1_%d.bmp",name);
    for(int nm=0;nm<10;nm++)GUI_ReadBmp(Path, name*72, nm*72);
    }
            //for(int aaa=0;aaa<10;aaa++){
switch(BitsPerPixel){
       
       case BitsPerPixel_1:{
            //EPD_IT8951_1bp_Refresh(Refresh_Frame_Buf, 0+aaa*72, 0, 72, HEIGHT,  A2_Mode, Init_Target_Memory_Addr,false);
            EPD_IT8951_1bp_Refresh(Refresh_Frame_Buf, 0,0, 720,720,  A2_Mode, Init_Target_Memory_Addr,false);
            //EPD_IT8951_1bp_Refresh(Refresh_Frame_Buf, 0,1200, WIDTH, 204,  A2_Mode, Init_Target_Memory_Addr,false);
            break;
        }
        case BitsPerPixel_2:{
            //EPD_IT8951_4bp_Refresh(Refresh_Frame_Buf, 0+aaa*72, 0, 72, HEIGHT, false, Init_Target_Memory_Addr,false);
            EPD_IT8951_2bp_Refresh(Refresh_Frame_Buf, 0, 0, 720, 720, false, Init_Target_Memory_Addr,false);
            //EPD_IT8951_2bp_Refresh(Refresh_Frame_Buf, 0, 1200, WIDTH,  204, false, Init_Target_Memory_Addr,false);
            break;
        }
        case BitsPerPixel_4:{
            //EPD_IT8951_4bp_Refresh(Refresh_Frame_Buf, 0+aaa*72, 0, 72, HEIGHT,  false, Init_Target_Memory_Addr,false);
            EPD_IT8951_4bp_Refresh(Refresh_Frame_Buf, 0, 0, 720, 720, false, Init_Target_Memory_Addr,false);
            //EPD_IT8951_4bp_Refresh(Refresh_Frame_Buf, 0, 1200, WIDTH,  204, false, Init_Target_Memory_Addr,false);
            break;
        }
    }//}


/*
    UDOUBLE Basical_Memory_Addr = Init_Target_Memory_Addr;
    UDOUBLE Target_Memory_Addr = Basical_Memory_Addr;
    for(int i=0; i < 10; i += 1){
        Paint_Clear(WHITE);
        sprintf(Path,"./pic/1872x1404_1_%d.bmp",i);
        GUI_ReadBmp(Path, 0, 0);
        //For color definition of all BitsPerPixel, you can refer to GUI_Paint.h
        //Paint_DrawNum(10, 10, i+1, &Font16, 0x00, 0xF0);
        //EPD_IT8951_Multi_Frame_Write(Refresh_Frame_Buf, 0+i*72,0+BitsPerPixel*72, 72,  72, Init_Target_Memory_Addr,false,BitsPerPixel);
        EPD_IT8951_Multi_Frame_Write(Refresh_Frame_Buf, 600+i*72,600+BitsPerPixel*72, 72,  72, Target_Memory_Addr,false,BitsPerPixel);
        EPD_IT8951_Multi_Frame_Write(Refresh_Frame_Buf, 600+i*72,816+BitsPerPixel*72, 72,  72, Target_Memory_Addr,false,BitsPerPixel);

        //EPD_IT8951_1bp_Multi_Frame_Write(Refresh_Frame_Buf, 600+i*72,600+BitsPerPixel*72, 72,  72, Target_Memory_Addr,false);
        Target_Memory_Addr += Imagesize;
    }

    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    Debug( "Write occupy %f second\n", duration );

    start = clock();

    Target_Memory_Addr = Basical_Memory_Addr;

    for(int i=0; i< 10; i += 1){
        EPD_IT8951_Multi_Frame_Refresh(600+i*72,600+BitsPerPixel*72, 72,  72, Target_Memory_Addr);
        //EPD_IT8951_1bp_Multi_Frame_Refresh(600+i*72,600+BitsPerPixel*72, 72,  72, Target_Memory_Addr);
        Target_Memory_Addr += Imagesize;
    }   
*/


    if(Refresh_Frame_Buf != NULL){
        free(Refresh_Frame_Buf);
        Refresh_Frame_Buf = NULL;
    }
    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    Debug( "Show occupy %f second\n", duration );
    return 0;
}

void My_init(char *vocm,char *epd){

    DEV_Module_Init();

    double temp;
    sscanf(vocm,"%lf",&temp);
    VCOM = (UWORD)(fabs(temp)*1000);
    //Debug("VCOM value:%d\r\n", VCOM);
    sscanf(epd,"%d",&epd_mode);
    //Debug("Display mode:%d\r\n", epd_mode);
    Dev_Info = EPD_IT8951_Init(VCOM);

    //get some important info from Dev_Info structure
    Panel_Width = Dev_Info.Panel_W;
    Panel_Height = Dev_Info.Panel_H;
    Init_Target_Memory_Addr = Dev_Info.Memory_Addr_L | (Dev_Info.Memory_Addr_H << 16);
    //char* LUT_Version = (char*)Dev_Info.LUT_Version;
    A2_Mode = 6;
    //Debug("A2 Mode:%d\r\n", A2_Mode);

    EPD_IT8951_Clear_Refresh( Dev_Info, Init_Target_Memory_Addr, INIT_Mode);
    /*
    Display_ColorPalette_Example(Panel_Width, Panel_Height, Init_Target_Memory_Addr);
    EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);*/
}

void My_print(int mode,int name){

    Display_BMP_Example(Panel_Width, Panel_Height, Init_Target_Memory_Addr, mode,name);
    
}

void My_exit(){

    //EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, GC16_Mode);
    DEV_Module_Exit();
}


int main(int argc, char *argv[])
{

    My_init(argv[1],argv[2]);

    for(int zjc=0;zjc<10;zjc++){
        //My_print(1,zjc);//0.14spc:72*72,0.17spc:720*72,0.54spc:720*720
        //DEV_Delay_ms(5000);
        My_print(2,zjc);//0.46spc:72*72,0.50spc:720*72,0.54spc:720*720
        DEV_Delay_ms(5000);
        My_print(4,zjc);//0.47spc:72*72,0.52spc:720*72,0.83spc:720*720
        DEV_Delay_ms(5000);
    }
    //My_print(1,0);
    //Total:4.8s
    //Write:0.5s
    //Show:4.3s
    //My_print(2,0);
    //Total:4.8s
    //Write:0.5s
    //Show:4.3s
    //My_print(4,0);
    //Total:5.3s
    //Write:1.0s
    //Show:4.3s
    
   
        //Display_ColorPalette_Example(Panel_Width, Panel_Height, Init_Target_Memory_Addr);
    //EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, );
    
    //My_print(1,5);

    //EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);

    //My_print(1);   
        
    //EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);

    //My_print(1);
    //My_print(1);

    EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);
    //EPD_IT8951_Sleep();
    //EPD_IT8951_Standby();
    //DEV_Delay_ms(5000);
    //DEV_Delay_ms(5000);
    //In case RPI is transmitting image in no hold mode, which requires at most 10s
    //EPD_IT8951_Sleep();
    //My_init(argv[1],argv[2]);

    My_exit();

    return 0;
}
