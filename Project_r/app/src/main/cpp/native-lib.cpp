#include <jni.h>
#include <string>
#include<queue>
#include<stack>
#include "opencv2/opencv.hpp"
#include <android/log.h>
using namespace std;
using namespace cv;
#define smallsquare 18
#define BFSJUMP 10
#define THRESHOLD 15
#define lim 123
#define gapp 1

extern "C" {

vector<vector<vector<vector<Point>>>> contours;      // 0~1
vector<vector<vector<pair<Point, Point>>>> squares;
vector<vector<vector<pair<Point, Point>>>> squares2;
vector<vector<vector<pair<Point, Point>>>> squares3;
vector<vector<vector<pair<Point, Point>>>> squares4;
vector<vector<pair<Point, Point>>> canErase;
vector<Mat> origin;
vector<Mat> gray_images;
vector<bool> isErased;
Mat second_img;
Mat third_img;
Mat contourImage;
Mat patch_res; // 기준 사진의 사각형들을 대해하는 patch들의 집합 // 생성자로 크기 조절해야함
int standard = 4;      // 기준 사진이 될 사진 번호


// p1 p2로 구성된 사각형 a와 p3 p4로 구성된 사각형 b가 교차하는지 구함
bool isIntersect(Point p1, Point p2, Point p3, Point p4, int gap) {
    int leftx = max(p2.x - gap, p4.x - gap);
    int rightx = min(p1.x + gap, p3.x + gap);
    int downy = max(p2.y - gap, p4.y - gap);
    int upy = min(p1.y + gap, p3.y + gap);
    if (leftx < rightx && downy < upy) return true;
    else return false;
}

void bouge(int object, int standard) {
    Mat srcImage1;
    cvtColor(origin[object], srcImage1, CV_RGB2GRAY);
    Mat srcImage2;
    cvtColor(origin[standard], srcImage2, CV_RGB2GRAY);

    vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;


    //OpenCV
    Ptr<ORB>orbF = ORB::create(1000);
    orbF->detectAndCompute(srcImage1, noArray(), keypoints1, descriptors1);
    orbF->detectAndCompute(srcImage2, noArray(), keypoints2, descriptors2);

    //Step 3 : Matching descriptor vectors
    vector<DMatch> matches;
    BFMatcher matcher(NORM_HAMMING);
    matcher.match(descriptors1, descriptors2, matches);


    if (matches.size() < 4) {
        cout << "no!!!!!!!!!!!!!!!!\n";
        return;
    }

    //find goodMatches such that matches[i].distance <= 4 * minDist
    double minDist, maxDist;
    minDist = maxDist = matches[0].distance;
    for (int i = 1; i < matches.size(); i++) {
        double dist = matches[i].distance;
        if (dist < minDist) minDist = dist;
        if (dist > maxDist) maxDist = dist;
    }

    vector<DMatch> goodMatches;
    double fTh = 4 * minDist;
    for (int i = 0; i < matches.size(); i++) {
        if (matches[i].distance <= max(fTh, 0.02))
            goodMatches.push_back(matches[i]);
    }

    if (goodMatches.size() < 4) {
        cout << "fuck!!!!!!!!!!!!!!!!!1\n";
        return;
    }

    //find Homography between keypoints1 and keypoints2
    vector<Point2f> obj;
    vector<Point2f> scene;

    for (int i = 0; i < goodMatches.size(); i++) {
        //Get the keypoints from the good matches
        obj.push_back(keypoints1[goodMatches[i].queryIdx].pt);
        scene.push_back(keypoints2[goodMatches[i].trainIdx].pt);
    }
    Mat H = findHomography(obj, scene, RANSAC);
    Mat RH = H.inv();

    Mat tem(origin[standard].rows, origin[standard].cols, origin[standard].type(), Scalar(0, 0, 0));

    vector<Point2f> before, after;
    for (int i = 0; i < origin[object].cols; i++) {
        for (int j = 0; j < origin[object].rows; j++) {
            before.push_back(Point2f(i, j));
        }
    }
    perspectiveTransform(before, after, RH);
    for (int k = 0; k < after.size(); ++k) {
        int x = (int)after[k].x;
        int y = (int)after[k].y;
        if (x < 0 || y < 0 || x >= origin[object].cols || y >= origin[object].rows) continue;
        else {
            int i = k / origin[object].rows;
            int j = k % origin[object].rows;
            tem.at<Vec3b>(j, i) = origin[object].at<Vec3b>(y, x);
        }
    }

    tem.copyTo(origin[object]);
}

int find(int u, vector<int>& parent) {
    if (parent[u] == u) return u;
    else return parent[u] = find(parent[u], parent);
}

void merge(int u, int v, vector<int>& parent) {
    u = find(u, parent);
    v = find(v, parent);
    if (u == v) return;
    parent[u] = v;
}

JNIEXPORT void JNICALL
Java_org_androidtown_project_1r_TouchActivity_returnArea(JNIEnv *env, jobject instance,
                                                         jstring path_, int idx, jlong addrInputImage) {

    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "입장");
    const char *path = env->GetStringUTFChars(path_, 0);

    origin.clear();
    gray_images.clear();
    standard = idx;

    Mat &img_input = *(Mat *)addrInputImage;

    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "Mat 연결완료");



    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "img_input 연결 완료");

    /*
    origin.push_back(imread("h1.jpg"));
    origin.push_back(imread("h2.jpg"));
    origin.push_back(imread("h3.jpg"));
    origin.push_back(imread("h4.jpg"));
    origin.push_back(imread("h5.jpg"));
    */
    char img[50];
    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "path : %s", path);
    strcpy(img, path);
    strcat(img, "/img0.jpg");
    origin.push_back(imread(img));
    strcpy(img, path);
    strcat(img, "/img1.jpg");
    origin.push_back(imread(img));
    strcpy(img, path);
    strcat(img, "/img2.jpg");
    origin.push_back(imread(img));
    strcpy(img, path);
    strcat(img, "/img3.jpg");
    origin.push_back(imread(img));
    strcpy(img, path);
    strcat(img, "/img4.jpg");
    origin.push_back(imread(img));

    Mat _temp(origin[0].rows, origin[0].cols, origin[0].type());
    patch_res = _temp;
    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "origin에 push_back 완료");
    //img_input = origin[0];

    for (int i = 0; i<5; ++i)
        cvtColor(origin[i], origin[i], CV_BGR2RGB);

    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "RGB 토글 완료");
    for (int i = 0; i < origin.size(); i++) {
        if (i == standard) continue;
        bouge(i, standard);
    }


    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "보정 완료");
    for (int i = 0; i < origin.size(); i++) {
        Mat tem;
        cvtColor(origin[i], tem, CV_RGB2GRAY);
        Mat tem2;
        //blur(tem, tem2, Size(5, 5));
        //blur(tem2, tem, Size(5, 5));
        gray_images.push_back(tem);
    }

    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "블러링 완료");

    contours.clear();
    contours.resize(origin.size());
    for (int i = 0; i < contours.size(); i++)
        contours[i].resize(origin.size());

    Mat tem(gray_images[0].rows, gray_images[0].cols, gray_images[0].type());
    for (int a = 0; a < gray_images.size(); a++) {   // 각 사진들의 contour를 구함
        for (int b = a + 1; b < gray_images.size(); b++) {

            for (int i = 0; i < tem.rows; i++) {
                for (int j = 0; j < tem.cols; j++) {
                    int dist = (int)gray_images[a].at<uchar>(i, j) - (int)gray_images[b].at<uchar>(i, j);
                    for (int x = -2; x <= 2; x++) {
                        for (int y = -2; y <= 2; y++) {
                            if (i + x < 0 || i + x >= tem.rows || j + y < 0 || j + y >= tem.cols) continue;
                            if (origin[a].at<Vec3b>(i + x, j + y)[0] == 0 && origin[a].at<Vec3b>(i + x, j + y)[1] == 0 && origin[a].at<Vec3b>(i + x, j + y)[2] == 0) {
                                dist = 0;
                            }
                            if (origin[b].at<Vec3b>(i + x, j + y)[0] == 0 && origin[b].at<Vec3b>(i + x, j + y)[1] == 0 && origin[b].at<Vec3b>(i + x, j + y)[2] == 0) {
                                dist = 0;
                            }
                        }
                    }
                    if (dist < 0) dist *= -1;
                    if (dist < THRESHOLD) dist = 0;
                    tem.at<uchar>(i, j) = dist;
                }
            }

            erode(tem, tem, Mat());
            erode(tem, tem, Mat());
            dilate(tem, tem, Mat());
            dilate(tem, tem, Mat());
            dilate(tem, tem, Mat());
            dilate(tem, tem, Mat());
            dilate(tem, tem, Mat());
            blur(tem, tem, Size(3, 3));

            findContours(tem,
                         contours[a][b],    // 외곽선 벡터
                         CV_RETR_EXTERNAL,  // 외부 외곽선 검색
                         CV_CHAIN_APPROX_NONE); // 각 외곽선의 모든 화소 탐색

            // 컴포넌트의 둘레에 대한 최소와 최대값 사용
            // 외곽선 벡터를 반복으로 조회하면서 잘못된 컴포넌트를 제거
            // 너무 짧거나 너무 긴 외곽선 제거
            int cmin = 200;  // 최소 외곽선 길이
            int cmax = 987654321; // 최대 외곽선 길이
            vector<vector<Point> >::const_iterator itc = contours[a][b].begin();
            while (itc != contours[a][b].end()) {
                if (itc->size() < cmin || itc->size() > cmax)
                    itc = contours[a][b].erase(itc);
                else
                    ++itc;
            }

        }
    }




    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "모든 사진의 차이 구하여 외곽선 추출");
    squares.clear();
    squares.resize(origin.size());
    for (int i = 0; i < squares.size(); i++)
        squares[i].resize(squares.size());

    for (int i = 0; i < squares.size(); i++) {
        for (int j = i + 1; j < squares.size(); j++) {
            vector<int> eraseind;
            for (int k = 0; k < contours[i][j].size(); k++) {
                int maxx = -100000000, minx = 100000000, maxy = -100000000, miny = 100000000;
                for (int z = 0; z < contours[i][j][k].size(); z++) {
                    if (contours[i][j][k][z].x > maxx)
                        maxx = contours[i][j][k][z].x;
                    if (contours[i][j][k][z].x < minx)
                        minx = contours[i][j][k][z].x;
                    if (contours[i][j][k][z].y > maxy)
                        maxy = contours[i][j][k][z].y;
                    if (contours[i][j][k][z].y < miny)
                        miny = contours[i][j][k][z].y;
                }
                if (maxx - minx < 6 || maxy - miny < 6) {
                    eraseind.push_back(k);
                    //contours[i][j].erase(contours[i][j].begin() + k);
                    continue;
                }
                squares[i][j].push_back(make_pair(Point(maxx, maxy), Point(minx, miny)));
            }
            for (int k = eraseind.size() - 1; k >= 0; k--) {
                int x = eraseind[k];
                contours[i][j].erase(contours[i][j].begin() + x);
            }
        }
    }

    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "사각형 합치기 작업1차");
    canErase.clear();
    squares2.clear();
    squares3.clear();
    squares4.clear();

    canErase.resize(origin.size());
    squares2.resize(origin.size());
    squares3.resize(origin.size());
    squares4.resize(origin.size());
    for (int i = 0; i < origin.size(); i++) {
        squares2[i].resize(origin.size());
        squares3[i].resize(origin.size());
        squares4[i].resize(origin.size());
    }

    for (int a = 0; a < squares.size(); a++) {
        for (int b = a + 1; b < squares.size(); b++) {
            int i = a;
            int j = b % squares.size();
            if (i > j) swap(i, j);
            vector<int> parent(squares[i][j].size());
            for (int x = 0; x < squares[i][j].size(); x++)
                parent[x] = x;
            vector<bool> visited(squares[i][j].size());
            for (int x = 0; x < squares[i][j].size(); x++) {
                for (int y = x + 1; y < squares[i][j].size(); y++) {
                    if (isIntersect(squares[i][j][x].first, squares[i][j][x].second, squares[i][j][y].first, squares[i][j][y].second, 5)) {
                        merge(x, y, parent);
                    }
                }
            }
            for (int x = 0; x < squares[i][j].size(); x++)
                parent[x] = find(x, parent);

            for (int x = 0; x < squares[i][j].size(); x++) {
                if (visited[parent[x]]) continue;
                visited[parent[x]] = true;
                int minx = squares[i][j][x].first.x;
                int miny = squares[i][j][x].first.y;
                int maxx = squares[i][j][x].second.x;
                int maxy = squares[i][j][x].second.y;
                for (int y = x + 1; y < squares[i][j].size(); y++) {
                    if (parent[y] != parent[x]) continue;
                    minx = max(minx, squares[i][j][y].first.x);
                    miny = max(miny, squares[i][j][y].first.y);
                    maxx = min(maxx, squares[i][j][y].second.x);
                    maxy = min(maxy, squares[i][j][y].second.y);
                }
                squares2[i][j].push_back(make_pair(Point(minx, miny), Point(maxx, maxy)));
            }
        }
    }

    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "사각형 합치기 작업2차");
    for (int a = 0; a < squares2.size(); a++) {
        for (int b = a + 1; b < squares2.size(); b++) {
            int i = a;
            int j = b % squares2.size();
            if (i > j) swap(i, j);
            vector<int> parent(squares2[i][j].size());
            for (int x = 0; x < squares2[i][j].size(); x++)
                parent[x] = x;
            vector<bool> visited(squares2[i][j].size());
            for (int x = 0; x < squares2[i][j].size(); x++) {
                for (int y = x + 1; y < squares2[i][j].size(); y++) {
                    if (isIntersect(squares2[i][j][x].first, squares2[i][j][x].second, squares2[i][j][y].first, squares2[i][j][y].second, 5)) {
                        merge(x, y, parent);
                    }
                }
            }
            for (int x = 0; x < squares2[i][j].size(); x++)
                parent[x] = find(x, parent);

            for (int x = 0; x < squares2[i][j].size(); x++) {
                if (visited[parent[x]]) continue;
                visited[parent[x]] = true;
                int minx = squares2[i][j][x].first.x;
                int miny = squares2[i][j][x].first.y;
                int maxx = squares2[i][j][x].second.x;
                int maxy = squares2[i][j][x].second.y;
                for (int y = x + 1; y < squares2[i][j].size(); y++) {
                    if (parent[y] != parent[x]) continue;
                    minx = max(minx, squares2[i][j][y].first.x);
                    miny = max(miny, squares2[i][j][y].first.y);
                    maxx = min(maxx, squares2[i][j][y].second.x);
                    maxy = min(maxy, squares2[i][j][y].second.y);
                }
                squares3[i][j].push_back(make_pair(Point(minx, miny), Point(maxx, maxy)));
            }

        }
    }
    squares2.clear();

    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "사각형 합치기 작업3차");
    for (int a = 0; a < squares3.size(); a++) {
        for (int b = a + 1; b < squares3.size(); b++) {
            int i = a;
            int j = b % squares3.size();
            if (i > j) swap(i, j);
            vector<int> parent(squares3[i][j].size());
            for (int x = 0; x < squares3[i][j].size(); x++)
                parent[x] = x;
            vector<bool> visited;
            visited.resize(squares3[i][j].size());
            for (int x = 0; x < squares3[i][j].size(); x++) {
                for (int y = x + 1; y < squares3[i][j].size(); y++) {
                    if (isIntersect(squares3[i][j][x].first, squares3[i][j][x].second, squares3[i][j][y].first, squares3[i][j][y].second, 5)) {
                        merge(x, y, parent);
                    }
                }
            }
            for (int x = 0; x < squares3[i][j].size(); x++)
                parent[x] = find(x, parent);

            for (int x = 0; x < squares3[i][j].size(); x++) {
                if (visited[parent[x]]) continue;
                visited[parent[x]] = true;
                int minx = squares3[i][j][x].first.x;
                int miny = squares3[i][j][x].first.y;
                int maxx = squares3[i][j][x].second.x;
                int maxy = squares3[i][j][x].second.y;
                for (int y = x + 1; y < squares3[i][j].size(); y++) {
                    if (parent[y] != parent[x]) continue;
                    minx = max(minx, squares3[i][j][y].first.x);
                    miny = max(miny, squares3[i][j][y].first.y);
                    maxx = min(maxx, squares3[i][j][y].second.x);
                    maxy = min(maxy, squares3[i][j][y].second.y);
                }
                squares4[i][j].push_back(make_pair(Point(minx, miny), Point(maxx, maxy)));
            }
        }
    }
    squares3.clear();

    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "사각형 합치기 작업4차");

    for (int a = 0; a < squares4.size(); a++) {
        vector<pair<Point, Point>> tem;
        vector<pair<Point, Point>> tem2;
        int i = a;
        int j = (a + 1) % origin.size();
        if (i > j) swap(i, j);

        for (int x = 0; x < squares4[i][j].size(); x++) {
            Mat xxx;
            origin[a].copyTo(xxx);
            tem.push_back(squares4[i][j][x]);
        }

        for (int c = a + 2; c < a + squares4[a].size(); c++) {
            for (int x = 0; x < tem.size(); x++) {
                int ii = a;
                int jj = c % origin.size();
                if (ii > jj) swap(ii, jj);
                for (int k = 0; k < squares4[ii][jj].size(); k++) {
                    if (isIntersect(squares4[ii][jj][k].first, squares4[ii][jj][k].second, tem[x].first, tem[x].second, 5)) {
                        tem2.push_back(make_pair(Point(min(squares4[ii][jj][k].first.x, tem[x].first.x), min(squares4[ii][jj][k].first.y, tem[x].first.y)), Point(max(squares4[ii][jj][k].second.x, tem[x].second.x), max(squares4[ii][jj][k].second.y, tem[x].second.y))));
                    }
                }
            }
            tem.clear();
            for (int i = 0; i < tem2.size(); i++)
                tem.push_back(tem2[i]);
            tem2.clear();
        }
        for (int i = 0; i < tem.size(); i++)
            canErase[a].push_back(tem[i]);
    }

    isErased.clear();
    origin[standard].copyTo(second_img);
    //origin[standard].copyTo(contourImage);


    isErased.resize(canErase[standard].size(), false);
    //for (int i = 0; i < canErase[standard].size(); i++)
    //    rectangle(second_img, canErase[standard][i].first, canErase[standard][i].second, Scalar(255, 0, 0));

    vector<vector<Point>> vtt;
    vtt.resize(1);
    vector<bool> checks(origin.size(), false);

    for (int a = 0; a < origin.size(); a++) {
        int i = standard;
        int j = a;
        if (i == j) continue;
        if (i > j) swap(i, j);
        for (int x = 0; x < squares[i][j].size(); x++) {
            for (int y = 0; y < canErase[standard].size(); y++) {
                if (checks[y]) continue;
                if (isIntersect(canErase[standard][y].first, canErase[standard][y].second, squares[i][j][x].first, squares[i][j][x].second, 0)) {
                    if (canErase[standard][y].first.x + 30 >= squares[i][j][x].first.x
                        && canErase[standard][y].first.y + 30 >= squares[i][j][x].first.y
                        && canErase[standard][y].second.x - 30 <= squares[i][j][x].second.x
                        && canErase[standard][y].second.y - 30 <= squares[i][j][x].second.y) {
                        drawContours(second_img, contours[i][j],
                                     x,       // 모든 외곽선 그리기
                                     Scalar(255, 0, 0), // 하얗게
                                     1);       // 두께
                        printf("y : %d\n", y);
                        checks[y] = true;
                        break;
                    }
                }
            }
        }
    }

    img_input = second_img;
    second_img.copyTo(third_img);











    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "교집합 완료");
    vector<pair<Point, Point> > square_idx;
    for (int idx = 0; idx < canErase[standard].size(); idx++) {
        int x1 = canErase[standard][idx].second.x;   // 사각형의 작은x좌표
        int x2 = canErase[standard][idx].first.x;      // 사각형의 큰 x좌표
        int y1 = canErase[standard][idx].second.y;   // 사각형의 작은 y좌표
        int y2 = canErase[standard][idx].first.y;      // 사각형의 큰 y좌표
        int copyind = 0;                     // 조각에서 붙여넣을 index


        for (int j = x1 - 30; j < x2 + 30; j += smallsquare) {   // 열
            for (int i = y1 - 30; i < y2 + 30; i += smallsquare) {   // 행
                int mindist = 987654321;
                int xoff = 0;
                int yoff = 0;
                for (int x = -gapp; x <= gapp; x++){
                    for (int y = -gapp; y <= gapp; y++) {
                        for (int k = 0; k < origin.size(); k++) {
                            if (k == standard) continue;
                            // k번째 사진에서 기준사진에 붙일수 있나 판단하는 부분
                            bool avail = true;
                            for (int aa = 0; aa < canErase[k].size(); aa++) {
                                if (isIntersect(Point(j + smallsquare, i + smallsquare), Point(j, i), canErase[k][aa].first, canErase[k][aa].second, 0))
                                    avail = false;
                                if (!avail) break;
                            }
                            if (!avail) continue;
                            // k번째 사진에서 기준사진에 붙일수 있나 판단하는 부분
                            int dist = 0, temp, cnt = 0;
                            for (int ii = i; ii < i + smallsquare; ii++) {
                                for (int z = 0; z < 3; z++) {
                                    temp = abs(origin[standard].at<Vec3b>(ii, j - 1)[z] - origin[k].at<Vec3b>(ii + x, j + y)[z]);
                                    if (temp > lim) cnt++;
                                    dist += temp;
                                }
                            }
                            for (int jj = j; jj < j + smallsquare; jj++) {
                                for (int z = 0; z < 3; z++) {
                                    temp = abs(origin[standard].at<Vec3b>(i - 1, jj)[z] - origin[k].at<Vec3b>(i + x, jj + y)[z]);
                                    if (temp > lim) cnt++;
                                    dist += temp;
                                }
                            }
                            if (cnt <= smallsquare / 2 && dist < mindist) {
                                mindist = dist;
                                copyind = k;
                                xoff = x;
                                yoff = y;
                            }
                            else{
                                xoff = x;
                                yoff = y;
                                copyind = k;
                            }
                        }
                    }
                }

                // 붙여넣는 부분
                for (int ii = i; ii < i + smallsquare; ii++) {
                    if (ii >= origin[0].rows) break;
                    for (int jj = j; jj < j + smallsquare; jj++) {
                        if (jj >= origin[0].cols) break;
                        if (origin[copyind].at<Vec3b>(ii + xoff, jj + yoff)[0] == 0
                            && origin[copyind].at<Vec3b>(ii + xoff, jj + yoff)[1] == 0
                            && origin[copyind].at<Vec3b>(ii + xoff, jj + yoff)[2] == 0)
                            continue;
                        for (int z = 0; z < 3; z++) {
                            if (ii + xoff >= origin[0].rows || jj + yoff >= origin[0].cols) break;
                            patch_res.at<Vec3b>(ii, jj)[z] = origin[copyind].at<Vec3b>(ii + xoff, jj + yoff)[z];
                        }
                    }
                }
            }
        }
    }


    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "영역 채우기 완료");
}

stack<pair<pair<Point, Point>, int> > stk;
vector<pair<Point, Point> > res_square; // 기준 사진에 존재하는 사각형들 집합 // resize해야함

//Mat first, second, third; // 참조자로 바꾸고 대응하는 것을 연결시켜 줘야함


JNIEXPORT jint JNICALL
Java_org_androidtown_project_1r_TouchActivity_checkSquare(JNIEnv *env, jobject instance,
                                                          int x, int y) {
    x = x*origin[0].rows / 2560;
    y = y*origin[0].cols / 1432;

    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "x : %d", x);
    __android_log_print(ANDROID_LOG_DEBUG, "TAG", "y : %d", y);
    int res_idx = -1;
    for (int i = 0; i<canErase[standard].size(); ++i){
        if (canErase[standard][i].second.x <= y&& y <= canErase[standard][i].first.x && canErase[standard][i].second.y <= x && x <= canErase[standard][i].first.y){
            res_idx = i;
            __android_log_print(ANDROID_LOG_DEBUG, "TAG", "check 성공");
            break;
        }
    }
    if (res_idx == -1)
        __android_log_print(ANDROID_LOG_DEBUG, "TAG", "check 실패");
    return res_idx;
}
JNIEXPORT void JNICALL
Java_org_androidtown_project_1r_TouchActivity_change(JNIEnv *env, jobject instance,
                                                     int res_idx, jlong addrInputImage) {

    // x와 y를 해상도에 맞게 비율 조절해야함

    // addInputImage로 들어오는 주소는 3개의 사진 중 마지막 사진(수정을 위한 영역에 빨간색선이 있는 사진)


    Mat &third = *(Mat *)addrInputImage;
    third = third_img;

    if (isErased[res_idx]) return;
    isErased[res_idx] = true;

    stk.push({ canErase[standard][res_idx], res_idx });

    for (int i = canErase[standard][res_idx].second.y - 30; i < canErase[standard][res_idx].first.y + 30; ++i){
        //if(i<0 || i>third_img.rows) continue;
        for (int j = canErase[standard][res_idx].second.x - 30; j < canErase[standard][res_idx].first.x + 30; ++j){
          //  if(j<0 || j>third_img.cols) continue;
            for (int k = 0; k < 3; ++k) {
                third.at<Vec3b>(i, j)[k] = patch_res.at<Vec3b>(i, j)[k];
            }
        }
    }


    // 기준 x,y부터 patch_res의 해당 영역을 붙혀넣는 작업
    // img_input에다가 patch집합을 붙혀넣자

}

JNIEXPORT jint JNICALL
Java_org_androidtown_project_1r_TouchActivity_rewind(JNIEnv *env, jobject instance, jlong addrInputImage) {

    if (stk.empty()){
        __android_log_print(ANDROID_LOG_DEBUG, "TAG", "stack이 비었음");
        return 0;
    }
    Mat& third = *(Mat *)addrInputImage;
    // img_input = 3번째 사진 Mat;
    third = third_img;
    // stack의 top 이 가진 정보 : 지워진 사각형의 양 모서리 끝 점
    //  2번째 사진(영역을 빨간색으로 색칠한 사진 원본)에서 top을 끝점으로 하는 사각형만큼 3번째 사진에 붙혀넣음
    // stack에서 pop

    int sx = stk.top().first.second.x;
    int sy = stk.top().first.second.y;
    int ex = stk.top().first.first.x;
    int ey = stk.top().first.first.y;
    int idx = stk.top().second;

    isErased[idx] = false;
    stk.pop();

    for (int i = sy - 30; i <= ey + 30; ++i){
        //if(i<0 || i>third_img.rows) continue;
        for (int j = sx - 30; j <= ex + 30; ++j){
          //  if(j<0 || j>third_img.cols) continue;
            for (int k = 0; k<3; ++k) {
                third.at<Vec3b>(i, j)[k] = second_img.at<Vec3b>(i, j)[k];
                // 2번째 사진에서 top을 끝점으로 하는 사각형만큼 3번째 사진에 붙혀넣음
            }
        }
    }
    // java코드에서 third를 bitmap 변환후 화면에 출력해야함
    return stk.size() + 1;
}


JNIEXPORT void JNICALL
Java_org_androidtown_project_1r_TouchActivity_save(JNIEnv *env, jobject instance, jlong addrInputImage) {

    // stack의 top 이 가진 정보 : 지워진 사각형의 양 모서리 끝 점과 사각형 집합에서의 idx
    // 1번째 사진(기준이 되는 원본사진) stack의 top의 사각형에 idx번째 patch들의 집합을 붙혀넣는다.
    // stack이 빌 때까지 pop하면서 같은 동작반복
    // 사진 저장

    Mat& first = *(Mat *)addrInputImage;
    first = origin[standard];
    int sx, sy, ex, ey, idx;

    while (!stk.empty()) {
        sx = stk.top().first.second.x;
        sy = stk.top().first.second.y;
        ex = stk.top().first.first.x;
        ey = stk.top().first.first.y;
        idx = stk.top().second;
        stk.pop();

        for (int i = sy - 30; i <= ey + 30; ++i){
            //if(i<0 || i>origin[standard].rows) continue;
            for (int j = sx - 30; j <= ex + 30; ++j) {
              //  if(j<0 || j>origin[standard].cols) continue;
                for (int k = 0; k<3; ++k) {
                    first.at<Vec3b>(i, j)[k] = patch_res.at<Vec3b>(i, j)[k];
                }
            }
        }
    }
    // java코드에서 first로 bitmap 변환 후 파일 저장해야함
}
}