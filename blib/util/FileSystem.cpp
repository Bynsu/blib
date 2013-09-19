#include "FileSystem.h"
#include <blib/util/Log.h>
#include <json/json.h>
#include <fstream>

#ifdef WIN32
#include <Windows.h>
#endif

namespace blib
{
	namespace util
	{
		StreamInFile* PhysicalFileSystemHandler::openRead( const std::string &fileName )
		{
			std::ifstream* stream = new std::ifstream(directory + "/" + fileName, std::ios_base::binary);
			if(stream->is_open())
			{
				return new StreamInFilePhysical(stream);
			}
			delete stream;
			return NULL;

		}

		StreamOut* PhysicalFileSystemHandler::openWrite( const std::string &fileName )
		{
			throw std::exception("The method or operation is not implemented.");
		}

		void PhysicalFileSystemHandler::getFileList(const std::string &path, std::vector<std::string> &files)
		{
#ifndef WIN32
			DIR *dp;
			struct dirent *ep;
			dp = opendir(path.c_str());
			if(dp)
			{
				while((ep = readdir(dp)))
				{
					if(strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
						continue;
						
					struct stat stFileInfo;
					stat((directory + "/" + ep->d_name).c_str(), &stFileInfo);
					
					if((stFileInfo.st_mode & S_IFDIR))// && recursive)
					{
						/*vector<string> dirContents = getFiles(dir + "/" + ep->d_name, filter, recursive);
						for(unsigned int i = 0; i < dirContents.size(); i++)
							files.push_back(dirContents[i]);*/
						files.push_back(std::string(ep->d_name) + "/");
					}
					else
					{
//						if(fnmatch(filter.c_str(), ep->d_name,0) == 0)
							files.push_back(ep->d_name);
					}
				}
				closedir(dp);
			}
			else
				cLog::add("Could not open directory '%s'", directory.c_str());
#else
			WIN32_FIND_DATA FileData;													// thingy for searching through a directory
			HANDLE hSearch;	
			hSearch = FindFirstFile(std::string(path + "/*.*").c_str(), &FileData);
			if (hSearch != INVALID_HANDLE_VALUE)										// if there are results...
			{
				while (true)														// loop through all the files
				{ 
					if((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
					{
						files.push_back(std::string(FileData.cFileName) + "/");
					}
					else
						files.push_back(FileData.cFileName);

					if (!FindNextFile(hSearch, &FileData))								// find next file in the resultset
					{
						if (GetLastError() == ERROR_NO_MORE_FILES)						// we're finished when there are no more files
							break;
						else 
							return;													// wow, something really weird happened
					}
				}
				FindClose(hSearch);
			}
#endif			

		}


		PhysicalFileSystemHandler::StreamInFilePhysical::StreamInFilePhysical(std::ifstream* stream)
		{
			this->stream = stream;
		}

		PhysicalFileSystemHandler::StreamInFilePhysical::~StreamInFilePhysical()
		{
			stream->close();
			delete stream;
			stream = NULL;
		}

		unsigned int PhysicalFileSystemHandler::StreamInFilePhysical::read( char* data, int count )
		{
			stream->read(data, count);
			return (unsigned int)stream->gcount();
		}

		char PhysicalFileSystemHandler::StreamInFilePhysical::get()
		{
			return stream->get();
		}

		bool PhysicalFileSystemHandler::StreamInFilePhysical::eof()
		{
			return stream->eof();
		}

		void PhysicalFileSystemHandler::StreamInFilePhysical::seek( int offset, StreamOffset offsetTo )
		{
			if(offsetTo == StreamSeekable::BEGIN)
				stream->seekg(offset, std::ios_base::beg);
			else if(offsetTo == StreamSeekable::CURRENT)
				stream->seekg(offset, std::ios_base::cur);
			else if(offsetTo == StreamSeekable::END)
				stream->seekg(offset, std::ios_base::end);
		}

		unsigned int PhysicalFileSystemHandler::StreamInFilePhysical::tell()
		{
			return (unsigned int)stream->tellg();
		}

		bool PhysicalFileSystemHandler::StreamInFilePhysical::opened()
		{
			if(!stream)
				return false;
			return stream->is_open();
		}





















		std::list<FileSystemHandler*> FileSystem::handlers;

		StreamInFile* FileSystem::openRead( const std::string &fileName )
		{
			for(std::list<FileSystemHandler*>::iterator it = handlers.begin(); it != handlers.end(); it++)
			{
				StreamInFile* stream = (*it)->openRead(fileName);
				if(stream)
					return stream;
			}
			return NULL;
		}

		void FileSystem::registerHandler( FileSystemHandler* handler )
		{
			handlers.push_back(handler);
		}

		int FileSystem::getData(const std::string &fileName, char* &data)
		{
			StreamInFile file(fileName);
			if(!file.opened())
				return 0;
			file.seek(0, StreamSeekable::END);
			unsigned int size = file.tell();
			data = new char[size];
			file.seek(0, StreamSeekable::BEGIN);
			file.read(data, size);
			return size;
		}
		std::string FileSystem::getData(const std::string &fileName)
		{
			StreamInFile file(fileName);
			if(!file.opened())
				return "";
			std::string ret;
			char buf[1024];

			while(!file.eof())
			{
				int rc = file.read(buf, 1024);
				ret += std::string(buf, rc);
			}
			return ret;
		}

		Json::Value FileSystem::getJson( const std::string &fileName )
		{
			std::string data = getData(fileName);
			if(data == "")
				return Json::nullValue;
			Json::Value ret;
			Json::Reader reader;
			if(!reader.parse(data, ret))
				Log::out<<reader.getFormattedErrorMessages()<<Log::newline;

			return ret;
		}


		std::vector<std::string> FileSystem::getFileList(const std::string &path)
		{
			std::vector<std::string> ret;
			for(std::list<FileSystemHandler*>::iterator it = handlers.begin(); it != handlers.end(); it++)
				(*it)->getFileList(path, ret);


			return ret;
		}

	}
}
