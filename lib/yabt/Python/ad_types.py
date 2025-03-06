import os
import subprocess
import yaml

REPO_URL = "https://bitbucket.org/bluetooth-SIG/public.git"
LOCAL_REPO_PATH = "lib/yabt/bluetooth-SIG"
YAML_FILE_PATH = os.path.join(LOCAL_REPO_PATH, "assigned_numbers/core/ad_types.yaml")
SOURCE_PATH = "lib/yabt/src/bluetooth-SIG"
SOURCE_H_FILE_PATH = os.path.join(SOURCE_PATH, "ad_types.hpp")
SOURCE_C_FILE_PATH = os.path.join(SOURCE_PATH, "ad_types.cpp")

# Step 1: Clone or update the repository
#def update_repo():
#    if not os.path.exists(LOCAL_REPO_PATH):
#        subprocess.run(["git", "clone", REPO_URL, LOCAL_REPO_PATH], check=True)
#    else:
#        subprocess.run(["git", "-C", LOCAL_REPO_PATH, "pull"], check=True)

# Step 2: Parse YAML and generate C code
def parse_yaml_and_generate_c():
    with open(YAML_FILE_PATH, "r", encoding="utf-8") as file:
        data = yaml.safe_load(file)
    
    identifiers = data.get("ad_types", [])
    
    # Generate C code
    header_content = """
#ifndef AD_TYPES_H
#define AD_TYPES_H

#include <string>

std::string get_ad_types_name(uint8_t code)

#endif // AD_TYPES_H
"""
    
    source_content = """#include "ad_types.hpp"

#include <stddef.h>

typedef struct {
    uint8_t value;
    const char* name;
} KeyValue;

static const KeyValue data[] = {
"""
    
    for entry in sorted(identifiers, key=lambda x: x["value"]):
        name = entry["name"].replace('"', '\\"')
        source_content += f'    {{ 0x{entry["value"]:02X}, "{name}" }},\n'
    
    source_content += """    { 0, NULL }
};

std::string get_ad_types_name(uint8_t code)
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
"""
    
    if not os.path.exists(SOURCE_PATH):
        os.makedirs(SOURCE_PATH)

    # Write to files
    with open(SOURCE_H_FILE_PATH, "w", encoding="utf-8") as h_file:
        h_file.write(header_content)
    
    with open(SOURCE_C_FILE_PATH, "w", encoding="utf-8") as c_file:
        c_file.write(source_content)

if __name__ == "__main__":
    # update_repo()
    parse_yaml_and_generate_c()
    print("C code generated successfully.")
