#include "../src/config.h"
#include "../src/log.h"
#include <yaml-cpp/yaml.h>
//#include "tao/env.h"
#include <iostream>

std::string path = "/home/tao/projects/hight-performance-server/bin/conf/test.yml";

tao::ConfigVar<int>::ptr g_int_value_config =
    tao::Config::Lookup("system.port", (int)8080, "system port");

tao::ConfigVar<float>::ptr g_float_value_config =
    tao::Config::Lookup("system.value", (float)10.2f, "system value");

tao::ConfigVar<std::vector<int> >::ptr g_vec_value_config = 
    tao::Config::Lookup("system.int_vec", std::vector<int>{1, 2}, "system int vec");

tao::ConfigVar<std::list<int> >::ptr g_int_list_value_config =
    tao::Config::Lookup("system.int_list", std::list<int>{0,2}, "system int list");

tao::ConfigVar<std::set<int> >::ptr g_int_set_value_config =
    tao::Config::Lookup("system.int_set", std::set<int>{1,3}, "system int set");

tao::ConfigVar<std::unordered_set<int> >::ptr g_int_uset_value_config =
    tao::Config::Lookup("system.int_uset", std::unordered_set<int>{2,3}, "system int uset");

tao::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config =
    tao::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k",2}}, "system str int map");

tao::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_umap_value_config =
    tao::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"k",2}}, "system str int map");

void print_yaml(const YAML::Node& node, int level) {
    if(node.IsScalar()) {
        TAO_LOG_INFO(TAO_LOG_ROOT()) << std::string(level * 4, ' ')
            << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if(node.IsNull()) {
        TAO_LOG_INFO(TAO_LOG_ROOT()) << std::string(level * 4, ' ')
            << "NULL - " << node.Type() << " - " << level;
    } else if(node.IsMap()) {
        for(auto it = node.begin();
                it != node.end(); ++it) {
            TAO_LOG_INFO(TAO_LOG_ROOT()) << std::string(level * 4, ' ')
                    << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if(node.IsSequence()) {
        for(size_t i = 0; i < node.size(); ++i) {
            TAO_LOG_INFO(TAO_LOG_ROOT()) << std::string(level * 4, ' ')
                << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    YAML::Node root = YAML::LoadFile(path);
    //TAO_LOG_INFO(TAO_LOG_ROOT()) << root;
    print_yaml(root, 0);
}


void test_yaml2() {
    TAO_LOG_INFO(TAO_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    TAO_LOG_INFO(TAO_LOG_ROOT()) << "before: " << g_float_value_config->toString();
    
#define XX(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for (auto& i : v) { \
            TAO_LOG_INFO(TAO_LOG_ROOT()) << #prefix " " #name ": " << i; \
        } \
        TAO_LOG_INFO(TAO_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
            TAO_LOG_INFO(TAO_LOG_ROOT()) << #prefix " " #name ": {" \
                    << i.first << " : " << i.second << "}"; \
        } \
        TAO_LOG_INFO(TAO_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }
    
    XX(g_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_uset_value_config, int_uset, before);
    XX_M(g_str_int_map_value_config, str_int_map, before);
    XX_M(g_str_int_umap_value_config, str_int_umap, before);

    YAML::Node root = YAML::LoadFile(path);
    tao::Config::LoadFromYaml(root);

    XX(g_vec_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_uset_value_config, int_uset, after);
    XX_M(g_str_int_map_value_config, str_int_map, after);
    XX_M(g_str_int_umap_value_config, str_int_umap, after);
}

class Person {
public:
    Person() {};
    std::string m_name = "Unknow";
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name=" << m_name
           << " age=" << m_age
           << " sex=" << m_sex
           << "]";
        return ss.str();
    }

    bool operator==(const Person& oth) const {
        return m_name == oth.m_name
            && m_age == oth.m_age
            && m_sex == oth.m_sex;
    }
};

namespace tao {
template<>
class Lexical_Cast<std::string, Person> {
public:
    Person operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
};

template<>
class Lexical_Cast<Person, std::string> {
public:
    std::string operator()(const Person& p) {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
}

tao::ConfigVar<Person>::ptr g_person =
    tao::Config::Lookup("class.person", Person(), "system person");

tao::ConfigVar<std::map<std::string, Person> >::ptr g_person_map =
    tao::Config::Lookup("class.map", std::map<std::string, Person>(), "system person");

tao::ConfigVar<std::map<std::string, std::vector<Person> > >::ptr g_person_vec_map =
    tao::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Person> >(), "system person");

void test_class() {
    TAO_LOG_INFO(TAO_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();
#define XX_PM(g_var, prefix) \
    { \
        auto m = g_var->getValue(); \
            for(auto& i : m) { \
            TAO_LOG_INFO(TAO_LOG_ROOT()) <<  prefix << ": " << i.first << " - " << i.second.toString(); \
        } \
        TAO_LOG_INFO(TAO_LOG_ROOT()) <<  prefix << ": size=" << m.size(); \
    }

    g_person->addListener([](const Person& old_value, const Person& new_value){
        TAO_LOG_INFO(TAO_LOG_ROOT()) << "old_value = " << old_value.toString()
                << " new_value = " << new_value.toString();
    });

    XX_PM(g_person_map, "class.map before");
    TAO_LOG_INFO(TAO_LOG_ROOT()) << "before: " << g_person_vec_map->toString();

    YAML::Node root = YAML::LoadFile(path);
    tao::Config::LoadFromYaml(root);

    TAO_LOG_INFO(TAO_LOG_ROOT()) << "after: " << g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.map after");
    TAO_LOG_INFO(TAO_LOG_ROOT()) << "after: " << g_person_vec_map->toString();
}

void test_log() {
    static tao::Logger::ptr system_log = TAO_LOG_NAME("system");
    TAO_LOG_INFO(system_log) << "hello system" << std::endl;
    std::cout << tao::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile(path);
    tao::Config::LoadFromYaml(root);
    std::cout << "===================" << std::endl;
    std::cout << tao::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    std::cout << "===================" << std::endl;
    TAO_LOG_INFO(system_log) << "hello system" << std::endl;
}

int main(int argc, char** argv) {

    //test_yaml();
    //test_yaml2();
    //test_class();
    test_log();

    tao::Config::Visit([](tao::ConfigVarBase::ptr var) {
        TAO_LOG_INFO(TAO_LOG_ROOT()) << "name=" << var->getName()
                    << " description=" << var->getDescription()
                    << " typename=" << var->getTypeName()
                    << " value=" << var->toString();
    });
    return 0;
}