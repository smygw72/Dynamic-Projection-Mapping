#pragma once
#include "head.h"

using namespace std;

/*-----------------------Global変数宣言-----------------------------*/

cv::Mat img, gray, shade;
RBF *rbf;

std::vector<cv::Point2i*> keypointList; // keypointの座標
std::vector<cv::Point2d> uvList; // uv座標
std::vector<Cluster*> clusterList;

/* フレーム番号 */
int tt;

/* 動画作成用 */
std::vector<cv::Mat> frames1, frames2, frames3;

/* 検出に成功したマーカの数 */
int numberOfDetectedCluster;
std::vector<int> numberOfDetectedKeypointsList;

bool displayLoop;


/*------------------------------------------------------------------*/

/*--------------------動画ファイルの選択----------------------*/

#if 0 // 動画
//	VideoCapture cap(0); // web camera
VideoCapture cap(InputVideo);
#endif

/*------------------------------------------------------------*/

/*----------------------Functions--------------------------*/

/* テクスチャの読み込み */
bool loadTexture()
{
	cv::Mat tex = cv::imread(TexturePath);

	cv::cvtColor(tex, tex, CV_BGR2RGB);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXWIDTH, TEXHEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	return true;
}

/* 変数の初期化 */
void initialize()
{
	srand((unsigned int)time(NULL));
	keypointList.clear();
	clusterList.clear();
}

void detection()
{
	dotClusterExtraction(keypointList, clusterList, img);
	getConnectivity(clusterList, img);
	identifyDotCluster();
}


void detectFalsePositiveTracking()
{

}


void interpolateLostCluster()
{

}



void tracking()
{
	detectFalsePositiveTracking();
	interpolateLostCluster();
}

/* マーカ部分の明度補間 */
void interpolateShade()
{
	/* ラベリング用の変数宣言 */
	cv::Mat label(img.size(), CV_32S);
	cv::Mat stats, centroids;

	/* ラベリング実行 */
	int nLabel = connectedComponentsWithStats(gray, label, stats, centroids);

	// ラベリング結果の描画色を決定
	std::vector<cv::Vec3b> colors(nLabel);
	colors[0] = cv::Vec3b(0, 0, 0);
	for (int i = 1; i < nLabel; ++i) {
		colors[i] = cv::Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
	}

	// ラベリング結果の描画
	cv::Mat Dst(img.size(), CV_8UC3);
	for (int i = 0; i < Dst.rows; ++i) {
		int *lb = label.ptr<int>(i);
		cv::Vec3b *pix = Dst.ptr<cv::Vec3b>(i);
		for (int j = 0; j < Dst.cols; ++j) {
			pix[j] = colors[lb[j]];
		}
	}

	/* RBFクラスに各ROIのMatを渡して補間する */
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = 0; i < nLabel; ++i) {

		int *param = stats.ptr<int>(i);

		int lt_x = param[cv::ConnectedComponentsTypes::CC_STAT_LEFT]; // left topのx座標
		int lt_y = param[cv::ConnectedComponentsTypes::CC_STAT_TOP]; // left topのy座標
		int height = param[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT];
		int width = param[cv::ConnectedComponentsTypes::CC_STAT_WIDTH];

		if (param[cv::ConnectedComponentsTypes::CC_STAT_AREA] > MIM_AREA_OF_KEYPOINTS && param[cv::ConnectedComponentsTypes::CC_STAT_AREA] < MAX_AREA_OF_KEYPOINTS) {

			// マーカ部分を切り抜く
			for (int y = lt_y; y < lt_y + height; ++y) {
				for (int x = lt_x; x < lt_x + width; ++x) {
					if (gray.at<uchar>(y, x) == 255) {
						shade.at<uchar>(y, x) = 0;
					}
				}
			}

			cv::rectangle(Dst, cv::Rect(lt_x, lt_y, width, height), cv::Scalar(0, 255, 0), 2);

			cv::Mat ROI = cv::Mat(cv::Size(width, height), CV_8UC1);

			// shadeの一部をROIに代入
			int shade_y = 0, shade_x = 0;
			for (int y = 0; y < height; ++y) {
				shade_x = 0;
				for (int x = 0; x < width; ++x) {
					ROI.at<uchar>(y, x) = shade.at<uchar>(lt_y + shade_y, lt_x + shade_x);
					++shade_x;
				}
				++shade_y;
			}
#if 0
			imshow("明度補間前", ROI);
			waitKey(0);
#endif
			// RBF補間実行
			RBF rbf; // 並列文の中じゃないとエラー起きる
					 //rbf = new RBF;
			rbf.Exe(ROI);
			//delete rbf;
#if 0
			imshow("明度補間後", ROI);
#endif

			// RBF補間の結果を代入
			shade_y = 0, shade_x = 0;
			for (int y = lt_y; y < lt_y + height; ++y) {
				shade_x = 0;
				for (int x = lt_x; x < lt_x + width; ++x) {
					shade.at<uchar>(y, x) = ROI.at<uchar>(shade_y, shade_x);
					++shade_x;
				}
				++shade_y;
			}

		}
	}
#if 0 // ラベリング結果の出力
	imshow("labels", Dst);
#endif
}

/* ラベリング */
void label()
{
	//ラべリング処理
	cv::Mat LabelImg;
	cv::Mat stats;
	cv::Mat centroids;
	int nLab = cv::connectedComponentsWithStats(gray, LabelImg, stats, centroids);

	// ラベリング結果の描画色を決定
	std::vector<cv::Vec3b> colors(nLab);
	colors[0] = cv::Vec3b(0, 0, 0);
	for (int i = 1; i < nLab; ++i) {
		colors[i] = cv::Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
	}

	// ラベリング結果の描画
	cv::Mat Dst(gray.size(), CV_8UC3);
	for (int i = 0; i < Dst.rows; ++i) {
		int *lb = LabelImg.ptr<int>(i);
		cv::Vec3b *pix = Dst.ptr<cv::Vec3b>(i);
		for (int j = 0; j < Dst.cols; ++j) {
			pix[j] = colors[lb[j]];
		}
	}

	//ROIの設定
	for (int i = 1; i < nLab; ++i) {

		int *param1 = stats.ptr<int>(i);

		int x = param1[cv::ConnectedComponentsTypes::CC_STAT_LEFT];
		int y = param1[cv::ConnectedComponentsTypes::CC_STAT_TOP];
		int height = param1[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT];
		int width = param1[cv::ConnectedComponentsTypes::CC_STAT_WIDTH];

		// 一定の面積以上の場合重心を登録
		if (param1[cv::ConnectedComponentsTypes::CC_STAT_AREA] > MIM_AREA_OF_KEYPOINTS && param1[cv::ConnectedComponentsTypes::CC_STAT_AREA] < MAX_AREA_OF_KEYPOINTS) {

			double *param2 = centroids.ptr<double>(i);

			int cenX = static_cast<int>(param2[0]);
			int cenY = static_cast<int>(param2[1]);

			keypointList.emplace_back(NEW cv::Point2i(cenX, cenY));

#if 0
			cv::rectangle(Dst, cv::Rect(x, y, width, height), cv::Scalar(0, 255, 0), 2);
#endif

#if 0
			std::cout << "面積値: " << param1[cv::ConnectedComponentsTypes::CC_STAT_AREA] << endl;
#endif
		}
	}

#if 0
	imshow("Labeling", Dst);
#endif
}

/* テクスチャマッピング */
void mapTexture() {

	glEnable(GL_TEXTURE_2D);

	for (int i = 0; i < (wid_num - 1) * mesh_num; ++i) {

		glBegin(GL_TRIANGLES);

		for (int j = 0; j < (wid_num - 1) * mesh_num; ++j) {

			glTexCoord2f(uvList[((wid_num - 1) * mesh_num + 1) * i + j].x, uvList[((wid_num - 1) * mesh_num + 1) * i + j].y);
			glVertex3f(clusterList[((wid_num - 1) * mesh_num + 1) * i + j]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * i + j]->point.y - HEIGHT / 2), 0.0);

			glTexCoord2f(uvList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j].x, uvList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j].y);
			glVertex3f(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j]->point.y - HEIGHT / 2), 0.0);

			glTexCoord2f(uvList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)].x, uvList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)].y);
			glVertex3f(clusterList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)]->point.y - HEIGHT / 2), 0.0);


			glTexCoord2f(uvList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j].x, uvList[((wid_num - 1) * mesh_num + 1)* (i + 1) + j].y);
			glVertex3f(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j]->point.y - HEIGHT / 2), 0.0);

			glTexCoord2f(uvList[((wid_num - 1) * mesh_num + 1) * (i + 1) + (j + 1)].x, uvList[((wid_num - 1) * mesh_num + 1) * (i + 1) + (j + 1)].y);
			glVertex3f(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + (j + 1)]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + (j + 1)]->point.y - HEIGHT / 2), 0.0);

			glTexCoord2f(uvList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)].x, uvList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)].y);
			glVertex3f(clusterList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)]->point.y - HEIGHT / 2), 0.0);
		}

		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
}

/* 可視化 */
void visualizeMesh()
{
	glLineWidth(3.0);
	glBegin(GL_LINES);
	glColor3f(0.3, 0.3, 0.3);

	for (int i = 0; i < (wid_num - 1) * mesh_num; ++i) {
		for (int j = 0; j < (wid_num - 1) * mesh_num; ++j) {

			glVertex3i(clusterList[((wid_num - 1) * mesh_num + 1) * i + j]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * i + j]->point.y - HEIGHT / 2), 0);
			glVertex3i(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j]->point.y - HEIGHT / 2), 0);

			glVertex3i(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j]->point.y - HEIGHT / 2), 0);
			glVertex3i(clusterList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)]->point.y - HEIGHT / 2), 0);

			glVertex3i(clusterList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)]->point.y - HEIGHT / 2), 0);
			glVertex3i(clusterList[((wid_num - 1) * mesh_num + 1) * i + j]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * i + j]->point.y - HEIGHT / 2), 0);

			glVertex3i(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + j]->point.y - HEIGHT / 2), 0);
			glVertex3i(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + (j + 1)]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + (j + 1)]->point.y - HEIGHT / 2), 0);

			glVertex3i(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + (j + 1)]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * (i + 1) + (j + 1)]->point.y - HEIGHT / 2), 0);
			glVertex3i(clusterList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)]->point.x - WIDTH / 2, -(clusterList[((wid_num - 1) * mesh_num + 1) * i + (j + 1)]->point.y - HEIGHT / 2), 0);
		}
	}
	glEnd();
	glColor3f(1.0, 1.0, 1.0);

}
void visualizeKeypoints()
{
	glPointSize(5.0);
	glColor3f(0.0, 0.5, 0.5);
	glBegin(GL_POINTS);
	for (int i = 0; i < keypointList.size(); ++i) {
		if (keypointList[i]->x != -1) {
			glVertex3i(keypointList[i]->x - WIDTH / 2, -(keypointList[i]->y - HEIGHT / 2), 0);
		}
	}
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
}
void visualizeCluster()
{
	glPointSize(15.0);
	glBegin(GL_POINTS);
	for (int i = 0; i < clusterList.size(); ++i)
	{
		if (clusterList[i]->numberOfKeypoint == 1) 	glColor3f(1.0, 0.0, 0.0); // red
		if (clusterList[i]->numberOfKeypoint == 2) 	glColor3f(0.0, 1.0, 0.0); // green
		if (clusterList[i]->numberOfKeypoint == 3) 	glColor3f(0.0, 0.0, 1.0); // blue
		if (clusterList[i]->numberOfKeypoint == 4) 	glColor3f(1.0, 1.0, 0.0); // yellow

		glVertex3i(clusterList[i]->point.x - WIDTH / 2, -(clusterList[i]->point.y - HEIGHT / 2), 0);
	}
	glEnd();
	glColor3f(1.0, 1.0, 1.0);
}

/* 動画作成 */
void makeVideo(vector<cv::Mat>& frames, const string name_input, const string name_output) {

	cv::Mat frame = cv::imread(name_input);
	frames.push_back(frame);

	if (tt == LastFrame) {
		videowriter(name_output, frames, LastFrame, fps);
	}
}

/* シェーディング */
void shadeKeypoints(const string name, const int tt)
{
	stringstream ss;
	ss << tt;

	cv::Mat shadeRGB_input, shadeRGB_output;
	cv::Mat shadeHSV_input, shadeHSV_output;
	vector<cv::Mat> planes;

	shadeRGB_input = cv::imread(name);
	cv::cvtColor(shadeRGB_input, shadeHSV_input, CV_BGR2HSV);
	split(shadeHSV_input, planes);

	imshow("shade(補間後)", shade);


	for (int y = 0; y < shadeHSV_input.rows; ++y) {
		for (int x = 0; x < shadeHSV_input.cols; ++x) {

			if (planes[2].at<uchar>(y, x) != shade.at<uchar>(y, x)) {
				planes[2].at<uchar>(y, x) = (float)planes[2].at<uchar>(y, x) * ((float)shade.at<uchar>(y, x) / 255.0);
			}
		}
	}

	merge(planes, shadeHSV_output);

	cv::cvtColor(shadeHSV_output, shadeRGB_output, CV_HSV2BGR);

	string shadedImg = TextureImagePath + ss.str() + ".bmp";
	imwrite(shadedImg, shadeRGB_output);

#if 0 // 確認用
	imshow("Shading", shadeRGB_output);
#endif
}


/*----------------------------------------------------------*/


/*---------------------コールバック関数---------------------*/

static void init()
{
	/* 背景色 */
	glClearColor(0.0, 0.0, 0.0, 1.0);

	/* テクスチャの読み込み */
	loadTexture();

	/* フレームの初期設定 */
	tt = 0;

	displayLoop = true;
}

static void display()
{
#if 0
	stringstream ss;
	ss << tt;

	/* StartFrameまですっ飛ばし */
	if (tt < StartFrame) {
		cout << "フレーム番号: " << tt << endl;
		cap >> img;
		++tt;
	}

	else {
#endif
		glClear(GL_COLOR_BUFFER_BIT);

		/* 初期化 */
		initialize();


#if 0 // 動画の場合
		/* ファイルの読み込み */
		cout << "フレーム番号: " << tt << endl;
		cap >> img;
#endif
#if 1 // 静止画の場合
		img = cv::imread(InputImage);
		cv::imshow("Input image", img);
#endif
		// Resize to size of display
		cv::resize(img, img, cv::Size(WIDTH, HEIGHT), cv::INTER_CUBIC);

		/* Image processing */
		cv::cvtColor(img, gray, CV_RGB2GRAY);// グレースケール変換
		cv::threshold(gray, gray, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
		//imwrite(ShadeImagePath, shade);
		label();
		//ShadeInterpolation();

		/* Main algorithm */
		detection();
		tracking();

		/* openCV --> openGL */
		cv::flip(img, img, 0);
		cv::cvtColor(img, img, CV_BGR2RGB);
		glDrawPixels(img.cols, img.rows, GL_RGB, GL_UNSIGNED_BYTE, img.data);
		cv::flip(img, img, 0);
		cv::cvtColor(img, img, CV_BGR2RGB);


		/* 可視化 */
		visualizeKeypoints();
		visualizeCluster();

		/* ポインタの辛みを知った Date:2017/7/14 */
		for (vector<cv::Point2i*>::iterator it = keypointList.begin(); it != keypointList.end(); ++it) {
			delete *it;
		}
		for (vector <Cluster*>::iterator it = clusterList.begin(); it != clusterList.end(); ++it) {
			delete *it;
		}


#if 0	/* Save three images------------------------------------------------------- */
		if (StartFrame != 0 && tt > StartFrame + 1) {}
		else
		{
			// テクスチャマッピングを行い画像を保存する
			flip(img, img, 0);
			cvtColor(img, img, CV_BGR2RGB);
			glDrawPixels(img.cols, img.rows, GL_RGB, GL_UNSIGNED_BYTE, img.data);
			TextureMapping();

			string ImageOutput_texture = TextureImagePath + ss.str() + ".bmp";
			WriteBitmap(ImageOutput_texture.c_str(), Width, Height);


			// 1つ目(テクスチャなし・メッシュあり)
			glDrawPixels(img.cols, img.rows, GL_RGB, GL_UNSIGNED_BYTE, img.data);
			MeshVisualization();
			PointVisualization();

			string ImageOutput_mesh = MeshImagePath + ss.str() + ".bmp";
			WriteBitmap(ImageOutput_mesh.c_str(), Width, Height);

			string VideoOutput_mesh = MeshVideoPath;
			makeVideo(frames2, ImageOutput_mesh, VideoOutput_mesh);


			// 2つ目(テクスチャあり・メッシュなし)
			Shading(ImageOutput_texture, tt);
			string VideoOutput_texture = TextureVideoPath;
			makeVideo(frames1, ImageOutput_texture, VideoOutput_texture);


			// 3つ目(テクスチャあり・メッシュあり)
			Mat shadedImg = imread(ImageOutput_texture); // 1つ目の画像の読み込み
			flip(shadedImg, shadedImg, 0);
			cvtColor(shadedImg, shadedImg, CV_BGR2RGB);

			glDrawPixels(shadedImg.cols, shadedImg.rows, GL_RGB, GL_UNSIGNED_BYTE, shadedImg.data);
			MeshVisualization();
			PointVisualization();

			string ImageOutput_meshontexture = MeshOnTextureImagePath + ss.str() + ".bmp";
			WriteBitmap(ImageOutput_meshontexture.c_str(), Width, Height);

			string VideoOutput_meshontexture = MeshOnTextureVideoPath;
			makeVideo(frames3, ImageOutput_meshontexture, VideoOutput_meshontexture);
		}
		/* ---------------------------------------------------------------------------- */


		/* マーカ検出数を記録 */
		if (StartFrame == 0) {
			NumbersOfDetection.push_back(NumberOfDetection);
		}

		/* 動画作成してプログラム終了 */
		if (tt == LastFrame) {
			ofstream ofs("NumbersOfDetection.txt");

			int count = NumbersOfDetection.size();
			for (int i = 0; i < count; ++i) {
				ofs << NumbersOfDetection[i] << endl;
			}

			exit(0);
		}
#endif
		++tt;
#if 0
	}
#endif
	glFlush();
}

static void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key) {
		/* q か Q か ESC をタイプしたら終了 */
	case '\033':
	case 'q':
	case 'Q':
		exit(0);
		break;
	case 'B':
		displayLoop = false;
	}
}

static void resize(GLFWwindow* window, int width, int height) {

	float aspect = (float)width / (float)height;

	glViewport(0, 0, width, height);
}

/*----------------------------------------------------------*/


/*-------------------------main文---------------------------*/

int main(int argc, char * argv[])
{
#ifdef _DEBUG
	// メモリリーク検出フラグ
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	/* glfw初期化 */
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Dynamic Projection Mapping", NULL, NULL);

	/* カレント画面の設定(テクスチャはGPUメモリに格納されているため) */
	glfwMakeContextCurrent(window);

	/* 初期化 */
	init();

	/* コールバック関数 */
	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, resize);

	/* 射影変換(GLとCVのスケールを一致させる) */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-WIDTH / 2, WIDTH / 2, -HEIGHT / 2, HEIGHT / 2, -1, 1);

	while (!glfwWindowShouldClose(window)) {
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
#if 0
		cv::waitKey(0);
#endif
		if (displayLoop == false) break;
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	system("pause");
	return 0;
}

/*----------------------------------------------------------*/