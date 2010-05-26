#ifndef _TEST_APP
#define _TEST_APP

#include <list>

#include "ofMain.h"

#include "ofMain.h"
#include "ofxSimpleGuiToo.h"
#include "ofxDirList.h"
#include "ofxSoundPlayer.h"

#include "smile.h"

#define CAMERA_WIDTH 640
#define CAMERA_HEIGHT 480

#define CAMERA_WIDTH_SCALED 640

// number of frames acculating smile value
#define SMILE_AVERAGES 8

// minimum period of time for saving photos in seconds
#define MIN_PHOTO_SAVE_PERIOD 30.0

#define FACE_PERIOD 15.0
#define NO_FACE_THRESHOLD 0.5

// minimum time period for sending messages
#define MIN_MESSAGE_PERIOD 30.0

// sounds
#define SOUNDS_SMILE "sounds/smile"
#define SOUNDS_FACE "sounds/face"

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
		float gui_happiness_dummy;

		ofxSimpleGuiToggle *gui_face_detected;
		bool face_detected;
		bool gui_face_detected_dummy;

		float face_period_max;
		float face_period;
		float gui_face_period_dummy;
		ofxSimpleGuiSliderFloat *gui_face_period;

		float max_happiness; // maximum happiness during a certain time period
		float gui_max_happiness_dummy;
		ofxSimpleGuiSliderFloat *gui_max_happiness;

		bool take_photo;
		bool rgb_bgr;
		bool disable_serial; // disables serial messages

		// trigger manual messages
		bool message1;
		bool message2;
		bool message3;

		// smile limits
		ofxSimpleGuiSliderFloat *gui_limit1;
		float limit1;
		ofxSimpleGuiSliderFloat *gui_limit2;
		float limit2;
		ofxSimpleGuiSliderFloat *gui_limit3;
		float limit3;

		// smile
		MPSmile smile_finder;
		VisualObject faces;
		void detect_smile();
		void draw_smiles(float x, float y, float w, float ow);

		float smile_averages[SMILE_AVERAGES];
		int smile_index;
		float happiness;

		float last_face_time; // last time a face is detected

		// font
		ofTrueTypeFont font;

		// image save
		void save_photo(bool force = false);
		ofImage imgsaver;

		// serial
		ofSerial serial;
		void send_arduino_message();
		void send_arduino_message(int i);
		bool serial_inited;

		// arduino messages
		static string messages[];

		void update_timestamp();
		char timestamp[256];

		// sounds
		void load_samples();
		void free_samples();
		void play_sample(ofxSoundPlayer **sounds, int n);

		int sounds_smile_count;
		ofxSoundPlayer **sounds_smile;
		int sounds_face_count;
		ofxSoundPlayer **sounds_face;

		ofxDirList dirlist;
};

#endif

