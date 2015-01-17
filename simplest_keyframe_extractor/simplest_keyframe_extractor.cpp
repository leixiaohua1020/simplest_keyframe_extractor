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

KEImage keimagelist[]={
	{KE_IMAGE_JPG,"jpg",PIX_FMT_YUVJ420P,CODEC_ID_MJPEG},
	{KE_IMAGE_PNG,"png",PIX_FMT_RGB24,CODEC_ID_PNG},
	{KE_IMAGE_BMP,"bmp",PIX_FMT_BGRA,CODEC_ID_BMP}
};

KERecord kerecordlist[]={
	{KE_RECORD_TXT,"txt",ke_record_txt_init,ke_record_txt_save,ke_record_txt_close,NULL,NULL},
	{KE_RECORD_MYSQL,"mysql",ke_record_mysql_init,ke_record_mysql_save,ke_record_mysql_close,NULL,NULL}
};


KEMethod kemethodlist[]={
	{KE_METHOD_IFRAME,"iframe",ke_method_iframe_init,ke_method_iframe_extract,ke_method_iframe_close,NULL,NULL},
	{KE_METHOD_BFM,"bfm",ke_method_bfm_init,ke_method_bfm_extract,ke_method_bfm_close,NULL,NULL},
	{KE_METHOD_INTERVAL,"interval",ke_method_interval_init,ke_method_interval_extract,ke_method_interval_close,NULL,NULL}
};


//Right
#define REFRESH_R_EVENT  (SDL_USEREVENT + 1)
//Left
#define REFRESH_L_EVENT  (SDL_USEREVENT + 2)

int show_thread(void *opaque){
	KEContext *pKECtx=(KEContext *)opaque;
	
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 

	pKECtx->screen = SDL_SetVideoMode((pKECtx->screen_w)*2+SDL_PADDING*3, pKECtx->screen_h+SDL_PADDING*2, 0, 0);
	if(!pKECtx->screen) {
		printf("SDL: could not set video mode - exiting\n");
		return -1;
	}

	pKECtx->bmp_l = SDL_CreateYUVOverlay(pKECtx->pixel_l_w, pKECtx->pixel_l_h,SDL_YV12_OVERLAY, pKECtx->screen); 
	pKECtx->bmp_r = SDL_CreateYUVOverlay(pKECtx->pixel_r_w, pKECtx->pixel_r_h,SDL_YV12_OVERLAY, pKECtx->screen); 
	//Left
	pKECtx->rect_l.x = SDL_PADDING;    
	pKECtx->rect_l.y = SDL_PADDING;    
	pKECtx->rect_l.w = pKECtx->screen_w;    
	pKECtx->rect_l.h = pKECtx->screen_h; 
	//Right
	pKECtx->rect_r.x = pKECtx->screen_w+SDL_PADDING*2;
	pKECtx->rect_r.y = SDL_PADDING;
	pKECtx->rect_r.w = pKECtx->screen_w;
	pKECtx->rect_r.h = pKECtx->screen_h;
	SDL_WM_SetCaption("Keyframe Extractor  [left]video content|[right]keyframe",NULL);


	SDL_Event event;
	while(pKECtx->mark_exit==0) {
		SDL_WaitEvent(&event);
		switch(event.type){
		case REFRESH_R_EVENT:{
			SDL_LockYUVOverlay(pKECtx->bmp_r);
			pKECtx->bmp_r->pixels[0]=pKECtx->pFrameEncode->data[0];
			pKECtx->bmp_r->pixels[2]=pKECtx->pFrameEncode->data[1];
			pKECtx->bmp_r->pixels[1]=pKECtx->pFrameEncode->data[2];     
			pKECtx->bmp_r->pitches[0]=pKECtx->pFrameEncode->linesize[0];
			pKECtx->bmp_r->pitches[2]=pKECtx->pFrameEncode->linesize[1];   
			pKECtx->bmp_r->pitches[1]=pKECtx->pFrameEncode->linesize[2];
			SDL_UnlockYUVOverlay(pKECtx->bmp_r);
			SDL_DisplayYUVOverlay(pKECtx->bmp_r, &pKECtx->rect_r); 
			break;
						   }
		case REFRESH_L_EVENT:{
			SDL_LockYUVOverlay(pKECtx->bmp_l);
			pKECtx->bmp_l->pixels[0]=pKECtx->pFrameShow->data[0];
			pKECtx->bmp_l->pixels[2]=pKECtx->pFrameShow->data[1];
			pKECtx->bmp_l->pixels[1]=pKECtx->pFrameShow->data[2];     
			pKECtx->bmp_l->pitches[0]=pKECtx->pFrameShow->linesize[0];
			pKECtx->bmp_l->pitches[2]=pKECtx->pFrameShow->linesize[1];   
			pKECtx->bmp_l->pitches[1]=pKECtx->pFrameShow->linesize[2];
			SDL_UnlockYUVOverlay(pKECtx->bmp_l); 
			SDL_DisplayYUVOverlay(pKECtx->bmp_l, &pKECtx->rect_l); 
			break;
							 }
		case SDL_QUIT:{
			pKECtx->mark_exit=1;
			break;
					  }

		}
	}
	return 0;
}


KEContext* ke_alloc_context(){
	KEContext *pKECtx=(KEContext *)malloc(sizeof(KEContext));
	memset(pKECtx,0,sizeof(KEContext));
	pKECtx->methodlist=kemethodlist;
	pKECtx->recordlist=kerecordlist;
	pKECtx->imagelist=keimagelist;

	pKECtx->method=kemethodlist;
	pKECtx->record=kerecordlist;
	pKECtx->image=keimagelist;

	pKECtx->screen_w=640;
	pKECtx->screen_h=360;
	pKECtx->pixel_l_w=640;
	pKECtx->pixel_l_h=360;
	pKECtx->pixel_r_w=640;
	pKECtx->pixel_r_h=360;
	return pKECtx;
}

int ke_free_context(KEContext *kectx){
	free(kectx);
	return 0;
}



//Encode 1 frame
int ke_encode_frame(KEContext *pKECtx){
	AVFormatContext* pFormatCtx;  
	AVOutputFormat* fmt;  
	AVStream* video_st;  
	AVCodecContext* pCodecCtx;  
	AVCodec* codec;
	double video_pts;
	uint8_t* picture_buf;
	AVFrame* picture;
	int size;
	int ret;

	char *file_suffix=(char *)malloc(4);
	switch(pKECtx->imagecur){
	case KE_IMAGE_JPG: file_suffix="jpg";break;
	case KE_IMAGE_PNG: file_suffix="png";break;
	case KE_IMAGE_BMP: file_suffix="bmp";break;
	default:  file_suffix="jpg";break;
	}

	sprintf(pKECtx->outfilename,"%d.%s",pKECtx->keyframenum,file_suffix);
	sprintf(pKECtx->outfilefullpath,"%s/%s",pKECtx->outfilefolder,pKECtx->outfilename);
	if(pKECtx->relevant_outfilefolder!=NULL){
		sprintf(pKECtx->relevant_outfilepath,"%s/%s",pKECtx->relevant_outfilefolder,pKECtx->outfilename);
	}
	
	avformat_alloc_output_context2(&pFormatCtx, NULL, NULL,pKECtx->outfilefullpath );
	
	video_st =NULL;

	video_st = av_new_stream(pFormatCtx, 0);  
	pCodecCtx = video_st->codec;  
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;  
	pCodecCtx->bit_rate = 400000;  
	pCodecCtx->width = pKECtx->pFrameEncode->width;  
	pCodecCtx->height = pKECtx->pFrameEncode->height;  
	//-------------
	pCodecCtx->time_base.num = 1;  
	pCodecCtx->time_base.den = 25;   
	pCodecCtx->gop_size = 12;  

	pCodecCtx->pix_fmt=pKECtx->image->pix_fmt;
	
	if (!video_st){
		return -1;
	}
	
	pCodecCtx = video_st->codec;

	codec = avcodec_find_encoder(pKECtx->image->codecid);

	if (!codec){
		return -1;
	}
	if (avcodec_open2(pCodecCtx, codec,NULL) < 0){
		return -1;
	}
	picture = avcodec_alloc_frame();
	avpicture_fill((AVPicture*)picture, NULL, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

	if((_access(pKECtx->outfilefolder, 0 )) == -1 ){  
		printf("Output Path is not exist. Create new Folder.\n");
		_mkdir(pKECtx->outfilefolder);
	}
	if (avio_open(&pFormatCtx->pb,pKECtx->outfilefullpath, AVIO_FLAG_READ_WRITE) < 0){
		printf("Cannot open output file.");
		return -1;
	}

	avformat_write_header(pFormatCtx,NULL);

	int got_picture;
	AVPacket pkt;

	av_new_packet(&pkt,pCodecCtx->width*pCodecCtx->height*4+2000);
		
	picture->data[0] = pKECtx->pFrameEncode->data[0];
	picture->data[1] = pKECtx->pFrameEncode->data[1];
	picture->data[2] = pKECtx->pFrameEncode->data[2];

	ret = avcodec_encode_video2(pCodecCtx, &pkt,picture, &got_picture);
	if (got_picture==1){
		pkt.stream_index = video_st->index;
		ret = av_write_frame(pFormatCtx, &pkt);
	}

	av_free_packet(&pkt);

	if (video_st)
	{
		avcodec_close(video_st->codec);
		av_free(picture);
	}

	av_write_trailer(pFormatCtx);
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);
	av_free_packet(&pkt);
	
	
	return 0;
}


void ke_usage(){
	printf("---------------------- Help ---------------------\n");
	printf("Keyframe Extractor\n\nThis software is used to:\n");
	printf("1.Get Keyframe of video data\n.");
	printf("2.Save these Keyframe to picture.\n");
	printf("3.Save these Keyframe's information.\n\n");
	printf("Options:\n");
	printf("-i  Input video's URL\n");
	printf("-o  Output folder to save keyframe.\n");
	printf("-f  Format of output keyframe(default is %s):\n",keimagelist[0].name);
	for(int i=0;i<(sizeof(keimagelist)/sizeof(keimagelist[0]));i++)
		printf("     (%d)%s\n",i,keimagelist[i].name);
	printf("-m  Method to extract keyframe(default is %s):\n",kemethodlist[0].name);
	for(int i=0;i<(sizeof(kemethodlist)/sizeof(kemethodlist[0]));i++)
		printf("     (%d)%s\n",i,kemethodlist[i].name);
	printf("-g  Graphically show the process.\n");
	printf("-r  Record the result using(default is %s):\n",kerecordlist[0].name);
	for(int i=0;i<(sizeof(kerecordlist)/sizeof(kerecordlist[0]));i++)
		printf("     (%d)%s\n",i,kerecordlist[i].name);
	printf("-v  Input video's ID[Used in databse]\n");
	printf("-p  Path of output folder[Used in databse]\n");
	printf("-h  Help menu.\n");
	printf("------------------ Examples ---------------------\n");
	printf("[1]Input file is \"F:/movie/titanic.flv\", Output folder to save keyframe is \"e:/mykey\".\n");
	printf("[Cmd]ske -i F:/movie/titanic.flv -o e:/mykey\n");
	printf("[2]Input file is \"F:/movie/titanic.flv\", Output folder to save keyframe is \"e:/mykey\","
		" method is bfm, show process graphically.\n");
	printf("[Cmd]ske -i F:/movie/titanic.flv -o e:/mykey -m 1 -g\n");
	printf("[3]Input file is \"titanic.flv\"(relative path), Output folder to save keyframe is \"mykey\"(relative path),"
		" format of output keyframe is PNG.\n");
	printf("[Cmd]ske -i titanic.flv -o mykey -f 2\n");
	printf("[4]Input file is \"titanic.flv\"(relative path), Output folder to save keyframe is \"e:/mykey\","
		" use MySQL to save result. In database, videoid=1, path=temp\n");
	printf("[Cmd]ske -i titanic.flv -o e:/mykey -r 1 -v 1 -p temp\n");
	printf("-------------------------------------------------\n");
}

int main(int argc, char* argv[])
{
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;

	AVFrame	*pFrame;
	uint8_t *out_buffer1,*out_buffer2;


	int ret, got_picture;
	static struct SwsContext *img_convert_ctx1;
	static struct SwsContext *img_convert_ctx2;
	AVPacket *packet;
	int y_size;

	extern char *optarg;
	int opt;
	printf("-------------------------------------------------\n");
	printf("Keyframe Extractor\n");
	printf("Lei Xiaohua\n");
	printf("leixiaohua1020@126.com\n");
	printf("Communication University of China / Digital TV Technology\n");
	printf("http://blog.csdn.net/leixiaohua1020\n");
	printf("-------------------------------------------------\n");

	KEContext *pKECtx=ke_alloc_context();

	if(argc==1){
		printf("Please set options!\nType -h for more information.\n");
		ke_usage();
		return 0;
	}

  while ((opt =getopt(argc, argv,"i:o:v:p:f:m:r:v:p:hg")) != -1)
    {
      switch (opt)
		{
		case 'h':
		  ke_usage();
		  return 0;
		case 'i':{
			strcpy(pKECtx->infilepath,optarg);
			break;
			}
		case 'o':{
			strcpy(pKECtx->outfilefolder,optarg);
			break;
			}
		case 'v':{
			pKECtx->videoid= atoi(optarg);
			break;
			}
		case 'p':{
			strcpy(pKECtx->relevant_outfilefolder,optarg);
			break;
			}
		case 'f':{
			pKECtx->imagecur= atoi(optarg);
			pKECtx->image=&(pKECtx->imagelist[pKECtx->imagecur]);
			break;
			}
		case 'm':{
			pKECtx->methodcur= atoi(optarg);
			pKECtx->method=&(pKECtx->methodlist[pKECtx->methodcur]);
			break;
			}
		case 'r':{
			pKECtx->recordcur= atoi(optarg);
			pKECtx->record= &(pKECtx->recordlist[pKECtx->recordcur]);
			break;
			}
		case 'g':{
			pKECtx->graphical=1;
			break;
			}
		default:
		  printf("Unknown option: %c\n", opt);
		  ke_usage();
		  return 0;
		}
    }
	if(pKECtx->infilepath==NULL||pKECtx->outfilefolder==NULL){
		printf("Input Path or Output Folder is null. Exit.\n");
		return 0;
	}
	printf("--------------------- Param ---------------------\n");
	printf("Input Path   : \t%s\n",pKECtx->infilepath);
	printf("Output Folder: \t%s\n",pKECtx->outfilefolder);
	if(pKECtx->recordcur==KE_RECORD_MYSQL){
		printf("[Submit data to MySQL Database]\n");
		printf("Video's ID: \t%s\n",pKECtx->infilepath);
		printf("Keyframe's Relative Path: \t%s\n",pKECtx->infilepath);
	}
	printf("-------------------------------------------------\n");

	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();
	if(avformat_open_input(&pFormatCtx,pKECtx->infilepath,NULL,NULL)!=0){
		printf("Couldn't of input file.\n");
		return -1;
	}
	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}
	videoindex=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++) 
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
	if(videoindex==-1){
		printf("Couldn't find a video stream.\n");
		return -1;
	}
	pCodecCtx=pFormatCtx->streams[videoindex]->codec;
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL){
		printf("Codec not found.\n");
		return -1;
	}
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
		printf("Couldn't open codec.\n");
		return -1;
	}
	//FIX
	pKECtx->pixel_r_w=pCodecCtx->width;
	pKECtx->pixel_r_h=pCodecCtx->height;

	pFrame=avcodec_alloc_frame();
	pKECtx->pFrameShow=avcodec_alloc_frame();
	pKECtx->pFrameEncode=avcodec_alloc_frame();

	out_buffer1=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pKECtx->pixel_l_w, pKECtx->pixel_l_h));
	avpicture_fill((AVPicture *)pKECtx->pFrameShow, out_buffer1, PIX_FMT_YUV420P, pKECtx->pixel_l_w, pKECtx->pixel_l_h);

	//
	PixelFormat pix_fmt_encode=pKECtx->image->pix_fmt;
	out_buffer2=(uint8_t *)av_malloc(avpicture_get_size(pix_fmt_encode, pKECtx->pixel_r_w, pKECtx->pixel_r_h));
	avpicture_fill((AVPicture *)pKECtx->pFrameEncode, out_buffer2, pix_fmt_encode, pKECtx->pixel_r_w, pKECtx->pixel_r_h);

	
	if(pKECtx->graphical){
		SDL_CreateThread(show_thread,pKECtx);
	}

	y_size = pCodecCtx->width * pCodecCtx->height;
	packet=(AVPacket *)malloc(sizeof(AVPacket));
	printf("-------------- File Information -----------------\n");
	av_dump_format(pFormatCtx,0,pKECtx->infilepath,0);
	printf("-------------------------------------------------\n");

	img_convert_ctx1 = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pKECtx->pixel_l_w, pKECtx->pixel_l_h, PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL); 
	img_convert_ctx2 = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pKECtx->pixel_r_w, pKECtx->pixel_r_h, pix_fmt_encode, SWS_FAST_BILINEAR, NULL, NULL, NULL); 

	pKECtx->method->init(&pKECtx);
	pKECtx->record->init(&pKECtx);
	
	while(av_read_frame(pFormatCtx, packet)>=0)
	{
		if(packet->stream_index==videoindex)
		{
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if(ret < 0){
				printf("Error while decoding.\n");
				return -1;
			}
			if(got_picture){
				
				sws_scale(img_convert_ctx1, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pKECtx->pFrameShow->data, pKECtx->pFrameShow->linesize);
				
				pKECtx->pFrameShow->width=pKECtx->pixel_l_w;
				pKECtx->pFrameShow->height=pKECtx->pixel_l_h;

				pKECtx->pFrameEncode->width=pKECtx->pixel_r_w;
				pKECtx->pFrameEncode->height=pKECtx->pixel_r_h;

				int iskeyframe=0;
				int framenum=0;
				//Extract
				pKECtx->method->extract(pKECtx,pCodecCtx,pFrame,&iskeyframe,&framenum);
				
				pKECtx->framenum=framenum;
				
				//Additional info
				float framerate=av_q2d(pFormatCtx->streams[videoindex]->r_frame_rate);
				//secs
				pKECtx->frametime=(float)framenum/framerate;



				//is keyframe
				if(iskeyframe==1){
					//Small fix
					int tns, thh, tmm, tss;
					char timestr[KE_STRLEN]={0};
					tns = pKECtx->frametime;
					thh  = tns / 3600;
					tmm  = (tns % 3600) / 60;
					tss  = (tns % 60);
					sprintf(timestr,"%02d:%02d:%02d",thh,tmm,tss);

					sws_scale(img_convert_ctx2, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pKECtx->pFrameEncode->data, pKECtx->pFrameEncode->linesize);
					
					printf("[key]%5d [number]%7d [time]%s ",pKECtx->keyframenum,pKECtx->framenum,timestr);
					if(ke_encode_frame(pKECtx)!=0){
						printf("[encode]Failed  ");
						return -1;
					}else{
						printf("[encode]Succeed ");
					}

					if(pKECtx->record->save(pKECtx)!=0){
						printf("[record]Failed  ");
						return -1;
					}else{
						printf("[record]Succeed ");
					}
					printf("\n");

					pKECtx->keyframenum++;
					//Show
					if(pKECtx->graphical){
						SDL_Event event;
						event.type = REFRESH_R_EVENT;
						SDL_PushEvent(&event);
					}
					
				}
				//Show
				if(pKECtx->graphical){
					SDL_Event event;
					event.type = REFRESH_L_EVENT;
					SDL_PushEvent(&event);
				}
			}
		}
			
		av_free_packet(packet);
	}
	pKECtx->mark_exit=1;

	pKECtx->method->close(&pKECtx);
	pKECtx->record->close(&pKECtx);

	sws_freeContext(img_convert_ctx1);
	sws_freeContext(img_convert_ctx2);


	av_free(out_buffer1);
	av_free(out_buffer2);
	av_free(pKECtx->pFrameShow);
	av_free(pKECtx->pFrameEncode);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	ke_free_context(pKECtx);

	return 0;
}

