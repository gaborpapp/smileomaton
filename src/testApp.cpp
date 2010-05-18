#include "testApp.h"

testApp::testApp() :
	camera_id(0),
	face_min_area(0),
	face_detected(false),
	face_period(0),
	take_photo(false),
	rgb_bgr(false),
	disable_serial(false),
	message1(false),
	message2(false),
	message3(false),
	message4(false),
	message5(false),
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

	gui_happy = &gui.addSlider("Happiness", gui_happiness_dummy, -6, 6);
	gui_happy->setNewColumn(true);

	gui_max_happiness = &gui.addSlider("Maximum", gui_max_happiness_dummy, -6, 6);

	gui.addSlider("Min face", face_min_area, 0.1, 1);
	gui_face_detected = &gui.addToggle("Face detected", gui_face_detected_dummy);
	gui_face_detected->setSize(168, 20);
	gui_face_period = &gui.addSlider("Face period", gui_face_period_dummy, 0, FACE_PERIOD);

	gui.addButton("Take photo", take_photo).setSize(128, 20);
	gui.addToggle("RGB-BGR", rgb_bgr).setSize(128, 20);
	gui.addToggle("Disable Messages", disable_serial);

	gui.addButton("Send A1", message1).setSize(128, 20);
	gui.addButton("Send A2", message2).setSize(128, 20);
	gui.addButton("Send A3", message3).setSize(128, 20);
	gui.addButton("Send A4", message4).setSize(128, 20);
	gui.addButton("Send A5", message5).setSize(128, 20);

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

// messages sent for each happiness threshold
string testApp::messages[] = { "a1", "a2", "a3", "a4", "a5" };

void testApp::send_arduino_message()
{
	static float last_msg_time = -MIN_MESSAGE_PERIOD;

	// happiness limits for the messages
	int limits[] = { 1, 2, 3, 4, 5 };

	const int limit_count = sizeof(limits)/sizeof(limits[0]);

	int happiness_index = -1;
	for (int i = limit_count - 1; i >= 0; i--)
	{
		if (max_happiness >= limits[i])
		{
			happiness_index = i;
			break;
		}
	}

	float time = ofGetElapsedTimef();
	if (serial_inited && (happiness_index >= 0))
	{
		if ((time - last_msg_time) > MIN_MESSAGE_PERIOD)
		{
			cout << timestamp << " sending " << messages[happiness_index] << endl;
			serial.writeBytes((unsigned char *)messages[happiness_index].c_str(), 2);
			last_msg_time = time;
		}
		else
		{
			cout << timestamp << " skipping message " << messages[happiness_index] << endl;
		}
	}
}

void testApp::detect_smile()
{
	int n = video_width * video_height;
	float *fpixels = new float[n];
	unsigned char *cpixels = video_grabber.getPixels();

	// r component for face detection
	int offset = rgb_bgr ? 2 : 0;
	for (int i = 0, j = 0; i < n; i++, j += 3)
	{
		fpixels[i] = (float)(cpixels[j + offset]);
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

void testApp::update_timestamp()
{
	static int index = 0;
	static int last_sec = 0;

	struct tm tm;
	time_t ltime;

	time(&ltime);
	localtime_r(&ltime, &tm);
	if (last_sec != tm.tm_sec)
		index = 0;
	sprintf(timestamp, "%02d%02d%02d%02d%02d%02d%02d", tm.tm_year-100,
			tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, index);
}

void testApp::save_photo(bool force)
{
	static float last_time = -MIN_PHOTO_SAVE_PERIOD;
	char datename[256];

	float curr_time = ofGetElapsedTimef();

	if ((curr_time > last_time + MIN_PHOTO_SAVE_PERIOD) || force)
	{

		imgsaver.setFromPixels(video_grabber.getPixels(), CAMERA_WIDTH, CAMERA_HEIGHT,
				OF_IMAGE_COLOR, rgb_bgr);
		sprintf(datename, "cheese-%s.png", timestamp);
		imgsaver.saveImage(datename);

		last_time = curr_time;
	}
}

void testApp::update()
{
	update_timestamp();

	static float face_start_time;
	static bool during_face_period = false; // during the period when maximum happiness is detected
	bool arduino_msg_trigger = false;

	// update video grabber
	video_grabber.update();
	if (video_grabber.isFrameNew())
	{
		video_texture.loadData(video_grabber.getPixels(),
				video_grabber.getWidth(), video_grabber.getHeight(), GL_RGB);
	}

	detect_smile();

	// check faces, maximum happiness for a time period
	float time = ofGetElapsedTimef();
	if (faces.size() == 0)
	{
		if ((time - last_face_time) >= NO_FACE_THRESHOLD)
		{
			face_detected = false;
			face_period = 0;

			if (during_face_period)
			{
				during_face_period = false;
				arduino_msg_trigger = true;
			}
			else
			{
				max_happiness = -10.0;
			}
		}
	}
	else // faces.size() > 0
	{
		last_face_time = time;

		if (face_detected)
		{
			face_period = max(.0, FACE_PERIOD - (time - face_start_time));
			if (face_period > .0)
			{
				if (happiness > max_happiness)
					max_happiness = happiness;
			}
			else
			{
				if (during_face_period)
				{
					during_face_period = false;
					arduino_msg_trigger = true;
				}
			}
		}
		else
		{
			during_face_period = true;
			max_happiness = -10.0;
			face_detected = true;
			face_period = FACE_PERIOD;
			face_start_time = time;
		}
	}

	gui_face_detected->set(face_detected);
	gui_face_period->set(face_period);
	gui_max_happiness->set(max_happiness);

	if ((happiness > HAPPINESS_THRESHOLD) || take_photo)
	{
		save_photo(take_photo);
		take_photo = false;
	}

	if (!disable_serial && arduino_msg_trigger)
	{
		send_arduino_message();
		arduino_msg_trigger = false;
	}

	// manual messages
	bool *msg_buttons[] = { &message1, &message2, &message3, &message4, &message5 };
	for (unsigned i = 0; i < sizeof(msg_buttons)/sizeof(msg_buttons[0]); i++)
	{
		bool *button = msg_buttons[i];
		if (*button)
		{
			unsigned char *msg = (unsigned char *)messages[i].c_str();
			cout << timestamp << " manually sending " << msg << endl;
			serial.writeBytes(msg, 2);
			*button = false;
		}
	}
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

