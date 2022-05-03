
#include "Camera.h"
#include "HOG.h"
#include <thread> 

void connect(){
	//Login to PTZ Camera
	Camera cam;
	cam.setLoginDetails("10.243.29.34", 8000, "admin", "test123");
	cam.login();
	
	//Write the videos of the location where the threat is detected.
	cam.WriteAVIvideo();
}

int main() {
	
	connect();
	//Predict the threat based on the trained model. 
	HOG hog;
/*	
	hog.createXmlDescriptor("./pos/img","Positive.xml",1000);

	hog.createXmlDescriptor("./neg/img","Negative.xml",996);
*/

	hog.SVMTraining();
  return 0;
}

