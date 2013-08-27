#include "SpriteBatch.h"
#include "Shader.h"
#include "Font.h"
#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <json/json.h>
#include <blib/Texture.h>

namespace blib
{
	SpriteBatch::SpriteBatch(Renderer* renderer)
	{
		active = false;
		this->renderer = renderer;
	}

	void SpriteBatch::initGl()
	{
		shader = Shader::fromData<Shader>("\
precision mediump float;\
attribute vec2 a_position;\
attribute vec2 a_texture;\
attribute vec4 a_color;\
varying vec2 texCoord;\
varying vec4 color;\
uniform mat4 matrix;\
uniform mat4 projectionmatrix;\
void main()\
{\
	color = a_color;\
	texCoord = a_texture;\
	gl_Position = projectionmatrix * matrix * vec4(a_position,0.0,1.0);\
}\
", "\
precision mediump float;\
uniform sampler2D s_texture;\
varying vec2 texCoord;\
varying vec4 color;\
void main()\
{\
	gl_FragColor = color*texture2D(s_texture, texCoord);\
}\
");
		vertices.reserve(MAX_SPRITES);
	/*	int index = 0;
		unsigned short* indices = new unsigned short[MAX_SPRITES*6];
		for(int i = 0; i < MAX_SPRITES; i+=6)
		{
			indices[i+0] = index+0;
			indices[i+1] = index+1;
			indices[i+2] = index+2;

			indices[i+3] = index+1;
			indices[i+4] = index+3;
			indices[i+5] = index+2;

			index+=4;
		}
		vio.setData(MAX_SPRITES*6, indices, GL_STATIC_DRAW);
		delete[] indices;*/
	}

	void SpriteBatch::begin(glm::mat4 matrix)
	{
		assert(!active);
		active = true;
		depth = 0;
		spriteCount = 0;
		currentTexture = NULL;
		this->matrix = matrix;
		materialIndices.clear();
		//vbo.mapData(GL_WRITE_ONLY);		
		vertices.clear();
	}

	void SpriteBatch::end()
	{
		assert(active);
	//	vbo.unmapData();
		active = false;

		if(spriteCount == 0)
			return;

		materialIndices.push_back(std::pair<Texture*, unsigned short>(currentTexture, (spriteCount/4) * 6));


		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	//	glEnableVertexAttribArray(2);
		shader->use();
		shader->setUniform("matrix", matrix);
		//vbo.bind();
		//vio.bind();
		//vertexDef::setAttribPointers();

		int lastIndex = 0;
		for(size_t i = 0; i < materialIndices.size(); i++)
		{
			//materialIndices[i].first->use();
//			glBindTexture(GL_TEXTURE_2D, materialIndices[i].first->texid);
//			glDrawElements(GL_TRIANGLES, materialIndices[i].second - lastIndex, GL_UNSIGNED_SHORT, (GLvoid*)(lastIndex * sizeof(unsigned short)));
			
			RenderState::activeRenderState->activeTexture[0] = materialIndices[i].first;
			RenderState::activeRenderState->activeShader = shader;


			renderer->drawTriangles<vertexDef>(*RenderState::activeRenderState, vertices);

			lastIndex = materialIndices[i].second;
		}

		active = false;
	}


	//TODO: make overload without src rectangle, so it doesn't have to clean it up
	void SpriteBatch::draw( Texture* texture, glm::mat4 transform, glm::vec2 center, blib::math::Rectangle src, glm::vec4 color)
	{
		assert(active);

		float fw = (float)src.width();
		float fh = (float)src.height();

		if(currentTexture != texture && currentTexture != NULL)
			materialIndices.push_back(std::pair<Texture*, unsigned short>(currentTexture, (spriteCount/4) * 6));
		currentTexture = texture;

/*		vertices[spriteCount].position = glm::vec2(transform * glm::vec4(fw*0 - center.x,						fh*0 - center.y,								0,1));		//1
		vertices[spriteCount].texCoord = glm::vec2(src.topleft.x,src.topleft.y);
		vertices[spriteCount].color = color;
//			vbo[spriteCount].position.z = (float)depth*0.01f;
		spriteCount++;

		vertices[spriteCount].position = glm::vec2(transform * glm::vec4(fw*0 - center.x,						fh*texture->originalHeight - center.y,		0,1));			//2
		vertices[spriteCount].texCoord = glm::vec2(src.topleft.x,src.bottomright.y);
		vertices[spriteCount].color = color;
//			vbo[spriteCount].position.z = (float)depth*0.01f;
		spriteCount++;

		vertices[spriteCount].position = glm::vec2(transform * glm::vec4(fw*texture->originalWidth - center.x,	fh*0 - center.y,							0,1));			//3
		vertices[spriteCount].texCoord = glm::vec2(src.bottomright.x,src.topleft.y);
		vertices[spriteCount].color = color;
//			vbo[spriteCount].position.z = (float)depth*0.01f;
		spriteCount++;


		vertices[spriteCount].position = glm::vec2(transform * glm::vec4(fw*0 - center.x,						fh*texture->originalHeight - center.y,		0,1));			//2
		vertices[spriteCount].texCoord = glm::vec2(src.topleft.x,src.bottomright.y);
		vertices[spriteCount].color = color;
		//			vbo[spriteCount].position.z = (float)depth*0.01f;
		spriteCount++;

		vertices[spriteCount].position = glm::vec2(transform * glm::vec4(fw*texture->originalWidth - center.x,	fh*0 - center.y,							0,1));			//3
		vertices[spriteCount].texCoord = glm::vec2(src.bottomright.x,src.topleft.y);
		vertices[spriteCount].color = color;
		//			vbo[spriteCount].position.z = (float)depth*0.01f;
		spriteCount++;

		vertices[spriteCount].position = glm::vec2(transform * glm::vec4(fw*texture->originalWidth - center.x,	fh*texture->originalHeight - center.y,	0,1));				//4
		vertices[spriteCount].texCoord = glm::vec2(src.bottomright.x,src.bottomright.y);
		vertices[spriteCount].color = color;
//			vbo[spriteCount].position.z = (float)depth*0.01f;
		spriteCount++;*/


		vertices.push_back(vertexDef(glm::vec2(transform * glm::vec4(fw*0 - center.x,						fh*0 - center.y,								0,1)),	glm::vec2(src.topleft.x,src.topleft.y), color)); // 1
		vertices.push_back(vertexDef(glm::vec2(transform * glm::vec4(fw*0 - center.x,						fh*texture->originalHeight - center.y,		0,1)),		glm::vec2(src.topleft.x,src.bottomright.y), color)); // 2
		vertices.push_back(vertexDef(glm::vec2(transform * glm::vec4(fw*texture->originalWidth - center.x,	fh*0 - center.y,							0,1)),		glm::vec2(src.bottomright.x,src.topleft.y), color)); // 3

		vertices.push_back(vertexDef(glm::vec2(transform * glm::vec4(fw*0 - center.x,						fh*texture->originalHeight - center.y,		0,1)), 		glm::vec2(src.topleft.x,src.bottomright.y), color)); // 2
		vertices.push_back(vertexDef(glm::vec2(transform * glm::vec4(fw*texture->originalWidth - center.x,	fh*0 - center.y,							0,1)),		glm::vec2(src.bottomright.x,src.topleft.y), color)); // 3
		vertices.push_back(vertexDef(glm::vec2(transform * glm::vec4(fw*texture->originalWidth - center.x,	fh*texture->originalHeight - center.y,	0,1)),			glm::vec2(src.bottomright.x,src.bottomright.y), color)); //4
	
		spriteCount+=6;

		depth++;
	}

	void SpriteBatch::draw( gl::TextureMap::TexInfo* texture, glm::mat4 transform )
	{
		assert(active);

	}

	void SpriteBatch::draw( Font* font, std::string text, glm::mat4 transform, glm::vec4 color )
	{
		glm::vec2 texFactor(1.0f / font->texture->width, 1.0f / font->texture->height);

		float x = 0;
		for(size_t i = 0; i < text.size(); i++)
		{
			if(font->charmap.find(text[i]) == font->charmap.end())
				continue;
			Glyph* g = font->charmap[text[i]];
			draw(font->texture, glm::translate(transform, glm::vec3(x+g->xoffset,g->yoffset,0)), glm::vec2(0,0), blib::math::Rectangle(g->x*texFactor.x,g->y*texFactor.y,g->width*texFactor.x,g->height*texFactor.y), color);

			x+=g->xadvance;
		}
	}

	void SpriteBatch::drawStretchyRect( Texture* sprite, glm::mat4 transform, blib::math::Rectangle src, blib::math::Rectangle innerSrc, glm::vec2 size, glm::vec4 color )
	{

		glm::vec2 factor(1.0f / sprite->width, 1.0f / sprite->height);

		glm::vec2 marginBottomRight(src.bottomright - innerSrc.bottomright);
		glm::vec2 marginTopLeft(innerSrc.topleft - src.topleft);

		float facWidth = (size.x-marginBottomRight.x-marginTopLeft.x) / innerSrc.width();
		float facHeight = (size.y-marginBottomRight.y-marginTopLeft.y) / innerSrc.height();

		draw(sprite, transform, glm::vec2(0,0), blib::math::Rectangle(src.topleft * factor, innerSrc.topleft * factor), color); //topleft
		draw(sprite, glm::translate(transform, glm::vec3(size.x - marginBottomRight.x, 0,0)), glm::vec2(0,0), blib::math::Rectangle(glm::vec2(innerSrc.bottomright.x , src.topleft.y) * factor, glm::vec2(src.bottomright.x, innerSrc.topleft.y) * factor), color); //topright
		draw(sprite, glm::translate(transform, glm::vec3(0, size.y - marginBottomRight.y,0)), glm::vec2(0,0), blib::math::Rectangle(glm::vec2(src.topleft.x, innerSrc.bottomright.y) * factor, glm::vec2(innerSrc.topleft.x, src.bottomright.y) * factor), color); //bottomleft
		draw(sprite, glm::translate(transform, glm::vec3(size.x - marginBottomRight.x, size.y - marginBottomRight.y,0)), glm::vec2(0,0), blib::math::Rectangle(innerSrc.bottomright * factor, src.bottomright * factor), color); //bottomright

		draw(sprite, glm::scale(glm::translate(transform, glm::vec3(marginTopLeft.x, 0,0)), glm::vec3(facWidth,1,1)), glm::vec2(0,0), blib::math::Rectangle(glm::vec2(innerSrc.topleft.x , src.topleft.y) * factor, glm::vec2(innerSrc.bottomright.x,innerSrc.topleft.y) * factor), color); //top
		draw(sprite, glm::scale(glm::translate(transform, glm::vec3(marginTopLeft.x, size.y - marginBottomRight.y,0)), glm::vec3(facWidth,1,1)), glm::vec2(0,0), blib::math::Rectangle(glm::vec2(innerSrc.topleft.x , innerSrc.bottomright.y) * factor, glm::vec2(innerSrc.bottomright.x,src.bottomright.y) * factor), color); //bottom

		draw(sprite, glm::scale(glm::translate(transform, glm::vec3(0,marginTopLeft.y,0)), glm::vec3(1,facHeight,1)), glm::vec2(0,0), blib::math::Rectangle(glm::vec2(src.topleft.x , innerSrc.topleft.y) * factor, glm::vec2(innerSrc.topleft.x,innerSrc.bottomright.y) * factor), color); //left
		draw(sprite, glm::scale(glm::translate(transform, glm::vec3(size.x - marginBottomRight.x, marginTopLeft.y,0)), glm::vec3(1,facHeight,1)), glm::vec2(0,0), blib::math::Rectangle(glm::vec2(innerSrc.bottomright.x, innerSrc.topleft.y) * factor, glm::vec2(src.bottomright.x,innerSrc.bottomright.y) * factor), color); //right

		draw(sprite, glm::scale(glm::translate(transform, glm::vec3(marginTopLeft,0)), glm::vec3(facWidth,facHeight,1)), glm::vec2(0,0), blib::math::Rectangle(innerSrc.topleft * factor, innerSrc.bottomright * factor), color); //center
	}

	void SpriteBatch::drawStretchyRect(Texture* sprite, glm::mat4 transform, Json::Value skin, glm::vec2 size, glm::vec4 color)
	{
		drawStretchyRect(
			sprite, 
			transform, 
			blib::math::Rectangle(glm::vec2(skin["left"]["pos"].asInt(), skin["top"]["pos"].asInt()), glm::vec2(skin["right"]["pos"].asInt() + skin["right"]["width"].asInt(), skin["bottom"]["pos"].asInt() + skin["bottom"]["height"].asInt())), 
			blib::math::Rectangle(glm::vec2(skin["left"]["pos"].asInt()+skin["left"]["width"].asInt(), skin["top"]["pos"].asInt()+skin["top"]["height"].asInt()), glm::vec2(skin["right"]["pos"].asInt(), skin["bottom"]["pos"].asInt())), 
			size, 
			color);
	}




	void SpriteBatch::Shader::resizeGl( int width, int height )
	{
		use();
		setUniform("projectionmatrix", glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1000.0f, 1.0f));
	}

	void SpriteBatch::Shader::init()
	{
		link();
		use();
		setUniform("a_position", 0);
		setUniform("a_texture", 1);
	}
}