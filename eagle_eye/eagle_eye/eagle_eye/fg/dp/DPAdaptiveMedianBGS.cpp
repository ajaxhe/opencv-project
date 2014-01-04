#include "DPAdaptiveMedianBGS.h"

DPAdaptiveMedianBGS::DPAdaptiveMedianBGS() : firstTime(true), frameNumber(0), showOutput(true), threshold(40), samplingRate(7), learningFrames(30)
{
  std::cout << "DPAdaptiveMedianBGS()" << std::endl;
}

DPAdaptiveMedianBGS::~DPAdaptiveMedianBGS()
{
  std::cout << "~DPAdaptiveMedianBGS()" << std::endl;
}

//void DPAdaptiveMedianBGS::process(const cv::Mat &img_input, cv::Mat &img_output, cv::Mat &img_bgmodel)
void DPAdaptiveMedianBGS::Process(IplImage* img_input)
{
	if (img_input == NULL)
		return;

	loadConfig();

	if(firstTime)
		saveConfig();

	//frame = new IplImage(img_input);
	frame = img_input;
  
	if(firstTime)
		frame_data.ReleaseMemory(false);
	frame_data = frame;

  if(firstTime)
  {
    int width	= img_input->width;
    int height = img_input->height;

    lowThresholdMask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    lowThresholdMask.Ptr()->origin = IPL_ORIGIN_BL;

    highThresholdMask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
    highThresholdMask.Ptr()->origin = IPL_ORIGIN_BL;

    params.SetFrameSize(width, height);
    params.LowThreshold() = threshold;
    params.HighThreshold() = 2*params.LowThreshold();	// Note: high threshold is used by post-processing 
    params.SamplingRate() = samplingRate;
    params.LearningFrames() = learningFrames;
    
    bgs.Initalize(params);
    bgs.InitModel(frame_data);
  }

  bgs.Subtract(frameNumber, frame_data, lowThresholdMask, highThresholdMask);
  lowThresholdMask.Clear();
  bgs.Update(frameNumber, frame_data, lowThresholdMask);
  
  // converts old-style IplImage to the new matrix; the data is not copied by default
  cv::Mat foreground(highThresholdMask.Ptr());
/*
  if(showOutput)
  {
    cv::imshow("Adaptive Median (McFarlane&Schofield)", foreground);
  }
*/

  /* The method copies the matrix data to another matrix. Before copying the data, the method invokes
  m.create(this->size(), this->type);
  so that the destination matrix is reallocated if needed. While m.copyTo(m);
  */
  //foreground.copyTo(img_output);

  // copy image head, the data is not copied
  _ipl_mask = foreground;

  //delete frame;
  firstTime = false;
  frameNumber++;
}

IplImage* DPAdaptiveMedianBGS::GetMask()
{
	return &_ipl_mask;
}

void DPAdaptiveMedianBGS::saveConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/DPAdaptiveMedianBGS.xml", 0, CV_STORAGE_WRITE);

  cvWriteInt(fs, "threshold", threshold);
  cvWriteInt(fs, "samplingRate", samplingRate);
  cvWriteInt(fs, "learningFrames", learningFrames);
  cvWriteInt(fs, "showOutput", showOutput);

  cvReleaseFileStorage(&fs);
}

void DPAdaptiveMedianBGS::loadConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("./config/DPAdaptiveMedianBGS.xml", 0, CV_STORAGE_READ);
  
  threshold = cvReadIntByName(fs, 0, "threshold", 40);
  samplingRate = cvReadIntByName(fs, 0, "samplingRate", 7);
  learningFrames = cvReadIntByName(fs, 0, "learningFrames", 30);
  showOutput = cvReadIntByName(fs, 0, "showOutput", true);

  cvReleaseFileStorage(&fs);
}

void DPAdaptiveMedianBGS::Release()
{

}