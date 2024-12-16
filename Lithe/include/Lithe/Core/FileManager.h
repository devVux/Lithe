#pragma once

#include "Lithe/Lithe.h"

#include <string>
#include <fstream>

namespace Lithe {

	struct File {
		const std::string path;
		std::string content;
	};

	namespace FileManager {

		static File load(const std::string& filename) {

			std::string absolutePath = ASSETS_DIR + filename;

			std::ifstream istream(absolutePath, std::ios::in);

			Lithe::Log::INFO("Reading \"{}\"", absolutePath);
			if (!istream.is_open())
				Lithe::Log::ERROR("Error while reading {}", absolutePath);

			std::stringstream buffer;
			buffer << istream.rdbuf();

			istream.close();

			return File(absolutePath, buffer.str());
		}

	}

}