#include "testApp.h"

testApp::testApp() :
	camera_id(0)
{
}

testApp::~testApp()
{
	video_grabber.close();
	serial.close();
}

void testApp::setup()
{
	font.loadFont("bitstream.ttf", 16);

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

	for (int i = 0; i < SMILE_AVERAGES; i++)
	{
		smile_averages[i] = 0;
	}
	smile_index = 0;
	happiness = 0;

	imgsaver.allocate(CAMERA_WIDTH, CAMERA_HEIGHT, OF_IMAGE_COLOR);
	imgsaver.setUseTexture(false);

	// setup arduino serial
	serial.enumerateDevices();
	serial.setup();
}

void testApp::send_arduino_message()
{
	int limits[] = { 1, 2, 3, 4, 5 };
	const int limit_count = sizeof(limits)/sizeof(limits[0]);

	unsigned happiness_char = 0;
	for (int i = limit_count - 1; i >= 0; i--)
	{
		if (happiness >= limits[i])
		{
			happiness_char = limits[i];
			break;
		}
	}

	serial.writeByte(happiness_char);
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

	smile_averages[smile_index] = 0;
	list<VisualObject *>::iterator face = faces.begin();
	for(int i = 0; face != faces.end(); ++face, i++)
	{
		FaceObject *fo = static_cast<FaceObject*>(*face);

		smile_averages[smile_index] += fo->activation;
	}
	smile_index = (smile_index + 1) % SMILE_AVERAGES;

	happiness = 0;
	for (int i = 0; i < SMILE_AVERAGES; i++)
	{
		happiness += smile_averages[i];
	}
	happiness /= SMILE_AVERAGES;
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

		ofSetColor(0xddf010);
		ofRect(fo->x, fo->y, fo->xSize, fo->ySize);

		snprintf(str, 256, "%3.2f", fo->activation);
		font.drawString(str, fo->x + 5, fo->y + font.getLineHeight());

		snprintf(str, 256, "%3.2f", happiness);
		font.drawString(str, fo->x + 5, fo->y + 2 * font.getLineHeight());
	}
	glPopMatrix();
}

void testApp::save_photo()
{
	static float last_time = -MIN_PHOTO_SAVE_PERIOD;
	static int index = 0;
	static int last_sec = 0;

	struct tm tm;
	time_t ltime;
	char datename[256];

	float curr_time = ofGetElapsedTimef();

	if (curr_time > last_time + MIN_PHOTO_SAVE_PERIOD)
	{
		time(&ltime);
		localtime_r(&ltime, &tm);
		if (last_sec != tm.tm_sec)
			index = 0;
		sprintf(datename, "cheese-%02d%02d%02d%02d%02d%02d%02d.png", tm.tm_year-100,
				tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, index);

		imgsaver.setFromPixels(video_grabber.getPixels(), CAMERA_WIDTH, CAMERA_HEIGHT,
				OF_IMAGE_COLOR, false);
		imgsaver.saveImage(datename);

		last_time = curr_time;
	}
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

	if (happiness > 1.0)
		save_photo();

	send_arduino_message();
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

