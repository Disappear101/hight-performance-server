#include "src/db/mysql.h"
#include "src/log.h"
#include "src/iomanager.h"
#include "src/config.h"
#include "src/util.h"

static int s_num = 40;

static tao::Logger::ptr g_logger = TAO_LOG_NAME("system");

//set params in code
void run() {
    do {
        std::map<std::string, std::string> params;
        params["host"] = "127.0.0.1";
        params["user"] = "root";
        params["passwd"] = "123456";
        params["dbname"] = "chatdb";

        tao::MySQL::ptr mysql(new tao::MySQL(params));
        if(!mysql->connect()) {
            std::cout << "connect fail" << std::endl;
            return;
        }

        tao::MySQLStmt::ptr stmt = tao::MySQLStmt::Create(mysql, "UPDATE test set score = ? where id = 2");
        ++s_num;
        stmt->bindInt32(1, s_num);
        int rt = stmt->execute();
        std::cout << "rt=" << rt << std::endl;

    } while(false);
    std::cout << "over" << std::endl;
}

//set params by conf file
void run2() {
    TAO_LOG_DEBUG(g_logger) << "load conf files";
    tao::Config::LoadFromConfDir("/home/tao/projects/hight-performance-server/bin/conf");
    tao::MySQL::ptr mysql = tao::MySQLMgr::GetInstance()->get("chat1");
    tao::MySQLStmt::ptr stmt = tao::MySQLStmt::Create(mysql, "UPDATE test set score = ? where id = 2");
    tao::MySQLTransaction::ptr tranc = tao::MySQLTransaction::Create(mysql, true);
    stmt->bindInt32(1, 28);
    int rt = stmt->execute();
    std::cout << "rt=" << rt << std::endl;
    //tranc->rollback();
}

//execute sql statement from .sql file
void run3() {
    TAO_LOG_DEBUG(g_logger) << "load conf files";
    tao::Config::LoadFromConfDir("/home/tao/projects/hight-performance-server/bin/conf");
    tao::MySQL::ptr mysql = tao::MySQLMgr::GetInstance()->get("chat1");
    tao::MySQLTransaction::ptr tranc = tao::MySQLTransaction::Create(mysql, true);

    std::ifstream file;
    tao::FSUtil::OpenForRead(file, "/home/tao/projects/hight-performance-server/bin/sql/test.sql", std::ios_base::in);
    std::stringstream fileStream;
    fileStream << file.rdbuf();
    std::string sql = fileStream.str();

    //mysql->query(sql.c_str());
    mysql->execute(sql);
}

int main(int argc, char** argv) {
    tao::IOManager iom(1);
    iom.schedule(run3);
    //iom.addTimer(1000, run, true);
    return 0;
}
