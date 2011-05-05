#include "game.h"

void loadAllFonts(FontMap &f) {
	dout << "--- loadAllFonts" << endl;
	FILE *fin = fopen("fonts.lst", "r");
	if(!fin) {
		dout << "ERROR- Could not open file fonts.lst" << endl;
		exit(1);
	}
	int loadedFonts = 0;

	while(IsNextString(fin, "fonts")) {
		int id = LoadInt(fin);
		int color = LoadInt(fin);
		int mask = LoadInt(fin);
		string fontFilePath = LoadString(fin);
		convertPathInPlace(fontFilePath);
		if(!IsNextString(fin, "end_fonts")) {
			dout << "ERROR- Could not find 'end_fonts' tag." << endl;
			exit(1);
		}
		char buf[256];
		sprintf(buf, "color: 0x%8.8X, mask: 0x%8.8X",  color, mask);
		dout << " loading fonts: " << id << ", from: '" << fontFilePath << "', " << buf << endl;
		Font_Struct *font = new Font_Struct;
		if(font->initFont(fontFilePath, color, mask)) {
			dout << "ERROR- could not load font..." << endl;
			exit(1);
		}
		if(f.find(id) != f.end()) {
			dout << "ERROR- font with id: " << id << " already exists" << endl;
			exit(1);
		}
		f.insert(make_pair<int, Font_Struct*>(id, font));
		dout << " loaded ok" << endl;
		loadedFonts++;

		// Little hack, while no configuration is possible.
		if(id == 0) {
			normalFont = font;
			debugFont = font;
			dialogNormalText = font;
		} else if(id == 1) {
			dialogVisitedText = font;			
		} else if(id == 2) {
			dialogSelectedText = font;
			dialogAnswerText = font;
		} else if(id == 3) {
			dialogQuestionText = font;
		}
	}
	fclose(fin);
	dout << " totally loaded: " << loadedFonts << " fonts" << endl;
	dout << "--- End loadAllFonts" << endl;
}
