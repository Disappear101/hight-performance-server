#include "config.h"
#include "env.h"
#include <sys/stat.h>

//A:
//  B: 10
//  C: str
//flatten the tree to
//"A.B", 10
//"A.C", str
namespace tao {

static tao::Logger::ptr g_logger = TAO_LOG_NAME("system");

ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
    RWMutexType::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}

static void ListAllMember(const std::string& prefix, 
                            const YAML::Node& node, 
                            std::list<std::pair<std::string, const YAML::Node> >& output) {
    if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_") 
            != std::string::npos) {
        TAO_LOG_ERROR(TAO_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
    }
    output.push_back(std::make_pair(prefix, node));
    if (node.IsMap()) {
        for (auto it = node.begin();
                it != node.end(); ++it){
            ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(), it->second, output);
        }
    }
}

void Config::LoadFromYaml(const YAML::Node& root){
    std::list<std::pair<std::string, const YAML::Node> > all_nodes;
    ListAllMember("", root, all_nodes);

    for (auto& i : all_nodes) {
        std::string key = i.first;
        if (key.empty()) {
            continue;
        }
        ConfigVarBase::ptr var = LookupBase(key);

        if (var) {
            if (i.second.IsScalar()) {
                var->fromString(i.second.Scalar());
            } else {
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
            }
        }
    }
}

static std::map<std::string, uint64_t> s_file2modifytime;
static tao::Mutex s_mutex;

void Config::LoadFromConfDir(const std::string &path, bool force)
{
    std::string abs_path = tao::EnvMgr::GetInstance()->getAbsolutePath(path);
    std::vector<std::string> files;
    FSUtil::ListAllFile(files, abs_path, ".yml");

    for (auto& i : files) {
        {
            struct stat st;
            lstat(i.c_str(), &st);
            tao::Mutex::Lock lock(s_mutex);
            if (!force && s_file2modifytime[i] == (uint64_t)st.st_mtime) {
                continue;
            }
            s_file2modifytime[i] = st.st_mtime;
        }
        try {
            YAML::Node root = YAML::LoadFile(i);
            LoadFromYaml(root);
            TAO_LOG_INFO(g_logger) << "LoadConfFile file=" 
                << i << " ok";
        } catch (...) {
            TAO_LOG_ERROR(g_logger) << "LoadConFile file="
                << i << " failed";
        }
    }
}

void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb) {
    RWMutexType::ReadLock lock(GetMutex());
    ConfigVarMap& m = GetDatas();
    for(auto it = m.begin();
            it != m.end(); ++it) {
        cb(it->second);
    }
}


}