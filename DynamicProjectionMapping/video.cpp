#include "video.h"

void videocapture(const char* videoname, std::vector<cv::Mat> &video) {

	//cv::VideoCapture cap("./input/inputvideo.MOV");//videoname);

	cv::VideoCapture cap(videoname);

	if (!cap.isOpened()) {
		exit(0);
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);


	int i = 0;
	while (1)
	{
		cv::Mat frame;
		cap >> frame;

		if (frame.empty()) break;

		video.push_back(frame.clone());
		//video[i] = frame.clone();

		i++;
	}

	cv::destroyWindow("image");

}

std::vector<cv::Mat> videocapture(const std::string videoname) {

	//cv::VideoCapture cap("./input/inputvideo.MOV");//videoname);
	std::vector<cv::Mat> video;

	cv::VideoCapture cap(videoname);

	if (!cap.isOpened()) {
		exit(0);
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);


	int i = 0;
	while (1)
	{
		cv::Mat frame;
		cap >> frame;

		if (frame.empty()) break;

		video.push_back(frame.clone());
		//video[i] = frame.clone();

		i++;
	}

	cv::destroyWindow("image");

	return video;
}


int getvideoframeNo(const char* videoname) {

	//cv::VideoCapture cap("./input/inputvideo.MOV");//videoname);

	cv::VideoCapture cap(videoname);

	if (!cap.isOpened()) {
		exit(0);
	}

	cv::namedWindow("image", cv::WINDOW_AUTOSIZE);


	std::vector<cv::Mat> video(cap.get(CV_CAP_PROP_FRAME_COUNT));

	int i = 0;
	while (1)
	{
		cv::Mat frame;
		cap >> frame;

		if (frame.empty()) break;

		video[i] = frame.clone();

		i++;
	}

	cv::destroyWindow("image");

	return video.size();
}


void videopreview(std::vector<cv::Mat> &video, int duration, double fps) {
	cv::namedWindow("video_preview");
	std::cout << "no of frames " << video.size() << std::endl;
	double spf = 1000.0 / fps;
	int i = 0;
	while (1) {
		std::cout << "i " << i << std::endl;
		cv::imshow("video_preview", video[i]);

		int c;
		c = cv::waitKey(spf);
		//cout << "aho" << i << endl;
		if (c == 27) break; //esc
		else if (c == 0x73)
		{ // 's'キー入力
			printf("'s'キーが押された\n");
			cv::waitKey(); //一時停止
		}
		//
		i++;
		if (i >= duration) i = 0;

	}
	cv::destroyWindow("video_preview");
}


void videowriter(std::string filename, std::vector<cv::Mat> &video, int duration, double fps) {

	cv::VideoWriter writer(filename + ".mp4", CV_FOURCC_MACRO('M', 'P', '4', 'V'), fps, cv::Size(video[0].cols, video[0].rows), true);

	if (!writer.isOpened()) {
		std::cout << "missed writing video!" << std::endl;
		system("pause");
		exit(0);
	}

	for (int i = 0; i < duration; i++) {
		writer << video[i];
	}
	std::cout << "succeeded writing video" << std::endl;

}
