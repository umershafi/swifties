#include "file_storage.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>

FileStorage::FileStorage(const std::string& base_path) : base_path_(base_path) {}

void FileStorage::write(const std::string& entity, int id, const std::string& data) {
    std::string path = getPath(entity, id);
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());

    std::ofstream ofs(path);
    ofs << data;
    ofs.flush();
    ofs.close();
}

std::string FileStorage::read(const std::string& entity, int id) {
    std::string path = getPath(entity, id);
    std::ifstream ifs(path);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

void FileStorage::remove(const std::string& entity, int id) {
    std::string path = getPath(entity, id);
    boost::filesystem::remove(path);
}

std::string FileStorage::getPath(const std::string& entity, int id) const {
    std::ostringstream oss;
    oss << base_path_ << entity << "/" << id;
    return oss.str();
}