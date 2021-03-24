//
//  video.h
//  PoissonBlending
//
//  Created by ŽRŒû ŽüŒå on 2015/02/07.
//  Copyright (c) 2015”N ŽRŒû ŽüŒå. All rights reserved.
//

#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

//using namespace std;
//using namespace cv;

void videocapture(const char* videoname, std::vector<cv::Mat> &video);
std::vector<cv::Mat> videocapture(const std::string videoname);
int getvideoframeNo(const char* videoname);
void videopreview(std::vector<cv::Mat> &video, int duration, double fps);
void videowriter(std::string filename, std::vector<cv::Mat> &video, int duration, double fps);
