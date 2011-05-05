#include "game.h"

unsigned LoadInventoryItemData(InventoryItem* i) {
	i->unselectedGfx = LoadAnimation(i->unselectedGfxFile);
	if(!i->unselectedGfx) {
		dout << "ERROR- Could not load unselected animation for inventory item: " << i->id << endl;
		exit(1);
	}
	if(i->selectedGfxFile.length() > 0) {
		i->selectedGfx = LoadAnimation(i->selectedGfxFile);
		if(!i->selectedGfx) {
			dout << "ERROR- Could not load selected animation for inventory item: " << i->id << endl;
			exit(1);
		}
	} else {
		i->selectedGfx = i->unselectedGfx;
	}
	return 0;
}
unsigned FreeInventoryItemData(InventoryItem* i) {
	if(i->selectedGfx != i->unselectedGfx) {
		if(i->unselectedGfx) {
			delete i->unselectedGfx;
		}
		if(i->selectedGfx) {
			delete i->selectedGfx;
		}
	} else {
		if(i->unselectedGfx) {
			delete i->unselectedGfx;
		}
	}
	i->unselectedGfx = NULL;
	i->selectedGfx = NULL;
	return 0;
}

InventoryItem* LoadInventoryItem(FILE* fin) {
	if(!IsNextString(fin, "inventory_item")) {
		dout << " Loading inventory item failed- no 'inventory_item' tag found" << endl;
		return NULL;
	}
	int id = LoadInt(fin);
	string shortName = LoadString(fin, true);
	string longName = LoadString(fin, true);
	string unselectedGfxFile = LoadString(fin);
	string selectedGfxFile = LoadString(fin);
	dout << "xxx " << unselectedGfxFile << " " << selectedGfxFile << endl;
	if(!IsNextString(fin, "end_inventory_item")) {
		dout << "ERROR- Loading inventory item failed- no 'end_inventory_item' tag found" << endl;
		exit(1);
//		return NULL;
	}
	InventoryItem* i = new InventoryItem;
	i->id = id;
	i->short_name = shortName;
	i->long_name = longName;
	i->unselectedGfxFile = unselectedGfxFile;
	i->selectedGfxFile = selectedGfxFile;
	return i;
}
void LoadAllInventoryItems(InventoryItemMap& m) {
	dout << "--- LoadAllInventoryItems" << endl;
	m.clear();

	vector<string> fileNames;
	loadListFile("inventory_items.lst", fileNames);
	
	FILE* fin;
	char str[256];
	InventoryItem* h;
	for(int currFile = 0; currFile < fileNames.size(); currFile++) {
		strcpy(str, fileNames[currFile].c_str());
		fin = fopen(str, "r");
		if(!fin) {
			dout << " ERROR- 'inventory_items.lst' refers to invalid file: " << str << endl;
			exit(1);
		}
		dout << " Processing file: " << str << endl;
		while((h = LoadInventoryItem(fin))) {
			if(m.find(h->id) != m.end()) {
				dout << "ERROR- inventory item with id: " << h->id << "already exists. File: " << str << endl;
				exit(1);
			}
			SMI smi = all_scripts.begin();
			Script *script;
			while(smi != all_scripts.end()) {
				script = smi->second;
				if((script->type == SCRIPT_COMBINE_ITEM) && (h->id == script->param2)) {
					h->scripts.insert(make_pair<int, int>(script->param1, script->id));
				}
				smi++;
			}

			m.insert(make_pair<unsigned, InventoryItem*>(h->id, h));
			dout << " Inventory item with id: " << h->id << " loaded" << endl;
		}
		fclose(fin);
	}
	dout << "--- End LoadAllInventoryItems" << endl << endl;
}
