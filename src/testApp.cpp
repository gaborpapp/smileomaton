#include "testApp.h"

testApp::testApp() :
	camera_id(0),
	face_min_area(0),
	face_detected(false),
	face_period(0),
	take_photo(false),
	rgb_bgr(false),
	disable_serial(false),
	last_face_time(-FACE_PERIOD)
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

	gui_happy = &gui.addSlider("Happiness", gui_happiness, -6, 6);
	gui_happy->setNewColumn(true);
	gui.addSlider("Min face", face_min_area, 0.1, 1);
	gui_face_detected = &gui.addToggle("Face detected", face_detected);
	gui_face_detected->setSize(168, 20);
	gui_face_period = &gui.addSlider("Face period", face_period, 0, FACE_PERIOD);

	gui.addButton("Take photo", take_photo).setSize(128, 20);
	gui.addToggle("RGB-BGR", rgb_bgr).setSize(128, 20);
	gui.addToggle("Disable", disable_serial).setSize(128, 20);

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
	serial_inited = serial.setup();
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

	if (serial_inited)
		serial.writeByte(happiness_char);
}

void testApp::detect_smile()
{
	int n = video_width * video_height;
	float *fpixels = new float[n];
	unsigned char *cpixels = video_grabber.getPixels();

	for (int i = 0, j = 0; i < n; i++, j += 3)
	{
		fpixels[i] = (float)(cpixels[j + 1]);
	}

	RImage<float> rimage(fpixels, video_width, video_height);

	faces.clear();
	smile_finder.findSmiles(rimage, faces);

	delete [] fpixels;

	smile_averages[smile_index] = 0;
	float min_area = face_min_area * CAMERA_WIDTH;

	list<VisualObject *>::iterator face = faces.begin();
	for(int i = 0; face != faces.end(); ++face, i++)
	{
		FaceObject *fo = static_cast<FaceObject*>(*face);

		float area = fo->xSize;
		if (area < min_area)
		{
			faces.erase(face);
			continue;
		}

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

		if (happiness > HAPPINESS_THRESHOLD)
		{
			ofSetColor(0xdd00ee);
		}
		else
		{
			ofSetColor(0xddf010);
		}
		ofRect(fo->x, fo->y, fo->xSize, fo->ySize);

		snprintf(str, 256, "%3.2f", fo->activation);
		font.drawString(str, fo->x + 5, fo->y + font.getLineHeight());

		snprintf(str, 256, "%3.2f", happiness);
		font.drawString(str, fo->x + 5, fo->y + 2 * font.getLineHeight());
	}
	glPopMatrix();

	gui_happy->set(happiness);
}

void testApp::save_photo(bool force)
{
	static float last_time = -MIN_PHOTO_SAVE_PERIOD;
	static int index = 0;
	static int last_sec = 0;

	struct tm tm;
	time_t ltime;
	char datename[256];

	float curr_time = ofGetElapsedTimef();

	if ((curr_time > last_time + MIN_PHOTO_SAVE_PERIOD) || force)
	{
		time(&ltime);
		localtime_r(&ltime, &tm);
		if (last_sec != tm.tm_sec)
			index = 0;
		sprintf(datename, "cheese-%02d%02d%02d%02d%02d%02d%02d.png", tm.tm_year-100,
				tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, index);

		imgsaver.setFromPixels(video_grabber.getPixels(), CAMERA_WIDTH, CAMERA_HEIGHT,
				OF_IMAGE_COLOR, rgb_bgr);
		imgsaver.saveImage(datename);

		last_time = curr_time;
	}
}

void testApp::update()
{
	static float face_start_time;

	// update video grabber
	video_grabber.update();
	if (video_grabber.isFrameNew())
	{
		video_texture.loadData(video_grabber.getPixels(),
				video_grabber.getWidth(), video_grabber.getHeight(), GL_RGB);
	}

	detect_smile();

	float time = ofGetElapsedTimef();
	if (faces.size() == 0)
	{
		if ((time - last_face_time) >= NO_FACE_THRESHOLD)
		{
			face_detected = false;
			face_period = 0;
		}
	}
	else // faces.size() > 0
	{
		last_face_time = time;

		if (face_detected)
		{
			face_period = max(.0, FACE_PERIOD - (time - face_start_time));
		}
		else
		{
			face_detected = true;
			face_period = FACE_PERIOD;
			face_start_time = time;
		}
	}

	gui_face_detected->set(face_detected);
	gui_face_period->set(face_period);

	if ((happiness > HAPPINESS_THRESHOLD) || take_photo)
	{
		save_photo(take_photo);
		take_photo = false;
	}

	if (!disable_serial)
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

