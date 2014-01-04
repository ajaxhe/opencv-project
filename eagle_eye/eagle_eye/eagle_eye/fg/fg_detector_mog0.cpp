#include "fg_detector_mog0.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

CFGDetectorMOG0::CFGDetectorMOG0()
{
	_mog.varThreshold = 5;
	_mog.nmixtures = 5;
}

void CFGDetectorMOG0::Process(IplImage* image)
{
	//Mat mat_image(image, 0);
	Mat mat_image = cvarrToMat(image);
	/*cvtColor(mat_image, _mat_gray, CV_BGR2GRAY);
	_mog(_mat_gray, _mat_mask);
	*/
	_mog(mat_image, _mat_mask);
	//erode(_mat_mask, _mat_mask, Mat());
	//dilate(_mat_mask, _mat_mask, Mat(), Point(-1,-1), 3);
	_ipl_mask = _mat_mask;
}

IplImage* CFGDetectorMOG0::GetMask()
{
	return &_ipl_mask;
}

void CFGDetectorMOG0::Release()
{

}