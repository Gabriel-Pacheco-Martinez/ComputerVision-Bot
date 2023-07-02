/*------------------------------------------------------- |
This file will serve to do symbol recognition and send    |
information to the Arduino to indicate which movement it  |
must perform. This code is an edited format of the file   |
Template_Win downloaded from Moodle                       |
University of Nottingham                                  |
EEEE-1002                                                 |
Gabriel Pacheco                                           |
---------------------------------------------------------*/

// Including files for required libraries
#include <stdio.h>
#include "opencv_aee.hpp"
#include "main.hpp"
#include "pi2c.h"

//Including others required
using namespace std;
using namespace cv;

//Function to setup the camera on the Raspberry Pi
void setup(void)
{
    /// Setup camera won't work if you don't have a compatible webcam
    setupCamera(320, 240);  // Enable the camera for OpenCV
}

//Including getContours() function which is at the bottom so it works in main
void getContours(Mat frameDilate,Mat frame,Mat *frameTransform);

void getShapeCount(Mat frameTransformBW,Mat frameTransform2, int *Count);

//Main function
int main( int argc, char** argv )
{
    //Call setup function to prepare IO and devices
    setup();

    //Create a GUI window called photo
    cv::namedWindow("Photo");

    //Declaration of slave device on the i2c Bus address "0x07"
    Pi2c Arduino(7);

    //Main loop to perform image processing
    while(1)
    {
        //Declaring variables needed for later comparation of images
        int MatchPercent1,MatchPercent2,MatchPercent3;
        Mat imageColorsGRAY,imageDistanceGRAY,imageShapeCountGRAY,frameTransformGRAY;
        int Count,n;

        //Reading templates to compare to the frame image and turning them to black and white for better comparation
        Mat imageColors=imread("RedShortCut.bmp");
        cvtColor(imageColors, imageColorsGRAY, COLOR_BGR2GRAY);
        Mat imageColorsBW = imageColorsGRAY >128;

        Mat imageDistance=imread("DistanceMeasurement.bmp");
        cvtColor(imageDistance, imageDistanceGRAY, COLOR_BGR2GRAY);
        Mat imageDistanceBW = imageDistanceGRAY >128;

        Mat imageShapeCount=imread("ShapeCounter.bmp");
        cvtColor(imageShapeCount, imageShapeCountGRAY, COLOR_BGR2GRAY);
        Mat imageShapeCountBW = imageShapeCountGRAY >128;

        //Declaration of all variables needed for the frame transformation
        Mat frame;
        Mat frameHSV;
        Mat imagePINK;
        Mat frameBlur;
        Mat frameEdges;
        Mat frameDilate;
        Mat frameTransform;

        // Can't capture frames without a camera attached. Use static images instead
        while(frame.empty())
        {
            /// Can't capture frames without a camera attached. Use static images instead
            frame = captureFrame(); // Capture a frame from the camera and store in a new matrix variable
            //frame = readImage("RealShapeCounter.bmp");
        }

        //Display the original image on the window
        cv::imshow("Photo", frame);

        //Turning the image read from camera to HSV and selecting only Pink Pixels
        cvtColor(frame, frameHSV, COLOR_BGR2HSV);
        inRange(frameHSV, Scalar(115, 79, 0), Scalar(179, 255, 255), imagePINK);

        //Bluring image to reduce noise and use canny to find edges
        GaussianBlur(imagePINK,frameBlur,Size(3,3),3,0);
        Canny(frameBlur, frameEdges, 100, 300);

        //Dilating the image
        Mat kernel = getStructuringElement(MORPH_RECT,Size(3,3));
        dilate(frameEdges,frameDilate,kernel);

        //Callback to function getContours()
        getContours(frameDilate,frame,&frameTransform);
        imshow("Transformed Frame",frameTransform);

        //Show the frame transformed and turn it to Black and White
        cvtColor(frameTransform, frameTransformGRAY, COLOR_BGR2GRAY);
        Mat frameTransformBW = frameTransformGRAY >128;

        //Compare frame transform with the templates loaded to see which image is more compatible
        MatchPercent1 = compareImages(imageColorsBW,frameTransformBW);
        MatchPercent2 = compareImages(imageDistanceBW,frameTransformBW);
        MatchPercent3 = compareImages(imageShapeCountBW,frameTransformBW);

        //If the image with the "S" is detected color detection will be implemented to see what color was the ball
        //and send different messages to the Arduino depending on the color
        if (MatchPercent1 >= 70)
        {
            cout<<"S Shape Detected"<<endl;

            //Declaration of variables needed
            Mat imageBLACK,imageRED,imageBLUE,imageGREEN,imageYELLOW;

            //Declaration of HSV ranges needed for the color of each ball
            inRange(frameHSV, Scalar(0, 0, 0), Scalar(179, 255, 116), imageBLACK);
            inRange(frameHSV, Scalar(0, 123, 0), Scalar(7, 255, 255), imageRED);
            inRange(frameHSV, Scalar(90, 12, 0), Scalar(109, 255, 255), imageBLUE);
            inRange(frameHSV, Scalar(55, 38, 0), Scalar(80, 255, 255), imageGREEN);
            inRange(frameHSV, Scalar(22, 38, 0), Scalar(31, 255, 255), imageYELLOW);

            //Verification of variables needed for comparation of amount of pixel colors
            int BK,R,B,G,Y;
            BK=countNonZero(imageBLACK);
            R=countNonZero(imageRED);
            B=countNonZero(imageBLUE);
            G=countNonZero(imageGREEN);
            Y=countNonZero(imageYELLOW);

            //If the ball is color Black
            if ((BK>R)&&(BK>B)&&(BK>G)&&(BK>Y))
            {
               cout<<"Black"<<endl;
               Arduino.i2cWriteArduinoInt(1);
            }
            //If the ball is color Red
            if ((R>BK)&&(R>B)&&(R>G)&&(R>Y))
            {
                cout<<"Red"<<endl;
                Arduino.i2cWriteArduinoInt(2);
            }
            //If the ball is color Blue
            if ((B>BK)&&(B>R)&&(B>G)&&(B>Y))
            {
                cout<<"Blue"<<endl;
                Arduino.i2cWriteArduinoInt(3);
            }
            //If the ball is color Green
            if ((G>BK)&&(G>R)&&(G>B)&&(G>Y))
            {
                cout<<"Green"<<endl;
                Arduino.i2cWriteArduinoInt(4);
            }
            //If the ball is color Yellow
            if ((Y>BK)&&(Y>R)&&(Y>B)&&(Y>G))
            {
                cout<<"Yellow"<<endl;
                Arduino.i2cWriteArduinoInt(5);
            }
        }

        //Symbol for distance measured detected, send a signal to the Arduino for the car to go forwards
        else if (MatchPercent2 >= 70)
        {
            cout<<"Distance Measurement Shape Detected"<<endl;
            Arduino.i2cWriteArduinoInt(6);
        }

        //Symbol for shape count detected, callback the value of count calculated in getContours() function
        //and send the value to the Arduino
        else if (MatchPercent3 >= 70)
        {
            cout<<"Shape Count Symbol Detected"<<endl;
            getShapeCount(frameTransformBW,frameTransform,&Count);
            n=Count;
            cout<<"Shape Count Gives Back:"<<n<<endl;
            Arduino.i2cWriteArduinoInt(n);
        }

        // Wait 1ms for a keypress (required to update windows)
        int key = cv::waitKey(1);

        // Check if the ESC key has been pressed
        key = (key==255) ? -1 : key;
        if (key == 27)
            break;
	}

	// Disable the camera and close any windows
	closeCV();

	// Exit main function
	return 0;
}

void getContours(Mat frameDilate,Mat frame,Mat *frameTransform)
{
    //Declaring variables
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;

    //Finding contours for the dilated Image
    findContours(frameDilate, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    //For loop
    for(size_t i = 0; i < contours.size(); i++) // Loop through all values of the contours
    {
        //Declaring variables
        vector< vector<Point> >conPoly(contours.size());
        int area = contourArea(contours[i]);
        //Finding the polygon closest to the type of symbol depending on the contours
        float peri = arcLength(contours[i],true);
        approxPolyDP(contours[i],conPoly[i],0.05*peri,true);
        drawContours(frame,conPoly,i,Scalar(255,255,0),2);

        //If statement used to filter unusable images
        if (area>5)
        {
            if(conPoly[i].size()==4)
            {
               *frameTransform = transformPerspective(conPoly[i],frame,320,240);
            }

            else
            {
                cout<<"4 points needed"<<endl;
            }
        }
    }
}

void getShapeCount(Mat frameTransformBW,Mat frameTransform2,int *Count)
{
    //Declaring variables
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;

    //Finding contours for the dilated Image
    findContours(frameTransformBW, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    //For loop
    for(size_t i = 0; i < contours.size(); i++) // Loop through all values of the contours
    {
        //Declaring variables
        vector< vector<Point> >conPoly(contours.size());

        //Finding the polygon closest to the type of symbol depending on the contours
        float peri = arcLength(contours[i],true);
        approxPolyDP(contours[i],conPoly[i],0.05*peri,true);
        drawContours(frameTransform2,conPoly,i,Scalar(255,255,0),2);
        imshow("CONTOURS",frameTransform2);
        waitKey(0);
        *Count=conPoly.size()-3; //Substract 3 because of outer edge
    }
}
