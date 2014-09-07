#pragma once

#include <vector>
#include <functional>

namespace Json { class Value; }

namespace blib
{
	namespace wm
	{
		class MenuItem;

		class Menu
		{
		public:
			Menu(const Json::Value &data);
			void setAction(std::string path, std::function<void() > callback);
			void linkToggle(std::string path, bool* linkBool);
			void setEnabled(std::string path, bool value);
			MenuItem* getItem(std::string path);

			void foreach(std::function<void(MenuItem*)> callback);
			void setMenu(std::string menuLoc, blib::wm::MenuItem* menuItem);
			std::vector<MenuItem*> menuItems;
		};
	}
}