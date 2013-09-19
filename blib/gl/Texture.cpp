#define _CRT_SECURE_NO_WARNINGS
#include "Texture.h"

#include <blib/util/Log.h>
using blib::util::Log;
#include <blib/util/FileSystem.h>
#include <blib/util/stb_image.h>
#include <string.h>


namespace blib
{
	std::map<std::string, Texture*> Texture::textureCache;

	Texture* Texture::loadCached(std::string fileName, int loadOptions)
	{
		if(textureCache.find(fileName) == textureCache.end())
			textureCache[fileName] = new gl::Texture(fileName, loadOptions);
		return textureCache[fileName];
	}


	void Texture::clearCache()
	{
		for(std::map<std::string, Texture*>::iterator it = textureCache.begin(); it != textureCache.end(); it++)
		{
			delete it->second;
		}
		textureCache.clear();
	}


	namespace gl
	{


		Texture::Texture()
		{
			this->texid = 0;
			this->data = NULL;
		}

		Texture::Texture(std::string fileName, int loadOptions)
		{
			this->texid = 0;
			this->data = NULL;
			fromFile(fileName, loadOptions);
		}


		Texture::Texture(unsigned char* data, int width, int height)
		{
			this->texid = 0;
			this->data = NULL;
			fromData(data, width, height);
		}





		void Texture::fromFile(std::string fileName, int loadOptions)
		{
			char* fileData = NULL;
			int length = blib::util::FileSystem::getData(fileName, fileData);
			if(length <= 0)
			{
				Log::err<<"Error loading texture '"<<fileName<<"'"<<Log::newline;
				return;
			}

			int depth;
			unsigned char* tmpData = stbi_load_from_memory((stbi_uc*)fileData, length, &originalWidth, &originalHeight, &depth, 0);
			delete[] fileData;
			data = new unsigned char[originalWidth*originalHeight*depth];
			memcpy(data, tmpData, originalWidth*originalHeight*depth);
			stbi_image_free(tmpData);

			if(data)
			{
				width = originalWidth;
				height = originalHeight;
				//make 4 bits
				if(depth == 3)
				{
					unsigned char* newData = new unsigned char[width*height*4];
					for(int y = 0; y < height; y++)
					{
						for(int x = 0; x < width; x++)
						{
							newData[4*(x+width*y)+0] = data[3*(x+width*y)+0];
							newData[4*(x+width*y)+1] = data[3*(x+width*y)+1];
							newData[4*(x+width*y)+2] = data[3*(x+width*y)+2];
							newData[4*(x+width*y)+3] = 255;
						}
					}
					delete[] data;
					data = newData;
					depth = 4;
				}


				//TODO: resize texture to power of 2 texture

				//fix transparency
				for(int y = 0; y < height; y++)
				{
					for(int x = 0; x < width; x++)
					{
						if(data[4*(x+width*y)+0] > 250 && data[4*(x+width*y)+1] < 5 && data[4*(x+width*y)+2] > 250)
						{
							int totalr = 0;
							int totalg = 0;
							int totalb = 0;
							int total = 0;
							for(int xx = -1; xx <= 1; xx++)
							{
								for(int yy = -1; yy <= 1; yy++)
								{
									int xxx = x+xx;
									int yyy = y+yy;
									if(xxx < 0 || xxx >= width || yyy < 0 || yyy >= height)
										continue;
									if(data[4*(xxx+width*yyy)+0] > 250 && data[4*(xxx+width*yyy)+1] < 5 && data[4*(xxx+width*yyy)+2] > 250)
										continue;
									if(data[4*(xxx+width*yyy)+3] == 0)
										continue;
									totalr += data[4*(xxx+width*yyy)+0];
									totalg += data[4*(xxx+width*yyy)+1];
									totalb += data[4*(xxx+width*yyy)+2];
									total++;
								}
							}
							if(total > 0)
							{
								data[4*(x+width*y)+0] = totalr / total;
								data[4*(x+width*y)+1] = totalg / total;
								data[4*(x+width*y)+2] = totalb / total;
							}
							data[4*(x+width*y)+3] = 0;
						}
					}
				}
				center = glm::vec2(originalWidth/2.0f, originalHeight/2.0f);
			}
		}



		void Texture::fromData(unsigned char* data, int width, int height)
		{
			this->width = this->originalWidth = width;
			this->height = this->originalHeight = height;


			/*
			FILE * shot;
			char buf[100];
			sprintf(buf, "Shot%i.tga", rand());
			if((shot=fopen(buf, "wb"))!=NULL)
			{
				unsigned char TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};

				int xfrom = 0;
				int yfrom = 0;
				int xto = width;
				int yto = height;

				int w= xto - xfrom;
				int h= yto - yfrom;

				unsigned char Header[6]={((int)(w%256)),((int)(w/256)),((int)(h%256)),((int)(h/256)),24,0};
				fwrite(TGAheader, sizeof(unsigned char), 12, shot);
				fwrite(Header, sizeof(unsigned char), 6, shot);

				for(int y = yfrom; y < yto; y++)
				{
					for(int x = xfrom; x < xto; x++)
					{
						fwrite(data+4*(x+y*width), sizeof(unsigned char), 3, shot);
					}
				}
				fclose(shot);
			}*/
			center = glm::vec2(originalWidth/2.0f, originalHeight/2.0f);
		}


		Texture::~Texture(void)
		{
			if(data != NULL)
				delete[] data;
			data = NULL;
			if(texid != 0)
				glDeleteTextures(1, &texid);
			texid = 0;
		}

		void Texture::use()
		{
			if(texid == 0 && data != NULL)
			{

				glGenTextures(1, &texid);
				glBindTexture(GL_TEXTURE_2D, texid);		
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

			/*	if((loadOptions & TextureWrap) == 0)
				{
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
				}*/

				//if((loadOptions & KeepData) == 0)
				{
					delete[] data;
					data = NULL;
				}
			}
			glBindTexture(GL_TEXTURE_2D, texid);
		}














		MultiTextureMap::MultiTextureMap(int width, int height)
		{
			this->width = width;
			this->height = height;
			this->count = 0;
			this->maxCount = 64;
			texid = 0;
		}



		MultiTextureMap::TexInfo* MultiTextureMap::addTexture(std::string filename)
		{
			if(info.find(filename) != info.end())
				return info[filename];

			char* fileData = NULL;
			int length = blib::util::FileSystem::getData(filename, fileData);
			if(length <= 0)
			{
				Log::err<<"Error loading texture "<<filename<<Log::newline;
				return NULL;
			}

			int depth = 4;
			int w,h;
			unsigned char* data = stbi_load_from_memory((stbi_uc*)fileData, length, &w, &h, &depth, 4);
			delete[] fileData;

			if(w != width)
				Log::err<<"Error loading texture "<<filename<<", width is not a multiple of 32"<<Log::newline;
			if(h != height)
				Log::err<<"Error loading texture "<<filename<<", height is not a multiple of 32"<<Log::newline;


//			stbi_image_free(data);
			count++;

			TexInfo* texinfo = new TexInfo();
			texinfo->data = data;
			texinfo->filename = filename;
			texinfo->t1 = glm::vec2(0,0);
			texinfo->t2 = glm::vec2(1,1);
			texinfo->depth = count-1;
			texinfo->texMap = this;
			texinfo->height = height;
			texinfo->width = width;
			texinfo->x = 0;
			texinfo->y = 0;

			info[filename] = texinfo;
			return texinfo;
		}

		void MultiTextureMap::use()
		{
			if(texid == 0)
			{
				glGenTextures(1,&texid);
				glBindTexture(GL_TEXTURE_2D_ARRAY,texid);
				glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, maxCount);

				glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_REPEAT);
			}
			glBindTexture(GL_TEXTURE_2D_ARRAY, texid);
			for(std::map<std::string, TexInfo*>::iterator it = info.begin(); it != info.end(); it++)
			{
				if(it->second->data)
				{
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, it->second->depth, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, it->second->data);
					stbi_image_free(it->second->data);
					it->second->data = NULL;

				}
			}
		}



	}
}