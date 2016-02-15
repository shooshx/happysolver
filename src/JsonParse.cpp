#include "JsonParse.h"

#include <stdlib.h> // atoi

namespace tinyjson {

class JsonException : public std::exception {
public:
    JsonException(const char* msg) : m_msg(msg) {}
    virtual const char* what() const throw() {
        return m_msg;
    }
    const char* m_msg;
};

#define JCHECK(cond) do { if (!(cond)) throw JsonException(#cond); } while(0)

struct Parser 
{
    const char* pos;


    void skipws() {
        while (*pos == ' ' || *pos == '\n' || *pos == '\r' || *pos == '\t')
            ++pos;
    }
    char readc() {
        skipws();
        char c = *pos;
        ++pos;
        return c;
    }
    string readTill(char d) {
        auto start = pos;
        while (*pos != d)
            ++pos;
        ++pos; // skip the d
        return string(start, pos - 1 - start);
    }
    int readNum() {
        auto start = pos;
        while (*pos >= '0' && *pos <= '9')
            ++pos;
        return atoi(start);
    }

    unique_ptr<JsonBase> readValue();

    unique_ptr<JsonBase> parseList()
    {
        auto r = new JsonLst;
        while (true)
        {
            auto val = readValue();
            r->values.push_back(std::move(val));
            auto comma = readc();
            if (comma == ']')
                break;
            JCHECK(comma == ',');
        }
        return unique_ptr<JsonBase>(r);
    }

    unique_ptr<JsonBase> parseObj()
    {
        auto r = new JsonObj;
        while (true)
        {
            auto q = readc();
            JCHECK(q == '"');
            string name = readTill('"');
            auto col = readc();
            JCHECK(col == ':');
            auto val = readValue();
            r->values.push_back(make_pair(name, std::move(val)));
            auto comma = readc();
            if (comma == '}')
                break;
            JCHECK(comma == ',');
        }
        return unique_ptr<JsonBase>(r);
    }
};

unique_ptr<JsonBase> Parser::readValue()
{
    auto nx = readc();
    unique_ptr<JsonBase> val;
    if (nx == '"')
        val.reset(new JsonStr(readTill('"')));
    else if (nx == '[')
        val = parseList();
    else if (nx == '{')
        val = parseObj();
    else if (nx >= '0' && nx <= '9')
        val.reset(new JsonInt(readNum()));
    return val;
}



unique_ptr<JsonBase> parse(const char* text)
{
    Parser p{text};
    p.skipws();
    auto c = p.readc();
    JCHECK(c == '{');
    return p.parseObj();
}




}