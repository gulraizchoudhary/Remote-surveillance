
#pragma once

/*
 * HOG.h
 *
 *  Created on: ???/???/????
 *  Author: Gulraiz
 */

#include "Camera.h"

class HOG {
public:
	HOG();
	HOG(Camera cam);
	void hogDetector();
	void createXmlDescriptor(string sFirstFileName, string hogFileName, int FileNum);
	void SVMTraining();
	virtual ~HOG();

private:
	Camera cam;
};


