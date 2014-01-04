#include "opencv2/video/background_segm.hpp"
#include "opencv2/legacy/blobtrack.hpp"
#include "opencv2/legacy/legacy.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc_c.h>

#include <stdio.h>
#include <string>
#include <iostream>

#include "common.h"
#include "fg/fg_detector_mog0.h"
#include "fg/dp/DPAdaptiveMedianBGS.h"

using namespace std;

// Select appropriate case insensitive string comparison function:
#if defined WIN32 || defined _MSC_VER
#include <windows.h>
# define MY_STRNICMP _strnicmp
# define MY_STRICMP _stricmp
# define MY_STRDUP _strdup
#else
# define MY_STRNICMP strncasecmp
# define MY_STRICMP strcasecmp
# define MY_STRDUP strdup
#endif

#ifdef WIN32
int getFiles(vector<string> &file_list, const char *target_dir, const char *suffix)
{
	int res = 0;
	int count = 0;
	HANDLE hfd;
	WIN32_FIND_DATA wfd;
	wchar_t old_dir[MAX_PATH];
	wchar_t wc_target_dir[MAX_PATH];
	wchar_t wc_path[MAX_PATH];
	wchar_t wc_suffix[MAX_PATH];
	char nstring[MAX_PATH];

	if (target_dir == NULL || suffix == NULL)
	{
		return -1;
	}

	// convert to wchar_t
	size_t orig_size = 0;
	size_t converted_chars = 0;
	orig_size = strlen(target_dir) + 1;
	mbstowcs_s(&converted_chars, wc_target_dir, orig_size, target_dir, _TRUNCATE);

	GetCurrentDirectory(MAX_PATH, old_dir);
	if ( !SetCurrentDirectory(wc_target_dir) )
	{
		return -1;
	}

	orig_size = strlen(suffix) + 1;
	mbstowcs_s(&converted_chars, wc_suffix, orig_size, suffix, _TRUNCATE);

	hfd = FindFirstFile(wc_suffix, &wfd);
	if (hfd != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				size_t path_size;
				swprintf_s(wc_path, MAX_PATH, L"%s/%s", wc_target_dir, wfd.cFileName);
				path_size = wcslen(wc_path) + 1;
				wcstombs_s(&converted_chars, nstring, path_size, wc_path, _TRUNCATE);
				file_list.push_back(nstring);
				//cout << nstring << endl;
			}
			res = FindNextFile(hfd, &wfd);

		}while (res != 0);
	}
	SetCurrentDirectory(old_dir);

	return count;
}
#endif // WIN32

int test_select[] = {4,0,1,0,0};

static CvFGDetector* cvCreatFGDetectorMOG0() 
{ 
	CvFGDetector* p_mog0 = new CFGDetectorMOG0; 
	return p_mog0;
}
static CvFGDetector* cvCreateFGDetectorDPAdaptiveMedianBGS()
{
	CvFGDetector* pDPAdaptive = new DPAdaptiveMedianBGS;
	return pDPAdaptive;
};
static CvFGDetector* cvCreateFGDetector0 () { return cvCreateFGDetectorBase(CV_BG_MODEL_FGD, NULL); }
static CvFGDetector* cvCreateFGDetector0Simple() { return cvCreateFGDetectorBase(CV_BG_MODEL_FGD_SIMPLE, NULL); }
static CvFGDetector* cvCreateFGDetector1      () { return cvCreateFGDetectorBase(CV_BG_MODEL_MOG,        NULL); }

struct SFGDetector_Module
{
	CvFGDetector* (*create)();
	const char* nickname;
	const char* description;
};

SFGDetector_Module FGDetector_Modules[] = 
{
	{cvCreateFGDetectorDPAdaptiveMedianBGS, "DPAdaptiveMedian", "DPAdaptiveMedianBGS"},
	{cvCreatFGDetectorMOG0 , "MOG_0", "An improved adaptive background mixture model for real-time tracking with shadow detection P. KadewTraKuPong and R. Bowden"},
	{cvCreateFGDetector0,"FG_0","Foreground Object Detection from Videos Containing Complex Background. ACM MM2003."},
	{cvCreateFGDetector0Simple,"FG_0S","Simplified version of FG_0"},
	{cvCreateFGDetector1,"FG_1","An Improved Adaptive Background Mixture Model for Real-time tracking with shadow detection 2001"},
	{NULL,NULL,NULL}
};

/* List of BLOB DETECTION modules: */
struct SBlobDetector_Module
{
	CvBlobDetector* (*create)();
	const char* nickname;
	const char* description;
};

SBlobDetector_Module BlobDetector_Modules[] =
{
	{cvCreateBlobDetectorCC0,"BD_CC0","Detect new blob by tracking CC of FG mask, modify by ajaxhe"},
	{cvCreateBlobDetectorCC,"BD_CC","Detect new blob by tracking CC of FG mask"},
	{cvCreateBlobDetectorSimple0,"BD_S0","Detect new blob by uniform moving of connected components of FG mask, modify by ajaxhe"},
	{cvCreateBlobDetectorSimple,"BD_Simple","Detect new blob by uniform moving of connected components of FG mask"},
	{NULL,NULL,NULL}
};

/* List of BLOB TRACKING modules: */
struct SBlobTracker_Module
{
	CvBlobTracker* (*create)();
	const char* nickname;
	const char* description;
};

SBlobTracker_Module BlobTracker_Modules[] =
{
	{cvCreateBlobTrackerCCMSPF,"CCMSPF","connected component tracking and MSPF resolver for collision"},
	{cvCreateBlobTrackerCC0,"CC0","Simple connected component tracking, modify by ajaxhe"},
	{cvCreateBlobTrackerCC,"CC","Simple connected component tracking"},
	{cvCreateBlobTrackerMS,"MS","Mean shift algorithm "},
	{cvCreateBlobTrackerMSFG,"MSFG","Mean shift algorithm with FG mask using"},
	{cvCreateBlobTrackerMSPF,"MSPF","Particle filtering based on MS weight"},
	{NULL,NULL,NULL}
};

// List of BLOB TRAJECTORY GENERATION modules:
struct SBlobTrackGen_Module
{
	CvBlobTrackGen* (*create)();
	const char* nickname;
	const char* description;
};

SBlobTrackGen_Module BlobTrackGen_Modules[] =
{
	{cvCreateModuleBlobTrackGenYML,"YML","Generate track record in YML format as synthetic video data"},
	{cvCreateModuleBlobTrackGen1,"RawTracks","Generate raw track record (x,y,sx,sy),()... in each line"},
	{NULL,NULL,NULL}
};

// List of BLOB TRAJECTORY POST PROCESSING modules:
struct SBlobTrackPostProc_Module
{
	CvBlobTrackPostProc* (*create)();
	const char* nickname;
	const char* description;
};

SBlobTrackPostProc_Module BlobTrackPostProc_Modules[] =
{
	{cvCreateModuleBlobTrackPostProcKalman, "Kalman", "Kalman filtering of blob position and size"},
	//{NULL, "None", "No post processing filter"},
	//    {cvCreateModuleBlobTrackPostProcTimeAverRect,"TimeAverRect","Average by time using rectangle window"},
	//    {cvCreateModuleBlobTrackPostProcTimeAverExp,"TimeAverExp","Average by time using exponential window"},
	{NULL,NULL,NULL}
};

struct SBlobTrackAnalysis_Module
{
	CvBlobTrackAnalysis* (*create)();
	const char* nickname;
	const char* description;
};

SBlobTrackAnalysis_Module BlobTrackAnalysis_Modules[] =
{
	{cvCreateModuleBlobTrackAnalysisHistPVS,"HistPVS","Histogram of 5D feature vector analysis (x,y,vx,vy,state)"},
	//{NULL,"None","No trajectory analiser"},
	{cvCreateModuleBlobTrackAnalysisHistP,"HistP","Histogram of 2D feature vector analysis (x,y)"},
	{cvCreateModuleBlobTrackAnalysisHistPV,"HistPV","Histogram of 4D feature vector analysis (x,y,vx,vy)"},
	{cvCreateModuleBlobTrackAnalysisHistSS,"HistSS","Histogram of 4D feature vector analysis (startpos,endpos)"},
	{cvCreateModuleBlobTrackAnalysisTrackDist,"TrackDist","Compare tracks directly"},
	{cvCreateModuleBlobTrackAnalysisIOR,"IOR","Integrator (by OR operation) of several analysers "},
	{NULL,NULL,NULL}
};

/* Read parameters from command line
* and transfer to specified module:
*/
static void setParams(int argc, char* argv[], CvVSModule* pM, const char* prefix, const char* module)
{
	int prefix_len = (int)strlen(prefix);
	int i;
	for(i=0; i<argc; ++i)
	{
		int j;
		char* ptr_eq = NULL;
		int   cmd_param_len=0;
		char* cmd = argv[i];
		if(MY_STRNICMP(prefix,cmd,prefix_len)!=0) continue;
		cmd += prefix_len;
		if(cmd[0]!=':')continue;
		cmd++;

		ptr_eq = strchr(cmd,'=');
		if(ptr_eq)
			cmd_param_len = (int)(ptr_eq-cmd);

		for(j=0; ; ++j)
		{
			int     param_len;
			const char*   param = pM->GetParamName(j);
			if(param==NULL) break;
			param_len = (int)strlen(param);
			if(cmd_param_len!=param_len) continue;
			if(MY_STRNICMP(param,cmd,param_len)!=0) continue;
			cmd+=param_len;
			if(cmd[0]!='=')continue;
			cmd++;
			pM->SetParamStr(param,cmd);
			printf("%s:%s param set to %g\n",module,param,pM->GetParam(param));
		}
	}

	pM->ParamUpdate();

}   /* setParams */

/* Print all parameter values for given module: */
static void printParams(CvVSModule* pM, const char* module, const char* log_name)
{
	FILE* log = log_name?fopen(log_name,"at"):NULL;
	int i;
	if(pM->GetParamName(0) == NULL ) return;


	printf("%s(%s) module parameters:\n",module,pM->GetNickName());
	if(log)
		fprintf(log,"%s(%s) module parameters:\n",module,pM->GetNickName());

	for (i=0; ; ++i)
	{
		const char*   param = pM->GetParamName(i);
		const char*   str = param?pM->GetParamStr(param):NULL;
		if(param == NULL)break;
		if(str)
		{
			printf("  %s: %s\n",param,str);
			if(log)
				fprintf(log,"  %s: %s\n",param,str);
		}
		else
		{
			printf("  %s: %g\n",param,pM->GetParam(param));
			if(log)
				fprintf(log,"  %s: %g\n",param,pM->GetParam(param));
		}
	}

	if(log) fclose(log);

}   // print_params 

// Run pipeline on all frames:
static int runBlobTrackingAuto( CvBlobTrackerAuto* pTracker, CvCapture* pCap, 
							   char* dir_name = NULL, char* fgavi_name = NULL, char* btavi_name = NULL )
{
	int OneFrameProcess = 0;
	int key;
	int FrameNum = 0;
	int res = 0;
	int isUsedLoadImageFunc = 0;
	CvVideoWriter* pFGAvi = NULL;
	CvVideoWriter* pBTAvi = NULL;
	vector<string> file_vec;
	vector<string>::iterator file_iter;

	//cvNamedWindow( "FG", 0 );
	if (dir_name)
	{
		res = getFiles(file_vec, dir_name, "*.jpg");
		if (res < 0)
		{
			return res;
		}
		sort(file_vec.begin(), file_vec.end());
	}

	// Main loop:
	for ( FrameNum=0, file_iter=file_vec.begin(); 
			(key=cvWaitKey(OneFrameProcess?0:1))!=27; 
			FrameNum++)
	{   /* Main loop: */
		IplImage*   pImg  = NULL;
		IplImage*   pMask = NULL;
		isUsedLoadImageFunc = 0;

		if (key != -1)
		{
			OneFrameProcess = 1;
			if(key=='r')OneFrameProcess = 0;
		}

		if (pCap)
		{
			pImg = cvQueryFrame(pCap);
		}
		else if (file_iter != file_vec.end())
		{
			// Must release it after, otherwise it will cause memory leak
			pImg = cvLoadImage(file_iter->c_str());
			isUsedLoadImageFunc = 1;
			++file_iter;
		}

		if (pImg == NULL) 
		{	
			break;
		}
		//cvShowImage("Raw image", pImg);

		// Process:
		pTracker->Process(pImg, pMask);

		if (pTracker->GetFGMask())
		{   // Debug FG: 
			IplImage*           pFG = pTracker->GetFGMask();
			CvSize              S = cvSize(pFG->width,pFG->height);
			static IplImage*    pI = NULL;

			if ( pI == NULL )
			{
				pI = cvCreateImage(S,pFG->depth,3);
			}
			cvCvtColor( pFG, pI, CV_GRAY2BGR );

			if(fgavi_name)
			{   // Save fg to avi file:
				if(pFGAvi==NULL)
				{
					pFGAvi=cvCreateVideoWriter(
						fgavi_name,
						CV_FOURCC('x','v','i','d'),
						25,
						S );
				}
				cvWriteFrame( pFGAvi, pI );
			}
/*
			if (pTracker->GetBlobNum() > 0)
			{   // Draw detected blobs:
				int i;
				for (i = pTracker->GetBlobNum(); i > 0; i--)
				{
					CvBlob* pB = pTracker->GetBlob(i-1);
					CvPoint p = cvPointFrom32f(CV_BLOB_CENTER(pB));
					CvSize  s = cvSize(MAX(1,cvRound(CV_BLOB_RX(pB))), MAX(1,cvRound(CV_BLOB_RY(pB))));
					int c = cvRound(255*pTracker->GetState(CV_BLOB_ID(pB)));
					cvEllipse( pI,
						p,
						s,
						0, 0, 360,
						CV_RGB(c,255-c,0), cvRound(1+(3*c)/255) );
				}   // Next blob: ;
			}
	*/
			cvNamedWindow("Foreground Image");
			cvShowImage("Foreground Image", pI);
		
		}   // Debug FG.
		

		// Draw debug info:
		if(pImg)
		{   // Draw all information about test sequence:
			char        str[1024];
			int         line_type = CV_AA;   // Change it to 8 to see non-antialiased graphics.
			CvFont      font;
			int         i;
			IplImage*   pI = cvCloneImage(pImg);

			cvInitFont( &font, CV_FONT_HERSHEY_PLAIN, 0.7, 0.7, 0, 1, line_type );

			for(i=pTracker->GetBlobNum(); i>0; i--)
			{
				CvSize  TextSize;
				CvBlob* pB = pTracker->GetBlob(i-1);
				/*
				CvPoint p = cvPoint(cvRound(pB->x*256),cvRound(pB->y*256));
				CvSize  s = cvSize(MAX(1,cvRound(CV_BLOB_RX(pB)*256)), MAX(1,cvRound(CV_BLOB_RY(pB)*256)));
				int c = cvRound(255*pTracker->GetState(CV_BLOB_ID(pB)));

				cvEllipse( pI, p, s,
					0, 0, 360,
					CV_RGB(c,255-c,0), cvRound(1+(3*0)/255), CV_AA, 8 );

				p.x >>= 8;
				p.y >>= 8;
				s.width >>= 8;
				s.height >>= 8;
				*/
				//CvSize  s = cvSize(MAX(1,cvRound(CV_BLOB_RX(pB))), MAX(1,cvRound(CV_BLOB_RY(pB))));
				CvSize  s = cvSize(CV_BLOB_RX(pB), CV_BLOB_RY(pB));
				int c = cvRound(255*pTracker->GetState(CV_BLOB_ID(pB)));
				CvPoint p = cvPoint(CV_BLOB_X(pB), CV_BLOB_Y(pB));
				CvPoint p1 = cvPoint(CV_BLOB_X(pB)-CV_BLOB_RX(pB), CV_BLOB_Y(pB)-CV_BLOB_RY(pB));
				CvPoint p2 = cvPoint(CV_BLOB_X(pB)+CV_BLOB_RX(pB), CV_BLOB_Y(pB)+CV_BLOB_RY(pB));
				cvRectangle(pI, p1, p2, CV_RGB(0,255,255));

				sprintf(str,"%03d",CV_BLOB_ID(pB));
				cvGetTextSize( str, &font, &TextSize, NULL );
				p.x -= s.width;
				p.y -= s.height;
				cvPutText( pI, str, p, &font, CV_RGB(0,255,255));
				{
					const char* pS = pTracker->GetStateDesc(CV_BLOB_ID(pB));

					if(pS)
					{
						char* pStr = MY_STRDUP(pS);
						char* pStrFree = pStr;

						while (pStr && strlen(pStr) > 0)
						{
							char* str_next = strchr(pStr,'\n');

							if(str_next)
							{
								str_next[0] = 0;
								str_next++;
							}

							p.y += TextSize.height+1;
							cvPutText( pI, pStr, p, &font, CV_RGB(0,255,255));
							pStr = str_next;
						}
						free(pStrFree);
					}
				}
			}   // Next blob.

			cvNamedWindow("Tracking");
			cvShowImage("Tracking", pI);

			if(btavi_name && pI)
			{   // Save to avi file:
				CvSize      S = cvSize(pI->width,pI->height);
				if(pBTAvi==NULL)
				{
					pBTAvi=cvCreateVideoWriter(
						btavi_name,
						CV_FOURCC('x','v','i','d'),
						25,
						S );
				}
				cvWriteFrame( pBTAvi, pI );
			}

			cvReleaseImage(&pI);
		}   // Draw all information about test sequence.

		// release it
		if (isUsedLoadImageFunc)
		{
			cvReleaseImage(&pImg);
		}
	}   //  Main loop.

	if(pFGAvi)cvReleaseVideoWriter( &pFGAvi );
	if(pBTAvi)cvReleaseVideoWriter( &pBTAvi );

	return 0;
}  // RunBlobTrackingAuto

int main(int argc, char* argv[])
{
	CvCapture* pCap = NULL;
	CvBlobTrackerAutoParam1 param = {0,0,0,0,0,0,0,0};
	CvBlobTrackerAuto* pTracker = NULL;

	SFGDetector_Module*           pFGModule = NULL;
	SBlobDetector_Module*         pBDModule = NULL;
	SBlobTracker_Module*          pBTModule = NULL;
	SBlobTrackPostProc_Module*    pBTPostProcModule = NULL;
	SBlobTrackGen_Module*         pBTGenModule = NULL;
	SBlobTrackAnalysis_Module*    pBTAnalysisModule = NULL;

	const char* fg_name = NULL; // foreground extraction alg name
	const char* bd_name = NULL; // blob detection alg name
	const char* bt_name = NULL; // blob tracking alg name
	const char* btpp_name = NULL; // blob tracking post proc alg name
	const char* btgen_name = NULL; // blob tracking gen alg name
	const char* bta_name = NULL; // blob track analysis
	char* track_name = NULL; // store the tracked trajectories
	char* bta_data_name = NULL;
	const char* FGTrainFrames = "20"; // FG training frames, default is 20

	const char* log_name = NULL;

	char* fgavi_name = NULL;
	char* btavi_name = NULL;

	char* dir_name = NULL;	// input picture samples directory
	char* avi_name = NULL; // input avi samples

	if(argc < 2)
	{   // Print help:
		int i;
		printf("blobtrack [fg=<fg_name>] [bd=<bd_name>]\n"
			"          [bt=<bt_name>] [btpp=<btpp_name>]\n"
			"          [bta=<bta_name>\n"
			"          [bta_data=<bta_data_name>\n"
			"          [bt_corr=<bt_corr_way>]\n"
			"          [btgen=<btgen_name>]\n"
			"          [track=<track_file_name>]\n"
			"          [scale=<scale val>] [noise=<noise_name>] [IVar=<IVar_name>]\n"
			"          [FGTrainFrames=<FGTrainFrames>]\n"
			"          [btavi=<avi output>] [fgavi=<avi output on FG>]\n"
			"          inputdir=<input_dir> | inputavi=<avi_file>\n");

		printf("  <bt_corr_way> is the method of blob position correction for the \"Blob Tracking\" module\n"
			"     <bt_corr_way>=none,PostProcRes\n"
			"  <FGTrainFrames> is number of frames for FG training\n"
			"  <track_file_name> is file name for save tracked trajectories\n"
			"  <bta_data_name> is file name for data base of trajectory analysis module\n"
			"  <avi_file> is file name of avi to process by BlobTrackerAuto\n"
			"  <input_dir> is the directory which contains jpgs or bmps\n");

		puts("\nModules:");
#define PR(_name,_m,_mt)\
	printf("<%s> is \"%s\" module name and can be:\n",_name,_mt);\
	for(i=0; _m[i].nickname; ++i)\
		{\
		printf("  %d. %s",i+1,_m[i].nickname);\
		if(_m[i].description)printf(" - %s",_m[i].description);\
		printf("\n");\
		}

		PR("fg_name",FGDetector_Modules,"FG/BG Detection");
		PR("bd_name",BlobDetector_Modules,"Blob Entrance Detection");
		PR("bt_name",BlobTracker_Modules,"Blob Tracking");
		PR("btpp_name",BlobTrackPostProc_Modules, "Blob Trajectory Post Processing");
		PR("btgen_name",BlobTrackGen_Modules, "Blob Trajectory Generation");
		PR("bta_name",BlobTrackAnalysis_Modules, "Blob Trajectory Analysis");
#undef PR
		return 0;
	}   // Print help.

	{   // Parse arguments:
		printf("Command line: %s ", argv[0]);
		int i;
		for (i=1; i<argc; ++i)
		{
			printf("%s ", argv[i]);
			int bParsed = 0;
			size_t len = strlen(argv[i]);
#define RO(_n1,_n2) if( strncmp(argv[i], _n1, strlen(_n1))==0 ) { _n2 = argv[i]+strlen(_n1); bParsed=1; };
			RO("fg=",fg_name);
			RO("fgavi=",fgavi_name);
			RO("btavi=",btavi_name);
			RO("bd=",bd_name);
			RO("bt=",bt_name);
			//RO("bt_corr=",bt_corr);
			RO("btpp=",btpp_name);
			RO("bta=",bta_name);
			//RO("bta_data=",bta_data_name);
			RO("btgen=",btgen_name);
			RO("track=",track_name);
			//RO("comment=",comment_name);
			//RO("FGTrainFrames=",FGTrainFrames);
			RO("log=",log_name);
			//RO("savestate=",savestate_name);
			//RO("loadstate=",loadstate_name);
			RO("inputdir=", dir_name);
			RO("inputavi=", avi_name);
#undef RO
		}
		printf("\n");
	}   // Parse arguments.

	if(track_name)
	{   // Set Trajectory Generator module:
		int i;
		if (!btgen_name)
		{
			btgen_name=BlobTrackGen_Modules[0].nickname;
		}

		for(i=0; BlobTrackGen_Modules[i].nickname; ++i)
		{
			if(MY_STRICMP(BlobTrackGen_Modules[i].nickname,btgen_name)==0)
				pBTGenModule = BlobTrackGen_Modules + i;
		}
	}  

	{   // Set default parameters for one processing: 
		//if(!bt_corr) bt_corr = "none";
		
		if(!fg_name) fg_name = FGDetector_Modules[test_select[0]].nickname;
		if(!bd_name) bd_name = BlobDetector_Modules[test_select[1]].nickname;
		if(!bt_name) bt_name = BlobTracker_Modules[test_select[2]].nickname;
		/*
		if(!fg_name) fg_name = FGDetector_Modules[2].nickname;
		if(!bd_name) bd_name = BlobDetector_Modules[0].nickname;
		if(!bt_name) bt_name = BlobTracker_Modules[2].nickname;
		
		if(!btpp_name) btpp_name = BlobTrackPostProc_Modules[0].nickname;
		if(!bta_name) bta_name = BlobTrackAnalysis_Modules[0].nickname;
		*/
	}

	for(pFGModule=FGDetector_Modules; pFGModule->nickname; ++pFGModule)
	{
		if( fg_name && MY_STRICMP(fg_name,pFGModule->nickname)==0 ) break;
	}

	for(pBDModule=BlobDetector_Modules; pBDModule->nickname; ++pBDModule)
	{
		if( bd_name && MY_STRICMP(bd_name,pBDModule->nickname)==0 ) break;
	}

	for(pBTModule=BlobTracker_Modules; pBTModule->nickname; ++pBTModule)
	{
		if( bt_name && MY_STRICMP(bt_name,pBTModule->nickname)==0 ) break;
	}

	for(pBTPostProcModule=BlobTrackPostProc_Modules; pBTPostProcModule->nickname; ++pBTPostProcModule)
	{
		if( btpp_name && MY_STRICMP(btpp_name,pBTPostProcModule->nickname)==0 ) break;
	}
		
	for(pBTAnalysisModule=BlobTrackAnalysis_Modules; pBTAnalysisModule->nickname; ++pBTAnalysisModule)
	{
		if( bta_name && MY_STRICMP(bta_name,pBTAnalysisModule->nickname)==0 ) break;
	}

	// Create source video:
	if(avi_name)
	{
		pCap = cvCaptureFromFile(avi_name);
		if(pCap==NULL)
		{
			printf("Can't open %s file\n",avi_name);
			return -1;
		}
	}

	{   // Display parameters:
		FILE* log = log_name?fopen(log_name,"at"):NULL;
		if(log)
		{   /* Print to log file: */
			fprintf(log,"\n=== Blob Tracking pipline in processing mode===\n");
			if(avi_name)
			{
				fprintf(log,"AVIFile: %s\n",avi_name);
			}
			else if (dir_name)
			{
				fprintf(log, "Sample Directory: %s\n", dir_name);
			}
			fprintf(log,"FGDetector:   %s\n", pFGModule->nickname);
			fprintf(log,"BlobDetector: %s\n", pBDModule->nickname);
			fprintf(log,"BlobTracker:  %s\n", pBTModule->nickname);
			fprintf(log,"BlobTrackPostProc:  %s\n", pBTPostProcModule->nickname);
			//fprintf(log,"BlobCorrection:  %s\n", bt_corr);

			fprintf(log,"Blob Trajectory Generator:  %s (%s)\n",
				pBTGenModule?pBTGenModule->nickname:"None",
				track_name?track_name:"none");

			fprintf(log,"BlobTrackAnalysis:  %s\n", pBTAnalysisModule->nickname);
			fclose(log);
		}

		printf("\n=== Blob Tracking pipline in %s mode===\n","processing");
		
		if(avi_name)
		{
			printf("AVIFile: %s\n",avi_name);
		}
		else if (dir_name)
		{
			printf("Sample Directory: %s\n", dir_name);
		}
		printf("FGDetector:   %s\n", pFGModule->nickname);
		printf("BlobDetector: %s\n", pBDModule->nickname);
		printf("BlobTracker:  %s\n", pBTModule->nickname);
		printf("BlobTrackPostProc:  %s\n", pBTPostProcModule?pBTPostProcModule->nickname:"None");
		//printf("BlobCorrection:  %s\n", bt_corr);

		printf("Blob Trajectory Generator:  %s (%s)\n",
			pBTGenModule?pBTGenModule->nickname:"None",
			track_name?track_name:"none");

		printf("BlobTrackAnalysis:  %s\n", pBTAnalysisModule?pBTAnalysisModule->nickname:"None");
	}   // Display parameters.

	{   // Create autotracker module and its components:
		param.FGTrainFrames = FGTrainFrames?atoi(FGTrainFrames):0;

		if (pFGModule && pFGModule->nickname)
		{
			/* Create FG Detection module: */
			param.pFG = pFGModule->create();
			if(!param.pFG)
				puts("Can not create FGDetector module");
			param.pFG->SetNickName(pFGModule->nickname);
			setParams(argc, argv, param.pFG, "fg", pFGModule->nickname);
		}

		/* Create Blob Entrance Detection module: */
		if (pBDModule && pBDModule->nickname)
		{
			param.pBD = pBDModule->create();
			if(!param.pBD)
				puts("Can not create BlobDetector module");
			param.pBD->SetNickName(pBDModule->nickname);
			setParams(argc, argv, param.pBD, "bd", pBDModule->nickname);
		}
		
		if (pBTModule && pBTModule->nickname)
		{
			/* Create blob tracker module: */
			param.pBT = pBTModule->create();
			if(!param.pBT)
				puts("Can not create BlobTracker module");
			param.pBT->SetNickName(pBTModule->nickname);
			setParams(argc, argv, param.pBT, "bt", pBTModule->nickname);
		}

		if (pBTGenModule && pBTGenModule->nickname)
		{
			/* Create blob trajectory generation module: */
			param.pBTGen = NULL;
			if(pBTGenModule && track_name && pBTGenModule->create)
			{
				param.pBTGen = pBTGenModule->create();
				param.pBTGen->SetFileName(track_name);
			}
			if(param.pBTGen)
			{
				param.pBTGen->SetNickName(pBTGenModule->nickname);
				setParams(argc, argv, param.pBTGen, "btgen", pBTGenModule->nickname);
			}
		}
		
		if (pBTPostProcModule && pBTPostProcModule->nickname)
		{
			/* Create blob trajectory post processing module: */
			param.pBTPP = NULL;
			if(pBTPostProcModule && pBTPostProcModule->create)
			{
				param.pBTPP = pBTPostProcModule->create();
			}
			if(param.pBTPP)
			{
				param.pBTPP->SetNickName(pBTPostProcModule->nickname);
				setParams(argc, argv, param.pBTPP, "btpp", pBTPostProcModule->nickname);
			}
		}
		
		//param.UsePPData = (bt_corr && MY_STRICMP(bt_corr,"PostProcRes")==0);

		if (pBTAnalysisModule && pBTAnalysisModule->nickname)
		{
			/* Create blob trajectory analysis module: */
			param.pBTA = NULL;
			if(pBTAnalysisModule && pBTAnalysisModule->create)
			{
				param.pBTA = pBTAnalysisModule->create();
				param.pBTA->SetFileName(bta_data_name);
			}
			if(param.pBTA)
			{
				param.pBTA->SetNickName(pBTAnalysisModule->nickname);
				setParams(argc, argv, param.pBTA, "bta", pBTAnalysisModule->nickname);
			}
		}
	} // Create autotracker module and its components:
	
	// Create whole pipline:
	pTracker = cvCreateBlobTrackerAuto0(&param);
	if(!pTracker)
		puts("Can not create BlobTrackerAuto");

	{   // Print module parameters:
		struct DefMMM
		{
			CvVSModule* pM;
			const char* name;
		} Modules[] = {
			{(CvVSModule*)param.pFG,"FGdetector"},
			{(CvVSModule*)param.pBD,"BlobDetector"},
			{(CvVSModule*)param.pBT,"BlobTracker"},
			{(CvVSModule*)param.pBTGen,"TrackGen"},
			{(CvVSModule*)param.pBTPP,"PostProcessing"},
			{(CvVSModule*)param.pBTA,"TrackAnalysis"},
			{NULL,NULL}
		};
		int     i;
		for(i=0; Modules[i].name; ++i)
		{
			if(Modules[i].pM)
				printParams(Modules[i].pM,Modules[i].name,log_name);
		}
	}   // Print module parameters. 

	// Run pipeline:
	runBlobTrackingAuto( pTracker, pCap, dir_name, fgavi_name, btavi_name );

	{   // release modules.
		if(param.pBT)cvReleaseBlobTracker(&param.pBT);
		if(param.pBD)cvReleaseBlobDetector(&param.pBD);
		if(param.pBTGen)cvReleaseBlobTrackGen(&param.pBTGen);
		if(param.pBTA)cvReleaseBlobTrackAnalysis(&param.pBTA);
		if(param.pFG)cvReleaseFGDetector(&param.pFG);
		if(pTracker)cvReleaseBlobTrackerAuto(&pTracker);

	}   // release modules.

	if(pCap)
		cvReleaseCapture(&pCap);

	return 0;
}