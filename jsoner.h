#ifndef JSONER_H
#define JSONER_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include <iostream>

namespace J {

using std::string;

/* Hlp contains small helping functions */
namespace Hlp {

/* extracts string from enclosed braces
 * ignores everything outside braces */
string extract_str(const string& str, const char b='"'){
    size_t l=str.find(b);
    return str.substr(l+1, str.find(b, l+1)-l-1);
}

size_t detect_closing_bracket(const string& str, size_t from, char l_bracket='{', char r_bracket='}'){

    using std::cout;
    using std::endl;

    size_t open_brackets=1;

    for (size_t i=from;i<str.size();++i){

//        cout << "open: " << open_brackets << endl;

        if (str[i]==l_bracket)
            ++open_brackets;

        if (str[i]==r_bracket)
            --open_brackets;

        if (open_brackets==0)
            return i;
    }

    return string::npos;
}

} //Hlp namespace

enum class JType{
    Number,
    Null,
    String,
    Object,
    Bool,
    Array
};

enum NType{
    i32=0,
    i64,
    d,
    ld
};

const char * ntype_tostr(NType t){
    switch (t) {
    case NType::i32:
        return "i32";
    case NType::i64:
        return "i64";
    case NType::d:
        return "double";
    case NType::ld:
        return "long double";
    }
}

JType detect_val_type(const std::string& str, size_t from=0){
    for (size_t j=from;j<str.size();++j){

        if (str[j]=='"')
            return JType::String;
        else if (str[j]=='t'||str[j]=='f')
            return JType::Bool;
        else if (str[j]=='n')
            return JType::Null;
        else if (::isdigit(str[j]))
            return JType::Number;
        else if (str[j]=='[')
            return JType::Array;
        else if (str[j]=='{')
            return JType::Object;
    }

    return JType::Null;
}

NType detect_num_type(const string& str){

    if (str.find_first_of("eE.")!=string::npos){

        long double res;

        res=std::stold(str);

        if (res==std::numeric_limits<double>::max())
            return NType::ld;
        else return NType::d;

    } else {

        if (str.size()>10){
            return NType::i64;
        }
        else return NType::i32;
    }
}

using std::string;
using std::stringstream;

using std::cout;
using std::endl;

std::string enclose(const std::string& str){
    return "\""+str+"\"";
}

/* Abstract property */

struct prop{

    prop()=default;

    prop(const std::string& name):m_name(name){}

    string Name(){ return m_name; }

    std::string m_name{""};

    virtual int getInt(){}
    virtual double getDouble(){}
    virtual int64_t getInt64(){}
    virtual long double getLDouble(){}
    virtual std::string getStr(){}
    virtual bool getBool(){}
    virtual std::string toStr() const {}
    virtual JType Type()=0;

    virtual ~prop(){}
};

template <typename T>
struct Num: prop{

    Num(const std::string& name,
        const T& val):prop(name),value(val){}

    Num(T val):value(val){}

    int64_t getInt64(){
        return value;
    }

    int32_t getInt(){
        return value;
    }

    double getDouble(){
        return value;
    }

    std::string toStr() const {
        string res=std::to_string(value);

        res.erase(res.find_last_not_of('0')+1, string::npos);
        res.erase(res.find_last_not_of('.')+1, string::npos);

        return enclose(m_name)+": "+res;
    }

    JType Type(){ return JType::Number; }

    T value;
};

struct Str: prop{

    Str(const std::string& name,
        const std::string &val):prop(name),value(val){}

    Str(const std::string& val):value(val){}

    std::string getStr(){
        return value;
    }

    std::string toStr() const {
        return enclose(m_name)+": "+enclose(value);
    }

    JType Type(){ return JType::String; }

    std::string value;
};

struct Boo: prop{

    Boo(const std::string& name,
        bool val):prop(name),value(val){}

    Boo(bool val):value(val){}

    bool getBool(){
        return value;
    }

    std::string toStr() const {
        return enclose(m_name)+": "+(value?"True":"False");
    }

    JType Type(){ return JType::Bool; }

    bool value;
};

struct Nul: prop{

    Nul(){}

    std::string toStr() const {
        return enclose(m_name)+": Null";
    }

    JType Type(){ return JType::Null; }

};
/* this is used when Null array is encountered */
struct Null_val {

};

template <typename T>
struct Arr: prop{

    Arr(std::vector<T> val):value(val){}

    ~Arr(){}

    std::string toStr() const {
        std::string res=enclose(m_name)+": ";

        res+="[ ";

        for (size_t i=0;i<value.size()-1;++i){
            res+=std::to_string(value[i])+", ";
        }

        res+=std::to_string(value[value.size()-1])+" ]";

        return res;
    }

    JType Type(){ return JType::Array; }

    std::vector<T> value;
};

template <>
string Arr<string>::toStr() const {
    std::string res=enclose(m_name)+": ";

    res+="[ ";

    for (size_t i=0;i<value.size()-1;++i){
        res+=enclose(value[i])+", ";
    }

    res+=enclose(value[value.size()-1])+" ]";

    return res;
}

template <>
string Arr<bool>::toStr() const {
    std::string res=enclose(m_name)+": ";

    res+="[ ";

    for (size_t i=0;i<value.size()-1;++i){
        res+=(value[i]?"true":"false");
        res+=", ";
    }

    res+=(value[value.size()-1]?"true":"false");
    res+=" ]";

    return res;
}

template <>
string Arr<Null_val>::toStr() const {
    std::string res=enclose(m_name)+": ";

    res+="[ ";

    for (size_t i=0;i<value.size()-1;++i)
        res+="Null, ";


    res+="Null ]";

    return res;
}

template <>
string Arr<double>::toStr() const {
    std::string res=enclose(m_name)+": ";

    res+="[ ";

    string tmp;

    for (size_t i=0;i<value.size()-1;++i){
        tmp=std::to_string(value[i]);

        tmp.erase(tmp.find_last_not_of('0')+1, string::npos);
        tmp.erase(tmp.find_last_not_of('.')+1, string::npos);

        res+=tmp+", ";
    }

    tmp=std::to_string(value[value.size()-1]);

    tmp.erase(tmp.find_last_not_of('0')+1, string::npos);
    tmp.erase(tmp.find_last_not_of('.')+1, string::npos);

    res+=tmp+" ]";

    return res;
}

template <>
string Arr<long double>::toStr() const {
    std::string res=enclose(m_name)+": ";

    res+="[ ";

    for (size_t i=0;i<value.size()-1;++i){
        string tmp=std::to_string(value[i]);

        tmp.erase(tmp.find_last_not_of('0')+1, string::npos);
        tmp.erase(tmp.find_last_not_of('.')+1, string::npos);

        res+=tmp+", ";
    }

    res+=std::to_string(value[value.size()-1])+" ]";

    return res;
}

struct Obj: prop{

    Obj(){}

    Obj(const std::string& name):prop(name){}

    void addProperty(const std::string& name, const int& value){
        prop* ptr=new Num<int>(name, value);
        props.push_back(ptr);
    }

    void addProperty(const std::string& name, const double& value){
        prop* ptr=new Num<double>(name, value);
        props.push_back(ptr);
    }

    void addProperty(const std::string& name, char * const value){
        prop* ptr=new Str(name, value);
        props.push_back(ptr);
    }

    void addProperty(const std::string& name, bool value){
        prop* ptr=new Boo(name, value);
        props.push_back(ptr);
    }

    template <typename T>
    void addProperty(const std::string& name, const std::vector<T>& tmp){
        prop* ptr=new Arr<T>(tmp);
        props.push_back(ptr);
    }

    void addObject(const std::string& json){
        Obj* ptr=new Obj();

        ptr->Parse(json);

        props.push_back(ptr);
    }

    void addObject(Obj& op2){
        this->addObject(op2.toStr());
    }

    prop* findProperty(const std::string& name){
        for (auto x: props){
            if (x->m_name==name){
                return x;
            }
        }
    }

    prop& operator[](const std::string& name){
        for (auto x: props)
            if (x->m_name==name)
                return *x;
    }

    std::string toStr() const {
        std::string res;

        if (!m_name.empty()){
            res=enclose(m_name)+": {";
        } else res="{";

        for (size_t i=0;i<props.size()-1;++i){
            res+=props[i]->toStr()+", ";
        }

        res+=props[props.size()-1]->toStr()+"}";

        return res;
    }

    void Parse(const string& input){

        size_t delim=input.find(':');
        size_t left=input.find('{');
        size_t right;

        if (left<delim){

            delim=0;
        } else {

            right=input.rfind('"', delim);
            left=input.rfind('"', right-1);

            if (left!=string::npos&&right!=string::npos)
                m_name=input.substr(left+1, right-left-1);
        }

//        cout << "Object: " << m_name << endl;

        /* name of next property */
        string child_name;

        /* value of next property */
        string val;

        for (;;){

            delim=input.find(':', delim+1);

            right=input.rfind('"', delim);
            left=input.rfind('"', right-1);

            child_name=input.substr(left+1, right-left-1);

            right=input.find_first_of(",}", delim);

            if (right==string::npos){
                break;
            }

//            cout << "child_name: " << child_name << endl;

            prop* child=nullptr;

            JType t=detect_val_type(input, delim+1);

            if (t==JType::Array){

//                cout << "Parsing Array" << endl;

                left=input.find('[', delim);

                right=Hlp::detect_closing_bracket(input, left+1, '[', ']');

                if (right==string::npos){
                    throw std::logic_error("'[' not closed "+std::to_string(left));
                }

//                cout << "val: " << input.substr(left, right-left+1) << endl;
                child=parse_arr(input.substr(left, right-left+1));

                delim=right;
            } else if (t==JType::Bool){

//                cout << "Parsing Bool" << endl;

                char c=input[input.find_first_of("tfTF", delim)];
//                cout << "val: " << (c=='t'||c=='T'?"True":"False") << endl;
                child=new Boo((c=='t'||c=='T')?true:false);

            } else if (t==JType::Null){

//                cout << "Parsing Null" << endl;

                child=new Nul();

            } else if (t==JType::Number){

//                cout << "Parsing number" << endl;

                left=input.find_first_of("-1234567890", delim);
                right=input.find_first_not_of("-1234567890.eE", left);

//                cout << "val: " << input.substr(left, right-left) << endl;
                child=parse_num(input.substr(left, right-left));

            } else if (t==JType::Object){

//                cout << "Parsing object" << endl;

                left=input.find('{', delim);
                right=Hlp::detect_closing_bracket(input, left+1);

                child=new Obj();

                ((Obj*)child)->Parse(input.substr(left, right-left+1));

            } else if (t==JType::String){

//                cout << "Parsing string" << endl;

                left=input.find('"', delim);
                right=input.find('"', left+1);

                val=input.substr(left+1, right-left-1);
//                cout << "val: " << val << endl;

                child=new Str(val);
            }

            if (child){
                child->m_name=child_name;
                props.push_back(child);
            }
        }
    }

    bool empty(){
        return (props.empty()&&m_name.empty());
    }

    void memfree(){
        for (auto x: props)
            delete x;
    }

    JType Type(){ return JType::Object; }

    std::vector<prop*>::iterator begin() {
        return props.begin();
    }

    std::vector<prop*>::iterator end() {
        return props.end();
    }

    size_t size() const {
        return props.size();
    }

    std::vector<prop*> props;

private:

    prop* parse_num(const string& input){

        NType nt=detect_num_type(input);

        switch (nt) {
        case NType::i32:
            return new Num<int32_t>(std::stol(input));
            break;
        case NType::i64:
            return new Num<int64_t>(std::stoll(input));
            break;
        case NType::d:
            return new Num<double>(std::stod(input));
            break;
        case NType::ld:
            return new Num<long double>(std::stold(input));
            break;
        }
    }

    prop* parse_arr(const string& input){

        using std::vector;

//        cout << "PARSE_ARR " << endl;
//        cout << input << endl;

        size_t left=input.find('[')+1;
        size_t right=input.find_first_of(",]", left);

        JType type=detect_val_type(input.substr(left, right-left));

        if (type==JType::String){

//            cout << "Detected type is String" << endl;

            std::vector<string> tmp;

            for (;right!=string::npos;left=input.find('"', right), right=input.find_first_of(",]", left))
                tmp.push_back(Hlp::extract_str(input.substr(left, right-left)));


            return new Arr<string>(tmp);
        } else if (type==JType::Bool){

//            cout << "Detected type is Bool" << endl;

            std::vector<bool> tmp;

            for (left=input.find_first_of("tf", 0);left!=string::npos;left=input.find_first_of("tf", left+1))
                tmp.push_back(input[left]=='t'?true:false);

            return new Arr<bool>(tmp);
        } else if (type==JType::Null){

//            cout << "Detected type is Null" << endl;

            size_t count=1;
            for (left=input.find(',', 0);left!=string::npos;++count, left=input.find(',', left+1));

            std::vector<Null_val> tmp(count);
            return new Arr<Null_val>(tmp);
        } else if (type==JType::Number){

//            cout << "Detected type is Num" << endl;

            NType arr_nt=NType::i32;

            vector<string> strnums;

            for (left=input.find_first_of("-123456789", 0);left!=string::npos;left=input.find_first_of("-1234567890", left+1)){

                right=input.find_first_not_of("-+1234567890.eE", left);

                strnums.push_back(input.substr(left, right-left));

                NType nt=detect_num_type(strnums.back());

                if (nt>arr_nt)
                    arr_nt=nt;

                left=right;
            }

            if (arr_nt==NType::i32){
                vector<int32_t> tmp;
                for (auto x: strnums)
                    tmp.push_back(std::stol(x));

                return new Arr<int32_t>(tmp);
            } else if (arr_nt==NType::i64){
                vector<int64_t> tmp;
                for (auto x: strnums)
                    tmp.push_back(std::stoll(x));

                return new Arr<int64_t>(tmp);
            } else if (arr_nt==NType::d){
                vector<double> tmp;
                for (auto x: strnums)
                    tmp.push_back(std::stod(x));

                return new Arr<double>(tmp);
            } else {
                vector<long double> tmp;
                for (auto x: strnums)
                    tmp.push_back(std::stold(x));

                return new Arr<long double>(tmp);
            }

            return nullptr;

        } else if (type==JType::Object){

//            cout << "Object detected" << endl;

            vector<Obj*> tmp;

            for (left=input.find('{', 0);left!=string::npos;left=input.find('{', right)){
                right=Hlp::detect_closing_bracket(input, left+1);

                Obj* t=new Obj();

                string val=input.substr(left, right-left+1);

                t->Parse(val);
                t->toStr();

                tmp.push_back(t);
            }

            return new Arr<Obj*>(tmp);

        } else if (type==JType::Array){

        } else {
//            cout << "SHIT" << endl;
            return nullptr;
        }
    }
};

template <>
string Arr<Obj*>::toStr() const {
    std::string res=enclose(m_name)+": ";

    res+="[ ";

    for (size_t i=0;i<value.size()-1;++i){
        res+=value[i]->toStr();

        res+=", ";
    }

    res+=value[value.size()-1]->toStr()+" ]";

    return res;
}

template <>
Arr<Obj*>::~Arr(){
    for (auto x: value){
        x->memfree();
        delete x;
    }
}

/* Main Object (Document) */

struct JSON{

    JSON(){}

    JSON(const std::string& name):m_obj(name){}

    ~JSON(){
        for (auto x: m_obj.props)
            delete x;
    }

    void Parse(const std::string& input){
        m_obj.Parse(input);
    }

    prop& operator[](const std::string& name){
        return m_obj[name];
    }

    Obj findObj(const std::string& name){
        if (name==m_obj.m_name)
            return m_obj;

        for (auto x: m_obj.props){
            if (x->m_name==name){
                return *dynamic_cast<Obj*>(x);
            }
        }

        return Obj();
    }

    void addProperty(const std::string& name, const int& value){
        prop* ptr=new Num<double>(name, value);
        m_obj.props.push_back(ptr);
    }

    void addProperty(const std::string& name, const double& value){
        prop* ptr=new Num<double>(name, value);
        m_obj.props.push_back(ptr);
    }

    void addProperty(const std::string& name, const int64_t& value){
        prop* ptr=new Num<int64_t>(name, value);
        m_obj.props.push_back(ptr);
    }

    void addProperty(const std::string& name, const long double& value){
        prop* ptr=new Num<long double>(name, value);
        m_obj.props.push_back(ptr);
    }

    void addProperty(const std::string& name, char * const value){
        prop* ptr=new Str(name, value);
        m_obj.props.push_back(ptr);
    }

    void addProperty(const std::string& name, bool value){
        prop* ptr=new Boo(name, value);
        m_obj.props.push_back(ptr);
    }

    template <typename T>
    void addProperty(const std::string& name, const std::vector<T>& tmp){
        prop* ptr=new Arr<T>(tmp);
        m_obj.props.push_back(ptr);
    }

    /* Add object in text representation */

    void addObject(const std::string& json){
        Obj* ptr=new Obj();

        ptr->Parse(json);

        m_obj.props.push_back(ptr);
    }

    /* op2 should be const but
     * toStr is not working */

    void addObject(const Obj& op2){
        Obj *ptr=new Obj();

        ptr->Parse(op2.toStr());

        m_obj.props.push_back(ptr);
    }

    std::string toStr(){
        return m_obj.toStr();
    }

    std::vector<prop*>::iterator begin(){
        return m_obj.begin();
    }

    std::vector<prop*>::iterator end(){
        return m_obj.end();
    }

private:
    Obj m_obj;
};

struct Jiter{

    Jiter()=default;

    Jiter(std::vector<prop*>::iterator op2){
        it=op2;
    }

    Jiter& operator=(std::vector<prop*>::iterator op2){
        it=op2;
    }

    Jiter& operator=(const Jiter& op2){
        it=op2.it;
    }

    Jiter& operator++(){
        ++it;
    }

    prop* operator->(){
        return (*it);
    }

    bool operator==(const Jiter& op2){
        return it==op2.it;
    }

    bool operator!=(const Jiter& op2){
        return it!=op2.it;
    }

    Obj* getObj(){
        return dynamic_cast<Obj*>(*it);
    }

    bool isNull(){
        return (*it)==nullptr;
    }

private:
    std::vector<prop*>::iterator it;
};

} //JSON namespace

#endif // JSONER_H
