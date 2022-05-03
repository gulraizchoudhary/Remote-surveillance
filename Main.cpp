
#include "Camera.h"
#include "HOG.h"
#include <thread> 

void connect(){
	Camera cam;
	cam.setLoginDetails("10.243.29.34", 8000, "admin", "test123");
	cam.login();
	
	cam.WriteAVIvideo();
}

int main() {
	
	//connect();

	HOG hog;
/*	
	hog.createXmlDescriptor("./pos/img","Positive.xml",1000);

	hog.createXmlDescriptor("./neg/img","Negative.xml",996);
*/

	hog.SVMTraining();
  return 0;
}

