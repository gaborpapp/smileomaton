#include <stdlib.h>

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
	last_face_time(-FACE_PERIOD),
	sounds_smile_count(0),
	sounds_face_count(0)
{
}

testApp::~testApp()
{
	video_grabber.close();
	serial.close();

	free_samples();
}

void testApp::setup()
{
	ofSetLogLevel(OF_LOG_ERROR);

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
	gui.addSlider("Face period max", face_period_max, 0, FACE_PERIOD);
	gui_face_period = &gui.addSlider("Face period", gui_face_period_dummy, 0, FACE_PERIOD);

	gui.addButton("Take photo", take_photo).setSize(128, 20);
	gui.addToggle("RGB-BGR", rgb_bgr).setSize(128, 20);
	gui.addToggle("Disable Messages", disable_serial);

	gui_limit1 = &gui.addSlider("Limit A1", limit1, -10, 10);
	gui_limit2 = &gui.addSlider("Limit A2", limit2, -10, 10);
	gui_limit3 = &gui.addSlider("Limit A3", limit3, -10, 10);

	gui.addButton("Send A1", message1).setSize(128, 20);
	gui.addButton("Send A2", message2).setSize(128, 20);
	gui.addButton("Send A3", message3).setSize(128, 20);

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

	// setup sounds
	load_samples();
}

void testApp::free_samples()
{
	for(int i = 0; i < sounds_face_count; i++)
	{
		delete sounds_face[i];
	}
	delete [] sounds_face;

	for(int i = 0; i < sounds_smile_count; i++)
	{
		delete sounds_smile[i];
	}
	delete [] sounds_smile;
}

void testApp::load_samples()
{
	dirlist.allowExt("wav");
	dirlist.allowExt("ogg");
	dirlist.allowExt("flac");

	int n = dirlist.listDir(SOUNDS_FACE);

	sounds_face_count = n;
	sounds_face = new ofxSoundPlayer* [n];
	for(int i = 0; i < n; i++)
	{
		//cout << "loading " << " " << dirlist.getPath(i) << endl;
		sounds_face[i] = new ofxSoundPlayer;
		sounds_face[i]->loadSound(dirlist.getPath(i));
	}

	n = dirlist.listDir(SOUNDS_SMILE);

	sounds_smile_count = n;
	sounds_smile = new ofxSoundPlayer* [n];
	for(int i = 0; i < n; i++)
	{
		//cout << "loading " << " " << dirlist.getPath(i) << endl;
		sounds_smile[i] = new ofxSoundPlayer;
		sounds_smile[i]->loadSound(dirlist.getPath(i));
	}
}

void testApp::play_sample(ofxSoundPlayer **sounds, int n)
{
	int i = random() % n;
	sounds[i]->play();
}

// messages sent for each happiness threshold
//string testApp::messages[] = { "a1", "b1", "c1" };
string testApp::messages[] = { "a1", "a2", "a3" };

void testApp::send_arduino_message(int i)
{
	cout << timestamp << " sending " << messages[i] << endl;
	if (serial_inited)
		serial.writeBytes((unsigned char *)messages[i].c_str(), 2);
	else
		cout << "arduino not connected" << endl;

	/*
	char max_msg_count = '5'; // after a5, b5, c1, it restarts from a1, b1, c1
	char c = messages[i][1];
	if (c == max_msg_count)
		c = '1';
	else
		c++;
	messages[i][1] = c;
	*/
}

void testApp::send_arduino_message()
{
	static float last_msg_time = -MIN_MESSAGE_PERIOD;

	// happiness limits for the messages
	int limits[] = { limit1, limit2, limit3 };

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
	if (happiness_index >= 0)
	{
		if ((time - last_msg_time) > MIN_MESSAGE_PERIOD)
		{
			send_arduino_message(happiness_index);
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

		if (happiness > limit1)
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
	static bool one_chocolate_per_face = false;

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

	if (faces.size() == 0) // no faces detected
	{
		if ((time - last_face_time) >= NO_FACE_THRESHOLD)
		{
			if (face_detected) // face disappears
			{
				if (during_face_period) // send message if it hasn't been sent yet
				{
					during_face_period = false;
					if (one_chocolate_per_face)
					{
						arduino_msg_trigger = true;
						one_chocolate_per_face = false;
					}
				}
			}
			face_detected = false;
		}
	}
	else // faces in the picture
	{
		last_face_time = time;
		if (!face_detected) // first time a face shows up
		{
			play_sample(sounds_face, sounds_face_count);
			one_chocolate_per_face = true;
		}
		face_detected = true;
	}

	// start countdown to detect widest smile during period
	if (face_detected && (happiness >= limit1) && (!during_face_period))
	{
		play_sample(sounds_smile, sounds_smile_count);

		during_face_period = true;
		max_happiness = -10.0;
		face_period = face_period_max;
		face_start_time = time;
	}

	// countdown
	if (during_face_period)
	{
		face_period = max(.0f, face_period_max - (time - face_start_time));
		if (face_period > .0)
		{
			if (happiness > max_happiness)
				max_happiness = happiness;
		}
		else
		{
			if (one_chocolate_per_face)
			{
				arduino_msg_trigger = true;
				one_chocolate_per_face = false;
			}
		}
	}

	// update gui
	gui_face_detected->set(face_detected);
	gui_face_period->set(face_period);
	gui_max_happiness->set(max_happiness);

	// update face period slier
	if (!during_face_period)
	{
		gui_face_period->max = face_period_max;
		gui_face_period->setup();
	}

	// take photo if smile detected
	if ((happiness > limit1) || take_photo)
	{
		save_photo(take_photo);
		take_photo = false;
	}

	// send messages to arduino
	if (!disable_serial && arduino_msg_trigger)
	{
		send_arduino_message();
		arduino_msg_trigger = false;
	}

	// manual messages
	bool *msg_buttons[] = { &message1, &message2, &message3 };
	for (unsigned i = 0; i < sizeof(msg_buttons)/sizeof(msg_buttons[0]); i++)
	{
		bool *button = msg_buttons[i];
		if (*button)
		{
			send_arduino_message(i);
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

