#include "Shader.h"
#include <blib/util/Log.h>
#include <blib/gl/Shader.h>
using blib::util::Log;

#include <gl/glew.h>


namespace blib
{
	void Shader::setState( char* state )
	{
	/*	for(std::map<std::string,	float>::iterator it = state.floatValues.begin(); it != state.floatValues.end(); it++)
			doUniform(it->first, it->second);
		for(std::map<std::string,	int>::iterator it = state.intValues.begin(); it != state.intValues.end(); it++)
			doUniform(it->first, it->second);
		for(std::map<std::string,	glm::vec2>::iterator it = state.vec2Values.begin(); it != state.vec2Values.end(); it++)
			doUniform(it->first, it->second);
		for(std::map<std::string,	glm::vec3>::iterator it = state.vec3Values.begin(); it != state.vec3Values.end(); it++)
			doUniform(it->first, it->second);
		for(std::map<std::string,	glm::vec4>::iterator it = state.vec4Values.begin(); it != state.vec4Values.end(); it++)
			doUniform(it->first, it->second);
		for(std::map<std::string,	glm::mat3>::iterator it = state.mat3Values.begin(); it != state.mat3Values.end(); it++)
			doUniform(it->first, it->second);
		for(std::map<std::string,	glm::mat4>::iterator it = state.mat4Values.begin(); it != state.mat4Values.end(); it++)
			doUniform(it->first, it->second);*/

		for (int i = 0; i < uniformCount; i++)
		{
			if (uniforms[i])
			{
				int location = glGetUniformLocation(((blib::gl::Shader*)this)->programId, uniforms[i]->name.c_str());
				switch (uniforms[i]->type)
				{
				case Float:
					glUniform1f(location, (float&)state[uniforms[i]->index]);
					break;
				case Int:
					glUniform1i(location, (int&)state[uniforms[i]->index]);
					break;
				case Mat4:
					glUniformMatrix4fv(location, 1, 0, &((float&)state[uniforms[i]->index]));
					break;
				default:
					Log::out << "Error in uniform type" << Log::newline;
				}

			}
		}

	}

	void Shader::bindAttributeLocation( std::string name, int index )
	{
		attributes[name] = index;
	}

	void Shader::initFromData( std::string vertexShader, std::string fragmentShader )
	{
		this->vertexShader = vertexShader;
		this->fragmentShader = fragmentShader;
	}

	Shader::Shader()
	{
		uniformCount = 0;
		uniformSize = 0;
		uniformData = NULL;
		for (int i = 0; i < 16; i++)
			uniforms[i] = NULL;
	}

	void Shader::finishUniformSetup()
	{
		uniformData = new char[uniformSize];
	}


}