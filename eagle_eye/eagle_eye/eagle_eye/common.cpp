#include "common.h"

void eeRectangle(IplImage* pImg, CvBlob* pB, CvScalar color)
{
	//CvPoint p = cvPoint(CV_BLOB_X(pB), CV_BLOB_Y(pB));
	CvPoint p1 = cvPoint( (int)CV_BLOB_X(pB) - (int)CV_BLOB_RX(pB), (int)CV_BLOB_Y(pB) - (int)CV_BLOB_RY(pB) );
	CvPoint p2 = cvPoint( (int)CV_BLOB_X(pB) + (int)CV_BLOB_RX(pB), (int)CV_BLOB_Y(pB) + (int)CV_BLOB_RY(pB) );
	cvRectangle(pImg, p1, p2, color);
}