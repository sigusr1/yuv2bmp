#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>  
#include <sys/types.h>  

#define COMMON_LOG(fmt, ...)    printf("%s:%d: " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)
  
#pragma pack(2)  
  
/* 位图文件头*/
typedef struct BITMAPFILEHEADER  
{   
    u_int16_t bfType;   
    u_int32_t bfSize;   
    u_int16_t bfReserved1;   
    u_int16_t bfReserved2;   
    u_int32_t bfOffBits;   
}BITMAPFILEHEADER;   

/* 位图信息*/  
typedef struct BITMAPINFOHEADER  
{   
    u_int32_t biSize;   
    u_int32_t biWidth;   
    u_int32_t biHeight;   
    u_int16_t biPlanes;   
    u_int16_t biBitCount;   
    u_int32_t biCompression;   
    u_int32_t biSizeImage;   
    u_int32_t biXPelsPerMeter;   
    u_int32_t biYPelsPerMeter;   
    u_int32_t biClrUsed;   
    u_int32_t biClrImportant;   
}BITMAPINFODEADER;  

/* 调色板*/  
typedef struct tagPALETTEENTRY {
  u_int8_t peRed; 
  u_int8_t peGreen; 
  u_int8_t peBlue; 
  u_int8_t peFlags; 
} PALETTEENTRY; 

/* 位图文件描述*/
typedef struct tagBITMAP_FILE{

  BITMAPFILEHEADER bitmapheader;
  BITMAPINFODEADER bitmapinfoheader;
  PALETTEENTRY palette[2];
  u_int8_t buffer[0];
} BITMAP_FILE;

#pragma pack()  

int main( int argc, char **argv)  
{  
    int x;
    int y;
    int index;

    int width = 1920;
    int height = 1080;

    unsigned char *src;
    unsigned char *dst;

    FILE *fp;

    BITMAP_FILE bp_file;

    fp = fopen("test.yuv", "rb");
    if (NULL == fp)
    {
        COMMON_LOG("%s", "file not exist");
        return -1;
    }

    /* yuv 转换成1bpp的bmp，只需要y分量即可，
            对于4:2:0的yuv图片，只需要前width * height 个字节,
            转换后需要width * height / 8个字节来表示。
            由于例子中的yuv是1080p的，下面未考虑width的4字节对齐
       */

    src = malloc(width * height);
    if (NULL == src)
    {
        COMMON_LOG("malloc %d bytes fail.", width * height);
        fclose(fp);
        return -1;
    }

    dst = malloc(width * height / 8);
    if (NULL == dst)
    {
        COMMON_LOG("malloc %d bytes fail.", width * height / 8);
        free(src);
        fclose(fp);
        return -1;
    }

    memset(&bp_file, 0, sizeof(bp_file));
    
    bp_file.bitmapheader.bfType = ('B' | ('M' << 8));
    bp_file.bitmapheader.bfSize = sizeof(BITMAP_FILE) + (width * height / 8);
    bp_file.bitmapheader.bfOffBits = sizeof(BITMAP_FILE);

    bp_file.bitmapinfoheader.biSize = sizeof(BITMAPINFODEADER);
    bp_file.bitmapinfoheader.biWidth = width;
    bp_file.bitmapinfoheader.biHeight = height;
    bp_file.bitmapinfoheader.biPlanes = 1;
    bp_file.bitmapinfoheader.biBitCount = 1;
    bp_file.bitmapinfoheader.biCompression = 0;
    bp_file.bitmapinfoheader.biSizeImage = width * height / 8;
    bp_file.bitmapinfoheader.biClrUsed = 0;
    bp_file.bitmapinfoheader.biClrImportant = 0;

    /* 黑*/
    bp_file.palette[0].peRed = 0;
    bp_file.palette[0].peGreen = 0;
    bp_file.palette[0].peBlue = 0;

    /* 白*/
    bp_file.palette[1].peRed = 255;
    bp_file.palette[1].peGreen = 255;
    bp_file.palette[1].peBlue = 255;

    memset(src, 0, (width * height));
    fread(src, width * height, 1, fp);
    fclose(fp);  

    memset(dst, 0, (width * height / 8));
    
    index = 0;
    /* 从左下角到右上角扫描*/
    for(y=height - 1; y >= 0; y--) 
    {
        for(x=0; x < width; x++)  
        {  
            if (src[y*width+x] > 128)
            {
                dst[index/8] |= (1 << (7 -(index % 8)));
            }
            index++;
        }
    }

    fp = fopen("test.bmp", "wb");
    fwrite(&bp_file, sizeof(bp_file), 1, fp);
    fwrite(dst, (1920*1080/8), 1, fp);
    fclose(fp);

    free(dst);
    free(src);
    return 0;  
}  
