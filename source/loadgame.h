#ifndef __LOADGAME_H__
#define __LOADGAME_H__

int readInt(FILE *fin);
string readString(FILE *fin);
double readDouble(FILE *fin);

int loadGame(string filePrefix = "00_");

#endif
