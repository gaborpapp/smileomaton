#include <list>

#include "testApp.h"

testApp::testApp() :
	camera_id(0)
{
}

testApp::~testApp()
{
	video_grabber.close();
}

void testApp::setup()
{
	font.loadFont("bitstream.ttf", 8);

	video_grabber.initGrabber(CAMERA_WIDTH, CAMERA_HEIGHT);
	video_grabber.setVerbose(false);
	video_grabber.listDevices();
	video_grabber.setDeviceID(camera_id);

	video_width = video_grabber.getWidth();
	video_height = video_grabber.getHeight();

	video_texture.allocate(video_width, video_height, GL_RGB);

	gui_video = &gui.addContent("Camera feed", video_texture, CAMERA_WIDTH_SCALED);
	gui.page(1).setName("Smile detector");

	gui.loadFromXML();

	gui.show();
}

void testApp::detect_smile()
{
	int n = video_width * video_height;
	float *fpixels = new float[n];
	unsigned char *cpixels = video_grabber.getPixels();

	for (int i = 0, j = 0; i < n; i++, j += 3)
	{
		fpixels[i] = (float)((cpixels[j] + cpixels[j + 1] + cpixels[j + 2]) / 3);
	}

	RImage<float> rimage(fpixels, video_width, video_height);

	faces.clear();
	smile_finder.findSmiles(rimage, faces);

	delete [] fpixels;

	/*
	   MPSmile smileFinder;

	   RImage<float> rimage(fpixels, img.width, img.height);
	   bool smileFound = false;
	   amount = 0;
	   if(!smileFinder.findSmiles(rimage, faces)) {
	   if (faces.size() == 1) {
	   FaceObject* face = static_cast<FaceObject*>(*(faces.begin()));
	   amount = face->activation;
	   smileFound = true;
	   }
	   }

	   delete [] fpixels;

	   return smileFound;
	   */
}

void testApp::draw_smiles(float x, float y, float w, float ow)
{
	float scale = w / ow;
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(scale, scale, 1.0);

	ofNoFill();

	char str[256];

	list<VisualObject *>::iterator face = faces.begin();
	for(int i = 0; face != faces.end(); ++face, i++)
	{
		FaceObject *fo = static_cast<FaceObject*>(*face);

		ofSetColor(0x303030);
		ofRect(fo->x, fo->y, fo->xSize, fo->ySize);

		snprintf(str, 256, "%f", fo->activation);
		font.drawString(str, fo->x, fo->y);
		/*
		// show result
		printf(" --- Result ---\n");
		printf(" Smile:%g\n", fo->activation);
		printf(" FaceBox[%f %f %f %f]\n", 
				fo->x, fo->y, fo->x+fo->xSize, fo->y+fo->ySize );
		//printf(" image columns:%d rows:%d\n", image.columns(),image.rows() );
		printf(" Processing Face %d\n", i++);
		*/
	}
	glPopMatrix();
}

void testApp::update()
{
	// update video grabber
	video_grabber.update();
	if (video_grabber.isFrameNew())
	{
		video_texture.loadData(video_grabber.getPixels(),
				video_grabber.getWidth(), video_grabber.getHeight(), GL_RGB);
	}

	detect_smile();
}

void testApp::draw()
{
	gui.draw();
	draw_smiles(15, 55, CAMERA_WIDTH_SCALED, CAMERA_WIDTH);
}

void testApp::keyPressed(int key)
{
}

void testApp::keyReleased(int key)
{
}

void testApp::mouseMoved(int x, int y)
{
}

void testApp::mouseDragged(int x, int y, int button)
{
}

void testApp::mousePressed(int x, int y, int button)
{
}

void testApp::mouseReleased(int x, int y, int button)
{
}

void testApp::windowResized(int w, int h)
{
}

