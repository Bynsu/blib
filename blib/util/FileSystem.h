#pragma once

#include <string>
#include <fstream>
#include <list>
#include <vector>

#include <blib/util/StreamInFile.h>
#include <blib/util/StreamSeekable.h>


namespace Json { class Value; }

namespace blib
{
	namespace util
	{
		class StreamOut;


		class FileSystemHandler
		{
		public:
			std::string name;
			FileSystemHandler(std::string name) : name(name) {};

			virtual StreamInFile* openRead(const std::string &fileName) { return NULL; };
			virtual StreamOut* openWrite(const std::string &fileName) { return NULL; };
			virtual void getFileList(const std::string &path, std::vector<std::string> &files) {};
		};

		class PhysicalFileSystemHandler : public FileSystemHandler
		{
			class StreamInFilePhysical : public StreamInFile
			{
				std::ifstream* stream;
			public:
				StreamInFilePhysical(std::ifstream* stream);
				~StreamInFilePhysical();
				virtual unsigned int read( char* data, int count );
				virtual char get();
				virtual bool eof();
				virtual void seek( int offset, StreamOffset offsetTo );
				virtual unsigned int tell();
				virtual bool opened();
			};
			std::string directory;
		public:
			PhysicalFileSystemHandler(std::string directory = ".") : FileSystemHandler("physical"), directory(directory) {}
			virtual StreamInFile* openRead( const std::string &fileName );
			virtual StreamOut* openWrite( const std::string &fileName );
			virtual void getFileList(const std::string &path, std::vector<std::string> &files);
		};

		class FileSystem
		{
		private:
			static std::list<FileSystemHandler*> handlers;
		public:
			static StreamInFile* openRead(const std::string &fileName);
			static StreamOut* openWrite(const std::string &fileName);
			static void registerHandler(FileSystemHandler* handler);
			static int getData(const std::string &fileName, char* &data);
			static std::string getData(const std::string &fileName);
			static Json::Value getJson(const std::string &fileName);
			static std::vector<std::string> getFileList(const std::string &path);
		};
	}
}