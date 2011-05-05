#include "game.h"
#include "defaultInventory.h"

DefaultInventory::DefaultInventory() {
	overInventoryItem = NULL;
	currentInventoryItem = NULL;
	resetSkin();
}

DefaultInventory::~DefaultInventory() {
}

int DefaultInventory::onLeftMouse() {
	if(currentInventoryItem) {
		IIMI i = inventory.begin();
		if(i == inventory.end()) {
			dout << "ERROR- currentInventoryItem != NULL, but inventory size is 0" << endl;
			exit(1);
		}
		InventoryItem *item = i->second;


		if((currentInventoryItem == item) && (overInventoryItem)){
			currentInventoryItem = overInventoryItem;
			return INVENTORY_NOTHING;
		}
		if(overInventoryItem == item) {
			currentInventoryItem = overInventoryItem;
			return INVENTORY_NOTHING;
		}
		if(overInventoryItem) {
			param1 = currentInventoryItem->id;
			param2 = overInventoryItem->id;
			return INVENTORY_COMBINE_ITEM;
		}
		Hotspot *h = IsOnHotspot(current_mouse_x + currentViewX, current_mouse_y + currentViewY);
		if(h) {
			if(h->scripts.find(currentInventoryItem->id) != h->scripts.end()) {
				param1 = currentInventoryItem->id;
				param2 = h->id;
				return INVENTORY_USE_ITEM;
			}
			if(h->scripts.find(ANY_INVENTORY_ITEM_ID) != h->scripts.end()) {
				param1 = ANY_INVENTORY_ITEM_ID;
				param2 = h->id;
				return INVENTORY_USE_ITEM;
			}
			return INVENTORY_NOTHING;
		}
	}
	return INVENTORY_MOVE_CHARACTER;
}

int DefaultInventory::onRightMouse() {
	if(currentInventoryItem) {
		IIMI i = inventory.begin();
		if(i == inventory.end()) {
			dout << "ERROR- currentInventoryItem != NULL, but inventory size is 0" << endl;
			exit(1);
		}
		InventoryItem *item = i->second;
		if(item != currentInventoryItem) {
			resetCurrentItem();
			return INVENTORY_NOTHING;
		}
	}
	wasCursorOnInventory = false;
	inventoryVisible = !inventoryVisible;
	return INVENTORY_NOTHING;
}

int DefaultInventory::onKey(int key) {
	switch(key) {
	case SDLK_s:
		{
//			dout << "inventory: set next inventory item" << endl;
			if(currentInventoryItem) {
				IIMI i = inventory.find(currentInventoryItem->id);
				if(i == inventory.end()) {
					return 1;
				}
				i++;
				if(i == inventory.end()) {
					i = inventory.begin();
				}
				currentInventoryItem = i->second;
//				dout << "inventory: inventory item changed to: " << currentInventoryItem->short_name << "(" 
//					<< currentInventoryItem->id << ")" << endl;
			} else {
//				dout << "inventory: no current inventory item" << endl;
			}
			return 1;
		}
	case SDLK_a:
		{
//			dout << "inventory: set previous inventory item" << endl;
			if(currentInventoryItem) {
				IIMI i = inventory.find(currentInventoryItem->id);
				if(i == inventory.end()) {
					return 1;
				}
				if(i == inventory.begin()) {
					i = --(inventory.end());
				} else {
					i--;
				}
				currentInventoryItem = i->second;
//				dout << "inventory: inventory item changed to: " << currentInventoryItem->short_name << "(" 
//					<< currentInventoryItem->id << ")" << endl;
			} else {
//				dout << "inventory: no current inventory item" << endl;
			}
			return 1;
		}
	}
	return 0;
}

int DefaultInventory::drawSkin() {

	Hotspot *h = IsOnHotspot(current_mouse_x + currentViewX, current_mouse_y + currentViewY); 

	drawItems();

	// In left botton corner write name of current inventory item
	if(currentInventoryItem) {
		normalFont->writeStr(screen, currentInventoryItem->long_name.c_str(), (screen->w / 40), screen->h - (screen->h / 30), NULL);
	}

	// Which item animation is played?
	bool shouldHilight = false;
	bool anyItemScript = false;
	h = IsOnHotspot(current_mouse_x + currentViewX, current_mouse_y + currentViewY); 
	if(h) {
		multimap<int, int>::iterator i = h->scripts.find(currentInventoryItem->id);
		if(i != h->scripts.end()) {
			SMI smi = all_scripts.find(i->second);
			Script *s = smi->second;
			if(s->execute_times > 0) shouldHilight = true;
		} else {
			i = h->scripts.find(ANY_INVENTORY_ITEM_ID);
			if(i != h->scripts.end()) {
				SMI smi = all_scripts.find(i->second);
				Script *s = smi->second;
				if(s->execute_times > 0) shouldHilight = true;
				anyItemScript = true;
			}
		}
	}
	if(overInventoryItem) {
		multimap<int, int>::iterator i = overInventoryItem->scripts.find(currentInventoryItem->id);
		if(i != overInventoryItem->scripts.end()) {
			SMI smi = all_scripts.find(i->second);
			Script *s = smi->second;
			if(s->execute_times > 0) shouldHilight = true;
		}
	}

	string compose;
	if(overInventoryItem) {
		compose = overInventoryItem->short_name;
	} else if(h) {
		if(shouldHilight) {
			if(anyItemScript) {
				compose = h->description;
			} else {
				compose = h->description;
			}
		} else {
			compose = h->description;
		}
	} else {
		// Neesam virs neviena hotspota vai inventory itema
		compose = currentInventoryItem->short_name;
	}

	int width, height;
	if (shouldHilight) {
		width = currentInventoryItem->selectedGfx->getCurrentFrameWidth();
		height = currentInventoryItem->selectedGfx->getCurrentFrameHeight();
	}
	else {
		width = currentInventoryItem->unselectedGfx->getCurrentFrameWidth();
		height = currentInventoryItem->unselectedGfx->getCurrentFrameHeight();
	}


	SDL_Rect rect;
	Font_Struct *font = dialogNormalText;
	if(shouldHilight) {
		font = dialogSelectedText;
	}

	font->strRect(compose.c_str(), &rect);

	int x = current_mouse_x + width;
	int y = current_mouse_y + (height - rect.h) / 2;
	if (y + rect.h > screen->h) y = current_mouse_y - rect.h;
	if (x + rect.w > screen->w) x = current_mouse_x - rect.w;

	font->writeStr(screen, compose.c_str(), x, y, NULL);
	return 0;
}

int DefaultInventory::drawMouseCursor() {
	// Draws mouse cursor with red point and with inventory item image.
	static bool wasOnHotspot = false;
	SDL_Rect r;
	if(currentInventoryItem) {
		//		DrawAnimation(screen, currentInventoryItem->gfx, current_mouse_x, current_mouse_y);
		if(current_game_state == GAME_STATE_NORMAL && 
			(currentInventoryItem->selectedGfx != currentInventoryItem->unselectedGfx)) {
			bool shouldHilight = false;
			Hotspot *h = IsOnHotspot(current_mouse_x + currentViewX, current_mouse_y + currentViewY); 
			if(h) {
				multimap<int, int>::iterator i = h->scripts.find(currentInventoryItem->id);
				if(i != h->scripts.end()) {
					int scriptId = i->second;
					SMI scmi = all_scripts.find(scriptId);
					if(scmi != all_scripts.end()) {
						Script *s = scmi->second;
						if(s->execute_times > 0) {
							shouldHilight = true;
						}
					}
				}
				if(!shouldHilight) {
					i = h->scripts.find(ANY_INVENTORY_ITEM_ID);
					if(i != h->scripts.end()) {
						int scriptId = i->second;
						SMI scmi = all_scripts.find(scriptId);
						if(scmi != all_scripts.end()) {
							Script *s = scmi->second;
							if(s->execute_times > 0) {
								shouldHilight = true;
							}
						}
					}
				}
			}
			if(!shouldHilight && overInventoryItem) {
				multimap<int, int>::iterator i = overInventoryItem->scripts.find(currentInventoryItem->id);
				if(i != overInventoryItem->scripts.end()) {
					int scriptId = i->second;
					SMI scmi = all_scripts.find(scriptId);
					if(scmi != all_scripts.end()) {
						Script *s = scmi->second;
						if(s->execute_times > 0) {
							shouldHilight = true;
						}
					}
				}
			}
			if(shouldHilight) {
				if(!wasOnHotspot) {
					resetAnimation(currentInventoryItem->selectedGfx);
					wasOnHotspot = true;
				}
				DrawAnimationClipped(screen, 0, 0, currentInventoryItem->selectedGfx, current_mouse_x, current_mouse_y);
				UpdateAnimation(currentInventoryItem->selectedGfx);
			} else {
				wasOnHotspot = false;
				DrawAnimationClipped(screen, 0, 0, currentInventoryItem->unselectedGfx, current_mouse_x, current_mouse_y);
				UpdateAnimation(currentInventoryItem->unselectedGfx);
			}
		}
		else {
			wasOnHotspot = false;
			DrawAnimationClipped(screen, 0, 0, currentInventoryItem->unselectedGfx, current_mouse_x, current_mouse_y);
			UpdateAnimation(currentInventoryItem->unselectedGfx);
		}
	}
	
	//#ifdef _DEBUG
	int dotColor = SDL_MapRGB(screen->format, 0xff, 0, 0);
	r.x = current_mouse_x;
	r.y = current_mouse_y;
	r.w = r.h = 2;
	myFillRect(screen, &r, dotColor);
	//#endif
	return 0;
}

int DefaultInventory::resetSkin() {
	inventoryVisible = false;
	overInventoryItem = NULL;
	maxItemHeight = 0;
	wasCursorOnInventory = false;
	resetCurrentItem();
	return 0;
}

int DefaultInventory::drawItems() {

	overInventoryItem = NULL;	

	if(!inventoryVisible) {
		return 1;
	}

	IIMI i = inventory.begin();
	InventoryItem *item;
	int count = 0;
	int currentx = 5;
	int currenty = 5;
	SDL_Rect r;

	
	while(i != inventory.end()) {
		item = i->second;
		r.x = currentx;
		r.y = currenty;
		r.w = item->unselectedGfx->getCurrentFrameWidth();
		r.h = item->unselectedGfx->getCurrentFrameHeight();

		maxItemHeight = r.y + r.w;

		int inventoryColor = SDL_MapRGB(screen->format, 0, 0x42, 0x31);
		myFillRect(screen, &r, inventoryColor, 0.5);

		DrawAnimation(screen, item->unselectedGfx, currentx, currenty);

		if((current_mouse_x > currentx) && 
			((current_mouse_x - currentx) < item->unselectedGfx->getCurrentFrameWidth()) && 
			(current_mouse_y > currenty) && 
			((current_mouse_y - currenty) < item->unselectedGfx->getCurrentFrameHeight())) {
			overInventoryItem = item;
		}

		currentx+=item->unselectedGfx->getCurrentFrameWidth() + 10;
		UpdateAnimation(item->unselectedGfx);
		count++;
		i++;
	}
	return 0;
}

int DefaultInventory::onAddItem(int item) {
	if(!currentInventoryItem) {
		resetCurrentItem();
	}
	return 0;
}

int DefaultInventory::onRemoveItem(int item) {
	if(currentInventoryItem->id == item) {
		resetCurrentItem();
	}
	if(overInventoryItem) {
		if(overInventoryItem->id == item) {
			overInventoryItem = NULL;
		}

	}
	return 0;
}


int DefaultInventory::onMouse() {
	if(inventoryVisible) {
		if(current_mouse_y > maxItemHeight) {
			if(wasCursorOnInventory) {
				inventoryVisible = !inventoryVisible;
				wasCursorOnInventory = false;
			}
		} else {
			wasCursorOnInventory = true;
		}
	} else {
		if(current_mouse_y < 5) {
			inventoryVisible = !inventoryVisible;
			wasCursorOnInventory = true;
		}
	}
	return 0;
}

int DefaultInventory::resetCurrentItem() {
	IIMI i = inventory.begin();
	if(i == inventory.end()) {
		currentInventoryItem = NULL;
		return 1;
	}
	currentInventoryItem = i->second;
	return 0;
}

int DefaultInventory::save(FILE *fout) {
	if(currentInventoryItem) {
		saveInt(fout, currentInventoryItem->id);
	} else {
		saveInt(fout, -1);
	}
	return 0;
}

int DefaultInventory::load(FILE *fin) {
	int itemId = readInt(fin);
	IIMI iimi = inventory.find(itemId);
	if(iimi != inventory.end()) {
		currentInventoryItem = iimi->second;
	} else {
		resetCurrentItem();
	}
	return 0;
}
