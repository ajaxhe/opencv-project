#ifndef COMMON_H
#define COMMON_H

#include "precomp.hpp"
//#include "opencv2/core/types_c.h" // for cvScalar

// blob track framework interface
CvBlobTrackerAuto* cvCreateBlobTrackerAuto0(CvBlobTrackerAutoParam1* param);

// blob detection interface
void cvFindBlobsByCCClasters(IplImage* pFG, CvBlobSeq* pBlobs, CvMemStorage* storage);
CvBlobDetector* cvCreateBlobDetectorSimple0();
CvBlobDetector* cvCreateBlobDetectorCC0(); // blob detection by using connected component

// blob tracking interface
CvBlobTracker* cvCreateBlobTrackerCC0();

// tool function
void eeRectangle(IplImage* pImg, CvBlob* pB, CvScalar color);

#endif // COMMON_H