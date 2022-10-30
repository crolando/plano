#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

/* Attribute.h
 *
 * Types for an attribute table.  An attribute is a key-value pair
 * where the key is always a string
 * and the value is a union-like type of a string, float or integer
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

/* Type: attr_value
 * This is basically a float / int / string union, that supports serailzation.
 *
 * Set with the set functions (passing in a float int or string)
 *
 * Get by one-two punch:
 *    1. Evaluate the return of get_type()
 *    2. Then you can safely read from the similar get_??? function.
 *
 * To seralize the value, use this pattern that is the C "multiple return" pattern.
 *    1. The caller creates a attr_type variable on the stack
 *    2. The caller passes that variable as the first argument to get_serial_value()
 *    3. The caller can use the value from the function return,
 *       and the type will be stored in the varable from 1 (modified in pass by reference function)
*/
class attr_value
{
public:
    // Default constructor so I can use map["blah"] = newValue;
    attr_value(){};

    // Copy constructors
    attr_value(const attr_string& Value);
    attr_value(const attr_fnumber Value);
    attr_value(const attr_inumber Value);

    // Setters
    void set(const attr_string& value);
    void set(const double value);
    void set(const long value);

    // Getters.  use get_type, then call the appropriate get_????();
    attr_type get_type(void);
    attr_string& get_string(void);
    attr_fnumber get_float(void);
    attr_inumber get_integer(void);

    // Seralizer.  This modifies the 1st argument to point to the type.
    std::string get_serial_value(attr_type& type) const;

private:
    attr_type       t;
    attr_string     s;
    attr_fnumber    f = 0.0;
    attr_inumber    i = 0;    
};

/* Type: attr_table
 *
 * This is a container.  It is a "map" where strings are the key,
 * and the values are attr_values, which you can read about above.
 * This container also supports seralization and deserialization.
 *
 * To get an attribute, please test to make sure its there first using has_attr(Key);
 * then, you use get_attr(key) - this gives you the attr_value object.  To use the
 * value, you'll have to follow the above instructions.
 *
 * To intialize a new key-value pair just use set_attr(key,value);
 *
 * To overwrite an existing one, just use     set_attr(key,value);
 *
 * To serialize, the caller will create a int varaiable.  Then, it calls serialize(int).
 * The caller has the return value, which is the serial data, and the int is set to the
 * number of lines in the serial data.  That int can be helpful in writing file parsers.
 *
 * To deserialize, you take the output of a previous call to serialize, and then just
 * feed that string into deseralize(string);
*/

class attr_table {
public:
    // guard for getters
    bool has_attr(const attr_name& Key);

    // String setters
    void set_attr(const attr_name& Key, attr_string& Value);
    void set_attr(const attr_name& Key, const char* Value);
    // Number setters
    void set_attr(const attr_name& Key, attr_fnumber Value);
    void set_attr(const attr_name& Key, attr_inumber Value);

    // Getter.  See attr_value documentation for safe use of return value;
    attr_value& get_attr(const attr_name& Key);

    // serializer.
    std::string serialize(int& entries) const;

    // deseralizer
    void deseralize(const std::string& serialized_table);


private:
    std::map <attr_name, attr_value> table;
};

// C to Instance adaptor to comply with API needs.
std::string Prop_Serialize(const attr_table& Prop_In, int& entries);
void Prop_Deserialize(attr_table& Prop_In, const std::string& serialized_table);


#endif // ATTRIBUTE_H
