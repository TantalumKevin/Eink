/*****************************************************************************
* | File      	:   GUI_BMPfile.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*                Used to shield the underlying layers of each master
*                and enhance portability
*----------------
* |	This version:   V2.0
* | Date        :   2018-11-12
* | Info        :
* 1.Change file name: GUI_BMP.c -> GUI_BMPfile.c
* 2.fix: GUI_ReadBmp()
*   Now Xstart and Xstart can control the position of the picture normally,
*   and support the display of images of any size. If it is larger than
*   the actual display range, it will not be displayed.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/

#include "GUI_BMPfile.h"
#include "GUI_Paint.h"
#include "../Config/Debug.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>//exit()
#include <string.h>//memset()
#include <math.h>//memset()
#include <stdio.h>
#include<time.h>


//global variables related to BMP picture display
UBYTE *bmp_dst_buf = NULL;
UBYTE *bmp_src_buf = NULL;
UDOUBLE bmp_width, bmp_height;
UBYTE  bmp_BitCount;
UDOUBLE bytesPerLine;
UDOUBLE imageSize;
UDOUBLE skip;
BMPRGBQUAD  palette[256];

static void Bitmap_format_Matrix(UBYTE *dst,UBYTE *src)
{
	UDOUBLE i,j,k;
    UBYTE *psrc = src;
    UBYTE *pdst = dst;
    UBYTE *p = psrc;
	UBYTE temp;
	UDOUBLE count;
	
	//Since the bmp storage is from the back to the front, it needs to be converted in reverse order.
	switch(bmp_BitCount)
	{
		case 1:
			pdst += (bmp_width * bmp_height);
			
			for(i=0;i<bmp_height;i++)
			{
				pdst -= bmp_width;
				count = 0;
				for (j=0;j<(bmp_width+7)/8;j++)
				{
					temp = p[j];
					
					for (k=0;k<8;k++)
					{
						pdst[0]= ((temp & (0x80>>k)) >> (7-k));
						count++;
						pdst++;
						if (count == bmp_width)
						{
							break;
						}
					}
				}
				pdst -= bmp_width;
				p += bytesPerLine;
			}
		break;
		case 4:
			pdst += (bmp_width * bmp_height);

			for(i=0;i<bmp_height;i++)
			{
				pdst -= bmp_width;
				count = 0;
				for (j=0;j<(bmp_width+1)/2;j++)
				{
					temp = p[j];
					pdst[0]= ((temp & 0xf0) >> 4);
					count++;
					pdst++;
					if (count == bmp_width)
					{
						break;
					}

					pdst[0] = temp & 0x0f;
					count++;
					pdst++;
					if (count == bmp_width)
					{
						break;
					}
				}
				pdst -= bmp_width;
				p += bytesPerLine;
			}
		break;
		case 8:
			pdst += (bmp_width*bmp_height);
			for(i=0;i<bmp_height;i++)
			{
				p = psrc+(i+1)*bytesPerLine;
				p -= skip;
				for(j=0;j<bmp_width;j++)
				{
					pdst -= 1;
					p -= 1;
					pdst[0] = p[0];
				}
			}
		break;
		case 16:
			pdst += (bmp_width*bmp_height*2);
			for(i=0;i<bmp_height;i++)
			{
				p = psrc+(i+1)*bytesPerLine;
				p -= skip;
				for(j=0;j<bmp_width;j++)
				{
					pdst -= 2;
					p -= 2;
					pdst[0] = p[1];
					pdst[1] = p[0];
				}
			}
		break;
		case 24:
			pdst += (bmp_width*bmp_height*3);
			for(i=0;i<bmp_height;i++)
			{
				p = psrc+(i+1)*bytesPerLine;
				p -= skip;
				for(j=0;j<bmp_width;j++)
				{
					pdst -= 3;
					p -= 3;
					pdst[0] = p[2];
					pdst[1] = p[1];
					pdst[2] = p[0];
				}
			}
		break;
		case 32:
			pdst += (bmp_width*bmp_height*4);
			for(i=0;i<bmp_height;i++)
			{
				p = psrc+(i+1)*bmp_width*4;
				for(j=0;j<bmp_width;j++)
				{
					pdst -= 4;
					p -= 4;
					pdst[0] = p[2];
					pdst[1] = p[1];
					pdst[2] = p[0];
					pdst[3] = p[3];
				}
			}
		break;
		
		default:
		break;
	}	
}

static void DrawMatrix(UWORD Xpos, UWORD Ypos,UWORD Width, UWORD High,UBYTE* Matrix)
{
	UWORD i,j,x,y;
	//UBYTE R,G,B;
	//UBYTE temp1,temp2;
	double Gray;
	//Debug("%d,%d,%d,%d\n",Xpos,Ypos,Width,High);
	for (y=0,j=Ypos;y<High;y++,j++)
	{
 		for (x=0,i=Xpos;x<Width;x++,i++)
		{
			/*switch(bmp_BitCount)
			{
				case 1:
				case 4:
				case 8:
					R = palette[Matrix[(y*Width+x)]].rgbRed;
					G = palette[Matrix[(y*Width+x)]].rgbGreen;
					B = palette[Matrix[(y*Width+x)]].rgbBlue;
				break;
				
				case 16:
					temp1 = Matrix[(y*Width+x)*2];
					temp2 = Matrix[(y*Width+x)*2+1];
					R = (temp1 & 0x7c)<<1;
					G = (((temp1 & 0x03) << 3 ) | ((temp2&0xe0) >> 5))<<3;
					B = (temp2 & 0x1f)<<3;
				break;
				
				case 24:
					R = Matrix[(y*Width+x)*3];
					G = Matrix[(y*Width+x)*3+1];
					B = Matrix[(y*Width+x)*3+2];
				break;
				
				case 32:
					R = Matrix[(y*Width+x)*4];
					G = Matrix[(y*Width+x)*4+1];
					B = Matrix[(y*Width+x)*4+2];
				break;
				
				default:
				break;
			}*/
			//Debug("%d,%d\n",x,y);
			Gray = Matrix[y*Width+x]*255;
			//Debug("%lf\n",Gray);
            Paint_SetPixel(i, j, Gray);
            //Debug("%d,%d\n",x,y);
		}
	}
}


int read_row(FILE* fp) {
	//fp = fopen("A.txt", "r");
	int c, m = 0;
	while ((c = fgetc(fp)) != EOF)
		if (c == '\n')
		{
			m++;
		}
	m++;
	rewind(fp);
	return m;
}

int read_rank(FILE* fp) //读取有几列
{
	int n = 0, c;
	while ((c = fgetc(fp)) != '\n')
		if (c == ' ')
			n++;
	n++;
	rewind(fp);
	return n;
}


UBYTE GUI_ReadBmp(const char *path, UWORD x, UWORD y)
{
	clock_t start, finish;
    double duration;
    start = clock();
	//bmp file pointer
	FILE *fp;
	//BMPFILEHEADER FileHead;
	//BMPINFOHEADER InfoHead;
	//UDOUBLE total_length;
	//UBYTE *buf = NULL;
	//UDOUBLE ret = -1;
	int row = 1872, column = 1404;
	
	fp = fopen(path,"r");
	row = read_row(fp)-1;
	column = read_rank(fp);
	UBYTE a[2000][1500];
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < column; j++)
		{
			fscanf(fp, "%hhd", &a[i][j]);
		}
	}
	fclose(fp);


/*尝试矩阵数据连接
	if (fp == NULL)
	{
		return(-1);
	}
 
	ret = fread(&FileHead, sizeof(BMPFILEHEADER),1, fp);
	if (ret != 1)
	{
		Debug("Read header error!\n");
		fclose(fp);
		return(-2);
	}

	//Detect if it is a bmp image, since BMP file type is "BM"(0x4D42)
	if (FileHead.bType != 0x4D42)
	{
		Debug("It's not a BMP file\n");
		fclose(fp);
		return(-3);
	}
	
	//Debug("*****************************************\n");
	//Debug("BMP_bSize:%d \n", FileHead.bSize);
 	//Debug("BMP_bOffset:%d \n", FileHead.bOffset);
	
	ret = fread((char *)&InfoHead, sizeof(BMPINFOHEADER),1, fp);
	if (ret != 1)
	{
		Debug("Read infoheader error!\n");
		fclose(fp);
		return(-4);
	}
	
	Debug("BMP_biInfoSize:%d \n", InfoHead.biInfoSize);
 	Debug("BMP_biWidth:%d \n", InfoHead.biWidth);
	Debug("BMP_biHeight:%d \n", InfoHead.biHeight);
	Debug("BMP_biPlanes:%d \n", InfoHead.biPlanes);
	Debug("BMP_biBitCount:%d \n", InfoHead.biBitCount);
	Debug("BMP_biCompression:%d \n", InfoHead.biCompression);
	Debug("BMP_bimpImageSize:%d \n", InfoHead.bimpImageSize);
	Debug("BMP_biXPelsPerMeter:%d \n", InfoHead.biXPelsPerMeter);
	Debug("BMP_biYPelsPerMeter:%d \n", InfoHead.biYPelsPerMeter);
	Debug("BMP_biClrUsed:%d \n", InfoHead.biClrUsed);
	Debug("BMP_biClrImportant:%d \n", InfoHead.biClrImportant);
	
	total_length = FileHead.bSize-FileHead.bOffset;
	bytesPerLine=((InfoHead.biWidth*InfoHead.biBitCount+31)>>5)<<2;
	imageSize=bytesPerLine*InfoHead.biHeight;
	skip=(4-((InfoHead.biWidth*InfoHead.biBitCount)>>3))&3;
	
	Debug("bimpImageSize:%d\n", InfoHead.bimpImageSize);
	Debug("total_length:%d\n", total_length);
	Debug("bytesPerLine = %d\n", bytesPerLine);
	Debug("imageSize = %d\n", imageSize);
	Debug("skip = %d\n", skip);
	Debug("*****************************************\n");
	
    bmp_width = InfoHead.biWidth;
    bmp_height = InfoHead.biHeight;
	bmp_BitCount = InfoHead.biBitCount;
	
	//This is old code, but allocate imageSize byte memory is more reasonable
    bmp_src_buf = (UBYTE*)calloc(1,total_length);
	//bmp_src_buf = (UBYTE*)calloc(1,imageSize);
    if(bmp_src_buf == NULL){
        Debug("Load > malloc bmp out of memory!\n");
        return -1;
    }
	//This is old code, but allocate imageSize byte memory is more reasonable
	bmp_dst_buf = (UBYTE*)calloc(1,total_length);
	//bmp_dst_buf = (UBYTE*)calloc(1,imageSize);
    if(bmp_dst_buf == NULL){
        Debug("Load > malloc bmp out of memory!\n");
        return -2;
    }

	 //Jump to data area
    fseek(fp, FileHead.bOffset, SEEK_SET);
	
	//Bytes per line
    buf = bmp_src_buf;
    while ((ret = fread(buf,1,total_length,fp)) >= 0) 
	{
        if (ret == 0) 
		{
            //DEV_Delay_us(10);
            continue;
        }
		buf = ((UBYTE*)buf) + ret;
        total_length = total_length - ret;
        if(total_length == 0)
            break;
    }
	
	//Jump to color pattern board
	switch(bmp_BitCount)
	{	
		case 1:
			fseek(fp, 54, SEEK_SET);
			ret = fread(palette,1,4*2,fp);
			if (ret != 8) 
			{
				Debug("Error: fread != 8\n");
				return -5;
			}

			//this is old code, will likely result in memory leak if use 1bp source bmp image
			 
			bmp_dst_buf = (UBYTE*)calloc(1,InfoHead.biWidth * InfoHead.biHeight);
			if(bmp_dst_buf == NULL)
			{
				Debug("Load > malloc bmp out of memory!\n");
				return -5;
			}
			
		break;
		
		case 4:
			fseek(fp, 54, SEEK_SET);
			ret = fread(palette,1,4*16,fp);
			if (ret != 64) 
			{
				Debug("Error: fread != 64\n");
				return -5;
			}
			//this is old code, will likely result in memory leak if use 4bp source bmp image
			
			bmp_dst_buf = (UBYTE*)calloc(1,InfoHead.biWidth * InfoHead.biHeight);
			if(bmp_dst_buf == NULL)
			{
				Debug("Load > malloc bmp out of memory!\n");
				return -5;
			}
			
		break;
		
		case 8:
			fseek(fp, 54, SEEK_SET);

			ret = fread(palette,1,4*256,fp);

			if (ret != 1024) 
			{
				Debug("Error: fread != 1024\n");
				return -5;
			}
		break;
		
		default:
		break;
	}
	


    start = clock();

	Bitmap_format_Matrix(bmp_dst_buf,bmp_src_buf);


	finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    Debug( "1 occupy %f second\n", duration );
*/

    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    Debug( "1 occupy %f second\n", duration );
    start = clock();


	DrawMatrix(x, y,row , column , a[0]);

finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    Debug( "2 occupy %f second\n", duration );
    start = clock();


    //free(bmp_src_buf);
    //free(bmp_dst_buf);

finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    Debug( "3 occupy %f second\n", duration );
    start = clock();


	//bmp_src_buf = NULL;
	//bmp_dst_buf = NULL;

	

	finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    Debug( "4 occupy %f second\n", duration );

	return(0);
}