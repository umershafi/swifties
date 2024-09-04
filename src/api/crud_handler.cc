#include "crud_handler.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>

CRUDHandler::CRUDHandler(const std::string& base_path) : file_storage_(base_path) {}

int CRUDHandler::getNextId(const std::string& entity) {
    // start at id(1) if no other entitties
    int id = 1;
    // get next available id
    while (boost::filesystem::exists(file_storage_.getPath(entity, id))) {
        ++id;
    }
    return id;
}

std::string CRUDHandler::create(const std::string& entity, const std::string& data) {
    int id = getNextId(entity);
    file_storage_.write(entity, id, data);
    std::ostringstream oss;
    oss << "{\"id\": " << id << "}";
    return oss.str();
}

std::string CRUDHandler::read(const std::string& entity, int id) {
    return file_storage_.read(entity, id);
}

bool CRUDHandler::update(const std::string& entity, int id, const std::string& data) {
    if (!CRUDHandler::exists(entity, id)) {
        return false;
    }
    file_storage_.write(entity, id, data);
    return true;
}

bool CRUDHandler::delete_(const std::string& entity, int id) {
    if (!CRUDHandler::exists(entity, id)) {
        return false;
    }
    file_storage_.remove(entity, id);
    return true;
}

bool CRUDHandler::exists(const std::string& entity, int id) {
    if (boost::filesystem::exists(file_storage_.getPath(entity, id))) {
        return true;
    }
    return false;
}