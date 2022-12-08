#include <internal/attribute.h>
#include <plano_api.h>

// C to Instance adaptor
std::string Prop_Serialize(const attr_table& Prop_In, unsigned long& entries)
{
    return Prop_In.serialize(entries);
}

void Prop_Deserialize(attr_table& Prop_In, const std::string& serialized_table)
{
    return Prop_In.deseralize(serialized_table);
}


std::string attr_table::serialize(unsigned long &entries) const
{
    std::string serialization;
    entries =  0;
    
    for(const auto& kv : pstring) {
        serialization.append(kv.first + "\n"); // write key
        serialization.append(kv.second + "\n"); // write value
        serialization.append("s\n"); // write type flag;
        entries++; // track property count
    }
    
    for(const auto& kv : pint) {
        serialization.append(kv.first + "\n"); // write key
        serialization.append(std::to_string(kv.second) + "\n"); // write value
        serialization.append("i\n"); // write type flag;
        entries++; // track property count
    }
    
    for(const auto& kv : pfloat) {
        serialization.append(kv.first + "\n"); // write key
        serialization.append(std::to_string(kv.second) + "\n"); // write value
        serialization.append("f\n"); // write type flag;
        entries++; // track property count
    }
    
    for(const auto& kv : pbool) {
        serialization.append(kv.first + "\n"); // write key
        if(kv.second) {
            serialization.append("1\n"); // write true
        } else {
            serialization.append("0\n"); // write false
        }
        serialization.append("b\n"); // write type flag;
        entries++; // track property count
    } ///////////////// HEY DO THE DESERIALIZER AND YOU SHOULD BE DONE AND GET BACK TO THE CHECKBOX
    
    
    
    return serialization;
}

void attr_table::clear(void) {
    pstring.clear();
    pint.clear();
    pfloat.clear();
}

void attr_table::deseralize(const std::string& serialized_table) {
    clear(); // attr_table::clear();
    if(serialized_table.size() == 0)
        return;

    std::istringstream iss(serialized_table);
    std::string line1, line2, line3;



    for(int line_num = 0; std::getline(iss,line1); line_num +=3) {
        std::getline(iss,line2);
        std::getline(iss,line3);
        switch (line3.at(0)) {
            case 's' : pstring[line1] = line2;            break;
            case 'f' : pfloat[line1]  = std::stof(line2); break;
            case 'i' : pint[line1]    = std::stoi(line2); break;
            case 'b' : pbool[line1]   = std::stoi(line2) == 1 ? true : false;
                        
        }
    }
}
