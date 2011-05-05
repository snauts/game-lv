#ifndef __DEFAULT_INVENTORY_H__
#define __DEFAULT_INVENTORY_H__

#include "inventory.h"

class DefaultInventory : public InventorySkin {
private:
	int drawItems();
	int resetCurrentItem();
	
	bool inventoryVisible;
	InventoryItem *currentInventoryItem;
	InventoryItem *overInventoryItem;
	
	int maxItemHeight;
	bool wasCursorOnInventory;
public:
	DefaultInventory();
	~DefaultInventory();
	
	int onLeftMouse();
	int onRightMouse();
	int onKey(int key);
	int drawSkin();
	int drawMouseCursor();
	int resetSkin();
	int onAddItem(int item);
	int onRemoveItem(int item);
	int onMouse();	
	int save(FILE *fout);
	int load(FILE *fin);
	
};


#endif
