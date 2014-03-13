#include "Menu.h"

#include "SubMenuMenuItem.h"
#include "MenuItem.h"
#include "ToggleMenuItem.h"
#include "ActionMenuItem.h"


#include <blib/util/Log.h>
using blib::util::Log;

#include <json/value.h>


blib::wm::Menu::Menu(const Json::Value &data)
{
	for (Json::ArrayIndex i = 0; i < data.size(); i++)
	{
		MenuItem* subItem = NULL;

		if (data[i]["type"].asString() == "menu")
		{
			subItem = new SubMenuMenuItem(data[i]["name"].asString(), new Menu(data[i]["subitems"]));
		}
		else if (data[i]["type"].asString() == "item")
		{
			subItem = new ActionMenuItem(data[i]["name"].asString());
		}
		else
			Log::err << "Unknown menu type: " << data[i]["type"].asString() << Log::newline;

		if (subItem)
			menuItems.push_back(subItem);
	}
}

void blib::wm::Menu::setAction(std::string path, std::function<void() > callback)
{
	std::string firstPart = path;
	if (firstPart.find("/") != std::string::npos)
	{
		firstPart = firstPart.substr(0, firstPart.find("/"));
	}

	for (size_t i = 0; i < menuItems.size(); i++)
	{
		if (menuItems[i]->name == firstPart)
		{
			SubMenuMenuItem* subMenu = dynamic_cast<SubMenuMenuItem*>(menuItems[i]);
			if (subMenu)
				subMenu->menu->setAction(path.substr(firstPart.size() + 1), callback);
			
			ActionMenuItem* item = dynamic_cast<ActionMenuItem*>(menuItems[i]);
			if (item)
				item->callback = callback;
		}
	}
	
}
