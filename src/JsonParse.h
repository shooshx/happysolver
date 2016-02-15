#pragma once

#include <vector>
#include <string>
#include <memory>

namespace tinyjson {

using namespace std;

struct JsonBase
{
    enum Type {
        STR, INT, LST, OBJ
    };
    JsonBase(Type t) : type(t) {}
    Type type;
    virtual ~JsonBase() {}
};

struct JsonInt : public JsonBase
{
    JsonInt(int v) : JsonBase(INT), value(v) {}
    int value;
};

struct JsonStr : public JsonBase
{
    JsonStr(const string& v) : JsonBase(STR), value(v) {}
    string value;
};

struct JsonLst : public JsonBase
{
    JsonLst() : JsonBase(LST) {}
    vector<unique_ptr<JsonBase>> values;
};

struct JsonObj : public JsonBase
{
    JsonObj() : JsonBase(OBJ) {}
    vector<pair<string, unique_ptr<JsonBase>>> values;
};

unique_ptr<JsonBase> parse(const char* text);


}