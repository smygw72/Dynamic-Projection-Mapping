#include "square.h"

Square::Square(const std::vector<Cluster*> clusterList_, const uint idOfSquare_[4])
{
	idB = idOfSquare_[0];
	idD = idOfSquare_[1];
	idA = idOfSquare_[2];
	idC = idOfSquare_[3];

	// Make vectors
	vecAB = cv::Vec2d(clusterList_[idB]->point.x - clusterList_[idA]->point.x, clusterList_[idB]->point.y - clusterList_[idA]->point.y);
	vecAC = cv::Vec2d(clusterList_[idC]->point.x - clusterList_[idA]->point.x, clusterList_[idC]->point.y - clusterList_[idA]->point.y);
	vecAD = cv::Vec2d(clusterList_[idD]->point.x - clusterList_[idA]->point.x, clusterList_[idD]->point.y - clusterList_[idA]->point.y);
	vecCB = cv::Vec2d(clusterList_[idB]->point.x - clusterList_[idC]->point.x, clusterList_[idB]->point.y - clusterList_[idC]->point.y);
	vecDB = cv::Vec2d(clusterList_[idB]->point.x - clusterList_[idD]->point.x, clusterList_[idB]->point.y - clusterList_[idD]->point.y);
	vecCD = cv::Vec2d(clusterList_[idD]->point.x - clusterList_[idC]->point.x, clusterList_[idD]->point.y - clusterList_[idC]->point.y);

	// Normalize
	vecAB /= cv::norm(vecAB);
	vecAC /= cv::norm(vecAC);
	vecAD /= cv::norm(vecAD);
	vecCB /= cv::norm(vecCB);
	vecDB /= cv::norm(vecDB);
	vecCD /= cv::norm(vecCD);
}

void Square::caluculateLsq()
{
	lsq = 1 - pow(vecAB.dot(vecAD), 2) / 3.0 - pow(vecCB.dot(vecCD), 2) / 3.0 - pow(vecAC.dot(vecDB), 2) / 3.0;
}

double Square::getLsq()
{
	return lsq;
}
