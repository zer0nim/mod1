#include <fstream>
#include <iostream>

#include "FileUtils.hpp"
#include "Logging.hpp"

namespace file {
	/**
	 * @brief Try if a file exist
	 *
	 * @param path The file to check
	 * @return true If the file exist
	 */
	bool isFile(std::string const & path) {
		std::filesystem::path	p1(path);

		return std::filesystem::is_regular_file(p1);
	}

	/**
	 * @brief Try if it's a directory
	 *
	 * @param path The directory path
	 * @return true If the directory exist
	 */
	bool	isDir(std::string const & path) {
		std::filesystem::path	p1(path);

		return std::filesystem::is_directory(p1);
	}

	/**
	 * @brief Create a directory
	 *
	 * @param path The path of the directory
	 * @param silent If true, don't show warning and errors
	 * @return false if failed to create directories
	 */
	bool	mkdir(std::string const & path, bool silent) {
		bool		error = false;
		std::string	msg = "";
		try {
			std::filesystem::create_directories(path);
			if (file::isDir(path) == false)
				error = true;
		}
		catch (std::exception const & e) {
			error = true;
			msg = std::string(" -> ") + e.what();
		}
		if (error) {
			if (!silent)
				logErr("Unable to create dir: " << path << msg);
			return false;
		}
		return true;
	}

	/**
	 * @brief Remove a file or a directory
	 *
	 * @param path The path to remove
	 * @param silent If true, don't show warning and errors
	 * @return false if failed to remove directory
	 */
	bool rm(std::string const & path, bool silent) {
		bool		error = false;
		std::string	msg = "";
		try {
			std::filesystem::remove_all(path);
			if (file::isDir(path))
				error = true;
		}
		catch (std::exception const & e) {
			error = true;
			msg = std::string(" -> ") + e.what();
		}
		if (error) {
			if (!silent)
				logErr("Unable to create dir: " << path << msg);
			return false;
		}
		return true;
	}

	/**
	 * @brief List files in a folder
	 *
	 * @param path The folder path
	 * @param silent If true, don't show warning and errors
	 * @return std::vector<std::string> The list of all paths
	 */
	std::vector<std::string>	ls(std::string const & path, bool silent) {
		std::vector<std::string> list;

		if (!isDir(path)) {
			if (!silent)
				logErr("In file::ls: " << path << " is not a valid directory");
			return list;
		}

		for (auto& p: std::filesystem::directory_iterator(path)) {
			if (std::filesystem::is_regular_file(p.path())) {
				std::string currentFile = p.path().string();
				list.push_back(currentFile);
			}
		}

		return list;
	}

}  // namespace file
