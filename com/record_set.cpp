#include "record_set.h"
#include <sstream>
#include <c++/iomanip>

std::string record_set::get_text(void) {
    std::ostringstream text;
    text << _col << "\x01" <<
    _row << "\x01";

    for (size_t i = 0; i < _fields.size(); i++) {
        text << _fields[i] << "\x01";
    }

    for (size_t i = 0; i < _values.size(); i++) {
        text << _values[i] << "\x01";
    }

    return text.str();
}

size_t record_set::set_text(const std::string &text, size_t pos) {
    _fieldNames.clear();
    _values.clear();
    _fields.clear();

    if (text.size() > 0) {
        size_t nitem = 0;
        long n = text.size();
        char field[1024];
        int j = 0;
        field[0] = '\0';
        for (int i = pos; i < n; i++) {
            if (text[i] == '\x01') {
                field[j] = '\0';
                nitem++;
                switch (nitem) {
                    case 1:
                        _col = atoi(field);
                        break;
                    case 2:
                        _row = atoi(field);
                        break;
                    default:
                        if (nitem - 2 <= _col)
                            add_field_name(field);
                        else
                            _values.push_back(field);

                        if (nitem - 2 - _col == _col * _row) {
                            pos = i + 1;
                            goto end;
                        }
                }

                j = 0;
                field[0] = '\0';
            }
            else {
                field[j++] = text[i];
            }
        }

        end:
        size_t tcol = _fields.size();
        size_t trow = 0;
        if (tcol > 0)
            trow = _values.size() / tcol;
        else
            trow = 0;

        if (tcol != _col) {
            throw (std::logic_error("解析失败，接收到的列数和收到的列不一致"));
        }

        if ((trow) != _row)
            throw (std::logic_error("解析失败，接收到的行数和收到的行不一致"));

    }
    else {
        _row = 0;
        _col = 0;
    }

    return pos;
}

void record_set::show_data(void) {
    //compute max item size
    size_t maxLen = 0;
    for (FIELDNAME::const_iterator it = _fieldNames.begin(); it != _fieldNames.end(); ++it) {
        maxLen = std::max(it->first.size(), maxLen);
        for (size_t i = 1; i <= this->get_row_size(); i++) {
            std::string v = this->get_field_value(it->second, i);
            maxLen = std::max(v.size(), maxLen);
        }
    }
    // Title
//    for(FIELDNAME::const_iterator it= _fieldNames.begin();it!= _fieldNames.end(); ++it){
//        std::cout << std::setw(maxLen+1) << it->first << "|";
//    }
    for (std::vector<std::string>::const_iterator it = _fields.begin(); it != _fields.end(); ++it) {
        std::cout << std::setw(maxLen + 1) << *it << "|";
    }

    std::cout << std::endl;
    // Value
//    for(size_t i=1;i<= this->get_row_size() ;i++) {
//        for(FIELDNAME::const_iterator it= _fieldNames.begin();it!= _fieldNames.end(); ++it){
//            std::string v = this->get_field_value(it->second, i);
//            std::cout << std::setw(maxLen+1) << v << "|";
//        }
//        std::cout << std::endl;
//    }

    for (size_t row = 1; row <= this->get_row_size(); row++) {
        for (size_t col = 0; col < _fields.size(); col++) {
            std::string v = this->get_field_value(col, row);
            std::cout << std::setw(maxLen + 1) << v << "|";
        }
        std::cout << std::endl;
    }
}