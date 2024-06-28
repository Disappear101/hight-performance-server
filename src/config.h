#ifndef __TAO_CONFIG_H__
#define __TAO_CONFIG_H__

#include <memory>
#include <string>
#include <sstream>
#include <exception>
#include <list>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <functional>
#include "log.h"
#include <yaml-cpp/yaml.h>
#include "mutex.h"

namespace tao {

//
template<class SRC, class DEST>
class Lexical_Cast {
public:
    DEST operator()(const SRC & v) {
        return boost::lexical_cast<DEST>(v);
    }
};

//partial specialization for vector
//Config::loadfromyaml(file -> "[10, 20]") -> ConfigVar::fromstring -> 
//Current template class("[10, 20]" -> nodes -> vector<T>)
template<class T>
class Lexical_Cast<std::string, std::vector<T> > {
public:
    std::vector<T> operator()(const std::string & v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i){
            ss.str("");
            ss << node[i];
            vec.push_back(Lexical_Cast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

//partial specialization for vector
template<class T>
class Lexical_Cast<std::vector<T>, std::string> {
public:
    std::string operator()(std::vector<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        typename std::vector<T> vec;
        for (auto& i : v) {
            node.push_back(YAML::Load(Lexical_Cast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class Lexical_Cast<std::string, std::list<T> > {
public:
    std::list<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> list;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            list.push_back(Lexical_Cast<std::string, T>()(ss.str()));
        }
        return list;
    }
};

template<class T>
class Lexical_Cast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(Lexical_Cast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class Lexical_Cast<std::string, std::set<T> > {
public:
    std::set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(Lexical_Cast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class Lexical_Cast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(Lexical_Cast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class Lexical_Cast<std::string, std::unordered_set<T> > {
public:
    std::unordered_set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(Lexical_Cast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class Lexical_Cast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(Lexical_Cast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class Lexical_Cast<std::string, std::unordered_map<std::string, T> > {
public:
    std::unordered_map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin();
                it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),
                        Lexical_Cast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

template<class T>
class Lexical_Cast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()( std::unordered_map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(Lexical_Cast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

template<class T>
class Lexical_Cast<std::string, std::map<std::string, T> > {
public:
    std::map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin();
                it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),
                        Lexical_Cast<std::string, T>()(ss.str())));
        }
        return vec;
    }
};

template<class T>
class Lexical_Cast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(std::map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(Lexical_Cast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

class ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVarBase>;
    /**
     * @brief 构造函数
     * @param[in] name 配置参数名称[0-9a-z_.]
     * @param[in] description 配置参数描述
     */
    ConfigVarBase(const std::string& name, const std::string& description = "")
        :m_name(name)
        ,m_description(description) {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }

    virtual ~ConfigVarBase() {}

    const std::string & getName() const { return m_name;}
    const std::string & getDescription() const { return m_description;}

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;
    virtual std::string getTypeName() const = 0;

protected:
    std::string m_name;
    std::string m_description;
};

//how to address partial specialization
template<class T, class FromStr = Lexical_Cast<std::string, T>
                , class ToStr = Lexical_Cast<T, std::string> >
class ConfigVar : public ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVar>;
    using on_change_cb = std::function<void (const T& old_value, const T& new_value)>; 
    using RWMutexType = RWMutex;

    ConfigVar(const std::string& name
            ,const T& default_val
            ,const std::string& description = std::string())
        :ConfigVarBase(name, description)
        ,m_val(default_val){

    }

    virtual ~ConfigVar() {}

    const T getValue() {
        RWMutexType::ReadLock lock(m_mutex);
        return m_val;
    }

    void setValue(const T & v) {
        {
            RWMutexType::ReadLock lock(m_mutex);
            if (v == m_val) {
            return;
            }
            for (auto& i : m_cbs) {
                i.second(m_val, v);
            }
        }
        RWMutexType::WriteLock lock(m_mutex);
        m_val = v;
    }

    std::string toString() override {
        try {
            //return boost::lexical_cast<std::string>(m_val);
            RWMutexType::ReadLock lock(m_mutex);
            return ToStr()(m_val);
        } catch (std::exception& e) {
            TAO_LOG_ERROR(TAO_LOG_ROOT()) << "ConfigVar::toString execption!"
                << e.what() << " convert: string to " << typeid(m_val).name(); 
        }
        return std::string();
    }

    bool fromString(const std::string& val) override {
        try {
            //m_val = boost::lexical_cast<T>(val);
            setValue(FromStr()(val));
        } catch (std::exception& e) {
            TAO_LOG_ERROR(TAO_LOG_ROOT()) << "ConfigVar::fromString exeception"
                << e.what() << " convert: string to " << typeid(m_val).name();
        }
        return false;
    }
    std::string getTypeName() const override { return typeid(T).name(); }

    uint64_t addListener(on_change_cb cb) {
        static uint64_t s_fun_id = 0;
        RWMutexType::WriteLock lock(m_mutex);
        ++s_fun_id;
        m_cbs[s_fun_id] = cb;
        return s_fun_id;
    }
    void delListener(uint64_t key) {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.erase(key);
    }
    on_change_cb getListener(uint64_t key) {
        RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }
    void clearListener() {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.clear();
    }
private:
    RWMutexType m_mutex;
    T m_val;
    //change call back. std::function has no operatpr==, so use map to wrap it
    std::map<uint64_t, on_change_cb> m_cbs;
};

class Config {
public:
    //the key of map cannot be template class, here used a father class to replace
    using ConfigVarMap = std::unordered_map<std::string, ConfigVarBase::ptr>;
    using RWMutexType = RWMutex;
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string & name
            ,const T& default_val, const std::string& description = std::string()) {
        RWMutexType::WriteLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if (it != GetDatas().end()) {
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
            if (tmp) {
                TAO_LOG_INFO(TAO_LOG_ROOT()) << "Lookup name=" << name << " exists";
                return tmp;
            } else {
                TAO_LOG_ERROR(TAO_LOG_ROOT()) << "Lookup name=" << name << " exists but type not "
                        << typeid(T).name() << " real_type=" << it->second->getTypeName()
                        << " " << it->second->toString();
                return nullptr;
            }
        }-

        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_") 
                != std::string::npos) {
            TAO_LOG_ERROR(TAO_LOG_ROOT()) << "lookup name invalid " << name;
            throw std::invalid_argument(name);
        }
        typename ConfigVar<T>::ptr v = std::make_shared<ConfigVar<T>>(name, default_val, description);
        GetDatas()[name] = v;
        return v;
    }

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string & name) {
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if (it == GetDatas().end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);
    static ConfigVarBase::ptr LookupBase(const std::string& name);

    //retrieve and modify all configvar
    static void Visit(std::function<void(ConfigVarBase::ptr)> cb);
private:
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }

    static RWMutexType& GetMutex() {
        static RWMutexType s_mutex;
        return s_mutex;
    }
};


}

#endif