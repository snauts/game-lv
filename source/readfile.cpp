#include "game.h"

static map<string, string> engineSymbolsMap;

int getLine(FILE *fin, string &str);
int getFormatLine(FILE *fin, string &str);
int getFormatLineMapped(FILE *fin, string &str, map<string, string> &symbolsMap);
void formatEngineString(string &tmpstr);

int loadSymbolsMap() {
	dout << "--- loadSymbolsMap" << endl;
	string tmpstr;
	int a;
	char ch;

	FILE *fin = fopen("symbols.lst", "r");
	if(!fin) {
		dout << "WARNING- could not open file symbols.lst: " << strerror(errno) << endl;
		dout << "  No translation symbols loaded" << endl;
		dout << "--- END loadSymbolsMap" << endl;
		return 1;
	}

	while(!getFormatLine(fin, tmpstr)) {
		for(a = 0; a < tmpstr.length(); a++) {
			ch = tmpstr[a];
			if(ch == ' ' || ch == '\t') break;
		}
		if(a == tmpstr.length()) {
			dout << "  Invalid string" << endl;
		} else {
			string key = tmpstr.substr(0, a);
			string value = tmpstr.substr(a);
			formatEngineString(key);
			formatEngineString(value);
			if((key.length() == 0) || (value.length() == 0)) {
				dout << "  Invalid key or value string" << endl;
				dout << "    key: '" << key << "'" << endl;
				dout << "    value: '" << value << "'" << endl;
			} else {
				engineSymbolsMap.insert(make_pair<string, string>(key, value));
				dout << "    '" << key <<"' -> '" << value << "'" << endl;
			}
		}
	}
	fclose(fin);
	dout << "  Total: " << engineSymbolsMap.size() << " translation symbols loaded" << endl;
	dout << "--- END loadSymbolsMap" << endl;
	return 0;
}

int getLine(FILE *fin, string &str) {
	static char strbuf[512];

	memset(strbuf, 0, 512);
	if(fgets(strbuf, 512, fin) == NULL) {
		if(feof(fin)) {
			return 1;
		} else {
			return 2;
		}
	}
	str = strbuf;
//	dout << "      getLine returned: '" << str << "'" << endl;
	return 0;
}

void formatEngineString(string &tmpstr) {
	int a;
	char ch;
	bool ok = false;

	// Remove spaces, tabs, cr, lf from left
	for(a = 0; a < tmpstr.length(); a++) {
		ch = tmpstr[a];
		if((ch == ' ') || (ch == '\t')
			|| (ch == '\r') || (ch == '\n')) continue;
		ok = true;
		tmpstr = tmpstr.substr(a);
		break;
	}

	if(!ok) {
		tmpstr = "";
		return;
	}

	// Remove comments
	bool stringEnded = true;
	for(a = 0; a < tmpstr.length(); a++) {
		ch = tmpstr[a];
		if(ch == '#' && stringEnded) {
			if(a > 0) {
				tmpstr = tmpstr.substr(0, a);
			} else {
				tmpstr = "";
			}
			break;
		}
		if(ch == '"') {
			stringEnded = !stringEnded;
		}
	}
	

	ok = true;
	// Remove spaces, tabs, cr, lf from right
	for(a = tmpstr.length() - 1; a >= 0; a--) {
		ch = tmpstr[a];
		if((ch == ' ') || (ch == '\t')
			|| (ch == '\r') || (ch == '\n')) continue;
		ok = true;
		tmpstr = tmpstr.substr(0, a + 1);
		break;
	}
	
	if(!ok) {
		tmpstr = "";
		return;
	}
}

int getFormatLine(FILE *fin, string &str) {
	bool doLoop = true;
	int ret;
	string tmpstr;
	while(doLoop) {
		ret = getLine(fin, tmpstr);
		if(ret) return ret;

		formatEngineString(tmpstr);

		// Check whether there something is left
		if(tmpstr.length() <= 0) continue;
		str = tmpstr;
//		dout << "    getFormatLine returned: '" << tmpstr << "'" << endl;
		doLoop = false;
	}
	return 0;
}

int getFormatLineMapped(FILE *fin, string &str, map<string, string> &symbolsMap) {
	string tmpstr;
	int ret = getFormatLine(fin, tmpstr);
	if(ret) return ret;
	map<string, string>::iterator i = symbolsMap.find(tmpstr);
	if(i != symbolsMap.end()) {
		dout << "    MAPPED STRING: '" << tmpstr << "' -> '" << i->second << "'" << endl;
		tmpstr = i->second;
	}
	str = tmpstr;
	return 0;
}



int LoadInt(FILE* fin) {
	string tmpstr;
	int ret = getFormatLineMapped(fin, tmpstr, engineSymbolsMap);
	if(ret) {
		dout << "ERROR- Could not load int" << endl;
		exit(1);
	}
	char ch;
	for(int a = 0; a < tmpstr.length(); a++) {
		ch = tmpstr[a];
		if(((ch >= '0') && (ch <= '9')) || (ch == '-')) continue;
		dout << "ERROR- Invalid string: '" << tmpstr << "' when integer value expected" << endl;
		exit(1);
	}
	return atol(tmpstr.c_str());
}

string LoadString(FILE* fin, bool replaceSpecialSymbols) {
	static char buf[1024];
	string tmpstr;
	int ret = getFormatLineMapped(fin, tmpstr, engineSymbolsMap);

	if(ret) {
		dout << "ERROR- Could not load string, reason: " << ret << endl;
		exit(1);
	}

	if(tmpstr.length() < 2) {
		dout << "ERROR- '" << tmpstr << "'" << endl;
		dout << "ERROR- Could not load string, invalid length" << endl;
		exit(1);
	}

	if((tmpstr[0] != '"') || (tmpstr[tmpstr.length() - 1] != '"')) {
		dout << "ERROR- '" << tmpstr << "'" << endl;
		dout << "ERROR- Could not load string, invalid format" << endl;
		exit(1);
	}

	if(replaceSpecialSymbols) {
		int pos = 1;
		int bufPos = 0;
		int endPos = tmpstr.length() - 1;
		const char *strBuf = tmpstr.c_str();
		char c;
		char cc;
		char a;
		int val1;
		int val2;
		while(pos < endPos) {
			if((a = strBuf[pos]) == '\\') {
				c = strBuf[pos+1];
				if(c == 'n') {
					a = '\n';
					pos++;
				} else if(c == 'r') {
					a = '\r';
					pos++;
				} else if(c == 't') {
					a = '\t';
					pos++;
				} else if(c == '\\') {
					a = '\\';
					pos++;
				} else if((c >= '0' && c <= '9') 
					|| (c >= 'a' && c <= 'f')
					|| (c >= 'A' && c <= 'F')) {
					cc = strBuf[pos+2];
					if((cc >= '0' && cc <= '9') 
						|| (cc >= 'a' && cc <= 'f')
						|| (cc >= 'A' && cc <= 'F')) {
						
						if(c >= '0' && c <= '9') {
							val1 = c - '0';
						} else if(c >= 'A' && c <= 'F') {
							val1 = c - 'A' + 10;
						} else if(c >= 'a' && c <= 'f') {
							val1 = c - 'a' + 10;
						}					
						
						if(cc >= '0' && cc <= '9') {
							val2 = cc - '0';
						} else if(cc >= 'A' && cc <= 'F') {
							val2 = cc - 'A' + 10;
						} else if(cc >= 'a' && cc <= 'f') {
							val2 = cc - 'a' + 10;
						}					
						a = val1 << 4 | val2;
						pos+=2;
					}
				}
			}
			buf[bufPos] = a;
			pos++;
			bufPos++;
		}
		buf[bufPos] = 0;
		return string(buf);
	}
	return tmpstr.substr(1, tmpstr.length() - 2);
}

bool ByPassString(FILE* fin, const string& str) {
	string tmpstr;
	while(!getFormatLineMapped(fin, tmpstr, engineSymbolsMap)) {
		if(tmpstr == str) {
			return true;
		}
	}
	return false;
}

bool IsNextString(FILE* fin, const string& str, bool bypass) {
	if(feof(fin)) {
		return false;
	}
	unsigned curr_pos = ftell(fin);

	string tmpstr;
	int ret = getFormatLineMapped(fin, tmpstr, engineSymbolsMap);
	if(ret) return false;

	bool res = true;
	if(str != tmpstr) res = false;

	if(!bypass || !res) fseek(fin, curr_pos, SEEK_SET);

	return res;
}

#ifdef unix
string convertPath(const string &str) {
    string tmp;
    tmp.reserve(str.size());
    for (unsigned i = 0; i < str.size(); i++) {
	if (str[i] == '\\') tmp.push_back('/'); else tmp.push_back(str[i]);
    }
    return tmp;
}
string &convertPathInPlace(string &str) {
    for (unsigned i = 0; i < str.size(); i++) {
	if (str[i] == '\\') str[i] = '/';	
    }
    return str;
}
char *convertPathInPlace(char *str) {
    for (unsigned i = 0; i < strlen(str); i++) {
	if (str[i] == '\\') str[i] = '/';	
    }
    return str;
}

#include <dirent.h>
int loadListFile(const char *fileName, vector<string> &fileNames) {
    FILE* lstfile = fopen(fileName, "r");
    if(!lstfile) {
	dout << "ERROR- could not open file '" << fileName << "'" << endl;
	exit(1);
    }
    dout << " Loading file '" << fileName << "'" << endl;
    char str[256];
    char pattern[256];
    fileNames.clear();	
    while(fgets(str, 256, lstfile)) {
	TrimWhitespaces(str);
	convertPathInPlace(str);
	if(strlen(str) <= 0) continue;
	if(!(str[strlen(str) - 1] == '/' || str[strlen(str) - 1] == '\\')) {
	    dout << "  File '" << str << "' added" << endl;
	    fileNames.push_back(str);
	} else {
	    DIR *dir = opendir(str);
	    if (!dir) {
		dout << "  ERROR: failed to open directory " << str << endl;
	    }
	    else {
		dout << "  Adding files from: '" << str << "'" << endl;
		struct dirent *de;
		while ((de = readdir(dir))) {
		    if (de->d_type == DT_DIR) continue;
		    dout << "    File '" << de->d_name << "' added" << endl;
		    fileNames.push_back(string(str) + de->d_name);
		}
		closedir(dir);
	    }	    
	}
    }
    fclose(lstfile);
    dout << " File '" << fileName << "' loaded" << endl;
    return 0;
}
#else
int loadListFile(const char *fileName, vector<string> &fileNames) {
	FILE* lstfile = fopen(fileName, "r");
	if(!lstfile) {
		dout << "ERROR- could not open file '" << fileName << "'" << endl;
		exit(1);
	}
	dout << " Loading file '" << fileName << "'" << endl;
	char str[256];
	char pattern[256];

	fileNames.clear();
	
	while(fgets(str, 256, lstfile)) {
		TrimWhitespaces(str);
		if(strlen(str) <= 0)
			continue;
		if(!(str[strlen(str) - 1] == '/' || str[strlen(str) - 1] == '\\')) {
			dout << "  File '" << str << "' added" << endl;
			fileNames.push_back(str);
		} else {
			strcpy(pattern, str);
			strcat(pattern, "*.*");
			_finddata_t fileInfo;
			long fileHandle = _findfirst(pattern, &fileInfo);
			if(fileHandle == -1) {
				dout << "  Error while searching for files matching: '" << pattern << "', " << strerror(errno) << endl;
			} else {
				dout << "  Adding files matching pattern: '" << pattern << "'" << endl;
				do {
					if(!(fileInfo.attrib & _A_SUBDIR)) {
						string fullFileName = string(str) + fileInfo.name;
						dout << "    File '" << fullFileName << "' added" << endl;
						fileNames.push_back(fullFileName);
					}
				} while (!(_findnext(fileHandle, &fileInfo)));
				_findclose(fileHandle);
			}
			
		}
	}
	fclose(lstfile);
	dout << " File '" << fileName << "' loaded" << endl;
	return 0;
}
#endif
