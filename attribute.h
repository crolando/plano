#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

/* Attribute.h
 * an "attribute" is a key-value pair, where the key is always a string
 * and the value can be a string, float or integer
 *
 * includes the "attribute table" class, which you would put on an object.
 * This lets you set and get attributes, and also can
 * serialize and deserialze the whole table, to file, for example.
 *
 */


#include <string>
#include <sstream>
#include <map>




typedef std::string attr_name;
typedef double      attr_fnumber;
typedef long        attr_inumber;
typedef std::string attr_string;

enum attr_type { string, fnumber, inumber};

class attr_value
{
public:
    // Default constructor so I can use map["blah"] = newValue;
    attr_value(){};
    attr_value(const attr_string& Value) {
        s = Value;
        t = string;
    };

    attr_value(const attr_fnumber Value) {
        f = Value;
        t = fnumber;
    }

    attr_value(const attr_inumber Value) {
        i = Value;
        t = inumber;
    }

    void set(const attr_string& value) {
        s = value;
        t = string;
        }
    void set(const double value) {
        f = value;
        t = fnumber;
    };
    void set(const long value) {
        i = value;
        t = inumber;
    };

    attr_type get_type(void) {return t;};
    attr_string& get_string(void) {return s;};
    attr_fnumber get_float(void) {return f;};
    attr_inumber get_integer(void) {return i;};
    std::string get_serial_value(attr_type& type) const {
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
private:
    attr_type       t;
    attr_string     s;
    attr_fnumber    f = 0.0;
    attr_inumber    i = 0;    
};

class attr_table {
public:
    bool has_attr(const attr_name& Key)
    {
        std::map <attr_name, attr_value>::iterator it;
        it = table.find(Key);
        if (it == table.end()) {
            return false;
        } else {
            return true;
        }
    }

    void set_attr(const attr_name& Key, attr_string& Value) {
        if(has_attr(Key) == false) {
            table[Key] = attr_value(Value);
        } else {
            table[Key].set(Value);
        }
    }

    // For ease of initialization
    void set_attr(const attr_name& Key, const char* Value) {
        if(has_attr(Key) == false) {
            table[Key] = attr_value(Value);
        } else {
            table[Key].set(Value);
        }
    }


    void set_attr(const attr_name& Key, attr_fnumber Value) {
        if(has_attr(Key) == false) {
            table[Key] = attr_value(Value);
        } else {
            table[Key].set(Value);
        }
    }

    void set_attr(const attr_name& Key, attr_inumber Value) {
        if(has_attr(Key) == false) {
            table[Key] = attr_value(Value);
        } else {
            table[Key].set(Value);

        }
    }

    attr_value& get_attr(const attr_name& Key) {
        return table[Key];
    }

    std::string serialize(int& entries) {
        std::string seralization;
        attr_type attribute_type;
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
        entries = count;
        return seralization;
    }

    void deseralize(const std::string& serialized_table) {
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


private:
    std::map <attr_name, attr_value> table;
};


#endif // ATTRIBUTE_H
