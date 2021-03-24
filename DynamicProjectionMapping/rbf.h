#pragma once
#include "head.h"

///////////////////////////////////////////////////////////////////////////////
//         　　マーカ部分の明度を補間するためにRBF補間を用いる 　      　    //
///////////////////////////////////////////////////////////////////////////////

using namespace std;

class RBF {

private:

	int N; // データの数
	vector<cv::Point2i> X;
	Eigen::MatrixXf Y;
	Eigen::MatrixXf W;
	Eigen::MatrixXf Phi;

public:

	RBF();
	~RBF() {};
	void SetY(const cv::Mat roi);
	void SetPhi();
	void SetW();
	int GetValue(const int x, const int y);
	void Exe(cv::Mat& roi);
};