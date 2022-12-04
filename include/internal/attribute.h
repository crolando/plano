#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

/* Attribute.h
 * Seralize-able string tables to widget properties
 * using this lets node-based widgets maintain data between frames, and seralize/deserialze them
 * Seralized data lets you easily save/load the widget settings to disk files for long term storage.
 */

#include <string>
#include <sstream>
#include <map>

/* Type: attr_table
 *
*/

class attr_table {
public:
    std::map <std::string, std::string> pstring;
    std::map <std::string, int>         pint;
    std::map <std::string, float>       pfloat;
    
    // serializer.
    // returns the serialized text.
    // returns by reference "entries" which specifies the number of prperties in this table. (intent: when writing to a file for long term storage, the parser
    // that reads it can know in advance where in the file to stop parsing for properties.)
    std::string serialize(unsigned long& entries) const;

    // deseralizer
    void deseralize(const std::string& serialized_table);
    
    void clear(void);
};

// C to Instance adaptor to comply with API needs.
std::string Prop_Serialize(const attr_table& Prop_In, unsigned long& entries);
void Prop_Deserialize(attr_table& Prop_In, const std::string& serialized_table);


#endif // ATTRIBUTE_H
