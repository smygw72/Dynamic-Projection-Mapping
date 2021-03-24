#pragma once
#include "rbf.h"

using namespace std;

float phi(const cv::Point2i x_i, const cv::Point2i x_j)
{
	return pow(pow(x_i.x - x_j.x, 2) + pow(x_i.y - x_j.y, 2), 0.5);
}

RBF::RBF()
{
	N = 0;
}

void RBF::SetY(const cv::Mat roi)
{
	for (int y = 0; y < roi.rows; ++y) {
		for (int x = 0; x < roi.cols; ++x) {
			if (roi.at<uchar>(y, x) != 0) {
				N++;
				X.push_back(cv::Point2i(x, y));
			}
		}
	}

	Y.resize(N, 1);

	for (int i = 0; i < N; ++i) {
		Y.coeffRef(i, 0) = roi.at<uchar>(X[i].y, X[i].x);
	}

}

void RBF::SetPhi()
{
	Phi.resize(N, N);

	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			Phi.coeffRef(i, j) = phi(X[i], X[j]);
		}
	}
}

void RBF::SetW()
{
	W.resize(N, 1);

	Eigen::PartialPivLU<Eigen::MatrixXf> solver(Phi);

	W = solver.solve(Y);
}

int RBF::GetValue(const int x, const int y)
{
	float fValue = 0.0f;

	for (int i = 0; i < N; ++i) {
		fValue += W.coeff(i, 0) * phi(cv::Point2i(x, y), X[i]);
	}

	return (int)fValue;
}

void RBF::Exe(cv::Mat& roi)
{
	SetY(roi);
	SetPhi();
	SetW();

	for (int y = 0; y < roi.rows; ++y) {
		for (int x = 0; x < roi.cols; ++x) {
			if (roi.at<uchar>(y, x) == 0) {
				roi.at<uchar>(y, x) = GetValue(x, y);

				// îÕàÕäOèàóù
				if (roi.at<uchar>(y, x) > 255) {
					roi.at<uchar>(y, x) = 255;
				}
				else if (roi.at<uchar>(y, x) < 0) {
					roi.at<uchar>(y, x) = 0;
				}
			}
		}
	}

#if 0
	imshow("ROI(interpolated)", roi);
#endif
}