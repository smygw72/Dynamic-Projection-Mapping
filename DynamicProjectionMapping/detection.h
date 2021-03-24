#pragma once
#include "head.h"

namespace fade = GEOM_FADE2D;

struct Cluster {
public:
	Cluster(const cv::Point2i _point, const uint _numberOfKeypoint)
		: point(_point)
		, numberOfKeypoint(_numberOfKeypoint) {}

public:
	cv::Point2i point;
	uint numberOfKeypoint;
};

void dotClusterExtraction(const std::vector<cv::Point2i*> _keypointList, std::vector<Cluster*>& _clusterList, cv::Mat& _img);

void getConnectivity(const std::vector<Cluster*> _clusterList, cv::Mat& _img);

void identifyDotCluster();

