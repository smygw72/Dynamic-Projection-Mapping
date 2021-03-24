#pragma once
#include "head.h"

///////////////////////////////////////////////////////////////////////////////
//         �@�@�}�[�J�����̖��x���Ԃ��邽�߂�RBF��Ԃ�p���� �@      �@    //
///////////////////////////////////////////////////////////////////////////////

using namespace std;

class RBF {

private:

	int N; // �f�[�^�̐�
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