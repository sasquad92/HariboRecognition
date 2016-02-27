#include "opencv2/opencv.hpp"
//#include <omp.h>
#include <fstream>
#include <iostream>
#include <cmath>

#define READ_FILE_PATH	"nazwy_zdjec\\nazwy_zdjec.txt"
#define WRITE_FILE_PATH	"wyniki\\Sommerrey_Krzysztof.txt"
#define FILE_READ_ERROR "Plik w podanej lokalizacji nie wystêpuje b¹dŸ jest uszkodzony!"
#define PI 3.14159265

using namespace cv;
using namespace std;

fstream file_with_names;
fstream file_with_result;
string photos_names[400];


int ReadNamesFromTxt()
{
	int i = 0;

	file_with_names.open( READ_FILE_PATH, ios::in );

	if ( file_with_names.is_open() ) // sprawdzenie uzyskania dostêpu do pliku
	{

		while ( !file_with_names.eof() )
		{
			getline( file_with_names, photos_names[i] );
			
			i++;
		}

		file_with_names.close();
	}
	else
		cout << FILE_READ_ERROR << endl;

	return i;

}

void WriteResultToTxt()
{
	file_with_result.open( WRITE_FILE_PATH, ios::out );

	bool isOpen = file_with_result.is_open();

	if ( isOpen ) // sprawdzenie uzyskania dostêpu do pliku
	{
//zrealizowaæ zapis wyniku dla danej sceny

		file_with_result.close();
	}
	else
		cout << FILE_READ_ERROR << endl;
}

void DeleteUnproperValue(vector<int> &haribo_amound)
{
	if (haribo_amound.size() != 0)
	{
		sort(haribo_amound.begin(), haribo_amound.end(), [](const int& c1, const int& c2)
		{
			return c1 < c2;
		});

		int sample_value = haribo_amound[0]; //haribo_amound[floor(haribo_amound.size()/2)]; // mo¿e braæ medianê ?



		for (int i = 0; i < haribo_amound.size(); i++)
		{
			if (haribo_amound[i] >(sample_value + 2) || haribo_amound[i] < (sample_value - 2))
			{
				haribo_amound.erase(haribo_amound.begin() + i);
				i = i - 1;
			}
		}
	}
	
}

int AverageOneColorHariboAmound(vector<int> haribo_amound)
{
	double sum=0;
	int average = 0;
	
	if (haribo_amound.size() != 0)
	{
		DeleteUnproperValue(haribo_amound);


		for (int i = 0; i < haribo_amound.size(); i++)
		{
			sum += haribo_amound[i];
		}

		average = round(sum / haribo_amound.size());
	}
	

	return average;
}


// http://www.cplusplus.com/forum/general/116020/
void ClockwiseOrder(vector<Point2f> &contours)
{
	// nie u¿ywana - tylko w razie debuga
	// img size 1040 x 585

	Point2f center = { 520, 242 };
	vector<Point2f> contours_tmp;
	double angle[4] = { 0, 0, 0, 0 };
	// changing center of system
	for (int j = 0; j < contours.size(); j++)
	{
		contours_tmp.push_back(contours[j]);
		contours_tmp[j].x = contours_tmp[j].x - center.x;
		contours_tmp[j].y = contours_tmp[j].y - center.y;

		angle[j] = atan2(contours_tmp[j].y, contours_tmp[j].x) * 180 / PI;
		//if ( angle[j] < 0 )
		//{
		//	angle[j] = abs(angle[j]) + 180;
		//}
	}
}

// http://www.cplusplus.com/forum/general/116020/
double ClockwiseOrderPoint(Point2f point)
{
	// img size 1040 x 585

	Point2f center = { 520, 242 }; // lepiej by³oby przekazywaæ obraz i operowaæ na obraz.width/2 i height/2
	double angle = 0;
	// changing center of system

	point.x = point.x - center.x;
	point.y = point.y - center.y;

	angle = atan2(point.y, point.x) * 180 / PI;

	return angle;
}

// http://www.cplusplus.com/forum/general/116020/
bool ClockwiseComparePoints(const Point2f &a, const Point2f &b)
{
	return ClockwiseOrderPoint(a) < ClockwiseOrderPoint(b);
}

void WhatShape(vector<vector<Point>> &contours)
{
	int circle = 0;
	int rectangle = 0;
	int hexagon = 0;
	int form = 0; // 1 - rect, 2 - hex, 3 - circ
	vector<vector<Point>> tmp;

	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() == 4)
		{
			rectangle++;
		}
		else if (contours[i].size() == 6)
		{
			hexagon++;
		}
		else if (contours[i].size() == 8)
		{
			circle++;
		}
	}

	if ((rectangle > hexagon) && (rectangle > circle))
	{
		form = 1;
	}
	if ((hexagon > rectangle) && (hexagon > circle))
	{
		form = 2;
	}
	if ((circle > rectangle) && (circle > hexagon))
	{
		form = 3;
	}

	if (form == 1)
	{
		for (int j = 0; j < contours.size(); j++)
		{
			if (contours[j].size() == 4)
			{
				tmp.push_back(contours[j]);
			}
		}
	}
	else if (form == 2)
	{
		for (int j = 0; j < contours.size(); j++)
		{
			if (contours[j].size() == 6)
			{
				tmp.push_back(contours[j]);
			}
		}
	}
	else if (form == 3)
	{
		for (int j = 0; j < contours.size(); j++)
		{
			if (contours[j].size() == 8)
			{
				tmp.push_back(contours[j]);
			}
		}
	}

	contours.clear();
	contours.resize(tmp.size());
	contours = tmp;
}

bool DistanceBetweenPoints(vector<Point2f> &contours)
{
	// dystans od pierwszej pary pkt z wektora do reszty
	vector<double> distance;
	double dx, dy;
	bool isSuccess = true;

	for (int j = 0; j < contours.size(); j++)
	{
		for (int i = 0; i < contours.size(); i++)
		{
			if ( i != j )
			{
				dx = pow((contours[j].x - contours[i].x), 2);
				dy = pow((contours[j].y - contours[i].y), 2);
				distance.push_back(sqrt(dx + dy));
			}
		}
	}
	
	// je¿eli dystans jest za ma³y to nie robiæ warp perspective

	for (int j = 0; j < distance.size(); j++)
	{
		if ( ( distance[j] < 150 ) )// || ( distance[j] > 300 ) )
		{
			isSuccess = false;
		}
	}

	return isSuccess;
}

bool DistanceBetweenPointsOLD(vector<Point2f> &contours)
{
	// dystans od pierwszej pary pkt z wektora do reszty
	vector<double> distance;
	double dx, dy;
	bool isSuccess = true;

	for (int i = 1; i < contours.size(); i++)
	{
		dx = pow((contours[0].x - contours[i].x), 2);
		dy = pow((contours[0].y - contours[i].y), 2);
		distance.push_back(sqrt(dx + dy));
	}
	// je¿eli dystans jest za ma³y to nie robiæ warp perspective

	for (int j = 0; j < distance.size(); j++)
	{
		if ((distance[j] < 150))// || ( distance[j] > 300 ) )
		{
			isSuccess = false;
		}
	}

	return isSuccess;
}

bool ContoursDetection(Mat &src, vector<vector<Point>> &contours)
{
	Scalar color = (0, 0, 255);
	Mat after_thresholding, after_dilatation, after_erodation;
	int dilation_size = 2;
	double track;
	int area[1000];
	
	vector<vector<Point>> contours_tmp;
	vector<vector<Point>> approx;
	bool isSuccess = false;

	contours_tmp.clear();

	//#pragma omp parallel for default(shared) private(track)
	for ( track = 44; contours_tmp.size() != 5 && track >= 30; track = -2 )
	{
		threshold( src, after_thresholding, track, 255, THRESH_BINARY );
		Mat element = getStructuringElement(MORPH_CROSS, Size(2 * dilation_size + 1, 2 * dilation_size + 1), Point(dilation_size, dilation_size));
		dilate(after_thresholding, after_dilatation, element);
		findContours( after_dilatation, contours_tmp, RETR_TREE, CHAIN_APPROX_SIMPLE );
	}

	if ( contours_tmp.size() < 5 )
	{
		return isSuccess = false;
	}
	else if ( contours_tmp.size() >= 5 )
	{
		approx.resize(contours_tmp.size());

		for ( size_t k = 0; k < contours_tmp.size(); k++ )
		{
			area[k] = contourArea( contours_tmp[k] );
			approxPolyDP( contours_tmp[k], approx[k], 4, true );

			if ( ( approx[k].size() == 4 || approx[k].size() == 6 || approx[k].size() == 8 )
				&& area[k] > 200 
				&& area[k] < 5000 )
			{
				//drawContours(after_dilatation, contours_tmp, k, color, CV_FILLED);
				contours.push_back( approx[k] );		
			}
		}
	}
	
	if ( contours.size() > 4 )
	{
		vector<vector<Point>> tmp;

		WhatShape(contours);

		if (contours.size() < 4)
		{
			isSuccess = false;
			return isSuccess;
		}

		//http://stackoverflow.com/questions/13495207/opencv-c-sorting-contours-by-their-contourarea
		sort( contours.begin(), contours.end(), []( const vector<Point>& c1, const vector<Point>& c2 )
		{
			return contourArea( c1, false ) > contourArea( c2, false );
		} );
		
		tmp.resize( 4 );

		for ( int i = 0; i < 4; i++ )
		{
			tmp[i] = contours[i];
		}
		
		contours.clear();
		contours = tmp;
		isSuccess = true;
	}
	else if (contours.size() < 4)
	{
		isSuccess = false;
	}
	else if (contours.size() == 4)
	{
		isSuccess = true;
	}

	for ( size_t i = 0; i < contours.size(); i++ )
	{
		drawContours( after_dilatation, contours, i, color, CV_FILLED );
	}
	return isSuccess;
}

Mat GetPerspective(Mat &color_img, vector<vector<Point>> &contours, bool &isSuccess)
{
	Mat obraz_wyprostowany;
	Mat matrix;
	isSuccess = false;

	// kod z zajêæ
	/// Get the moments:
	vector<Moments> mu(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		mu[i] = moments(contours[i], false);
	}

	///  Get the mass centers:
	vector<Point2f> mc(contours.size());

	int k = 0;
	for (int i = 0; i < contours.size(); i++)
	{

		mc[i] = Point2f(mu[k].m10 / mu[k].m00, mu[k].m01 / mu[k].m00); // obliczanie œrodka ciê¿koœci
		k++;
	}

	vector<Point2f> dest_points;
	Point2f p1, p2, p3, p4;

	//ClockwiseOrder(mc); // debug only

	sort(mc.begin(), mc.end(), ClockwiseComparePoints);

	isSuccess = DistanceBetweenPoints(mc);

	if (isSuccess)
	{
		if (contours[0].size() != 4) // znaczniki w naro¿nikach - warunek do póŸniejszej zmiany
		{
			// znaczniki w naro¿nikach
			p4.x = 0;
			p4.y = 400;

			p3.x = 570;
			p3.y = 400;

			p1.x = 0;
			p1.y = 0;

			p2.x = 570;
			p2.y = 0;
		}
		else
		{
			// znaczniki na œrodku krawêdzi
			p4.x = 285;
			p4.y = 400;

			p1.x = 0;
			p1.y = 200;

			p3.x = 570;
			p3.y = 200;

			p2.x = 285;
			p2.y = 0;
		}

		dest_points = { p1, p2, p3, p4 };

		matrix = getPerspectiveTransform(mc, dest_points); // transformowanie i prostowanie
		warpPerspective(color_img, obraz_wyprostowany, matrix, Size(570, 400));
	}
	return obraz_wyprostowany;
}

int HariboCnt(Mat &BGR_img, int color)
{
	vector<vector<Point>> contours;
	Mat HSV_img, one_color_bears, tmp1, tmp2;
	int amount = 0;
	int area[1000];
	vector<int> filtered_area;
	Mat after_dilatation, after_erodation;
	int dilation_size = 2;

	cvtColor(BGR_img, HSV_img, CV_BGR2HSV);

	switch ( color )
	{
	case 0: // czerwone ciemne
	{
		inRange(HSV_img, Scalar(169, 90, 0), Scalar(179, 255, 130), one_color_bears); // wypas
		break;
	}
		
	case 1: // czerwone jasne
	{
		inRange(HSV_img, Scalar(0, 170, 100), Scalar(4, 255, 255), tmp1);
		inRange(HSV_img, Scalar(178, 170, 100), Scalar(180, 255, 255), tmp2);
		one_color_bears = tmp1 + tmp2;
		break;
	}
		
	case 2: // zielone
	{
		inRange(HSV_img, Scalar(10, 80, 0), Scalar(50, 255, 110), one_color_bears);
		break;
	}
		
	case 3: // pomarañczowe
	{
		inRange(HSV_img, Scalar(6, 120, 100), Scalar(14, 255, 255), one_color_bears);
		break;
	}
	case 4: // bia³e
	{
		inRange(HSV_img, Scalar(11, 50, 100), Scalar(24, 150, 255), one_color_bears);
		break;
	}
	case 5: // zó³te
	{
		inRange(HSV_img, Scalar(15, 150, 120), Scalar(45, 255, 255), one_color_bears);
		break;
	}
	default: // podana inna liczba
		break;
	}

	Mat element = getStructuringElement(MORPH_OPEN, Size(2 * dilation_size + 1, 2 * dilation_size + 1), Point(dilation_size, dilation_size));
	erode(one_color_bears, after_erodation, element);
	dilate(after_erodation, after_dilatation, element);

	findContours(after_dilatation, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

	Scalar col = (255, 255, 255);

	for (size_t i = 0; i < contours.size(); i++)
	{
		area[i] = contourArea(contours[i]);
		if ( area[i] > 550 )//&& area[i] < 6000) // 500 // 535
		{
			drawContours(BGR_img, contours, i, color, 2);
			drawContours(HSV_img, contours, i, color, 2);
			filtered_area.push_back(area[i]);
		}
	}

	if (filtered_area.size() > 0)
	{
		sort(filtered_area.begin(), filtered_area.end(), [](const int& c1, const int& c2)
		{
			return c1 < c2;
		});

		int simple_area = filtered_area[0];
		double cnt;

		if (simple_area > 1800)
		{
			simple_area = 950;
		}

		for (int k = 0; k < filtered_area.size(); k++)
		{
			cnt = floor(filtered_area[k] / simple_area);
			amount += cnt;
		}
	}
	

	return amount;
}

int main( int, char )
{
	Mat src, resized_gray, resized_color, warped_img;
	vector<vector<Point>> contours;
	vector<vector<Point>> contours_haribo;
	bool isFindingContoursSuccess = false;
	bool isWarpingImgSuccess = false;
	int number_of_photos = 0;
	vector < int > dark_red_haribo;
	vector < int > light_red_haribo;
	vector < int > green_haribo;
	vector < int > orange_haribo;
	vector < int > white_haribo;
	vector < int > yellow_haribo;
	
	string previous_scene;
	string current_scene;


	number_of_photos = ReadNamesFromTxt();

	file_with_result.open(WRITE_FILE_PATH, ios::out);
	//
	//file_with_result << "cc jc z  p  b  ¿" << endl;
	//

	for (int i = 0; i < number_of_photos; i++)
	{
		if (photos_names[i] != "")
		{
			src = imread("zdjecia\\" + photos_names[i], CV_LOAD_IMAGE_ANYCOLOR);
			resize(src, resized_color, src.size() / 4);
			cvtColor(resized_color, resized_gray, CV_BGR2GRAY);

			contours.clear();
			current_scene.clear();

			if (i == 0)
				previous_scene.insert(0, photos_names[0], 6, 3);

			isFindingContoursSuccess = ContoursDetection(resized_gray, contours);

			if (isFindingContoursSuccess)
			{
				warped_img = GetPerspective(resized_color, contours, isWarpingImgSuccess);

				if (isWarpingImgSuccess)
				{
					current_scene.insert(0, photos_names[i], 6, 3);

					if (current_scene != previous_scene) // zmiana sceny
					{
						DeleteUnproperValue(white_haribo);


						file_with_result << AverageOneColorHariboAmound(dark_red_haribo) << ", "
							<< AverageOneColorHariboAmound(light_red_haribo) << ", "
							<< AverageOneColorHariboAmound(green_haribo) << ", "
							<< AverageOneColorHariboAmound(orange_haribo) << ", "
							<< AverageOneColorHariboAmound(white_haribo) << ", "
							<< AverageOneColorHariboAmound(yellow_haribo) << endl;

						dark_red_haribo.clear();
						light_red_haribo.clear();
						green_haribo.clear();
						orange_haribo.clear();
						white_haribo.clear();
						yellow_haribo.clear();
					}
					dark_red_haribo.push_back(HariboCnt(warped_img, 0));
					light_red_haribo.push_back(HariboCnt(warped_img, 1));
					green_haribo.push_back(HariboCnt(warped_img, 2));
					orange_haribo.push_back(HariboCnt(warped_img, 3));
					white_haribo.push_back(HariboCnt(warped_img, 4));
					yellow_haribo.push_back(HariboCnt(warped_img, 5));

					previous_scene.clear();
					previous_scene = current_scene;
				}
			}
		}
	}

	// zapis ostatniej sceny w pliku tekstowym
	file_with_result << AverageOneColorHariboAmound(dark_red_haribo) << ", "
		<< AverageOneColorHariboAmound(light_red_haribo) << ", "
		<< AverageOneColorHariboAmound(green_haribo) << ", "
		<< AverageOneColorHariboAmound(orange_haribo) << ", "
		<< AverageOneColorHariboAmound(white_haribo) << ", "
		<< AverageOneColorHariboAmound(yellow_haribo) << endl;

	file_with_result.close();
	return 0;
}

