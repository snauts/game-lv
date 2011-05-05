#include "game.h"
#include "structs.h"

void resetTransform(Animation *anim, long resetTime);
unsigned updateTransform(Animation *anim);
void resetTransformData(AnimationTransform *transform, long resetTime);

Animation::Animation() {
	blocks = NULL;
	currentBlock = NULL;
	currentBlockIdx = 0;
	totalBlocks = 0;
	currentFrame = 0;
	currentLoops = 0;
	
	transforms = NULL;
	currentTransform = NULL;
	totalTransforms = 0;
	currentTransformIdx = 0;
	currentTransformLoops = 0;

	isTransformSynchronised = false;
	lastUpdate = 0;
}

Animation::~Animation() {
	for(unsigned a = 0; a < totalBlocks; a++) {
		if(blocks[a]) delete blocks[a];
	}
	delete[] blocks;
}

AnimationBlock::AnimationBlock() {
	totalFrames = 0;
	img = NULL;
	speed = 0;
	xOfs = 0;
	yOfs = 0;
	hasAlpha = false;
	hasColorKey = false;
	alpha = 100;
	colorKey = 0;
}

AnimationBlock::~AnimationBlock(){
	for(unsigned a = 0; a < totalFrames; a++) {
		if(img[a]) {
			freeImage(img[a]);
		}
	}
	delete[] img;
}

Animation* LoadAnimation(const string& file_name, bool useVideoMemory, bool loadImageData) {
	dout << " Load animation: '" << file_name << "'" << endl;
	FILE* fin = fopen(convertPath(file_name).c_str(), "r");
	if(!fin) {
		dout << "ERROR- LoadAnimation could not open file: " << file_name << endl;
		exit(1);
	}

	vector<AnimationBlock*> allBlocks;
	SDL_Surface* tmp_surf;
	string fname;

	bool isAnimSynchronised = false;
	vector<AnimationTransform*> allTransforms;

	while(IsNextString(fin, "animation_transform")) {
		dout << "  Loading animation transformation" << endl;
		AnimationTransform *transform = NULL;
		int type = LoadInt(fin);
		int length = LoadInt(fin);		
		dout << "   type: " << type << ", length: " << length << endl;
		switch(type) {
		case TRANSFORM_LINEAR:
			{
				dout << "   Linear transformation:" << endl;
				LinearAnimationTransform *tmpTransform = new LinearAnimationTransform;
				transform = (AnimationTransform*) tmpTransform;

				tmpTransform->startX = LoadInt(fin);
				tmpTransform->startY = LoadInt(fin);
				tmpTransform->endX = LoadInt(fin);
				tmpTransform->endY = LoadInt(fin);
				tmpTransform->transform.currentX = tmpTransform->startX;
				tmpTransform->transform.currentY = tmpTransform->startY;

				dout << "    start: " << tmpTransform->startX << ", " << tmpTransform->startY << endl;
				dout << "    end: " << tmpTransform->endX << ", " << tmpTransform->endY << endl;
				
				break;
			}
		case TRANSFORM_SPLINE:
			{
				dout << "   Spline transformation:" << endl;
				SplineAnimationTransform *tmpTransform = new SplineAnimationTransform;
				transform = (AnimationTransform*) tmpTransform;
				PointStruct tempPoint;
				int pointCount, subPointCount;
				
				pointCount = LoadInt(fin);
				subPointCount = LoadInt(fin);
				Spline spline;

				for(int temp = 0; temp < pointCount; temp++) {
					tempPoint.x = (float)LoadInt(fin);
					tempPoint.y = (float)LoadInt(fin);
					spline.pushBackPoint(tempPoint);
				}

				spline.getPath(tmpTransform->points, subPointCount);
				
				tmpTransform->transform.currentX = (int)tmpTransform->points[0].x;
				tmpTransform->transform.currentY = (int)tmpTransform->points[0].y;

				dout << "	Spline transformation parameter load - SUCCESS!" << endl;
				
				break;
			}
		default:
			{
				dout << "ERROR- Unknown transformation type specified: " << type << endl;
				exit(1);
			}
		}

		transform->type = type;
		transform->length = length;

		allTransforms.push_back(transform);

		if(!IsNextString(fin, "end_animation_transform")) {
			dout << "ERROR- missing end_animation_transform tag in animation: " << file_name << endl;
			exit(1);
		}
		dout << "  Transformation loaded" << endl;
	}
	dout << "  Totaly: " << allTransforms.size() << " transformations loaded" << endl;

	if(IsNextString(fin, "synchronise_animation_with_transform")) {
		dout << "  Animation will be synchronised with transformation" << endl;
		isAnimSynchronised = true;
	} else {
		dout << "  Animation wont be synchronised with transformation" << endl;
	}

	while(IsNextString(fin, "animation_block", false) || IsNextString(fin, "animation_file", false)) {
		if(IsNextString(fin, "animation_block")) {
			AnimationBlock *block = new AnimationBlock;
			
			block->xOfs = LoadInt(fin);
			block->yOfs = LoadInt(fin);
			block->totalFrames = LoadInt(fin);
			block->speed = LoadInt(fin);
			block->hasColorKey = (LoadInt(fin) != 0);
			block->colorKey = LoadInt(fin);
			block->hasAlpha= (LoadInt(fin) != 0);
			block->alpha = LoadInt(fin);
			block->img = new SDL_Surface*[block->totalFrames];
			
			block->imgFiles.reserve(block->totalFrames);
			
			for(unsigned a = 0; a < block->totalFrames; a++) {
				fname = LoadString(fin);
				block->imgFiles.push_back(fname);
				if(loadImageData) {
					tmp_surf = loadImage(fname, block->hasColorKey, 
						block->colorKey, block->hasAlpha, 
						block->alpha, true, useVideoMemory);
				}
				else {
					tmp_surf = NULL;
				}
				if(loadImageData && tmp_surf == NULL) {
					dout << "ERROR- LoadAnimation could not load image: " << fname << endl;
					exit(1);
				}
				block->img[a] = tmp_surf;
			}
			allBlocks.push_back(block);
			
			if(!IsNextString(fin, "end_animation_block")) {
				dout << "ERROR- missing end_animation_block tag in animation: " << file_name << endl;
				exit(1);
			}
		} else {
			// Load other animations
			if(IsNextString(fin, "animation_file")) {
				while(!feof(fin) && !IsNextString(fin, "end_animation_file")) {
					string animFile = LoadString(fin);
					dout << "  Loading included animation: " << animFile << endl;
					Animation *tmpAnim = LoadAnimation(animFile, false, false);
					if(!tmpAnim) {
						dout << "ERROR- failed to load included animation: '" << animFile << "'" << endl;
						exit(1);
					}
					dout << "  Included animation loaded ok" << endl;
					for(unsigned a = 0; a < tmpAnim->totalBlocks; a++) {
						allBlocks.push_back(tmpAnim->blocks[a]);
						tmpAnim->blocks[a] = NULL;
					}
					dout << "    Copied: " << tmpAnim->totalBlocks << " blocks" << endl;
					delete tmpAnim;
				}
				if(feof(fin)) {
					dout << "ERROR- missing end_animation_file in animation: " << file_name << endl;
					exit(1);
				}
			}
		}
	}

	if(allBlocks.size() == 0) {
		dout << "ERROR- 0 animation blocks in animation: " << file_name << endl;
		exit(1);
	}
	Animation* anim = new Animation;

	anim->totalBlocks = allBlocks.size();
	anim->blocks = new AnimationBlock*[allBlocks.size()];
	anim->currentBlock = allBlocks[0];
	anim->useVideoMemory = useVideoMemory;
	anim->isImageDataLoaded = loadImageData;
	
	unsigned tmpVal = 0;
	for(tmpVal = 0; tmpVal < allBlocks.size(); tmpVal++) {
		anim->blocks[tmpVal] = allBlocks[tmpVal];
	}
	
	anim->isTransformSynchronised = isAnimSynchronised;
	anim->totalTransforms = allTransforms.size();
	if(anim->totalTransforms) {
		anim->transforms = new AnimationTransform*[allTransforms.size()];
		anim->currentTransform = allTransforms[0];
		for(tmpVal = 0; tmpVal < allTransforms.size(); tmpVal++) {
			anim->transforms[tmpVal] = allTransforms[tmpVal];
		}
	}
	


	
	fclose(fin);
	dout << " Animation loaded" << endl;
	return anim;
}

unsigned DrawAnimation(SDL_Surface *dst, Animation *anim, int x, int y) {
	SDL_Rect r1, r2;
	SDL_Surface* img = getCurrentFrame(anim);
	r1.x = x + anim->currentBlock->xOfs;
	r1.y = y + anim->currentBlock->yOfs;
	if(anim->currentTransform) {
		r1.x += anim->currentTransform->currentX;
		r1.y += anim->currentTransform->currentY;
	}
	r1.w = r2.w = img->w;
	r1.h = r2.h = img->h;
	r2.x = 0;
	r2.y = 0;
	myBlitSurface(img, &r2, dst, &r1);
	return 0;
}

unsigned DrawAnimationClipped(SDL_Surface *dst, int viewx, int viewy, Animation *anim, int x, int y) {
	return DrawAnimation(dst, anim, x - viewx, y - viewy);
}

unsigned DrawAnimationScaled(SDL_Surface *dst, Animation *anim, int x, int y, double scaleCoef, bool antialiased) {

	SDL_Surface *currentFrame = getCurrentFrame(anim);
	
	SDL_Rect blitRect, srcRect;

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = currentFrame->w;
	srcRect.h = currentFrame->h;

	blitRect.x = (Sint16) (x + anim->currentBlock->xOfs * scaleCoef);
	blitRect.y = (Sint16) (y + anim->currentBlock->yOfs * scaleCoef);
	blitRect.w = currentFrame->w * scaleCoef;
	blitRect.h = currentFrame->h * scaleCoef;;

	myBlitSurface(currentFrame, &srcRect, dst, &blitRect);
	return 0;
}

unsigned updateTransform(Animation *anim) {
	AnimationTransform *t = anim->currentTransform;

	while(t->endTime < current_time) {
		anim->currentTransformIdx++;
		if(anim->currentTransformIdx >= anim->totalTransforms) {
			anim->currentTransformIdx = 0;
			anim->currentTransformLoops++;
		}
		anim->currentTransform = anim->transforms[anim->currentTransformIdx];
		resetTransformData(anim->currentTransform, t->endTime);
		t = anim->currentTransform;
	}

	t->lastUpdate = current_time;

	switch(t->type) {
	case TRANSFORM_LINEAR:
		{
			LinearAnimationTransform *tmpTransform = (LinearAnimationTransform*) t;
			double k = (double)(t->lastUpdate - t->startTime) / (double)t->length;
			t->currentX = (int) ((tmpTransform->endX - tmpTransform->startX) * k + tmpTransform->startX);
			t->currentY = (int) ((tmpTransform->endY - tmpTransform->startY) * k + tmpTransform->startY);
			break;
		}
	case TRANSFORM_SPLINE:
		{
			SplineAnimationTransform *tmpTransform = (SplineAnimationTransform*) t;
			double progress = (double)(t->lastUpdate - t->startTime) / (double)t->length;
			if(progress >= 1.0) {
				t->currentX = (int) (tmpTransform->points[tmpTransform->points.size() - 1].x);
				t->currentY = (int) (tmpTransform->points[tmpTransform->points.size() - 1].y);
			}
			else {
				double total = tmpTransform->points.size() - 1;
				double current = progress * (double) total;
				double subProgress = (progress - (int)current / total) * total;
				PointStruct startPoint = tmpTransform->points[(int) current];
				PointStruct endPoint = tmpTransform->points[(int) (current + 1)];
				t->currentX = (int) ((endPoint.x - startPoint.x) * subProgress + startPoint.x);
				t->currentY = (int) ((endPoint.y - startPoint.y) * subProgress + startPoint.y);
			}
			break;
		}
	}
	
	return 0;
}

unsigned UpdateAnimation(Animation *anim) {
	AnimationBlock *b = anim->currentBlock;
	int dif;
	int frame;
	if(b->speed > 0) {
		while((current_time - anim->lastUpdate) > b->speed) {
			dif = current_time - anim->lastUpdate;
			if(dif > b->speed) {
				frame = dif / b->speed;
				if((anim->currentFrame + frame) >= b->totalFrames) {
					anim->lastUpdate+= (b->totalFrames - anim->currentFrame) * b->speed;
					anim->currentBlockIdx++;
					if(anim->currentBlockIdx >= anim->totalBlocks) {
						anim->currentBlockIdx = 0;
						anim->currentLoops++;
						if(anim->isTransformSynchronised) {
							resetTransform(anim, anim->lastUpdate);
							anim->currentTransformLoops++;
						}
					}
					anim->currentBlock = anim->blocks[anim->currentBlockIdx];
					anim->currentFrame = 0;
					b = anim->currentBlock;
				} else {
					anim->currentFrame+=frame;
					anim->lastUpdate += b->speed * frame;
				}
			}
		}
	}

	if(anim->totalTransforms) {
		updateTransform(anim);
	}

	return 0;
}

void resetTransformData(AnimationTransform *transform, long resetTime) {
	transform->startTime = resetTime;
	transform->endTime = transform->startTime + transform->length;
	transform->lastUpdate = transform->startTime;
	
	switch(transform->type) {
	case TRANSFORM_LINEAR:
		{
			LinearAnimationTransform *tmpTransform = (LinearAnimationTransform*)transform;
			transform->currentX = tmpTransform->startX;
			transform->currentY = tmpTransform->startY;
			break;
		}
	case TRANSFORM_SPLINE:
		{
			SplineAnimationTransform *tmpTransform = (SplineAnimationTransform*)transform;
			transform->currentX = (int) tmpTransform->points[0].x;
			transform->currentY = (int) tmpTransform->points[0].y;
			break;
		}
	}
}

void resetTransform(Animation *anim, long resetTime) {
	if(!anim->totalTransforms) {
		return;
	}
	
	AnimationTransform *transform = anim->transforms[0];

	anim->currentTransform = transform;
	anim->currentTransformIdx = 0;

	resetTransformData(transform, resetTime);
}

void resetAnimation(Animation *anim) {
	anim->currentBlock = anim->blocks[0];
	anim->currentBlockIdx = 0;
	anim->currentFrame = 0;
	anim->lastUpdate = current_time;
	resetTransform(anim, current_time);
}

int loadAnimationData(Animation *anim) {
    for(int a = 0; a < anim->totalBlocks; a++) {
	AnimationBlock *block = anim->blocks[a];
	for(int b = 0; b < block->totalFrames; b++) {
	    if(!block->img[b]) {
		block->img[b] = loadImage(block->imgFiles[b],
					  block->hasColorKey, 
					  block->colorKey,
					  block->hasAlpha, 
					  block->alpha, 
					  true,
					  anim->useVideoMemory);
		if(!block->img[b]) {
		    dout << "ERROR- loadAnimationData could not load image: " << block->imgFiles[b] << endl;
		    exit(1);
		}
	    }
	}
	}
    anim->isImageDataLoaded = true;
    return 0;
}

int freeAnimationData(Animation *anim) {
	for(int a = 0; a < anim->totalBlocks; a++) {
		AnimationBlock *block = anim->blocks[a];
		for(int b = 0; b < block->totalFrames; b++) {
			if(block->img[b]) {
				freeImage(block->img[b]);
				block->img[b] = NULL;
			}
		}
	}
	anim->isImageDataLoaded = false;
	return 0;
}
