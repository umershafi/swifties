// mime_types.cc
#include "mime_types.h"

namespace mime_types {

std::string extension_to_type(const std::string& extension) {
    for (const mapping& m : mappings) {
        if (m.extension == extension) {
            return m.mime_type;
        }
    }
    return "text/plain";
}

} // namespace mime_types
