#pragma once
#include "head.h"

struct Cluster;

class Square {

private:
	uint idA, idB, idC, idD;
	cv::Vec2d vecAB, vecAC, vecAD, vecCB, vecDB, vecCD;
	double lsq;

public:
	Square() {};
	Square(const std::vector<Cluster*> clusterList_, const uint idOfSquare[4]);
	~Square() {};
	void caluculateLsq();
	double getLsq();

};
