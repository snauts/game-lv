#include "game.h"
#include "structs.h"

unsigned startDialog(Dialog *d, int firstQuestionId) {
	int first = d->firstQuestionId;
	if(firstQuestionId != -1) first = firstQuestionId;
	map<int, DialogQuestion>::iterator qi = d->questions.find(first);
	currentDialogQuestion = &qi->second;
	currentDialogAnswer = NULL;
	currentDialog = d;

	if(current_game_state == GAME_STATE_NORMAL_DISABLED) {
		enterGameState(GAME_STATE_DIALOG_DISABLED);
	} else {
		enterGameState(GAME_STATE_DIALOG);
	}

	state_var1 = DIALOG_SHOULD_START_DIALOG;

	//neapklusina muuziku dialoga laikaa (barvins)
	//musicPlayer.doSpecialEffect(MUSIC_FADE_TO_VOLUME, 100, 30);

	return 0;
}

bool IsValidDialog(Dialog *d) {
	map<int, DialogQuestion>::iterator i = d->questions.find(d->firstQuestionId);
	map<int, DialogQuestion>::iterator answer_id;
	DialogQuestion *q;
	if(i == d->questions.end()) {
		dout << "ERROR: dialog: " << d->id << " has ivalid first question id: " << d->firstQuestionId << endl;
		exit(1);
//		return false;
	}
	i = d->questions.begin();
	while(i != d->questions.end()) {
		q = &i->second;
		for(int a = 0; a < q->answers.size(); a++) {
			answer_id = d->questions.find(q->answers[a].nextQuestionId);
			if(answer_id == d->questions.end()) {
				dout << "ERROR: in dialog " << d->id << endl << "  question: " << q->questions[0] << endl << ",  answer: " 
					<< q->answers[a].answerChoose[0] << endl << "  refers to non-existing dialog question: " 
					<< q->answers[a].nextQuestionId << endl;
				exit(1);
//				return false;
			}
		}
		if((q->answers.size() == 0) && (q->nextDialogQuestion != -1)){
			if(d->questions.find(q->nextDialogQuestion) == d->questions.end()) {
				dout << "ERROR: in dialog " << d->id << endl << "  question: " << q->id << endl 
					<< "  non existing next question id: " << q->nextDialogQuestion << endl;
				exit(1);
//				return false;
			}
		}
		i++;
	}
	return true;
}

Dialog *LoadDialog(FILE *fin) {
	if(!IsNextString(fin, "dialog")) {
		dout << " Loading dialog failed- no 'dialog' tag found." << endl;
		return NULL;
	}
	
	Dialog *d = new Dialog;
	d->id = LoadInt(fin);
	d->firstQuestionId = LoadInt(fin);
	while(IsNextString(fin, "dialog_question")) {
		DialogQuestion q;
		q.id = LoadInt(fin);
		q.inScriptId = LoadInt(fin);
		q.outScriptId = LoadInt(fin);
		q.questionSelectMode = LoadInt(fin);
		q.enabledAnswers = 0;
		q.nextDialogQuestion = -1;
		
		do {
			string question = LoadString(fin, true);
			string soundFile = LoadString(fin);
			q.questions.push_back(question);
			q.soundFiles.push_back(soundFile);
		} while(!IsNextString(fin, "dialog_answer", false) 
			&& !IsNextString(fin, "end_dialog_question", false) 
			&& !IsNextString(fin, "next_dialog_question", false)
			&& !feof(fin));

		while(IsNextString(fin, "dialog_answer")) {
			DialogAnswer a;
			a.nextQuestionId = LoadInt(fin);
			a.inScriptId = LoadInt(fin);
			a.outScriptId = LoadInt(fin);
			a.answerSelectMode = LoadInt(fin);
			do {
				string answer = LoadString(fin, true);
				string choose, real;

				if(!parseSayString(answer, choose, real)) {
					real = choose;
				}

				string soundFile = LoadString(fin);
				a.answerChoose.push_back(choose);
				a.answerReal.push_back(real);
				a.soundFiles.push_back(soundFile);
			} while(!IsNextString(fin, "end_dialog_answer", false) 
				&& !IsNextString(fin, "dialog_answer_state", false) 
				&& !feof(fin));

			if(IsNextString(fin, "dialog_answer_state")) {
				a.id = LoadInt(fin);
				a.enabled = (LoadInt(fin) > 0);
				if(!IsNextString(fin, "end_dialog_answer_state")) {
					dout << "ERROR- no 'end_dialog_answer_state' tag found" << endl;
					exit(1);
				}
			}

			if(!IsNextString(fin, "end_dialog_answer")) {
				dout << "ERROR- No 'end_dialog_answer' tag found" << endl;
				exit(1);
			}
			if(a.enabled) q.enabledAnswers++;
			q.answers.push_back(a);
		}

		if(IsNextString(fin, "next_dialog_question")) {
			q.nextDialogQuestion = LoadInt(fin);
			if(!IsNextString(fin, "end_next_dialog_question")) {
				dout << "ERROR- No 'end_next_dialog_question' tag found" << endl;
				exit(1);
			}
		}



		if((q.answers.size() == 0) && (IsNextString(fin, "next_dialog_question"))) {
			q.nextDialogQuestion = LoadInt(fin);
			if(!IsNextString(fin, "end_next_dialog_question")) {
				dout << "ERROR: No 'end_next_dialog_question' tag found" << endl;
				exit(1);
			}
		}
		if(!IsNextString(fin, "end_dialog_question")) {
			dout << "ERROR: No 'end_dialog_question' tag found" << endl;
			exit(1);
		}
		d->questions.insert(make_pair<int, DialogQuestion>(q.id, q));
	}
	if(!IsNextString(fin, "end_dialog")) {
		dout << "ERROR: Loading dialog failed- no 'end_dialog' tag found." << endl;
		delete d;
		return NULL;
	}
	if(!IsValidDialog(d)) {
		delete d;
		return NULL;
	}
	return d;	
}


void LoadAllDialogs(DialogMap &m) {
	dout << "--- LoadAllDialogs" << endl;
	m.clear();
	
	vector<string> fileNames;
	loadListFile("dialogs.lst", fileNames);
	
	FILE* fin;
	char str[256];
	Dialog* d;
	for(int currFile = 0; currFile < fileNames.size(); currFile++) {
		strcpy(str, fileNames[currFile].c_str());
		fin = fopen(str, "r");
		if(!fin) {
			dout << "ERROR: 'dialog.lst' refers to invalid file: " << str << endl;
			exit(1);
		}
		dout << " Processing file: " << str << endl;
		while((d = LoadDialog(fin))) {
			if(m.find(d->id) != m.end()) {
				dout << "ERROR: dialog with id: " << d->id << "already exists. File: " << str << endl;
				exit(1);
			}
			m.insert(make_pair<unsigned, Dialog*>(d->id, d));
			dout << " Dialog with id: " << d->id << " loaded" << endl;
		}
		fclose(fin);
	}

	// Create dialog with id -3
	{
		dout << " Creating special dialog, with id -3" << endl;
		Dialog *tmpDialog = new Dialog;
		DialogQuestion tmpQuestion;
		
		tmpDialog->id = -3;
		tmpDialog->firstQuestionId = 1;
		tmpQuestion.id = 1;
		tmpQuestion.questions.push_back("");
		tmpQuestion.soundFiles.push_back("");
		tmpDialog->questions.insert(make_pair<int, DialogQuestion>(tmpQuestion.id, tmpQuestion));
		m.insert(make_pair<unsigned, Dialog*>(tmpDialog->id, tmpDialog));
	}
	
	// Create dialog with id -2
	{
		dout << " Creating special dialog, with id -2" << endl;
		Dialog *tmpDialog = new Dialog;
		DialogQuestion tmpQuestion;
		DialogQuestion tmpQuestionEnd;
		DialogAnswer tmpAnswer;
		
		tmpDialog->id = -2;
		tmpDialog->firstQuestionId = 1;
		tmpQuestion.id = 1;
		tmpQuestion.questions.push_back("");
		tmpQuestion.soundFiles.push_back("");
		tmpQuestionEnd.id = 2;
		tmpQuestionEnd.questions.push_back("");
		tmpQuestionEnd.soundFiles.push_back("");
		tmpAnswer.answerChoose.push_back("");
		tmpAnswer.answerReal.push_back("");
		tmpAnswer.soundFiles.push_back("");
		tmpAnswer.nextQuestionId = 2;
		tmpQuestion.answers.push_back(tmpAnswer);
		tmpQuestion.enabledAnswers = 1;
		tmpDialog->questions.insert(make_pair<int, DialogQuestion>(tmpQuestion.id, tmpQuestion));
		tmpDialog->questions.insert(make_pair<int, DialogQuestion>(tmpQuestionEnd.id, tmpQuestionEnd));
		m.insert(make_pair<unsigned, Dialog*>(tmpDialog->id, tmpDialog));
	}
	
	dout << " Special dialogs created" << endl;
	dout << "--- End LoadAllDialogs" << endl << endl;
}

int parseSayString(const string &str, string &part1, string &part2) {

	part1.clear();
	part2.clear();

	int ret = 0;

	int pos = str.find("##");

	if(pos != string::npos) {
		part1 = str.substr(0, pos);
		part2 = str.substr(pos+2);
		dout << "    part1: '" << part1 << "'" << endl;
		dout << "    part2: '" << part2 << "'" << endl;
		ret = 1;
	} else {
		part1 = str;
		dout << "    no split in: '" << str << "'" << endl;
	}

	return ret;
}
