#pragma once

#include <iostream>
#include <cv.h>
#include <highgui.h>

//#include "../IBGS.h"
#include <opencv2/legacy/blobtrack.hpp>
#include "AdaptiveMedianBGS.h"

using namespace Algorithms::BackgroundSubtraction;

class DPAdaptiveMedianBGS : public CvFGDetector
{
private:
  bool firstTime;
  long frameNumber;
  IplImage* frame;
  RgbImage frame_data;
  IplImage _ipl_mask;

  AdaptiveMedianParams params;
  AdaptiveMedianBGS bgs;
  BwImage lowThresholdMask;
  BwImage highThresholdMask;

  int threshold;
  int samplingRate;
  int learningFrames;
  bool showOutput;

public:
  DPAdaptiveMedianBGS();
  ~DPAdaptiveMedianBGS();

  //void process(const cv::Mat &img_input, cv::Mat &img_output, cv::Mat &img_bgmodel);
  void Process(IplImage* image);
  IplImage* GetMask();
  void Release();

private:
  void saveConfig();
  void loadConfig();
};

