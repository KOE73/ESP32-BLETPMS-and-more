#include "{TYPE_NAME}.hpp"

#include <stddef.h>
#include <sstream>

typedef struct {
    {VALUE_TYPE} value;
    const char* name;
} KeyValue;

static const KeyValue data[] = {
{DATA_ENTRIES}
};

std::string get_{TYPE_NAME}_name({VALUE_TYPE} code)
{
    for (size_t i = 0; data[i].name != nullptr; i++)
    {
        if (data[i].value == code)
        {
            return data[i].name;
        }
    }
    std::ostringstream oss;
    oss << "Unknown [" << static_cast<int>(code) << "]";
    return oss.str();
}