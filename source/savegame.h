#ifndef __SAVEGAME_H__
#define __SAVEGAME_H__

int saveGame(string filePrefix = "00_");

int saveInt(FILE *fout, int val);
int saveString(FILE *fout, const string &val);
int saveDouble(FILE *fout, double val);

#endif
