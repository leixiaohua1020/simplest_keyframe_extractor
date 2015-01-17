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


//MySQL
typedef struct KEMysqlContext{
	char confpath[KE_STRLEN];
	char dbuser[KE_STRLEN];
	char dbpasswd[KE_STRLEN];
	char dbip[KE_STRLEN];
	char dbname[KE_STRLEN];
	char dbtablename[KE_STRLEN];
	MYSQL * con;
}KEMysqlContext;

int ke_record_mysql_load_configure(KEMysqlContext *pMysqlCtx,char *confpath){
	if(confpath==NULL){
#if 0
		GetModuleFileNameA(NULL,pMysqlCtx->confpath,KE_STRLEN);
		strrchr( pMysqlCtx->confpath, '\\')[0]= '\0';
		strcat(pMysqlCtx->confpath,"\\configure.ini");
#else
		GetFullPathNameA("configure.ini",KE_STRLEN,pMysqlCtx->confpath,NULL);
#endif
		printf("[Configure Url]%s\n",pMysqlCtx->confpath);
	}else{
		strcpy(pMysqlCtx->confpath,confpath);
	}
	GetPrivateProfileStringA("Database","dbuser",NULL,pMysqlCtx->dbuser,KE_STRLEN,pMysqlCtx->confpath);
	GetPrivateProfileStringA("Database","dbpasswd",NULL,pMysqlCtx->dbpasswd,KE_STRLEN,pMysqlCtx->confpath);
	GetPrivateProfileStringA("Database","dbip",NULL,pMysqlCtx->dbip,KE_STRLEN,pMysqlCtx->confpath);
	GetPrivateProfileStringA("Database","dbname",NULL,pMysqlCtx->dbname,KE_STRLEN,pMysqlCtx->confpath);
	GetPrivateProfileStringA("Database","dbtablename",NULL,pMysqlCtx->dbtablename,KE_STRLEN,pMysqlCtx->confpath);
	return 0;
}

int ke_record_mysql_init(KEContext **pKECtx){

	int privCtx_size=sizeof(KEMysqlContext);
	(*pKECtx)->record->priv_data_size=privCtx_size;
	(*pKECtx)->record->priv_data=(char *)malloc(privCtx_size);
	memset((*pKECtx)->record->priv_data,0,privCtx_size);

	KEMysqlContext *privCtx=(KEMysqlContext *)(*pKECtx)->record->priv_data;


	ke_record_mysql_load_configure(privCtx,NULL);
	char* confpath=privCtx->confpath;
	char* dbuser=privCtx->dbuser;
	char* dbpasswd=privCtx->dbpasswd;
	char* dbip=privCtx->dbip;//cannot write "localhost"
	char* dbname=privCtx->dbname;
	//Init
	privCtx->con = mysql_init((MYSQL*) 0); 
	//Connect
	if ( privCtx->con !=NULL && mysql_real_connect(privCtx->con,dbip,dbuser,dbpasswd,dbname,3306,NULL,0) ){  
		//Select Database
		if (!mysql_select_db(privCtx->con,dbname)){
			printf("succeeded to select database!\n");
			printf("IP: %s\n",dbip);
			printf("User: %s\n",dbuser);
			printf("Database: %s\n",dbname);
			return 0;
		}else{
			printf("Failed to select database!\n");
			return -1;
		}
	}else{
		printf("Failed to connect to database!\n");
		return -1;
	}
}

//Save info use MySQL
int ke_record_mysql_save(KEContext *pKECtx){
	MYSQL * con=NULL;
	MYSQL_RES *res=NULL;
	MYSQL_ROW row;
	char query[KE_STRLEN]={0};
	int t;
	int count = 0;
	int rt;
	KEMysqlContext *privCtx=(KEMysqlContext *)pKECtx->record->priv_data;

	//Generate SQL
	if(pKECtx->relevant_outfilepath!=NULL&&pKECtx->videoid!=-1){
		sprintf(query,"insert into %s (name,path,videoid,keyframenum,framenum) values ('%s','%s',%d,%d,%d)",
			privCtx->dbtablename,pKECtx->outfilename,pKECtx->relevant_outfilepath,pKECtx->videoid,pKECtx->keyframenum,pKECtx->framenum);
	}else{
		sprintf(query,"insert into %s (name,keyframenum,framenum) values ('%s',%d,%d)",
			privCtx->dbtablename,pKECtx->outfilename,pKECtx->keyframenum,pKECtx->framenum); 
	}
	rt=mysql_real_query(privCtx->con,query,strlen(query)); 
	if (rt){
		printf("Query Error: %s! \n",mysql_error(privCtx->con));
		return -1;
	}

	return 0;
}

int ke_record_mysql_close(KEContext **pKECtx){
	KEMysqlContext *privCtx=(KEMysqlContext *)(*pKECtx)->record->priv_data;
	mysql_close(privCtx->con);
	free(privCtx);

	return 0;
}

//TXT
typedef struct KETxtContext{
	char path[KE_STRLEN];
	FILE *fp;
}KETxtContext;

int ke_record_txt_init(KEContext **pKECtx){
	int privCtx_size=sizeof(KETxtContext);
	(*pKECtx)->record->priv_data_size=privCtx_size;
	(*pKECtx)->record->priv_data=(char *)malloc(privCtx_size);
	memset((*pKECtx)->record->priv_data,0,privCtx_size);

	KETxtContext *privCtx=(KETxtContext *)(*pKECtx)->record->priv_data;

	privCtx->fp=fopen("record.txt","ab+");
	if(!privCtx->fp){
		printf("Cannot Open Output File!\n");
		return -1;
	}
	fprintf(privCtx->fp,"[Video URL]%s\n",(*pKECtx)->infilepath); 
	fprintf(privCtx->fp,"filename,keyframenum,framenum,time\n"); 
	return 0;
}

//Save info use TXT
int ke_record_txt_save(KEContext *pKECtx){
	KETxtContext *privCtx=(KETxtContext *)pKECtx->record->priv_data;
	int tns, thh, tmm, tss;
	char timestr[KE_STRLEN]={0};
	tns = pKECtx->frametime;
	thh  = tns / 3600;
	tmm  = (tns % 3600) / 60;
	tss  = (tns % 60);
	sprintf(timestr,"%02d:%02d:%02d",thh,tmm,tss);

	fprintf(privCtx->fp,"%s,%d,%d,%s\n",pKECtx->outfilename,pKECtx->keyframenum,pKECtx->framenum,timestr); 
	return 0;
}

int ke_record_txt_close(KEContext **pKECtx){
	KETxtContext *privCtx=(KETxtContext *)(*pKECtx)->record->priv_data;
	fprintf(privCtx->fp,"====================================\n"); 
	fclose(privCtx->fp);
	free(privCtx);
	return 0;
}
