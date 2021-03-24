#include "detection.h"

/* リストから検索をして、見つかったらインデックス返す */
bool searchElementFromList(const int element, const vector<vector<int>> list, int& index)
{
	if (list.empty()) {
		return true;
	}
	else {
		for (int i = 0; i < list.size(); ++i) {
			// 見つけた場合
			if (find(list[i].begin(), list[i].end(), element) != list[i].end()) {
				index = i;
				return false;
			}
		}
	}
	return true;
}

/* グリッドごとにKeypointを検出してクラスタリングをする */
void dotClusterExtraction(const std::vector<cv::Point2i*> _keypointList, std::vector<Cluster*>& _clusterList, cv::Mat& _img)
{
#if	1 // Gridの可視化
	for (int x = 0; x < numberOfGridX; ++x) {
		cv::line(_img, cv::Point(x*WIDTH_OF_GRID, 0), cv::Point(x*WIDTH_OF_GRID, HEIGHT), cv::Scalar(0, 0, 0), 1, CV_AA);
	}
	for (int y = 0; y < numberOfGridY; ++y) {
		cv::line(_img, cv::Point(0, y*WIDTH_OF_GRID), cv::Point(WIDTH, y*WIDTH_OF_GRID), cv::Scalar(0, 0, 0), 1, CV_AA);
	}
#endif

	std::vector<std::vector<cv::Point2i*>> keypointsInGridList;	// 各グリッド内に存在するKeyPointsのリスト
	keypointsInGridList.resize(numberOfGridY*numberOfGridX);

	/* グリッド内のKeypointsを調べる */
	for (int y = 0; y < numberOfGridY; ++y) {
		for (int x = 0; x < numberOfGridX; ++x) {

			// 初期化
			int numberOfKeypoint = 0;
			keypointsInGridList[y*numberOfGridX + x].clear();

			cv::Point2i upperLeft = cv::Point2i(x*WIDTH_OF_GRID, y*WIDTH_OF_GRID); // gridの左上の座標

			for (int i = 0; i < _keypointList.size(); ++i)
			{
				if (_keypointList[i]->x >= upperLeft.x && _keypointList[i]->y >= upperLeft.y &&
					_keypointList[i]->x <= (upperLeft.x + WIDTH_OF_GRID) && _keypointList[i]->y <= (upperLeft.y + WIDTH_OF_GRID))
				{
					numberOfKeypoint++;
					keypointsInGridList[y*numberOfGridX + x].emplace_back(_keypointList[i]);
				}

			}

			// 一定数以上であればそのgridは削除する
			if (numberOfKeypoint > MAX_NUMBER_OF_KEYPOINTS_IN_GRID) {
				for (int i = 0; i < numberOfKeypoint; ++i) {
					keypointsInGridList[y*numberOfGridX + x].clear();
				}
			}
		}
	}

#if 0 // 確認用
	for (int i = 0; i < keypointsInGridList.size(); ++i) {
		for (int j = 0; j < keypointsInGridList[i].size(); ++j) {
			cout << keypointsInGridList[i][j] << " ";
		}
		cout << endl;
	}
#endif

#if 1 // グリッドごとのKeypointsの可視化
	for (int i = 0; i < keypointsInGridList.size(); ++i) {
		cv::Scalar color(rand() & 255, rand() & 255, rand() & 255);
		for (int j = 0; j < keypointsInGridList[i].size(); ++j) {
			cv::circle(_img, cv::Point2i(keypointsInGridList[i][j]->x, keypointsInGridList[i][j]->y), 5, color, -1, CV_AA);
		}
	}
#endif

	/* クラスタの決定 */
	// i番目のグリッドについて
	for (int i = 0; i < keypointsInGridList.size(); ++i) {
#if 0	
		cout << i << "番目のGridのクラスタ--------------" << endl;
#endif
		vector<vector<int>> indexOfKeypointListInCluster; // 各クラスタに所属するkeypointのインデックス

		for (int j = 0; j < keypointsInGridList[i].size(); ++j) {

			int index = 0;
			bool resize = searchElementFromList(j, indexOfKeypointListInCluster, index);

			// クラスタに所属していない場合
			if (resize) {
				indexOfKeypointListInCluster.resize(indexOfKeypointListInCluster.size() + 1);
				indexOfKeypointListInCluster.back().push_back(j);
#if 0
				cout << indexOfKeypointListInCluster.size() - 1 << "ベクトルを追加しました " << endl;
				cout << endl;
#endif
			}

			if (j != keypointsInGridList[i].size() - 1) {
				for (int k = j + 1; k < keypointsInGridList[i].size(); ++k)
				{
#if 0
					cout << "[" << j << "番目と" << k << "番目のkeypointについて]" << endl;
#endif					
					if (!resize && find(indexOfKeypointListInCluster[index].begin(), indexOfKeypointListInCluster[index].end(), k) != indexOfKeypointListInCluster[index].end()) {
#if 0
						cout << k << "番目のkeypointはすでに" << index << "番目のクラスタに所属されています" << endl;
						cout << endl;
#endif
						continue;
					}

					double dis = sqrt(pow(keypointsInGridList[i][j]->x - keypointsInGridList[i][k]->x, 2) + pow(keypointsInGridList[i][j]->y - keypointsInGridList[i][k]->y, 2));

					if (dis < MAX_DISTANCE_WITHIN_CLUSTERS)
					{
						if (resize) {
							indexOfKeypointListInCluster.back().push_back(k);
						}
						else {
							indexOfKeypointListInCluster[index].push_back(k);
						}
					}

#if 0				
					for (int l = 0; l < indexOfKeypointListInCluster.size(); ++l) {
						cout << l << "クラスタ: ";
						for (int m = 0; m < indexOfKeypointListInCluster[l].size(); ++m) {
							cout << indexOfKeypointListInCluster[l][m] << " ";
						}
						cout << endl;
					}
					cout << endl;
#endif
				}
			}
		}

		/* clusterListに追加する */
		// i番目のグリッドのj番目のクラスタについて
		for (int j = 0; j < indexOfKeypointListInCluster.size(); ++j)
		{
			cv::Point2d centroid = cv::Point2d(0.0, 0.0); // 重心

			const int numberOfKeypoint = indexOfKeypointListInCluster[j].size(); // 1~4をとりうる

			for (int k = 0; k < numberOfKeypoint; ++k) {
				centroid.x += keypointsInGridList[i][indexOfKeypointListInCluster[j][k]]->x;
				centroid.y += keypointsInGridList[i][indexOfKeypointListInCluster[j][k]]->y;
			}
			centroid.x = centroid.x / numberOfKeypoint;
			centroid.y = centroid.y / numberOfKeypoint;

			Cluster* cluster = NEW Cluster(cv::Point2i(centroid), numberOfKeypoint);
			_clusterList.emplace_back(cluster);
		}
	}

#if 1 // 確認
	for (int i = 0; i < _clusterList.size(); ++i) {
		std::cout << i << "番目のクラスタ: " << _clusterList[i]->point << ", " << _clusterList[i]->numberOfKeypoint << std::endl;
	}
#endif
}

void getConnectivity(const std::vector<Cluster*> _clusterList, cv::Mat& _img)
{
	/* Delaunay Triangulation */
	vector<fade::Point2> pointsList;
	for (int i = 0; i < _clusterList.size(); ++i) {
		fade::Point2 point = fade::Point2(_clusterList[i]->point.x, _clusterList[i]->point.y);
		point.setCustomIndex(i);
		pointsList.emplace_back(point);

#if 1 // 確認用の描画
		cv::putText(_img, to_string(i), cv::Point2i(point.x() + 10, point.y()), CV_FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0));
#endif
	}

	// Initialize Fade 2D
	fade::Fade_2D dt((uint)pointsList.size());
	vector<fade::Point2*> dtVertexsList;
	dt.insert(pointsList, dtVertexsList);

	// Draw points
	for (auto vertexPtr = pointsList.begin(); vertexPtr != pointsList.end(); ++vertexPtr) {
		cv::circle(_img, cv::Point(vertexPtr->x(), vertexPtr->y()), 4, cv::Scalar(0, 0, 255), -1);
	}

	vector<fade::Triangle2*> dtTrianglesList;
	dt.getTrianglePointers(dtTrianglesList);

	// Square class
	vector<Square*> allSquareList;
	vector<pair<int, int>> indexOfEdgeList;

	// Make Square class and draw lines
	for (auto trianglePtr = dtTrianglesList.begin(); trianglePtr != dtTrianglesList.end(); ++trianglePtr)
	{
		fade::Point2 *pt[3] = { (*trianglePtr)->getCorner(0), (*trianglePtr)->getCorner(1), (*trianglePtr)->getCorner(2) };

		uint idOfTriangle[3];
		for (int i = 0; i < 3; ++i) {
			idOfTriangle[i] = pt[i]->getCustomIndex();
		}
#if 1
		fade::Vector2 vec1, vec2;
		vec1 = fade::Vector2(_clusterList[idOfTriangle[1]]->point.x - _clusterList[idOfTriangle[0]]->point.x, _clusterList[idOfTriangle[1]]->point.y - _clusterList[idOfTriangle[0]]->point.y);
		vec2 = fade::Vector2(_clusterList[idOfTriangle[2]]->point.x - _clusterList[idOfTriangle[0]]->point.x, _clusterList[idOfTriangle[2]]->point.y - _clusterList[idOfTriangle[0]]->point.y);
		double product = (vec1.x()*vec2.x() + vec1.y()*vec2.y()) / (vec1.length()*vec2.length());
		if (abs(product) > 0.95) {
			continue;
		}
#endif
		// Draw lines
		cv::Point2i point[3];
		for (int i = 0; i < 3; ++i) {
			point[i] = _clusterList[idOfTriangle[i]]->point;
		}
		cv::Scalar randomColor = cv::Scalar(rand() % 255, rand() % 255, rand() % 255);
		cv::line(_img, point[0], point[1], randomColor, 1, CV_AA);
		cv::line(_img, point[0], point[2], randomColor, 1, CV_AA);
		cv::line(_img, point[1], point[2], randomColor, 1, CV_AA);

		/* Make Square class */
		for (int i = 0; i < 3; ++i) {

			// i番目のvertex以外がedgeとなる場合
			std::pair<int, int> pair, pairInverse;
			if (i == 0) pair = make_pair(idOfTriangle[1], idOfTriangle[2]);
			if (i == 1) pair = make_pair(idOfTriangle[0], idOfTriangle[2]);
			if (i == 2) pair = make_pair(idOfTriangle[0], idOfTriangle[1]);
			if (i == 0) pairInverse = make_pair(idOfTriangle[2], idOfTriangle[1]);
			if (i == 1) pairInverse = make_pair(idOfTriangle[2], idOfTriangle[0]);
			if (i == 2) pairInverse = make_pair(idOfTriangle[1], idOfTriangle[0]);

			// エッジ(頂点2つのインデックスの組み合わせ)がリストにない場合
			if (find(indexOfEdgeList.begin(), indexOfEdgeList.end(), pair) == indexOfEdgeList.end()
				&& find(indexOfEdgeList.begin(), indexOfEdgeList.end(), pairInverse) == indexOfEdgeList.end())
			{
				fade::Triangle2* pNeigT((*trianglePtr)->getOppositeTriangle(i));

				if (pNeigT == NULL) {
					continue; // No adjacent triangle at this edge
				}
				else {
					uint idOfSquare[4]; // 四角形の頂点のindex
					if (i == 0) idOfSquare[0] = idOfTriangle[1], idOfSquare[1] = idOfTriangle[2], idOfSquare[2] = idOfTriangle[0];
					if (i == 1) idOfSquare[0] = idOfTriangle[0], idOfSquare[1] = idOfTriangle[2], idOfSquare[2] = idOfTriangle[1];
					if (i == 2) idOfSquare[0] = idOfTriangle[0], idOfSquare[1] = idOfTriangle[1], idOfSquare[2] = idOfTriangle[2];

					uint idOfNeighborTriangle[3];
					fade::Point2 *point_dash[3] = { (pNeigT)->getCorner(0), (pNeigT)->getCorner(1), (pNeigT)->getCorner(2) };

					for (int i = 0; i < 3; ++i) {
						idOfNeighborTriangle[i] = point_dash[i]->getCustomIndex();
					}
					for (int i = 0; i < 3; ++i) {
						for (int j = 0; j < 3; ++j) {
							if (idOfNeighborTriangle[i] == idOfTriangle[j])	break;
							idOfSquare[3] = idOfNeighborTriangle[i];
						}
					}

					std::cout << "エッジ(" << pair.first << ", " << pair.second << "): " << idOfTriangle[0] << " " << idOfTriangle[1] << " " << idOfTriangle[2] << " " << idOfNeighborTriangle[0] << " " << idOfNeighborTriangle[1] << " " << idOfNeighborTriangle[2] << endl;
					
					allSquareList.emplace_back(NEW Square(_clusterList, idOfSquare));
					
					indexOfEdgeList.emplace_back(pair);
				}
			}
			// 既にSquareクラスが作られていた場合
			else {
				continue;
			}
		}
	}

#if 0
	cout << indexOfEdgeList.size() << endl;
	for (int i = 0; i < indexOfEdgeList.size(); ++i) {
		if (indexOfEdgeList[i].first > indexOfEdgeList[i].second) {
			uint buf = indexOfEdgeList[i].first;
			indexOfEdgeList[i].first = indexOfEdgeList[i].second;
			indexOfEdgeList[i].second = buf;
		}
	}
	sort(indexOfEdgeList.begin(), indexOfEdgeList.end());
	for (int i = 0; i < indexOfEdgeList.size(); ++i) {
		cout << indexOfEdgeList[i].first << " " << indexOfEdgeList[i].second << endl;
	}
#endif

	/* Obtain the connectivity among dot clusters based BFS */
	// Prepare and sort {S(n)}
	std::vector<Square*> partSquareList;

	for (int i = 0; i < allSquareList.size(); ++i)
	{
		allSquareList[i]->caluculateLsq();
		double lsq = allSquareList[i]->getLsq();

		if (lsq > MIN_VALUE_OF_FUNCION_L_SQUARE) {
			partSquareList.emplace_back((allSquareList[i]));
		}
	}
	std::sort(allSquareList.begin(), allSquareList.end());


	for (int i = 0; i < allSquareList.size(); ++i) {
		delete allSquareList[i];
	}
}

void identifyDotCluster()
{

}

