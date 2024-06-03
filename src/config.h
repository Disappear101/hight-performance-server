#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include <memory>
#include <string>
#include <sstream>
#include <exception>
#include <boost/lexical_cast.hpp>
#include <unordered_map>
#include "log.h"

namespace sylar {

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

protected:
    std::string m_name;
    std::string m_description;
};

template<class T>
class ConfigVar : public ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVar>;

    ConfigVar(const std::string& name
            ,const T& default_val
            ,const std::string& description = std::string())
        :ConfigVarBase(name, description)
        ,m_val(default_val){

    }

    virtual ~ConfigVar() {}

    const T getValue() {
        return m_val;
    }

    void setValue(const T & v) {
        m_val = v;
    }

    std::string toString() override {
        try {
            return boost::lexical_cast<std::string>(m_val);
        } catch (std::exception& e) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString execption!"
                << e.what() << " convert: string to " << typeid(m_val).name(); 
        }
        return std::string();
    }

    bool fromString(const std::string& val) override {
        try {
            m_val = boost::lexical_cast<T>(val);
        } catch (std::exception& e) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exeception"
                << e.what() << " convert: string to " << typeid(m_val).name();
        }
        return false;
    }


private:
    T m_val;
};

class Config {
public:
    //the key of map cannot be template class, here used a father class to replace
    using ConfigVarMap = std::unordered_map<std::string, ConfigVarBase::ptr>;

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string & name
            ,const T& default_val, const std::string& description = std::string()) {
        auto tmp = Lookup<T>(name);
        if (tmp) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "look up name = " << name << "exists";
            return tmp;
        }

        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_") 
                != std::string::npos) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "lookup name invalid " << name;
            throw std::invalid_argument(name);
        }
        typename ConfigVar<T>::ptr v = std::make_shared<ConfigVar<T>>(name, default_val, description);
        GetDatas()[name] = v;
        return v;
    }

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string & name) {
        auto it = GetDatas().find(name);
        if (it == GetDatas().end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

private:
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }
};



}

#endif