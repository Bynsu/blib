#include "BmlBox.h"

#include <blib/ResourceManager.h>
#include <blib/SpriteBatch.h>
#include <blib/wm/WM.h>
#include <blib/util/Tree.h>
#include <blib/linq.h>

#include <glm/gtc/matrix_transform.hpp>

namespace blib
{
	namespace wm
	{
		namespace widgets
		{
			namespace bml
			{
				class BmlRenderBlock
				{
				public:
					glm::ivec2 topleft;
					glm::ivec2 size;

					glm::ivec2 cursor;


					BmlRenderBlock(const glm::ivec2 &topleft, const glm::ivec2 &size)
					{
						this->topleft = topleft;
						this->size = size;
						cursor = glm::ivec2(0, 0);
					}
				};



				class Element
				{
				public:
					virtual BmlRenderBlock draw(SpriteBatch* spriteBatch, glm::mat4 matrix, const BmlRenderData* renderData, BmlFontProperties fontProperties, BmlRenderBlock box) const = 0;
				};

				template<class T>
				class ElementContainer : public Element, public blib::util::DynTree<T>
				{
				public:
					void add(T* t)
					{
						blib::util::DynTree<T>::children.push_back(t);
					}
					virtual BmlRenderBlock draw(SpriteBatch* spriteBatch, glm::mat4 matrix, const BmlRenderData* renderData, BmlFontProperties fontProperties, BmlRenderBlock box) const
					{
						BmlRenderBlock subBox = box;
						for (typename std::list<T*>::const_iterator it = blib::util::DynTree<T>::children.cbegin(); it != blib::util::DynTree<T>::children.cend(); it++)
						{
							BmlRenderBlock b2 = (*it)->draw(spriteBatch, matrix, renderData, fontProperties, subBox);
							subBox.topleft.y += b2.size.y;
						}
						return box;
					}
				};


				class TextNode : public Element
				{
				public:
					std::string text;
					TextNode(const std::string &text)
					{
						this->text = text;
					}
					virtual BmlRenderBlock draw(SpriteBatch* spriteBatch, glm::mat4 matrix, const BmlRenderData* renderData, BmlFontProperties fontProperties, BmlRenderBlock box) const
					{
						spriteBatch->draw(renderData->getFont(fontProperties), text, glm::translate(matrix, glm::vec3(box.topleft,0)), fontProperties.color);
						box.cursor.y += 14;
						return box;
					}
				};

				class Header : public ElementContainer<Element>
				{
				public:
					Header() {};
					Header(Element* el) { add(el); };
					virtual BmlRenderBlock draw(SpriteBatch* spriteBatch, glm::mat4 matrix, const BmlRenderData* renderData, BmlFontProperties fontProperties, BmlRenderBlock box) const
					{
						fontProperties.size += 0.5f;
						return ElementContainer<Element>::draw(spriteBatch, matrix, renderData, fontProperties, box);
					}
				};

				class Bold : public ElementContainer<Element>
				{
				public:
					Bold(Element* el) { add(el); };
					virtual BmlRenderBlock draw(SpriteBatch* spriteBatch, glm::mat4 matrix, const BmlRenderData* renderData, BmlFontProperties fontProperties, BmlRenderBlock box) const
					{
						fontProperties.bold = true;
						return ElementContainer<Element>::draw(spriteBatch, matrix, renderData, fontProperties, box);
					}
				};

				class ListItem : public ElementContainer<Element>
				{
				public:
					ListItem(Element* el) { add(el); };
					virtual BmlRenderBlock draw(SpriteBatch* spriteBatch, glm::mat4 matrix, const BmlRenderData* renderData, BmlFontProperties fontProperties, BmlRenderBlock box) const;
				};

				class List : public ElementContainer<ListItem>
				{
				public:
					virtual BmlRenderBlock draw(SpriteBatch* spriteBatch, glm::mat4 matrix, const BmlRenderData* renderData, BmlFontProperties fontProperties, BmlRenderBlock box) const;
				};

				class Paragraph : public ElementContainer<Element>
				{
				public:
					Paragraph(){ };
					Paragraph(Element* el) { add(el); };

					virtual BmlRenderBlock draw(SpriteBatch* spriteBatch, glm::mat4 matrix, const BmlRenderData* renderData, BmlFontProperties fontProperties, BmlRenderBlock box) const
					{
						box.topleft += glm::ivec2(5, 5);
						box.size -= glm::ivec2(10, 10);
						return ElementContainer::draw(spriteBatch, matrix, renderData, fontProperties, box);
					}

				};



				class Document : public ElementContainer<Element>
				{
				public:
/*					virtual BmlRenderBlock draw(SpriteBatch* spriteBatch, glm::mat4 matrix, const BmlRenderData* renderData, BmlFontProperties fontProperties, BmlRenderBlock box) const
					{
						ElementContainer
					}*/
				};








			}





			BmlBox::BmlBox(ResourceManager* resourceManager)
			{
				bml::BmlFontProperties font;

			/*	width = 550;


				font.size = 2;
				commands.push_back(new bml::Text("Hello World\n", font, glm::ivec2(0, width)));
				font.size = 1;
				commands.push_back(new bml::Text("This is some text", font, glm::ivec2(0, width)));
				font.bold = true;
				commands.push_back(new bml::Text(" and this is in bold... ", font, glm::ivec2(0, width)));
				font.bold = false;
				font.italic = true;
				commands.push_back(new bml::Text(" but italic is possible too....", font, glm::ivec2(0, width)));
				font.italic = false;

				commands.push_back(new bml::NewLine(24));
				commands.push_back(new bml::Text("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam mi enim, mollis at feugiat a, vulputate sit amet dolor. Nam ac est mollis, ullamcorper nibh et, ullamcorper dui. Fusce in ornare ex. Etiam vitae consectetur diam. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum viverra libero justo, ut vulputate elit efficitur sed. Nulla id nibh fringilla, mattis nulla non, porta sapien. Sed sagittis elit at sollicitudin malesuada. Duis luctus est rhoncus aliquam viverra. Duis sagittis augue pellentesque, congue turpis in, venenatis felis. Ut lobortis tellus sed arcu mollis, eu pretium elit molestie.", font, glm::ivec2(0, width)));
				commands.push_back(new bml::NewLine(24));
				commands.push_back(new bml::Text("Nullam ornare, ligula vitae eleifend sagittis, odio metus auctor elit, sed condimentum metus erat vel nisi.Nulla sed arcu nisi.Nunc ultrices imperdiet nulla vitae congue.Sed nec ligula eleifend dui semper ultricies sit amet in nisi.Aenean condimentum eget nisi ut blandit.Etiam sit amet enim quis lacus viverra suscipit.Aenean ultrices faucibus ligula eu consequat.Quisque vel enim elementum, accumsan felis in, venenatis tortor.", font, glm::ivec2(0, width)));
				commands.push_back(new bml::NewLine(24));
				commands.push_back(new bml::Text("Donec nisl dui, hendrerit et interdum sed, tempor eget justo.Suspendisse efficitur convallis malesuada.Nullam ut commodo orci.Quisque rhoncus ante a mi placerat placerat.Maecenas mauris mauris, rutrum eu molestie id, pharetra sed augue.Cras at semper urna.In eros nisi, tincidunt vitae egestas id, molestie ac metus.Cras convallis, ante id dictum interdum, metus nisl commodo velit, at laoreet augue dolor vitae nibh.Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae;", font, glm::ivec2(0, width)));
				*/
				renderData.resourceManager = resourceManager;

				commands = bml::parseBml("= Header =\n\
This is some text.Then there's some ways to add layouts\n\
*bold text*\n\
_italic text_\n\
[pagebookmark | You can add links]\n\
group text together in paragraphs using double newlines\n\
\n\
this will be a new paragraph with text.blablabla bla\n\
- This is a listitem\n\
- this is another listitem\n\
- and another one, but this one is a bit longer. This way it should wrap around, and I'm not sure if that will work. Let's see, lalalala yay omg woot I am getting bored now...\n\
\n\
== header2 == \n\
this is a little header\n\
yay\n\
\n\
\n\
> This will be in a panel\n\
> inside the panel you can draw more things\n\
> yay more text\n\
> -listitem\n\
> -another listitem\n\
> -etc");

			}

			


			void BmlBox::draw(SpriteBatch &spriteBatch, glm::mat4 matrix, Renderer* renderer) const
			{
//				document->draw(&spriteBatch, glm::translate(matrix, glm::vec3(x, y, 0)), &renderData, bml::BmlFontProperties(), bml::BmlRenderBlock(glm::ivec2(0, 0), glm::ivec2(width, 0)));

				matrix = glm::translate(matrix, glm::vec3(x, y, 0));

				glm::vec2 cursor;
				for (std::list<bml::Command*>::const_iterator it = commands.cbegin(); it != commands.cend(); it++)
				{
					(*it)->draw(spriteBatch, matrix, cursor, renderData);
				}



			//	spriteBatch.draw(WM::getInstance()->font, text, glm::translate(matrix, glm::vec3(x, y, 0)), glm::vec4(0, 0, 0, 1));
			}

			void BmlBox::setBml(std::string data)
			{
				blib::linq::deleteall(commands);
				commands = bml::parseBml(data);
			}

			namespace bml
			{
				std::list<Command*> parseBml(std::string data)
				{
					std::list<Command*> commands;
					BmlFontProperties font;

					std::string token;

					bool newline = true;
					bool escape = false;
					bool inlist = false;

					glm::ivec2 margins(0, 0);

					auto finishToken = [&](){
						while (token.size() > 0 && token[0] == ' ')
							token = token.substr(1);

						if (token == "")
							return;

						Text* text = new Text(token, font, margins);
						token = "";
						commands.push_back(text);
					};

					for (size_t i = 0; i < data.size(); i++)
					{
						if (data[i] == '-' && newline)
						{//list
							finishToken();
							token += "- ";
							finishToken();
							margins.x += 30;
							inlist = true;
						}
						else if (data[i] == '\\' && !escape)
						{
							escape = true;
						}
						else if (data[i] == '*' && !escape)
						{
							finishToken();
							font.bold = !font.bold;
						}
						else if (data[i] == '_' && !escape)
						{
							finishToken();
							font.italic = !font.italic;
						}
						else if (data[i] == '=' && !escape)
						{
							finishToken();
							font.size = 1 - font.size;
						}
						else if (data[i] == ' ')
						{
							token += ' ';
							finishToken();
						}
						else if (data[i] == '\r')
							continue;
						else if (data[i] == '\n')
						{
							newline = true;
							if (inlist)
							{
								finishToken();
								inlist = false;
								margins.x -= 30;
							}
							token += data[i];
						}
						else
						{
							token += data[i];
							newline = false;
						}

					}
					finishToken();




					return commands;
				}
			}



		}
	}
}








/*



= Header =
This is some text. Then there's some ways to add layouts
*bold text*
_italic text_
[pagebookmark|You can add links]
group text together in paragraphs using double newlines

this will be a new paragraph with text. blablabla bla
- This is a listitem
- this is another listitem
- and another one

== header2 ==
this is a little header
yay


> This will be in a panel
> inside the panel you can draw more things
> yay more text
> - listitem
> - another listitem
> - etc


*/