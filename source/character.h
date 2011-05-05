#ifndef __CHARACTER_H__
#define __CHARACTER_H__

void SetCurrentCharacterAnimation(Character* c, bool shouldRestartSound = false, bool synchroniseFrames = false);
int startCharacterMovement(int new_x, int new_y);
int stopCharacterMovement();
void UpdateCharacter();
//void StartNextWaypoint();
unsigned SetCharacterSpeed();
void setCharacterState(int state);
unsigned setCharacterAnimationSet(int animationSetId);



#endif
