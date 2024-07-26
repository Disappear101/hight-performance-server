#include "src/db/mysql.h"
#include "src/log.h"
#include "src/iomanager.h"

static int s_num = 40;

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

void run2() {
    tao::MySQL::ptr mysql = tao::MySQLMgr::GetInstance()->get("chat1");
    tao::MySQLStmt::ptr stmt = tao::MySQLStmt::Create(mysql, "UPDATE test set score = ? where id = 2");
    stmt->bindInt32(1, 100);
    int rt = stmt->execute();
    std::cout << "rt=" << rt << std::endl;
}

int main(int argc, char** argv) {
    tao::IOManager iom(1);
    iom.schedule(run2);
    //iom.addTimer(1000, run, true);
    return 0;
}
