/**
 * 最简单的视频关键帧提取器
 * Simplest Keyframe Extractor
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序实现了视频数据中关键帧的提取。
 * This software extract keyframe from input video data.
 */
#include <stdio.h>

#include <io.h>
#include <direct.h>

//Param
#include "Getopt.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
	//SDL
#include "sdl/SDL.h"
#include "sdl/SDL_thread.h"
	//MySQL
#include "winsock.h"
#include "mysql/mysql.h" 
};

#define KE_STRLEN 500
//SDL border
#define SDL_PADDING 20

//Image format
enum KE_IMAGE_ID{
	KE_IMAGE_JPG,
	KE_IMAGE_PNG,
	KE_IMAGE_BMP
};

//Method to extract keyframe
enum KE_METHOD_ID{
	KE_METHOD_IFRAME,
	KE_METHOD_BFM,
	KE_METHOD_INTERVAL
};

//Record the result
enum KE_RECORD_ID{
	KE_RECORD_TXT,
	KE_RECORD_MYSQL
};


struct KEContext;

typedef struct KEImage{
	int frametype;
	char name[50];
	PixelFormat pix_fmt;
	CodecID codecid;
}KEImage;


typedef struct KEMethod{
	int id;
	char name[50];
	int (*init)(KEContext **kectx);
	int (*extract)(KEContext *pKECtx,AVCodecContext *pCodecCtx,
		AVFrame *pFrame,
		int *iskeyframe,int *framenum);
	int (*close)(KEContext **kectx);
	char *priv_data;
	int priv_data_size;
}KEMethod;

typedef struct KERecord{
	int id;
	char name[50];
	int (*init)(KEContext **kectx);
	int (*save)(KEContext *kectx);
	int (*close)(KEContext **kectx);
	char *priv_data;
	int priv_data_size;
}KERecord;


//Global
typedef struct KEContext{
	//SDL
	//Use SDL?
	int graphical;
	int mark_exit;
	SDL_Surface *screen; 
	SDL_Overlay *bmp_l;
	SDL_Overlay *bmp_r;
	SDL_Rect rect_l;
	SDL_Rect rect_r;
	int screen_w;
	int screen_h;
	int pixel_l_w;
	int pixel_l_h;
	int pixel_r_w;
	int pixel_r_h;

	//Show
	AVFrame *pFrameShow;
	//Encode
	AVFrame *pFrameEncode;
	int keyframenum;
	int framenum;
	int frametime;//ms
	int videoid;
	char outfilename[KE_STRLEN];
	char outfilefolder[KE_STRLEN];
	char infilepath[KE_STRLEN];
	char outfilefullpath[KE_STRLEN];
	char relevant_outfilefolder[KE_STRLEN];
	char relevant_outfilepath[KE_STRLEN];
	
	//SDL
	int enablesdl;

	KEMethod *methodlist;
	KERecord *recordlist;
	KEImage *imagelist;

	int imagecur;
	int methodcur;
	int recordcur;
	KEMethod *method;
	KERecord *record;
	KEImage *image;

}KEContext;


//Method
//iframe
int ke_method_iframe_init(KEContext **pKECtx);
int ke_method_iframe_extract(KEContext *pKECtx,AVCodecContext *pCodecCtx,AVFrame *pFrame,
	int *iskeyframe,int *framenum);
int ke_method_iframe_close(KEContext **pKECtx);

//Bfm
int ke_method_bfm_init(KEContext **pKECtx);
int ke_method_bfm_extract(KEContext *pKECtx,AVCodecContext *pCodecCtx,AVFrame *pFrame,
	int *iskeyframe,int *framenum);
int ke_method_bfm_close(KEContext **pKECtx);

//Interval
int ke_method_interval_init(KEContext **pKECtx);
int ke_method_interval_extract(KEContext *pKECtx,AVCodecContext *pCodecCtx,AVFrame *pFrame,
	int *iskeyframe,int *framenum);
int ke_method_interval_close(KEContext **pKECtx);

//Record

//MySQL
int ke_record_mysql_init(KEContext **pKECtx);
int ke_record_mysql_save(KEContext *pKECtx);
int ke_record_mysql_close(KEContext **pKECtx);
//TXT
int ke_record_txt_init(KEContext **pKECtx);
int ke_record_txt_save(KEContext *pKECtx);
int ke_record_txt_close(KEContext **pKECtx);