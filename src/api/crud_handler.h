#ifndef CRUD_HANDLER_H
#define CRUD_HANDLER_H

#include <string>
#include "file_storage.h"

// CRUD handler interface class to handle CRUD operations as assigned by the api request handler
class ICRUDHandler {
public:
    ICRUDHandler() = default;
    virtual std::string create(const std::string& entity, const std::string& data) = 0;
    virtual std::string read(const std::string& entity, int id) = 0;
    virtual bool update(const std::string& entity, int id, const std::string& data) = 0;
    virtual bool delete_(const std::string& entity, int id) = 0;
    virtual bool exists(const std::string& entity, int id) = 0;
    virtual ~ICRUDHandler() = default;
};

class CRUDHandler : public ICRUDHandler{
public:
    CRUDHandler(const std::string& base_path);
    
    std::string create(const std::string& entity, const std::string& data) override;
    std::string read(const std::string& entity, int id) override;
    bool update(const std::string& entity, int id, const std::string& data) override;
    bool delete_(const std::string& entity, int id) override;
    bool exists(const std::string& entity, int id) override;

private:
    FileStorage file_storage_;
    int getNextId(const std::string& entity);
};

#endif // CRUD_HANDLER_H