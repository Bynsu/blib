#ifndef __VERTEX_H__
#define __VERTEX_H__

#ifdef ANDROID
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#endif

#include <glm/glm.hpp>

class Vertex
{
protected:
public:
	static void setAttribPointers(int &index, int totalSize = size())
	{
	}
	static const int size() { return 0; };
};


#define VertexDefBegin(className, memberName, memberType, count, base)	\
	class className : public base \
	{ \
	public: \
	\
		static void setAttribPointers(int &index, int totalSize = size()) \
		{\
			base::setAttribPointers(index, totalSize);\
			glVertexAttribPointer(index++, count, GL_FLOAT, GL_FALSE, totalSize, (void*)base::size());\
		}\
		static int size() { return base::size() + count*sizeof(GL_FLOAT); }\
		\
		memberType memberName; \
		\

#define VertexDefEnd() }





VertexDefBegin(VertexPosition3,								position,	glm::vec3, 3, Vertex)
	VertexPosition3(glm::vec3 position) : position(position) {};
VertexDefEnd();

VertexDefBegin(VertexPosition2,								position,	glm::vec2, 2, Vertex)
	VertexPosition2(glm::vec2 position) : position(position) {};
VertexDefEnd();

VertexDefBegin(VertexPosition3Texture2,						texCoord,	glm::vec2, 2, VertexPosition3)
	VertexPosition3Texture2(glm::vec3 position, glm::vec2 texCoord) : VertexPosition3(position), texCoord(texCoord) {};
VertexDefEnd();

VertexDefBegin(VertexPosition3Texture2Color4,				color,		glm::vec4, 4, VertexPosition3Texture2)
	VertexPosition3Texture2Color4(glm::vec3 position, glm::vec2 texCoord, glm::vec4 color) : VertexPosition3Texture2(position, texCoord), color(color) {};
VertexDefEnd();



#endif
