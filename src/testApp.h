#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"

#include "ofMain.h"
#include "ofxSimpleGuiToo.h"

#include "smile.h"

#define CAMERA_WIDTH 320
#define CAMERA_HEIGHT 240

#define CAMERA_WIDTH_SCALED 500

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

		// smile
		MPSmile smile_finder;
		VisualObject faces;
		void detect_smile();
		void draw_smiles(float x, float y, float w, float ow);

		// font
		ofTrueTypeFont font;
};

#endif

