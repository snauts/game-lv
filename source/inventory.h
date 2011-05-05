#ifndef __INVENTORY_H__
#define __INVENTORY_H__

#include "structs.h"

class InventorySkin {
public:
	InventorySkin() : param1(0), param2(0) {};
	virtual ~InventorySkin() {};
	virtual int resetSkin() = 0;
	virtual int onMouse() = 0;
	virtual int onLeftMouse() = 0;
	virtual int onRightMouse() = 0;
	virtual int onKey(int key) = 0;
	virtual int drawMouseCursor() = 0;
	virtual int drawSkin() = 0;
	virtual int onAddItem(int item) = 0;
	virtual int onRemoveItem(int item) = 0;
	virtual int save(FILE *fout) = 0;
	virtual int load(FILE *fin) = 0;
	virtual void getActionParams(int &par1, int &par2) {
		par1 = param1;
		par2 = param2;
	}

protected:
	int param1;
	int param2;
};


#endif
