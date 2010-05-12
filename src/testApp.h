#ifndef _TEST_APP
#define _TEST_APP

#include <list>

#include "ofMain.h"

#include "ofMain.h"
#include "ofxSimpleGuiToo.h"

#include "smile.h"

#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480

#define CAMERA_WIDTH_SCALED 640

// number of frames acculating smile value
#define SMILE_AVERAGES 8

// minimum period of time for saving photos in seconds
#define MIN_PHOTO_SAVE_PERIOD 30.0

#define HAPPINESS_THRESHOLD 0.5

class testApp : public ofBaseApp
{
	public:
		testApp();
		~testApp();

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

	private:
		// video grabber
		ofVideoGrabber video_grabber;
		int camera_id;

		// video source
		int video_width;
		int video_height;
		ofTexture video_texture;

		// gui
		ofxSimpleGuiContent *gui_video;
		ofxSimpleGuiSliderFloat *gui_happy;
		float face_min_area;
		float gui_happiness;

		// smile
		MPSmile smile_finder;
		VisualObject faces;
		void detect_smile();
		void draw_smiles(float x, float y, float w, float ow);

		float smile_averages[SMILE_AVERAGES];
		int smile_index;
		float happiness;

		// font
		ofTrueTypeFont font;

		// image save
		void save_photo();
		ofImage imgsaver;

		// serial
		ofSerial serial;
		void send_arduino_message();
		bool serial_inited;
};

#endif

