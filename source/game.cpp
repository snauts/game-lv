#include "game.h"
#include "gamestrings.h"
#include "structs.h"
#include "consts.h"

SDL_Surface  *screen = NULL;
debug_out dout("dout");
volatile long current_time = 0;
volatile bool isTimeTicking = true;
MusicPlayer musicPlayer;
SoundPlayer soundPlayer;

bool isSoundPossible = false;

unsigned current_game_state = GAME_STATE_NORMAL;
// Variables for currently game state (state specific)
int state_var1 = 0;
int state_var2 = 0;

InventorySkin *currentInventorySkin;

// Game state meanings for GAME_STATE_DIALOG
// state_var1 == 1
//	Question is showing and voice is playing.
//	state_var2 is time when answers will be shown or dialog will end, if there are no answers. (if there is speech
//	this value does not matter).
//
// state_var1 == 2
//	User is choosing answer.
//	state_var2 currently selected answer.
//
// state_var1 == 3
//	Answer is selected and voice is acting + only visible is this answer
//	state_var2 is time when next question will be shown (if there is speech this value does not matter).
//
// state_var1 == 4
//
// state_var1 == 5
//


ScriptMap all_scripts;
HotspotMap all_hotspots;
SceneMap all_scenes;
InventoryItemMap all_inventory_items;
DialogMap all_dialogs;
Character *main_character = NULL;
Scene *current_scene = NULL;
InventoryItemMap inventory;

Dialog *currentDialog = NULL;
DialogQuestion *currentDialogQuestion = NULL;
DialogAnswer *currentDialogAnswer = NULL;

int current_mouse_x = 0;
int current_mouse_y = 0;
int current_buttons = 0;

//const int scaleAmount = 10;
int currentViewX = 0;
int currentViewY = 0;

state_machine<string, int, debug_out> game_states;

map<int, ScriptTrigger> triggeredScripts;

bool should_draw_text = true;
list<TextItem> onscreen_text;

FontMap allFonts;
Font_Struct *normalFont = NULL;
Font_Struct *debugFont = NULL;
Font_Struct *dialogNormalText = NULL;
Font_Struct *dialogVisitedText = NULL;
Font_Struct *dialogSelectedText = NULL;
Font_Struct *dialogQuestionText = NULL;
Font_Struct *dialogAnswerText = NULL;


SDL_Surface *savedScreen = NULL;
GLuint savedScreenTexture = 0;
int savedScreenTextureW = 0;
int savedScreenTextureH = 0;
float savedScreenTextureX = 0;
float savedScreenTextureY = 0;

bool shouldFade = false;
int fadeMethod = 0;
int fadeSpeed = 0;
int fadeProgress = 0;

bool doCharacterAntialiasing = false;

string saveGameDir;

//SoundCollection *uiSound = NULL;
map<int, SoundCollection*> scriptSounds;

int maximalTimeSkip = 2000000; // Set this value BIG, so all will be skipped
bool debugConsole = false;
bool clearDebugConsole = false;

int showDebugOutput();

unsigned TimerCallback(unsigned interval) {
	if(isTimeTicking) current_time++;
//	dout << "time: " << current_time << endl;
	return interval;
}

unsigned ResumeGame() {
//	dout << "resumed in:" << current_time << endl;
	// Values for framerate fuction.
	bool drawSavedScreen = false;
	bool skipOneFrame = false;
	time_t st = time(NULL);
	int currentFps = 0;
	int fr = 0;
	// Event variable.
	resumeTime();

	SDL_Event e;
	while(1) {
		while(SDL_PollEvent(&e)) {
			switch(e.type) {

			case SDL_MOUSEMOTION:
				{
					current_mouse_x = e.motion.x;
					current_mouse_y = e.motion.y;
					currentInventorySkin->onMouse();
					break;
				}
				
			case SDL_MOUSEBUTTONDOWN:
				{
					int shouldExecuteScript = 0;
					if(e.button.button == 1) {
						current_buttons |= 1;
						shouldExecuteScript = currentInventorySkin->onLeftMouse();
					} else {
						current_buttons |= 2;
						shouldExecuteScript = currentInventorySkin->onRightMouse();
					}



					switch(current_game_state) {
					case GAME_STATE_NORMAL:
						{
							if(shouldExecuteScript != INVENTORY_NOTHING) {

								if(main_character->suspended_script) {
									cancelSuspendedScript();
								}
								
								switch(shouldExecuteScript) {
								case INVENTORY_COMBINE_ITEM:
									{
										int itemId;
										int onItemId;
										IIMI iimi;
										InventoryItem *item;
										InventoryItem *onItem;

										currentInventorySkin->getActionParams(itemId, onItemId);

										iimi = inventory.find(itemId);
										if(iimi == inventory.end()) {
											dout << "ERROR- Used item not in inventory: " << itemId << endl;
											exit(1);
										}
										item = iimi->second;

										iimi = inventory.find(onItemId);
										if(iimi == inventory.end()) {
											dout << "ERROR- Used on item that is not in inventory: " << itemId << endl;
											exit(1);
										}
										onItem = iimi->second;

										dout << "Use: '" << item->short_name << "'(" 
											<< item->id << ") on '" << onItem->short_name << "'(" 
											<< onItem->id << ")" << endl;
										
										multimap<int, int>::iterator scriptIterator = 
											onItem->scripts.find(item->id);
										
										if(scriptIterator != onItem->scripts.end()) {
											if(main_character->suspended_script) {
												cancelSuspendedScript();
											} else {
												stopCharacterMovement();
											}
										}
										
										while((scriptIterator != onItem->scripts.end()) && 
											(scriptIterator->first == item->id)) {
											executeScriptById(scriptIterator->second);
											scriptIterator++;
										}
										break;
									}
								case INVENTORY_USE_ITEM:
									{
										int itemId;
										int hotspotId;
										currentInventorySkin->getActionParams(itemId, hotspotId);
										HMI hmi = all_hotspots.find(hotspotId);
										if(hmi == all_hotspots.end()) {
											dout << "ERROR- could not find hotspot with id: " << hotspotId << endl;
											exit(1);
										}
										Hotspot *h = hmi->second;

										if(main_character->suspended_script) {
											cancelSuspendedScript();
										}
										
										SMI si;
										multimap<int, int>::iterator i;
										bool scriptExecuted = false;
										
										// Execute scripts associated with current inventory item									
										i = h->scripts.find(itemId);
										while((i != h->scripts.end()) && (i->first == itemId)) {
											if(!executeScriptById(i->second)) scriptExecuted = true;
											i++;
										}

										// No script executed, see whether there are "any inventory item" scripts 
										// for this hotspot available
										if(!scriptExecuted && itemId != ANY_INVENTORY_ITEM_ID) {
											i = h->scripts.find(ANY_INVENTORY_ITEM_ID);
											while((i != h->scripts.end()) && (i->first == ANY_INVENTORY_ITEM_ID)) {
												executeScriptById(i->second);
												i++;
											}
										}

										break;
									}
								case INVENTORY_MOVE_CHARACTER:
									{
										startCharacterMovement(current_mouse_x + currentViewX, current_mouse_y + currentViewY);
										break;
									}
								default:
									{
										dout << "ERROR- Unknown value returned from inventory:" << shouldExecuteScript << endl;
										exit(1);
										break;
									}
								}
							}
							break;
						}
					case(GAME_STATE_DIALOG):
						{
							if(state_var1 == DIALOG_CHOOSE_ANSWER) {
								if(current_buttons & 1) {
									if(currentDialogAnswer) {
										currentDialogAnswer->visited = true;
										state_var1 = DIALOG_IN_ANSWER_SCRIPT;
										executeScriptById(currentDialogAnswer->inScriptId);
									}
								}
							} else if((state_var1 == DIALOG_SHOW_QUESTION) || (state_var1 == DIALOG_SHOW_ANSWER)) {
								// Do time skip.
								state_var2 = current_time;
								soundPlayer.stopDialogSound();
							}
							break;
						}
					case(GAME_STATE_NORMAL_DISABLED):
						{
							break;
						}
					case(GAME_STATE_DIALOG_DISABLED):
						{
							if((state_var1 == DIALOG_SHOW_QUESTION) || (state_var1 == DIALOG_SHOW_ANSWER)) {
								// Do time skip.
								state_var2 = current_time;
								soundPlayer.stopDialogSound();
							}
							break;
						}
					}
					break;
				}
			case SDL_MOUSEBUTTONUP:
				{
					if(e.button.button == 1) {
						current_buttons &= ~1;
					} else {
						current_buttons &= 1;
					}
					break;
				}

			case SDL_KEYDOWN:
				{
					break;
				}

			case SDL_KEYUP:
				{
					if(currentInventorySkin->onKey(e.key.keysym.sym)) {
						// Inventory has processed key.
						break;
					}

					switch(e.key.keysym.sym) {
						case SDLK_r:
							{
								drawSavedScreen = !drawSavedScreen;
								if(drawSavedScreen) {
									glEnable(GL_TEXTURE_2D);
									glBindTexture(GL_TEXTURE_2D, savedScreenTexture);
									glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, savedScreenTextureW, savedScreenTextureH, 0);
									dout << "save screen: " << savedScreenTextureW  << ", " << savedScreenTextureH << endl;
								}
								break;
							}
						case SDLK_SPACE:
							{
								switch(current_game_state) {
								case GAME_STATE_NORMAL:
									{
										if(main_character->state == CHARACTER_WALK) {
											dout << "Want's to skip character movement in: " << current_time << endl;

											int maxWalkSkip = current_scene->walkMask->getTimeSkip();
											int maxTrigerScript = 0;
											int totalSkip = 0;

											if(maxWalkSkip > 0) {
												dout << "	walk skip possible till: " << maxWalkSkip << endl;
												if(maxWalkSkip > maximalTimeSkip) {
													maxWalkSkip = maximalTimeSkip;
												}
												maxWalkSkip += current_time;
											} else {
												dout << "	walk skip not possible (why?)" << endl;
											}

											map<int, ScriptTrigger>::iterator trigIter = triggeredScripts.begin();
											while(trigIter != triggeredScripts.end()) {
												ScriptTrigger &trig = trigIter->second;
												if(maxTrigerScript < trig.triggeredTill) {
													maxTrigerScript = trig.triggeredTill;
												}
												trigIter++;
											}
											if(maxTrigerScript) {
												dout << "	trigger skip possible till: " << maxTrigerScript << endl;
											} else {
												dout << "	no scripts currently triggered" << endl;
											}

											if(maxTrigerScript > maxWalkSkip) {
												totalSkip = maxTrigerScript;
											} else {
												totalSkip = maxWalkSkip;
											}

											if(totalSkip) {
												int diffTime = totalSkip - current_time;

												dout << "Time will be skipped till:" << totalSkip << endl;
												current_time = totalSkip;

												dout << "Skipping hotspot sound for: " << diffTime << " (1/100 sec)" << endl;

												int hotspotChannels[TOTAL_HOTSPOT_CHANNELS];
												map<unsigned, HotspotSoundInfo> hotspotSounds = soundPlayer.getCurrentlyPlayingHotspots();
												int count = hotspotSounds.size();
												int currChan = 0;
												if(count > 0) {
													map<unsigned, HotspotSoundInfo>::iterator hsi = hotspotSounds.begin();
													while(hsi != hotspotSounds.end()) {
														HotspotSoundInfo &info = hsi->second;
														hotspotChannels[currChan] = info.channel;
														dout << "	Should skip sound for hotspot: " << info.hotspotId 
															<< " on channel: " << info.channel << endl;
														currChan++;
														hsi++;
													}
												} else {
													dout << "	No hotspot sounds currently playing" << endl;
												}
											} else {
												dout << "Time won't be skipped" << endl;
											}

												
										}
										break;
									}
								case GAME_STATE_DIALOG:
									{
										if((state_var1 == DIALOG_SHOW_ANSWER) || (state_var1 == DIALOG_SHOW_QUESTION)) {
											// Do time skip.
											state_var2 = current_time;
											soundPlayer.stopDialogSound();
										}
										break;										
									}
								default:
									{
										break;
									}
								}
								break;
							}
						case SDLK_F5:
							{
								doCharacterAntialiasing = !doCharacterAntialiasing;
								break;
							}
						case SDLK_ESCAPE:
							{
								stopTime();
								return 0;
							}
						case SDLK_F10:
							{
								dout << "Immediate exit forced- 'q' pressed" << endl;
								exit(1);
								break;
							}
						case SDLK_F1:
							{
								debugConsole = !debugConsole;
								dout << "Debug on-screen output " << (debugConsole ? "enabled" : "disabled") << endl;
								break;
							}
						case SDLK_F2:
							{
								clearDebugConsole = true;
								break;
							}
					}
					break;
				}
			case SDL_QUIT:
				{
				    return (unsigned) -1;
				}
			case GAME_EVENT_END_CURRENT_GAME:
				{
					if(e.user.code == 0) {
						// Wants end current game
						return 1;
					} else if(e.user.code == 1) {
						// Wants to exit game
					    return (unsigned) -1;
					} else {
						dout << "ERROR- invalid game end code value passed to GAME_EVENT_END_CURRENT_GAME: " << e.user.code << endl;
						exit(1);
					}
				}
			}
		}
//		dout << "events processed in: " << current_time << endl;

		if(drawSavedScreen) {
			float dx = (float)screen->w / savedScreenTextureW;
			float dy = (float)screen->h / savedScreenTextureH;
			SDL_Rect r;
			r.x = 100;
			r.y = 100;
			r.w = 100;
			r.h = 200;
			myBlitTexture(savedScreenTexture, dx, dy, &r, &r, screen->w, screen->h);
			SDL_GL_SwapBuffers();
			continue;
		}


		// Check whenever some triggered script does not needs to be executed.
		if(!triggeredScripts.empty()) {
			map<int, ScriptTrigger>::iterator triggerIter = triggeredScripts.begin();
			int executedScripts = 0;
			while((triggerIter != triggeredScripts.end()) && (triggerIter->first <= current_time)) {
				ScriptTrigger &trigger = triggerIter->second;
				dout << "Triggered script execution. Id: " << trigger.scriptId
					 << ", type: " 
					 << (trigger.triggerType == SCRIPT_TRIGGER_INTERRUPT ? "Interuptable" : "Resume") 
					 << " should have expired in: " << trigger.triggeredTill 
					 << " current time: " << current_time << endl;

				SMI scriptIter = all_scripts.find(trigger.scriptId);
				if(scriptIter == all_scripts.end()) {
					dout << "Error could not trigger script: " << trigger.scriptId << " no such script" << endl;
					exit(1);
				} else {
					// Adjust game state if required.
					if(trigger.triggerType == SCRIPT_TRIGGER_INTERRUPT) {
						if((current_game_state == GAME_STATE_DIALOG) || (current_game_state == GAME_STATE_DIALOG_DISABLED)){
							dout << "Changing game state from (disabled) dialog -> normal " << endl;
							soundPlayer.stopDialogSound(); 
							currentDialog = NULL;
							currentDialogAnswer = NULL;
							currentDialogQuestion = NULL;
						}
						if(main_character->suspended_script) {
							cancelSuspendedScript();
						}
						if(main_character->state != CHARACTER_STAND) {
							dout << "Stopping character." << endl;
							stopCharacterMovement();
						} else {
							dout << "Character is already standing" << endl;
						}
						if(current_game_state != GAME_STATE_NORMAL) {
							dout << "Entering game state normal" << endl;
							enterGameState(GAME_STATE_NORMAL, 0, 0);
						} else {
							dout << "Game state is normal, so there is no need to change" << endl;
						}
					}
		
					// Execute script.
					Script *script = scriptIter->second;
					executeScript(script);
					dout << "Triggered script execution ended." << endl;
				}
				executedScripts++;
				triggerIter++;
			}
			if(executedScripts) {
				triggeredScripts.erase(triggeredScripts.begin(), triggerIter);
			}
		}

		// Executing walk scripts if needed & updating character.
		if(main_character->coord.inScript >= 0) {
			int scriptId = main_character->coord.inScript;
			main_character->coord.inScript = -1;
			dout << "Executing walk-in script: " << scriptId << endl;
			executeScriptById(scriptId);
		}
		UpdateCharacter();
		if(main_character->coord.outScript >= 0) {
			int scriptId = main_character->coord.outScript;
			main_character->coord.outScript = -1;
			dout << "Executing walk-out script: " << scriptId << endl;
			executeScriptById(scriptId);
		}

		if(main_character->suspended_script) {
			if(main_character->isActionComplete()) {
				completeSuspendedScript();
			}
		}
//		dout << "character updated: " << current_time << endl;

		// Scroll screen if needed.
		currentViewX = main_character->coord.x - (screen->w / 2);
		if(currentViewX < 0) {
			currentViewX = 0;
		} else if(currentViewX + screen->w >= current_scene->w) {
			currentViewX = current_scene->w - screen->w;
		}
		currentViewY = main_character->coord.y - (screen->h / 2);
		if(currentViewY < 0) {
			currentViewY = 0;
		} else if(currentViewY + screen->h >= current_scene->h) {
			currentViewY = current_scene->h - screen->h;
		}

		glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);

		if(!skipOneFrame) {
			// Draw scene.
			switch(current_game_state) {
			case(GAME_STATE_NORMAL):
				{
					DrawSceneNormal();
					break;
				}
			case(GAME_STATE_NORMAL_DISABLED):
				{
					DrawSceneNormalDisabled();
					break;
				}
			case(GAME_STATE_DIALOG):
				{
					DrawSceneDialog();
					break;
				}
			case(GAME_STATE_DIALOG_DISABLED):
				{
					DrawSceneDialogDisabled();
					break;
				}
			}
			if(shouldFade) {
				shouldFade = drawFadeEffect(fadeMethod, fadeSpeed, fadeProgress, savedScreen);
				//			dout << "DrawFadeEffect returned: " << (int)shouldFade << endl;
			}
			fr++;

			// Draw some debug info
#ifdef _DEBUG
			static char coord_buf[256];
			SDL_Rect dbgRect;
			int dbgHeight = 4;
			
			sprintf(coord_buf, "x: %3.3i, y: %3.3i (depth: %3.3i), scale: %3.2f", main_character->coord.x, 
				main_character->coord.y, 
				current_scene->h - current_mouse_y, main_character->coord.scaleCoefficient);
			debugFont->writeStr(screen, coord_buf, 4, dbgHeight, &dbgRect);
			dbgHeight += dbgRect.h;
			
			if(st != time(NULL)) {
				st = time(NULL);
				currentFps = fr;
				fr = 0;
			}
			sprintf(coord_buf, "FPS: %i", currentFps);
			debugFont->writeStr(screen, coord_buf, 4, dbgHeight, &dbgRect);
			dbgHeight += dbgRect.h;
			sprintf(coord_buf, "Scale: %3.2f", main_character->coord.scaleCoefficient);
			debugFont->writeStr(screen, coord_buf, 4, dbgHeight, &dbgRect);
			dbgHeight += dbgRect.h;
			sprintf(coord_buf, "View x: %3.3i, y: %3.3i", currentViewX, currentViewY); 
			debugFont->writeStr(screen, coord_buf, 4, dbgHeight, &dbgRect);
			dbgHeight += dbgRect.h;
			sprintf(coord_buf, "Mouse x: %3.3i, y: %3.3i", current_mouse_x, current_mouse_y); 
			debugFont->writeStr(screen, coord_buf, 4, dbgHeight, &dbgRect);
			dbgHeight += dbgRect.h;
			//		sprintf(coord_buf, "Time: %6.6i", current_time); 
			//		debugFont->writeStr(screen, coord_buf, 4, dbgHeight, &dbgRect);
			//		dbgHeight += dbgRect.h;
			
			Graph &g = current_scene->walkMask->wGraph();
			SDL_Rect r;
			for(int a = 0; a < g.nodeAmount(); a++) {
				GraphNode *n = g.node_pk(a);
				r.x = (n->x - 1) - currentViewX;
				r.y = (n->y - 1) - currentViewY;
				r.w = 3;
				r.h = 3;
				myFillRect(screen, &r, 0x3257);
			}
			
			if(debugConsole) {
				showDebugOutput();
			}
			
#endif
			SDL_GL_SwapBuffers();
			//dout << "Clear screen" << endl;
			//glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);

		} else {
			skipOneFrame = false;
		}
		
		//		dout << "state updated: " << current_time << endl;
	

		// Do some additional processing.
		if(current_game_state == GAME_STATE_DIALOG || current_game_state == GAME_STATE_DIALOG_DISABLED) {
			switch(state_var1) {
			case DIALOG_SHOW_QUESTION:
				{
					if((state_var2 < current_time) && (!soundPlayer.isDialogPlaying())) {
						// Ok, dialoga questions jau ir raadiits pietiekami ilgi.
						
						// Execute out-question script.
						state_var1 = DIALOG_OUT_QUESTION_SCRIPT;
						executeScriptById(currentDialogQuestion->outScriptId);
						skipOneFrame = true;
					}
					break;
				}
			case DIALOG_CHOOSE_ANSWER:
				{
					break;
				}
			case DIALOG_SHOW_ANSWER:
				{
					if((state_var2 < current_time) && (!soundPlayer.isDialogPlaying())) {
						// Ok answer has been shown.
						soundPlayer.stopDialogSound();
						setCharacterState(CHARACTER_STAND);
						SetCurrentCharacterAnimation(main_character);
						
						// Execute out-answer script.
						state_var1 = DIALOG_OUT_ANSWER_SCRIPT;
						executeScriptById(currentDialogAnswer->outScriptId);
						skipOneFrame = true;
					}
					break;
				}
			case DIALOG_IN_ANSWER_SCRIPT:
				{
					if((main_character->suspended_script) && 
						(main_character->suspended_script->id == currentDialogAnswer->inScriptId)) {
						// Script is still executing.
					} else {
						// Ok, lets diplay chosen answer and play sound.
						state_var1 = DIALOG_SHOW_ANSWER;
						
						// If text is long, don't display it too long.
						unsigned len = currentDialogAnswer->answerReal[currentDialogAnswer->currentAnswerIdx].length();

						if(len > 50) {
							state_var2 = current_time + len * 8;
						} else if((len > 0) && (len < 10)) {
							state_var2 = current_time + 100;
						} else {
							state_var2 = current_time + len * 10;
						}
						
						setCharacterState(CHARACTER_TALK);
						SetCurrentCharacterAnimation(main_character);
						soundPlayer.playDialogSound(currentDialogAnswer->soundFiles[currentDialogAnswer->currentAnswerIdx]);
						if(soundPlayer.isDialogPlaying()) {
							state_var2 = current_time;
						}
					}
					break;
				}
			case DIALOG_OUT_ANSWER_SCRIPT:
				{
					if((main_character->suspended_script) && 
						(main_character->suspended_script->id == currentDialogAnswer->outScriptId)) {
						// Script is still executing.
					} else {
						map<int, DialogQuestion>::iterator dialogIter = 
							currentDialog->questions.find(currentDialogAnswer->nextQuestionId);
						if(dialogIter == currentDialog->questions.end()) {
							dout << "ERROR- dialog answer refers to nonexisting question: "  
								<< currentDialogAnswer->nextQuestionId << endl;
							exit(1);
						}
						currentDialogQuestion = &(dialogIter->second);
						currentDialogAnswer = NULL;
						state_var1 = DIALOG_IN_QUESTION_SCRIPT;
						executeScriptById(currentDialogQuestion->inScriptId);
						skipOneFrame = true;
					}
					break;
				}
			case DIALOG_IN_QUESTION_SCRIPT:
				{
					if((main_character->suspended_script) && 
						(main_character->suspended_script->id == currentDialogQuestion->inScriptId)) {
						// Script is still executing.
					} else {
						// Ok, lets show dialog question.
						state_var1 = DIALOG_SHOW_QUESTION;
						switch(currentDialogQuestion->questionSelectMode) {
						case QUESTION_SELECT_FIRST:
							{
								currentDialogQuestion->currentQuestionIdx = 0;
								break;
							}
						case QUESTION_SELECT_LOOP:
							{
								currentDialogQuestion->currentQuestionIdx++;
								if(currentDialogQuestion->currentQuestionIdx >= currentDialogQuestion->questions.size()) {
									currentDialogQuestion->currentQuestionIdx = 0;
								}
								break;
							}
						case QUESTION_SELECT_RANDOM:
							{
								currentDialogQuestion->currentQuestionIdx = rand() % currentDialogQuestion->questions.size();
								break;
							}
						default:
							{
								dout << "ERROR- Unknown dialog question select mode: " 
									<< currentDialogQuestion->questionSelectMode << endl;
								exit(1);
							}
						}
						// If text is long, don't display it too long.
						unsigned len = currentDialogQuestion->questions[currentDialogQuestion->currentQuestionIdx].length();
						if(len > 50) {
							state_var2 = current_time + len * 8;
						} else if((len > 0) && (len < 10)) {
							state_var2 = current_time + 100;
						} else {
							state_var2 = current_time + len * 10;
						}
						
						soundPlayer.playDialogSound(currentDialogQuestion->soundFiles[currentDialogQuestion->currentQuestionIdx]);
						
						// If there is sound, then we show text only till there is sound playing.
						if(soundPlayer.isDialogPlaying()) {
							state_var2 = current_time;
						}
					}
					break;
				}
			case DIALOG_OUT_QUESTION_SCRIPT:
				{
					if((main_character->suspended_script) && 
						(main_character->suspended_script->id == currentDialogQuestion->outScriptId)) {
						// Script is still executing.
					} else { 
						// Script finished
						// Check whether this question has any answers to show.
						if(currentDialogQuestion->enabledAnswers <= 0) {
							
							// Maybe we should show next dialog question?
							if(currentDialogQuestion->nextDialogQuestion >= 0) {
								
								map<int, DialogQuestion>::iterator dialogIter = 
									currentDialog->questions.find(currentDialogQuestion->nextDialogQuestion);
								
								if(dialogIter != currentDialog->questions.end()) {
									// Should show next dialog question.
									
									currentDialogQuestion = &(dialogIter->second);
									currentDialogAnswer = NULL;
									
									state_var1 = DIALOG_IN_QUESTION_SCRIPT;
									executeScriptById(currentDialogQuestion->inScriptId);
									skipOneFrame = true;
									
									break;
								} else {
									dout << "ERROR- dialog: " << currentDialog->id 
										<< ", question: " << currentDialogQuestion->id 
										<< " has invalid next_question_id value: " 
										<< currentDialogQuestion->nextDialogQuestion 
										<< " Unknown question with such id" << endl;
									exit(1);
								}
							}
							
							// No answers & next question, so let's end dialog.
							currentDialogQuestion = NULL;
							currentDialog = NULL;
							currentDialogAnswer = NULL;

							musicPlayer.doSpecialEffect(MUSIC_FADE_TO_VOLUME, 100, 127);

							if(current_game_state == GAME_STATE_DIALOG_DISABLED) {
								enterGameState(GAME_STATE_NORMAL_DISABLED);
							} else {
								enterGameState(GAME_STATE_NORMAL);
							}
							
						} else {
							// Update answer texts
							for(int a = 0; a < currentDialogQuestion->answers.size(); a++) {
								DialogAnswer &answ = currentDialogQuestion->answers[a];
								switch(answ.answerSelectMode) {
								case ANSWER_SELECT_FIRST:
									{
										answ.currentAnswerIdx = 0;
										break;
									}
								case ANSWER_SELECT_LOOP:
									{
										answ.currentAnswerIdx++;
										if(answ.currentAnswerIdx >= answ.answerReal.size()) {
											answ.currentAnswerIdx = 0;
										}
										break;
									}
								case ANSWER_SELECT_RANDOM:
									{
										answ.currentAnswerIdx = rand() % answ.answerReal.size();
										break;
									}
								default:
									{
										dout << "ERROR- Invalid answer select mode in dialog: " << currentDialog->id 
											<< ", question id: " << currentDialogQuestion->id 
											<< ", answer: " << a << endl;
										exit(1);
									}
								}
								if(answ.currentAnswerIdx < 0 || answ.currentAnswerIdx >= answ.answerReal.size()) {
									dout << "ERROR- Invalid answer index generated in dialog: " << currentDialog->id 
										<< ", question id: " << currentDialogQuestion->id 
										<< ", answer: " << a << ", idx: " << answ.currentAnswerIdx << endl;
									exit(1);
								}
							}

							if(currentDialogQuestion->enabledAnswers == 1) {
								// If there is only 1 answer available, then select it automatically.
								DialogAnswer *answ;
								int a = 0;
								for(a = 0; a < currentDialogQuestion->answers.size(); a++) {
									answ = &(currentDialogQuestion->answers[a]);
									if(answ->enabled) {
										currentDialogAnswer = answ;
										break;
									}
								}
								if(a == currentDialogQuestion->answers.size()) {
									dout << "ERROR- Enabled dialog answers == 1, but no enabled answers found in dialog: " 
										<< currentDialog->id << endl;
									exit(1);
								}
								currentDialogAnswer->visited = true;
								
								state_var1 = DIALOG_IN_ANSWER_SCRIPT;
								executeScriptById(currentDialogAnswer->inScriptId);
								skipOneFrame = true;
							} else {
								// There are more than one answer, so let player choose.
								if(current_game_state == GAME_STATE_DIALOG_DISABLED) {
									enterGameState(GAME_STATE_DIALOG);
								}
								state_var1 = DIALOG_CHOOSE_ANSWER;
								state_var2 = 0;
							}
						}
						break;
					}
				}
			case DIALOG_SHOULD_START_DIALOG:
				{
					state_var1 = DIALOG_IN_QUESTION_SCRIPT;
					executeScriptById(currentDialogQuestion->inScriptId);
					skipOneFrame = true;
					break;
				}
			default:
				{
					dout << "FATAL ERROR- Unknown dialog state: " << state_var1 << endl;
					exit(1);
					break;
				}
			}
		}
		
		if(musicPlayer.isMusicEffectsOn) {
			musicPlayer.updateSpecialEffect();
		}

//		dout << "frame: " << fr << endl;
	}
	return 0; // Generally, execution thread should not come here, but anyway...
}
unsigned genericInit() {
	dout << "Game init" << endl;
	loadAllFonts(allFonts);
	
	// Checks, whether all fonts are ok, so there are no problems later.
	if(!normalFont) {
		dout << "ERROR- Normal font (id = 0) not loaded" << endl;
		exit(1);
	}
	if(!debugFont) {
		dout << "ERROR- Debug font (id = 0) not loaded" << endl;
		exit(1);
	}
	if(!dialogNormalText) {
		dout << "ERROR- Dialog normal font (id = 0) not loaded" << endl;
		exit(1);
	}
	if(!dialogVisitedText) {
		dout << "ERROR- Dialog visited font (id = 1) not loaded" << endl;
		exit(1);
	}
	if(!dialogSelectedText) {
		dout << "ERROR- Dialog selected font (id = 2) not loaded" << endl;
		exit(1);
	}
	if(!dialogQuestionText) {
		dout << "ERROR- Dialog question font (id = 3) not loaded" << endl;
		exit(1);
	}
	if(!dialogAnswerText) {
		dout << "ERROR- Dialog answer font (id = 3) not loaded" << endl;
		exit(1);
	}

	myFillRect(screen, NULL, 0);
	SDL_Rect r;
	normalFont->strRect(STRING_GAME_INITIALIZING, &r);
	normalFont->writeStr(screen, STRING_GAME_INITIALIZING, (screen->w - r.w) / 2, (screen->h - r.h) / 2, NULL);

	SDL_GL_SwapBuffers();

	currentInventorySkin = new DefaultInventory;

	//	uiSound = loadSoundCollection("sound\\sound_ui");
	//	if(!uiSound) {
	//		dout << "WARNING- could not load sound\\sound_ui" << endl;
	//	}

	// Allocate fade surface.
	savedScreen = SDL_CreateRGBSurface(screen->flags, screen->w, screen->h,
		screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask,
		screen->format->Bmask, screen->format->Amask);

	if(!savedScreen) {
		dout << "ERROR- could not allocate tmp surface" << endl;
		exit(1);
	}

	savedScreenTextureW = nearestPow2(screen->w);
	savedScreenTextureH = nearestPow2(screen->h);

	glGenTextures(1, &savedScreenTexture);
	glBindTexture(GL_TEXTURE_2D, savedScreenTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, savedScreenTextureW, savedScreenTextureH, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

	loadScriptSounds(scriptSounds);

	//paarvietots uz shejieni no newGameInit,
	//lai loadojot speeli muuzikas liste jau buutu ielaadeeta (barvins)
	musicPlayer.loadSongs("music.lst");  
	musicPlayer.setVolume(127);
	
	dout << "Game init completed" << endl << endl;
	return 0;
}
unsigned newGameInit() {
	dout << "New game init" << endl;

	myFillRect(screen, NULL, 0);
	
	SDL_Rect r;
	normalFont->strRect(STRING_STARTING_NEW_GAME, &r);
	normalFont->writeStr(screen, STRING_STARTING_NEW_GAME, (screen->w - r.w) / 2, (screen->h - r.h) / 2, NULL);

	SDL_GL_SwapBuffers();

	stopTime();

	current_time = 0;

	current_game_state = GAME_STATE_NORMAL;
	state_var1 = 0;
	state_var2 = 0;

	LoadAllScripts(all_scripts);
	LoadAllHotspots(all_hotspots);
	LoadAllScenes(all_scenes);
	LoadAllInventoryItems(all_inventory_items);
	LoadAllDialogs(all_dialogs);
	
	main_character = LoadCharacter("character.lst");
	if(!main_character) {
		dout << "Could not load main character\n" << endl;
		exit(0);
	}

	dout << "New game init completed" << endl << endl;
	return 0;
}

unsigned DrawBackground() {
	// Draws scene background.
	DrawAnimationClipped(screen, currentViewX, currentViewY, current_scene->gfx, 0, 0);
	return 0;
}

unsigned DrawSceneObjects() {
	// Draws and updates  hotspots & character animations
	Hotspot* h;
	int curAnimLoops;
	list<Hotspot*>::iterator i = current_scene->zbuflist.begin();
	while(i != current_scene->zbuflist.end()) {
		h = *i;
		if(h->depth < (current_scene->h - main_character->coord.y))
			break;
//		DrawAnimation(screen, h->anim, h->x, h->y);
		DrawAnimationClipped(screen, currentViewX, currentViewY, h->anim, h->x, h->y);
		if(h->restartSoundOnAnimLoop) {
			curAnimLoops = h->anim->currentLoops;
		}
		UpdateAnimation(h->anim);
		if(h->restartSoundOnAnimLoop && (curAnimLoops != h->anim->currentLoops)) {
			soundPlayer.playHotspotSound(h->id, h->sound);
		}
#ifdef _DEBUG
		SDL_Rect r;
		r.x = h->x - currentViewX;
		r.y = (current_scene->h - h->depth) - currentViewY;
		r.h = 1;
		r.w = h->w;
		myFillRect(screen, &r, 0xf800);
#endif
		i++;
	}

	// Draw all character animations.

	for(unsigned tmpVal = 0; tmpVal < main_character->currentAnimSize; tmpVal++) {
		if(main_character->visible) {

			// Draw character not scaled.
//			DrawAnimation(screen, main_character->current_anim[tmpVal], 
//				main_character->coord.x - currentViewX, main_character->coord.y - currentViewY);

			// Draw character scaled.
			DrawAnimationScaled(screen, main_character->current_anim[tmpVal], 
				main_character->coord.x - currentViewX, main_character->coord.y - currentViewY, 
				main_character->coord.scaleCoefficient, doCharacterAntialiasing);
		}
		UpdateAnimation(main_character->current_anim[tmpVal]);
	}
	
#ifdef _DEBUG
	SDL_Rect r;
	r.x = (main_character->coord.x - 1) - currentViewX;
	r.y = (main_character->coord.y - 1) - currentViewY;
	r.w = r.h = 3;
	myFillRect(screen, &r, 0xf800);
#endif
	
	while(i != current_scene->zbuflist.end()) {
		h = *i;
//		DrawAnimation(screen, h->anim, h->x, h->y);
		DrawAnimationClipped(screen, currentViewX, currentViewY, h->anim, h->x, h->y);
		if(h->restartSoundOnAnimLoop) {
			curAnimLoops = h->anim->currentLoops;
		}
		UpdateAnimation(h->anim);
		if(h->restartSoundOnAnimLoop && (curAnimLoops != h->anim->currentLoops)) {
			soundPlayer.playHotspotSound(h->id, h->sound);
		}
		
#ifdef _DEBUG
		SDL_Rect r;
		r.x = h->x - currentViewX;
		r.y = (current_scene->h - h->depth) - currentViewY;
		r.h = 1;
		r.w = h->w;
		myFillRect(screen, &r, 0xf800);
#endif
		i++;
	}
	return 0;
}

unsigned DrawOnscreenText() {
	list<TextItem>::iterator i = onscreen_text.begin();
	int current_y = 120;
	SDL_Rect r;
	while(i != onscreen_text.end()) {
		i->font->writeStr(screen, i->text.c_str(), 10, current_y, &r);
		current_y += r.h;
		if(i->expireTime < current_time) {
			i = onscreen_text.erase(i);
		}
		else {
			i++;
		}
	}
	if(onscreen_text.size() == 0) {
		should_draw_text = false;
	}
	return 0;
}

unsigned DrawSceneNormalDisabled() {

	DrawBackground();
	DrawSceneObjects();
	
	// Draws any text if there is one.
	if(should_draw_text) {
		DrawOnscreenText();
	}
	
	return 0;
}
unsigned DrawSceneNormal() {
	DrawSceneNormalDisabled();
	currentInventorySkin->drawSkin();
	currentInventorySkin->drawMouseCursor();
	return 0;
}

unsigned DrawDialogText() {
	SDL_Rect r;
	int current_y = 0;
	int start_x = 40;
	int start_y = 320;
	int delta;


	switch(state_var1) {
	case DIALOG_SHOW_QUESTION:
		{
			// Draws dialog question.
			if(currentDialogQuestion->questions[currentDialogQuestion->currentQuestionIdx].length()) {
				dialogQuestionText->strRect(currentDialogQuestion->questions[currentDialogQuestion->currentQuestionIdx].c_str(), &r);
				r.x = 0;
				r.y = 145;
				r.w = screen->w;
				r.h +=10;
//				myFillRect(screen, &r, 0x0f);
				dialogQuestionText->writeStr(screen, currentDialogQuestion->questions[currentDialogQuestion->currentQuestionIdx].c_str(), 40, 150, NULL);
			}
			break;
		}
	case DIALOG_CHOOSE_ANSWER:
		{
			currentDialogAnswer = NULL;

			// Draws dialog question.
			if(currentDialogQuestion->questions[currentDialogQuestion->currentQuestionIdx].length()) {
				dialogQuestionText->strRect(currentDialogQuestion->questions[currentDialogQuestion->currentQuestionIdx].c_str(), &r);
				r.x = 0;
				r.y = 145;
				r.w = screen->w;
				r.h +=10;
				dialogQuestionText->writeStr(screen, currentDialogQuestion->questions[currentDialogQuestion->currentQuestionIdx].c_str(), 40, 150, NULL);
			}

			// Draws answers

			for(unsigned a = 0; a < currentDialogQuestion->answers.size(); a++) {
				if(!currentDialogQuestion->answers[a].enabled) {
					continue;
				}
				DialogAnswer &answ = currentDialogQuestion->answers[a];
				if(answ.visited) {
					dialogVisitedText->strRect(answ.answerChoose[answ.currentAnswerIdx].c_str(), &r);
				} else {
					dialogNormalText->strRect(answ.answerChoose[answ.currentAnswerIdx].c_str(), &r);
				}
					
				delta = current_mouse_y - (start_y + current_y);
				if((delta >= 0) && (delta < r.h)) {
					dialogSelectedText->strRect(answ.answerChoose[answ.currentAnswerIdx].c_str(), &r);

					SDL_Rect col_rec;
					col_rec.x = 0;
					col_rec.y = start_y + current_y;
					col_rec.h = r.h;
					col_rec.w = screen->w;
//					myFillRect(screen, &col_rec, 0x4321);

					dialogSelectedText->writeStr(screen, answ.answerChoose[answ.currentAnswerIdx].c_str(), 
						start_x, start_y + current_y, &r);
					
					currentDialogAnswer = &(currentDialogQuestion->answers[a]);
				} else {
					if(currentDialogQuestion->answers[a].visited) {
						dialogVisitedText->writeStr(screen, answ.answerChoose[answ.currentAnswerIdx].c_str(), 
							start_x, start_y + current_y, &r);
					} else {
						dialogNormalText->writeStr(screen, answ.answerChoose[answ.currentAnswerIdx].c_str(), 
							start_x, start_y + current_y, &r);
					}
				}
				current_y += r.h + 1;
			}
			break;
		}
	case DIALOG_SHOW_ANSWER:
		{
			// Draws dialog answer.
			string &answ = currentDialogAnswer->answerReal[currentDialogAnswer->currentAnswerIdx];
			if(answ.length()) {
				dialogAnswerText->strRect(answ.c_str(), &r);
				r.x = 0;
				r.y = 145;
				r.w = screen->w;
				r.h +=10;
//				myFillRect(screen, &r, 0x0400);
				dialogAnswerText->writeStr(screen, answ.c_str(), 40, 150, &r);
			}
			break;
		}
	}
	return 0;
}

unsigned DrawSceneDialog() {
	unsigned ret = DrawSceneDialogDisabled();
	currentInventorySkin->drawMouseCursor();

	return ret;
}

unsigned DrawSceneDialogDisabled() {
	DrawSceneNormalDisabled();
	unsigned ret = DrawDialogText();
	return ret;
}

unsigned enterGameState(unsigned gameState, int var1, int var2) {
	current_game_state = gameState;
	state_var1 = var1;
	state_var2 = var2;
	return gameState;
}

unsigned resetGame() {

	dout << " resetGame" << endl;

	musicPlayer.stopMusic();
	soundPlayer.stopAll();

	delete main_character;
	main_character = NULL;

	current_game_state = 0xffff;
	
	SMI smi = all_scripts.begin();
	while(smi != all_scripts.end()) {
		delete smi->second;
		smi++;
	}
	all_scripts.clear();

	HMI hmi = all_hotspots.begin();
	while(hmi != all_hotspots.end()) {
		delete hmi->second;
		hmi++;
	}
	all_hotspots.clear();

	SCMI scmi = all_scenes.begin();
	while(scmi != all_scenes.end()) {
		delete scmi->second;
		scmi++;
	}
	all_scenes.clear();

	inventory.clear();
	
	IIMI iimi = all_inventory_items.begin();
	while(iimi != all_inventory_items.end()) {
		delete iimi->second;
		iimi++;
	}
	all_inventory_items.clear();

	DMI dmi = all_dialogs.begin();
	while(dmi != all_dialogs.end()) {
		delete dmi->second;
		dmi++;
	}
	all_dialogs.clear();

	triggeredScripts.clear();

	current_scene = NULL;
	currentDialog = NULL;
	currentDialogQuestion = NULL;
	currentDialogAnswer = NULL;

	delete currentInventorySkin;
	currentInventorySkin = new DefaultInventory;

	onscreen_text.clear();
	should_draw_text = false;

	currentViewX = 0;
	currentViewY = 0;
	
	shouldFade = false;
	fadeMethod = 0;
	fadeSpeed = 0;
	fadeProgress = 0;
	
	
	game_states.clear();

	delete main_character;

	dout << " End resetGame" << endl << endl;
	

	return 0;
}


void stopTime() {
	isTimeTicking = false;
}
void resumeTime() {
	isTimeTicking = true;
}

bool drawFadeEffect(int method, int speed, int &progress, SDL_Surface *savedScr) {
//	dout << "DRAWFADEEFFECT, effect: " << method << endl;
	switch(method) {
	case 0:
		{
			return doMoveScreenRight(method, speed, progress, savedScr);
		}
	case 1:
		{
			return doMoveScreenLeft(method, speed, progress, savedScr);
		}
	case 2:
		{
			return doCheckerEffectHorizontal(method, speed, progress, savedScr);
		}
	case 3:
		{
			return doFadeIntoBox(method, speed, progress, savedScr);
		}
	case 4:
		{
			return doMoveScreenUp(method, speed, progress, savedScr);
		}
	case 5:
		{
			return doMoveScreenDown(method, speed, progress, savedScr);
		}
	case 6:
		{
			return doCheckerEffectVertical(method, speed, progress, savedScr);
		}
	case 7:
		{
			return doCrossFade(method, speed, progress, savedScr);
		}
	default:
		{
			dout << "ERROR- unknown fade method specified: " << method << endl;
			exit(0);
			break;
		}
	}
	return false;
}

int showDebugOutput() {
	static FILE *fin = NULL;
	static list<string> debugString;
	static int lastReadTime = current_time;
	int minHeight = 80;
	int maxDebugLines = 50;

	if(!fin) {
		fin = fopen("dout", "r");
		if(!fin) {
			dout << "ERROR- could not open dout file for reading: " << strerror(errno) << endl;
			dout << "Debug on-screen console disabled" << endl;
			debugConsole = false;
			return 1;
		}
	}

	// Read from file every 1/2 sec.
	if(current_time - lastReadTime >= 50) {
		static char buf[512];
		while(fgets(buf, 512, fin)) {
			debugString.push_front(string(buf));
		}
		while(debugString.size() > maxDebugLines) {
			debugString.pop_back();
		}
		if(clearDebugConsole) {
			debugString.clear();
			clearDebugConsole = false;
		}
		lastReadTime = current_time;
	}

	int h = screen->h - 20;
	SDL_Rect r;
	list<string>::iterator i = debugString.begin();
	while(i != debugString.end() && h > minHeight) {
		const char *s = i->c_str();
		debugFont->strRect(s, &r);
		h-= r.h;
		debugFont->writeStr(screen, s, 4, h, NULL);
		i++;
	}
	

	return 0;
}

int nearestPow2(int x) {
	int i = 1;
	while(i < x) {i *= 2;}
	return i;
}
