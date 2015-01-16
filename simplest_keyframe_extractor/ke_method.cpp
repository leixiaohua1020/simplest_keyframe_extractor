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
#include "ke.h"

//Iframe
typedef struct KEIframeContext{
	int framenum;
}KEIframeContext;

int ke_method_iframe_init(KEContext **pKECtx){
	int privCtx_size=sizeof(KEIframeContext);
	(*pKECtx)->method->priv_data_size=privCtx_size;
	(*pKECtx)->method->priv_data=(char *)malloc(privCtx_size);
	memset((*pKECtx)->method->priv_data,0,privCtx_size);
	return 0;
}

int ke_method_iframe_close(KEContext **pKECtx){
	KEIframeContext *privCtx=(KEIframeContext *)(*pKECtx)->method->priv_data;
	free(privCtx);
	privCtx=NULL;
	return 0;
}

int ke_method_iframe_extract(KEContext *pKECtx,AVCodecContext *pCodecCtx,AVFrame *pFrame,int *iskeyframe,int *framenum){
	KEIframeContext *privCtx=(KEIframeContext *)pKECtx->method->priv_data;
	if(pFrame->key_frame){
		*iskeyframe=1;
	}else{
		*iskeyframe=0;
	}
	*framenum=privCtx->framenum;
	privCtx->framenum++;
	return 0;
}

//Interval
typedef struct KEIntervalContext{
	int framenum;
	int interval;
}KEIntervalContext;

int ke_method_interval_init(KEContext **pKECtx){
	int privCtx_size=sizeof(KEIntervalContext);
	(*pKECtx)->method->priv_data_size=privCtx_size;
	(*pKECtx)->method->priv_data=(char *)malloc(privCtx_size);
	memset((*pKECtx)->method->priv_data,0,privCtx_size);

	KEIntervalContext *privCtx=(KEIntervalContext *)(*pKECtx)->method->priv_data;
	privCtx->interval=50;

	return 0;
}

int ke_method_interval_close(KEContext **pKECtx){
	KEIntervalContext *privCtx=(KEIntervalContext *)(*pKECtx)->method->priv_data;
	free(privCtx);
	privCtx=NULL;
	return 0;
}

int ke_method_interval_extract(KEContext *pKECtx,AVCodecContext *pCodecCtx,AVFrame *pFrame,int *iskeyframe,int *framenum){
	KEIntervalContext *privCtx=(KEIntervalContext *)pKECtx->method->priv_data;
	if(privCtx->framenum%privCtx->interval==0){
		*iskeyframe=1;
	}else{
		*iskeyframe=0;
	}
	*framenum=privCtx->framenum;
	privCtx->framenum++;
	return 0;
}



//Bfm
typedef struct KEBfmContext{
	int framenum;
	int avg_y_pre;
}KEBfmContext;

int ke_method_bfm_init(KEContext **pKECtx){

	int privCtx_size=sizeof(KEBfmContext);
	(*pKECtx)->method->priv_data_size=privCtx_size;
	(*pKECtx)->method->priv_data=(char *)malloc(privCtx_size);
	memset((*pKECtx)->method->priv_data,0,privCtx_size);

	return 0;
}

int ke_method_bfm_close(KEContext **pKECtx){
	KEBfmContext *privCtx=(KEBfmContext *)(*pKECtx)->method->priv_data;
	free(privCtx);
	privCtx=NULL;
	return 0;
}

int ke_method_bfm_extract(KEContext *pKECtx,AVCodecContext *pCodecCtx,AVFrame *pFrame,int *iskeyframe,int *framenum){
	int avg_y=0;
	int total_y=0;
	int score=0;
	KEBfmContext *privCtx=(KEBfmContext *)pKECtx->method->priv_data;
	for(int j=0;j<(pFrame->height);j++){
		for(int i=0;i<(pFrame->width);i++){
			total_y+=*(pFrame->data[0]+pFrame->linesize[0]*j+i);
		}
	}
	avg_y=total_y/(pFrame->width*pFrame->height);
	score=abs(avg_y-privCtx->avg_y_pre);
	privCtx->avg_y_pre=avg_y;
	if(score>10){
		*iskeyframe=1;
	}else{
		*iskeyframe=0;
	}
	*framenum=privCtx->framenum;
	privCtx->framenum++;
	return 0;
}

