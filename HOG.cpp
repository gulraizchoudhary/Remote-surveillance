
#pragma once

/*
 * HOG.cpp
 *HOG :: Detect people
 *  Created on: 9/1/2015
 *  Author: Gulraiz
 */

#include "HOG.h"

using namespace std;
using namespace cv;


HOG::HOG(){}
HOG::HOG(Camera cam) {
	this->cam= cam;
	hogDetector();
}

void HOG::createXmlDescriptor(string sFirstFileName, string hogFileName, int FileNum) {

	//variables
	 char FullFileName[100];
	 char FirstFileName[100];

	 strcpy(FirstFileName,sFirstFileName.c_str());
	// char SaveHogDesFileName[100] = "Positive.xml";


	 vector< vector < float> > v_descriptorsValues;
	 vector< vector < Point> > v_locations;


	 for(int i=0; i< FileNum; ++i)
	 {
	  sprintf(FullFileName, "%s%d.bmp", FirstFileName, i+1);
	  printf("%s\n", FullFileName);

	  //read image file
	  Mat img, img_gray;
	  img = imread(FullFileName);

	  //resizing
	  resize(img, img, Size(64,48) ); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );
	  //gray
	  cvtColor(img, img_gray, CV_RGB2GRAY);

	  //extract feature
	  HOGDescriptor d( Size(32,16), Size(8,8), Size(4,4), Size(4,4), 9);
	  vector< float> descriptorsValues;
	  vector< Point> locations;
	  d.compute( img_gray, descriptorsValues, Size(0,0), Size(0,0), locations);

	  //printf("descriptor number =%d\n", descriptorsValues.size() );
	  v_descriptorsValues.push_back( descriptorsValues );
	  v_locations.push_back( locations );
	  //show image
	 // imshow("origin", img_gray);

	  //waitKey(0);
	 }

	 //refer to this address -> http://feelmare.blogspot.kr/2014/04/the-example-source-code-of-2d-vector.html
	 //save to xml
	 FileStorage hogXml(hogFileName, FileStorage::WRITE); //FileStorage::READ
	 //2d vector to Mat
	 int row=v_descriptorsValues.size(), col=v_descriptorsValues[0].size();
	 //printf("col=%d, row=%d\n", row, col);
	 Mat M(row,col,CV_32F);
	 //save Mat to XML
	 for(int i=0; i< row; ++i)
	  memcpy( &(M.data[col * i * sizeof(float) ]) ,v_descriptorsValues[i].data(),col*sizeof(float));

	 printf("\n","Writing in XML file...");
	 //write xml
	 write(hogXml, "Descriptor_of_images",  M);

	 //write(hogXml, "Descriptor", v_descriptorsValues );
	 //write(hogXml, "locations", v_locations );
	 hogXml.release();
}

void HOG::SVMTraining() {
	//Read Hog feature from positive XML file
	 ///////////////////////////////////////////////////////////////////////////
	 printf("1. Feature data from positive xml load\n");
	 
	 //create xml to read
	 FileStorage read_PositiveXml("Positive.xml", FileStorage::READ);
	 
	 //Positive Mat
	 Mat pMat;
	 read_PositiveXml["Descriptor_of_images"] >> pMat;

	 //Read Row, Cols
	 int pRow,pCol;
	 pRow = pMat.rows; pCol = pMat.cols;

	 //release
	 read_PositiveXml.release();

	//Read Hog feature from negative XML file
	 ///////////////////////////////////////////////////////////////////////////
	 printf("2. Feature data from negative xml load\n");
	 FileStorage read_NegativeXml("Negative.xml", FileStorage::READ);
	 //Negative Mat
	 Mat nMat;
	 read_NegativeXml["Descriptor_of_images"] >> nMat;
	 //Read Row, Cols
	 int nRow,nCol;
	 nRow = nMat.rows; nCol = nMat.cols;

	 //Rows, Cols printf
	 printf("   pRow=%d pCol=%d, nRow=%d nCol=%d\n", pRow, pCol, nRow, nCol );
	 
	 //release
	 read_NegativeXml.release();
	 /////////////////////////////////////////////////////////////////////////////////

	 //Make training data for SVM
	 /////////////////////////////////////////////////////////////////////////////////
	 printf("3. Make training data for SVM\n");
	 //descriptor data set
	 Mat PN_Descriptor_mtx( pRow + nRow, pCol, CV_32FC1 ); //in here pCol and nCol is descriptor number, so two value must be same;
	 memcpy(PN_Descriptor_mtx.data, pMat.data, sizeof(float) * pMat.cols * pMat.rows );
	 int startP = sizeof(float) * pMat.cols * pMat.rows;
	 memcpy(&(PN_Descriptor_mtx.data[ startP ]), nMat.data, sizeof(float) * nMat.cols * nMat.rows );

	 //data labeling
	 Mat labels( pRow + nRow, 1, CV_32FC1, Scalar(-1.0) );
	    labels.rowRange( 0, pRow ) = Scalar( 1.0 );
	 /////////////////////////////////////////////////////////////////////////////////

	 //Set svm parameter
	 /////////////////////////////////////////////////////////////////////////////////
	 printf("4. SVM training\n");
	 CvSVM svm;
	 CvSVMParams params;
	 params.svm_type = CvSVM::C_SVC;
	    params.kernel_type = CvSVM::LINEAR;
	    params.term_crit = cvTermCriteria( CV_TERMCRIT_ITER, 10000, 1e-6 );
	 /////////////////////////////////////////////////////////////////////////////////

	 //Training
	 /////////////////////////////////////////////////////////////////////////////////
	 svm.train(PN_Descriptor_mtx, labels, Mat(), Mat(), params);

	 //Trained data save
	 /////////////////////////////////////////////////////////////////////////////////
	 printf("5. SVM xml save\n");
	 svm.save( "trainedSVM.xml" );

	// FileStorage hogXml("testXML.xml", FileStorage::WRITE); //FileStorage::READ
	// write(hogXml, "Data", PN_Descriptor_mtx);
	// write(hogXml, "Label", labels);
	// hogXml.release();

}
void HOG::hogDetector(){
	
	    Mat img;
	    HOGDescriptor hog;
	    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	    namedWindow("video capture", CV_WINDOW_AUTOSIZE);
	    while (true)
	    {
			//img= cam.getFrame();

	        if (!img.data)
	            continue;

	        vector<Rect> found, found_filtered;
	        hog.detectMultiScale(img, found, 0, Size(8,8), Size(32,32), 1.05, 2);

	        size_t i, j;
	        for (i=0; i<found.size(); i++)
	        {
	            Rect r = found[i];
	            for (j=0; j<found.size(); j++)
	                if (j!=i && (r & found[j])==r)
	                    break;
	            if (j==found.size())
	                found_filtered.push_back(r);
	        }
	        for (i=0; i<found_filtered.size(); i++)
	        {
		    Rect r = found_filtered[i];
	            r.x += cvRound(r.width*0.1);
		    r.width = cvRound(r.width*0.8);
		    r.y += cvRound(r.height*0.06);
		    r.height = cvRound(r.height*0.9);
		    rectangle(img, r.tl(), r.br(), cv::Scalar(0,255,0), 2);
		}
	        imshow("video capture", img);
	        if (waitKey(20) >= 0)
	            break;
	    }
}

HOG::~HOG() {}

