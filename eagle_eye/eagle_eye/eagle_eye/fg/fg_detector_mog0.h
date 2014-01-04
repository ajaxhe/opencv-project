// fg_detector_mog0.h -- fg detector by using Model Of Gaussion

#ifndef FG_DETECTOR_MOG0_H
#define FG_DETECTOR_MOG0_H

#include <opencv2/video/background_segm.hpp>
#include <opencv2/legacy/blobtrack.hpp>

class CFGDetectorMOG0 : public CvFGDetector
{
private:
	cv::BackgroundSubtractorMOG _mog;
	cv::Mat _mat_gray;
	cv::Mat _mat_mask;
	IplImage _ipl_mask;

public:
	CFGDetectorMOG0();

	IplImage* GetMask();
	void Process(IplImage* image);
	void Release();
};

#endif // FG_DETECTOR_MOG0_H