// mime_types.h
#ifndef MIME_TYPES_H
#define MIME_TYPES_H

#include <string>

namespace mime_types {

struct mapping {
    const char* extension;
    const char* mime_type;
};

// Add your MIME mappings here
const mapping mappings[] = {
    {"gif", "image/gif"},
    {"htm", "text/html"},
    {"html", "text/html"},
    {"jpg", "image/jpeg"},
    {"png", "image/png"},
    {"pdf", "application/pdf"}
};

std::string extension_to_type(const std::string& extension);

} // namespace mime_types

#endif // MIME_TYPES_H
