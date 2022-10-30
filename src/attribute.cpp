#include <internal/attribute.h>
#include <string>
#include <sstream>
#include <map>
#include <plano_api.h>


// C to Instance adaptor
std::string Prop_Serialize(const attr_table& Prop_In, int& entries)
{
    return Prop_In.serialize(entries);
}

void Prop_Deserialize(attr_table& Prop_In, const std::string& serialized_table)
{
    return Prop_In.deseralize(serialized_table);
}


attr_value::attr_value(const attr_string& Value) {
    s = Value;
    t = string;
}

attr_value::attr_value(const attr_fnumber Value) {
    f = Value;
    t = fnumber;
}

attr_value::attr_value(const attr_inumber Value) {
    i = Value;
    t = inumber;
}

void attr_value::set(const attr_string& value) {
    s = value;
    t = string;
}

void attr_value::set(const double value) {
    f = value;
    t = fnumber;
}

void attr_value::set(const long value) {
    i = value;
    t = inumber;
}

attr_type    attr_value::get_type   (void) {return t;};
attr_string& attr_value::get_string (void) {return s;};
attr_fnumber attr_value::get_float  (void) {return f;};
attr_inumber attr_value::get_integer(void) {return i;};

std::string  attr_value::get_serial_value(attr_type& type) const {
    switch (t) {
        case attr_type::string:
            type = attr_type::string;
            return s;
            break;
        case attr_type::fnumber:
            type = attr_type::fnumber;
            return std::to_string(f);
            break;
        case attr_type::inumber:
            type = attr_type::inumber;
            return std::to_string(i);
            break;
        }
}

bool attr_table::has_attr(const attr_name& Key)
{
    std::map <attr_name, attr_value>::iterator it;
    it = table.find(Key);
    if (it == table.end()) {
        return false;
    } else {
        return true;
    }
}

void attr_table::set_attr(const attr_name& Key, attr_string& Value) {
    if(has_attr(Key) == false) {
        table[Key] = attr_value(Value);
    } else {
        table[Key].set(Value);
    }
}

// For ease of initialization
void attr_table::set_attr(const attr_name& Key, const char* Value) {
    if(has_attr(Key) == false) {
        table[Key] = attr_value(Value);
    } else {
        table[Key].set(Value);
    }
}


void attr_table::set_attr(const attr_name& Key, attr_fnumber Value) {
    if(has_attr(Key) == false) {
        table[Key] = attr_value(Value);
    } else {
        table[Key].set(Value);
    }
}

void attr_table::set_attr(const attr_name& Key, attr_inumber Value) {
    if(has_attr(Key) == false) {
        table[Key] = attr_value(Value);
    } else {
        table[Key].set(Value);

    }
}

attr_value& attr_table::get_attr(const attr_name& Key) {
    return table[Key];
}


std::string attr_table::serialize(int &entries) const
{
    std::string seralization;
    attr_type attribute_type;
    entries = table.size();
    int count = 0;
    for (const auto& kv : table) {
        seralization.append(kv.first);
        seralization.append("\n");
        seralization.append(kv.second.get_serial_value(attribute_type));
        seralization.append("\n");
        switch(attribute_type) {
            case attr_type::string :
                seralization.append("s\n");
                break;
            case attr_type::fnumber:
                seralization.append("f\n");
                break;
            case attr_type::inumber:
                seralization.append("i\n");
                break;
        }
        count +=3;
    }
    return seralization;
}

void attr_table::deseralize(const std::string& serialized_table) {
    table.clear();
    if(serialized_table.size() == 0)
        return;

    std::istringstream iss(serialized_table);
    std::string line1, line2, line3;



    for(int line_num = 0; std::getline(iss,line1); line_num +=3) {
        std::getline(iss,line2);
        std::getline(iss,line3);
        if (line3.at(0) == 's') {
            set_attr(line1,line2);
        } else if (line3.at(0) == 'f') {
            set_attr(line1,std::stod(line2));
        } else {
            set_attr(line1,std::stol(line2));
        }
    }
}
