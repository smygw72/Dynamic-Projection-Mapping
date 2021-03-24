#pragma once
#include "head.h"

using namespace std;

/*-----------------------Global�ϐ��錾-----------------------------*/

cv::Mat img, gray, shade;
RBF *rbf;

std::vector<cv::Point2i*> keypointList; // keypoint�̍��W
std::vector<cv::Point2d> uvList; // uv���W
std::vector<Cluster*> clusterList;

/* �t���[���ԍ� */
int tt;

/* ����쐬�p */
std::vector<cv::Mat> frames1, frames2, frames3;

/* ���o�ɐ��������}�[�J�̐� */
int numberOfDetectedCluster;
std::vector<int> numberOfDetectedKeypointsList;

bool displayLoop;


/*------------------------------------------------------------------*/

/*--------------------����t�@�C���̑I��----------------------*/

#if 0 // ����
//	VideoCapture cap(0); // web camera
VideoCapture cap(InputVideo);
#endif

/*------------------------------------------------------------*/

/*----------------------Functions--------------------------*/

/* �e�N�X�`���̓ǂݍ��� */
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

/* �ϐ��̏����� */
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

/* �}�[�J�����̖��x��� */
void interpolateShade()
{
	/* ���x�����O�p�̕ϐ��錾 */
	cv::Mat label(img.size(), CV_32S);
	cv::Mat stats, centroids;

	/* ���x�����O���s */
	int nLabel = connectedComponentsWithStats(gray, label, stats, centroids);

	// ���x�����O���ʂ̕`��F������
	std::vector<cv::Vec3b> colors(nLabel);
	colors[0] = cv::Vec3b(0, 0, 0);
	for (int i = 1; i < nLabel; ++i) {
		colors[i] = cv::Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
	}

	// ���x�����O���ʂ̕`��
	cv::Mat Dst(img.size(), CV_8UC3);
	for (int i = 0; i < Dst.rows; ++i) {
		int *lb = label.ptr<int>(i);
		cv::Vec3b *pix = Dst.ptr<cv::Vec3b>(i);
		for (int j = 0; j < Dst.cols; ++j) {
			pix[j] = colors[lb[j]];
		}
	}

	/* RBF�N���X�ɊeROI��Mat��n���ĕ�Ԃ��� */
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = 0; i < nLabel; ++i) {

		int *param = stats.ptr<int>(i);

		int lt_x = param[cv::ConnectedComponentsTypes::CC_STAT_LEFT]; // left top��x���W
		int lt_y = param[cv::ConnectedComponentsTypes::CC_STAT_TOP]; // left top��y���W
		int height = param[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT];
		int width = param[cv::ConnectedComponentsTypes::CC_STAT_WIDTH];

		if (param[cv::ConnectedComponentsTypes::CC_STAT_AREA] > MIM_AREA_OF_KEYPOINTS && param[cv::ConnectedComponentsTypes::CC_STAT_AREA] < MAX_AREA_OF_KEYPOINTS) {

			// �}�[�J������؂蔲��
			for (int y = lt_y; y < lt_y + height; ++y) {
				for (int x = lt_x; x < lt_x + width; ++x) {
					if (gray.at<uchar>(y, x) == 255) {
						shade.at<uchar>(y, x) = 0;
					}
				}
			}

			cv::rectangle(Dst, cv::Rect(lt_x, lt_y, width, height), cv::Scalar(0, 255, 0), 2);

			cv::Mat ROI = cv::Mat(cv::Size(width, height), CV_8UC1);

			// shade�̈ꕔ��ROI�ɑ��
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
			imshow("���x��ԑO", ROI);
			waitKey(0);
#endif
			// RBF��Ԏ��s
			RBF rbf; // ���񕶂̒�����Ȃ��ƃG���[�N����
					 //rbf = new RBF;
			rbf.Exe(ROI);
			//delete rbf;
#if 0
			imshow("���x��Ԍ�", ROI);
#endif

			// RBF��Ԃ̌��ʂ���
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
#if 0 // ���x�����O���ʂ̏o��
	imshow("labels", Dst);
#endif
}

/* ���x�����O */
void label()
{
	//���׃����O����
	cv::Mat LabelImg;
	cv::Mat stats;
	cv::Mat centroids;
	int nLab = cv::connectedComponentsWithStats(gray, LabelImg, stats, centroids);

	// ���x�����O���ʂ̕`��F������
	std::vector<cv::Vec3b> colors(nLab);
	colors[0] = cv::Vec3b(0, 0, 0);
	for (int i = 1; i < nLab; ++i) {
		colors[i] = cv::Vec3b((rand() & 255), (rand() & 255), (rand() & 255));
	}

	// ���x�����O���ʂ̕`��
	cv::Mat Dst(gray.size(), CV_8UC3);
	for (int i = 0; i < Dst.rows; ++i) {
		int *lb = LabelImg.ptr<int>(i);
		cv::Vec3b *pix = Dst.ptr<cv::Vec3b>(i);
		for (int j = 0; j < Dst.cols; ++j) {
			pix[j] = colors[lb[j]];
		}
	}

	//ROI�̐ݒ�
	for (int i = 1; i < nLab; ++i) {

		int *param1 = stats.ptr<int>(i);

		int x = param1[cv::ConnectedComponentsTypes::CC_STAT_LEFT];
		int y = param1[cv::ConnectedComponentsTypes::CC_STAT_TOP];
		int height = param1[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT];
		int width = param1[cv::ConnectedComponentsTypes::CC_STAT_WIDTH];

		// ���̖ʐψȏ�̏ꍇ�d�S��o�^
		if (param1[cv::ConnectedComponentsTypes::CC_STAT_AREA] > MIM_AREA_OF_KEYPOINTS && param1[cv::ConnectedComponentsTypes::CC_STAT_AREA] < MAX_AREA_OF_KEYPOINTS) {

			double *param2 = centroids.ptr<double>(i);

			int cenX = static_cast<int>(param2[0]);
			int cenY = static_cast<int>(param2[1]);

			keypointList.emplace_back(NEW cv::Point2i(cenX, cenY));

#if 0
			cv::rectangle(Dst, cv::Rect(x, y, width, height), cv::Scalar(0, 255, 0), 2);
#endif

#if 0
			std::cout << "�ʐϒl: " << param1[cv::ConnectedComponentsTypes::CC_STAT_AREA] << endl;
#endif
		}
	}

#if 0
	imshow("Labeling", Dst);
#endif
}

/* �e�N�X�`���}�b�s���O */
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

/* ���� */
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

/* ����쐬 */
void makeVideo(vector<cv::Mat>& frames, const string name_input, const string name_output) {

	cv::Mat frame = cv::imread(name_input);
	frames.push_back(frame);

	if (tt == LastFrame) {
		videowriter(name_output, frames, LastFrame, fps);
	}
}

/* �V�F�[�f�B���O */
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

	imshow("shade(��Ԍ�)", shade);


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

#if 0 // �m�F�p
	imshow("Shading", shadeRGB_output);
#endif
}


/*----------------------------------------------------------*/


/*---------------------�R�[���o�b�N�֐�---------------------*/

static void init()
{
	/* �w�i�F */
	glClearColor(0.0, 0.0, 0.0, 1.0);

	/* �e�N�X�`���̓ǂݍ��� */
	loadTexture();

	/* �t���[���̏����ݒ� */
	tt = 0;

	displayLoop = true;
}

static void display()
{
#if 0
	stringstream ss;
	ss << tt;

	/* StartFrame�܂ł�����΂� */
	if (tt < StartFrame) {
		cout << "�t���[���ԍ�: " << tt << endl;
		cap >> img;
		++tt;
	}

	else {
#endif
		glClear(GL_COLOR_BUFFER_BIT);

		/* ������ */
		initialize();


#if 0 // ����̏ꍇ
		/* �t�@�C���̓ǂݍ��� */
		cout << "�t���[���ԍ�: " << tt << endl;
		cap >> img;
#endif
#if 1 // �Î~��̏ꍇ
		img = cv::imread(InputImage);
		cv::imshow("Input image", img);
#endif
		// Resize to size of display
		cv::resize(img, img, cv::Size(WIDTH, HEIGHT), cv::INTER_CUBIC);

		/* Image processing */
		cv::cvtColor(img, gray, CV_RGB2GRAY);// �O���[�X�P�[���ϊ�
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


		/* ���� */
		visualizeKeypoints();
		visualizeCluster();

		/* �|�C���^�̐h�݂�m���� Date:2017/7/14 */
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
			// �e�N�X�`���}�b�s���O���s���摜��ۑ�����
			flip(img, img, 0);
			cvtColor(img, img, CV_BGR2RGB);
			glDrawPixels(img.cols, img.rows, GL_RGB, GL_UNSIGNED_BYTE, img.data);
			TextureMapping();

			string ImageOutput_texture = TextureImagePath + ss.str() + ".bmp";
			WriteBitmap(ImageOutput_texture.c_str(), Width, Height);


			// 1��(�e�N�X�`���Ȃ��E���b�V������)
			glDrawPixels(img.cols, img.rows, GL_RGB, GL_UNSIGNED_BYTE, img.data);
			MeshVisualization();
			PointVisualization();

			string ImageOutput_mesh = MeshImagePath + ss.str() + ".bmp";
			WriteBitmap(ImageOutput_mesh.c_str(), Width, Height);

			string VideoOutput_mesh = MeshVideoPath;
			makeVideo(frames2, ImageOutput_mesh, VideoOutput_mesh);


			// 2��(�e�N�X�`������E���b�V���Ȃ�)
			Shading(ImageOutput_texture, tt);
			string VideoOutput_texture = TextureVideoPath;
			makeVideo(frames1, ImageOutput_texture, VideoOutput_texture);


			// 3��(�e�N�X�`������E���b�V������)
			Mat shadedImg = imread(ImageOutput_texture); // 1�ڂ̉摜�̓ǂݍ���
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


		/* �}�[�J���o�����L�^ */
		if (StartFrame == 0) {
			NumbersOfDetection.push_back(NumberOfDetection);
		}

		/* ����쐬���ăv���O�����I�� */
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
		/* q �� Q �� ESC ���^�C�v������I�� */
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


/*-------------------------main��---------------------------*/

int main(int argc, char * argv[])
{
#ifdef _DEBUG
	// ���������[�N���o�t���O
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	/* glfw������ */
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Dynamic Projection Mapping", NULL, NULL);

	/* �J�����g��ʂ̐ݒ�(�e�N�X�`����GPU�������Ɋi�[����Ă��邽��) */
	glfwMakeContextCurrent(window);

	/* ������ */
	init();

	/* �R�[���o�b�N�֐� */
	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, resize);

	/* �ˉe�ϊ�(GL��CV�̃X�P�[������v������) */
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