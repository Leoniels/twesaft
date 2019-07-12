/*****************************************************************************
*   Non-Rigid Face Tracking
******************************************************************************
*   by Jason Saragih, 5th Dec 2012
*   http://jsaragih.org/
******************************************************************************
*   Ch6 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/
/*
  visualize_face_tracker: perform face tracking from a video/camera stream
  Jason Saragih (2012)
*/
#include "opencv_hotshots/ft/ft.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <fstream>
#include <iostream>
#define fl at<float>
const char* usage = "usage: ./visualise_face_tracker tracker [video_file]";
//==============================================================================
void
draw_string(Mat img,                       //image to draw on
        const string text)             //text to draw
{
  Size size = getTextSize(text,FONT_HERSHEY_COMPLEX,0.6f,1,NULL);
  putText(img,text,Point(0,size.height),FONT_HERSHEY_COMPLEX,0.6f,
      Scalar::all(0),1,LINE_AA);
  putText(img,text,Point(1,size.height+1),FONT_HERSHEY_COMPLEX,0.6f,
      Scalar::all(255),1,LINE_AA);
}
//==============================================================================
bool
parse_help(int argc,char** argv)
{
  for(int i = 1; i < argc; i++){
    string str = argv[i];
    if(str.length() == 2){if(strcmp(str.c_str(),"-h") == 0)return true;}
    if(str.length() == 6){if(strcmp(str.c_str(),"--help") == 0)return true;}
  }return false;
}
//==============================================================================
int main(int argc,char** argv)
{
  //parse command line arguments
  if(parse_help(argc,argv)){cout << usage << endl; return 0;}
  if(argc < 2){cout << usage << endl; return 0;}
  int argoffset = 0;
  int pt2track = -1;
  ofstream proctimefile;
  bool writeVid = false;
  for(int i=1; i<argc; i++){
	string str = argv[i];
	if(strcmp(str.c_str(),"-m") == 0){
		proctimefile.open("proctime", ios::trunc);
		argoffset++;
	} else if (strcmp(str.c_str(),"-p") == 0){
		pt2track = atoi(argv[i+1]);
		argoffset += 2;
	} else if (strcmp(str.c_str(),"-o") == 0){
		writeVid = true;
		argoffset++;
	}
  }
  
  //load detector model
  face_tracker tracker = load_ft<face_tracker>(argv[1+argoffset]);

  //create tracker parameters
  face_tracker_params p; p.robust = false;
  p.ssize.resize(3);
  p.ssize[0] = Size(21,21);
  p.ssize[1] = Size(11,11);
  p.ssize[2] = Size(5,5);

  //open video stream
  VideoCapture cam; 
  if(argc > 2+argoffset)cam.open(argv[2+argoffset]); else cam.open(0);
  if(!cam.isOpened()){
    cerr << "Failed opening video file." << endl
     << usage << endl; return 0;
  }
  //open video output
  VideoWriter outputVideo;
  if(writeVid){
    int codec = VideoWriter::fourcc('X', 'V', 'I', 'D');
    outputVideo.open("out.avi", codec, static_cast<int>(cam.get(CAP_PROP_FPS)),
                     Size(static_cast<int>(cam.get(CAP_PROP_FRAME_WIDTH)),
                          static_cast<int>(cam.get(CAP_PROP_FRAME_HEIGHT))));
    if(!outputVideo.isOpened()){
	    cerr << "ERROR OPENING OUTPUT VIDEO WRITER" << endl;
	    return 1;
    }
  }
  int64 processingTime = getTickCount();
  //detect until user quits
  namedWindow("face tracker");
  Mat im;
  while(cam.read(im)/*cam.get(CAP_PROP_POS_AVI_RATIO) < 0.999999*/){
	int64 tp0 = getTickCount();
	int res = tracker.track(im, p);
	processingTime = getTickCount()-tp0;
    if(res)tracker.draw(im);
    if(writeVid)outputVideo << im;
    draw_string(im,"d - redetection");
    tracker.timer.display_fps(im,Point(1,im.rows-1));
    imshow("face tracker",im);
    int c = waitKey(10);
    if(proctimefile.is_open())
      proctimefile <<
      cv::format("%.4f", (double)(processingTime)*1000.0f/getTickFrequency())
      << endl;
    if(pt2track >= 0)
      cout << (int)(tracker.points[pt2track].x) << ", " << (int)(tracker.points[pt2track].y) << endl;
    if(c == 'q')break;
    else if(c == 'd')tracker.reset();
  }
  destroyWindow("face tracker"); cam.release(); 
  if(proctimefile.is_open())proctimefile.close();
  if(writeVid)outputVideo.release(); return 0;
}
//==============================================================================
