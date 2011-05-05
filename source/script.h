#ifndef __SCRIPT_H__
#define __SCRIPT_H__

extern isActionCompleteHandler suspendedScriptActionHandlers[SUSPENDABLE_ACTIONS];

int executeScript(Script* s);
int executeScriptById(int scriptId);
int resumeScript(Script* s, ScriptAction* a, bool skip_this_action = false);
unsigned executeSceneStartupScripts(Scene *curr_scene, Scene *next_scene);
unsigned doStartupScript();

int suspendScript(ScriptAction *a);
int completeSuspendedScript();
int cancelSuspendedScript();


#endif
