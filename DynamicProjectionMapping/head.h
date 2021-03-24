#pragma once

#ifdef _DEBUG
#include <new>
#include <memory>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define NEW  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <vector>
#include <list>
#include <iterator>
#include <algorithm>
#include <queue>

#include <iostream>
#include <fstream>
#include <sstream>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <glfw3.h>
#include <Eigen/Dense>
#include <Fade_2D.h>

#include "detection.h"
#include "square.h"
#include "bitmap.h"
#include "rbf.h"
#include "video.h"

#pragma warning(disable:4996)

/* サイズ設定 */
const int WIDTH = 512;
const int HEIGHT = 512;

const int TEXWIDTH = 891; /* テクスチャの幅 */
const int TEXHEIGHT = 891; /* テクスチャの高さ */


/* Setting */
const int MIM_AREA_OF_KEYPOINTS = 1; // Minimum area of keypoints
const int MAX_AREA_OF_KEYPOINTS = 15; // Maximum area of keypoint
const int WIDTH_OF_GRID = WIDTH; // Width of square grid cells
const int MAX_NUMBER_OF_KEYPOINTS_IN_GRID = 500; // Maximum number of keypoint in each grid
const double MAX_DISTANCE_WITHIN_CLUSTERS = 13.0;
const double MIN_VALUE_OF_FUNCION_L_SQUARE = 0.95;
const double sita_parallel = 0.05; // Condition(1)
const double sita_vertical = 0.05; // Condition(2)
const double alpha_ROI = 0.6;
const double d_hat = 1.4;

const int numberOfGridX = (double)WIDTH / (double)WIDTH_OF_GRID;
const int numberOfGridY = (double)HEIGHT / (double)WIDTH_OF_GRID;



/* マーカー設定 */
const int wid_num = 9;
const int hei_num = 9;


/* その他設定 */
const int CLASS = 6;
const int mesh_num = 5;
const int StartFrame = 0;
const int LastFrame = 500;
const int fps = 30;


/* Path */
// 入力
const std::string InputImage = "marker.png";
const std::string InputVideo = "data/video/MymarkerOcclusion.mp4";

// テクスチャ画像
const std::string TexturePath = "data/texture/texture1.jpg";

// ディレクトリ
const std::string targetPath = "data/output/shirt/yamashin/MymarkerUp/trackingOFF";

// 結果画像
const std::string TextureImagePath = targetPath + "/Texture/";
const std::string MeshOnTextureImagePath = targetPath + "/MeshOnTexture/";
const std::string MeshImagePath = targetPath + "/Mesh/";
const std::string ShadeImagePath = targetPath + "/Shade.bmp";

const std::string TextureVideoPath = targetPath + "/Texture.mp4";
const std::string MeshOnTextureVideoPath = targetPath + "/MeshOnTexture.mp4";
const std::string MeshVideoPath = targetPath + "/Mesh.mp4";