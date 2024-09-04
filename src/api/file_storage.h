#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#include <string>

// file storage class to handle file I/O for CRUD api calls
class FileStorage {
public:
    FileStorage(const std::string& base_path);
    
    void write(const std::string& entity, int id, const std::string& data);
    std::string read(const std::string& entity, int id);
    void remove(const std::string& entity, int id);
    std::string getPath(const std::string& entity, int id) const;

private:
    std::string base_path_;
};

#endif // FILE_STORAGE_H