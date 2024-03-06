#include "Tcpserver.h"
#include "config.h"
// #include "AStar.h"
#include "RF.h"
#include "http_server.h"
// #include "python.hpp"
// #include <python3.10/Python.h>
using namespace lyserver;
static lyserver::Logger::ptr g_logger = LY_LOG_NAME("root1");
static lyserver::Logger::ptr system_logger = LY_LOG_NAME("system");
void run_RF()
{
    using namespace Random_Forest;
    RF::ptr rf(new RF);
    rf->load_inputList();
    rf->execute_model();

}
void run_AStar()
{
    AStar::Generator generator;
    generator.setWorldSize({8, 40}); //(6,36)
    generator.init_map();
    // AStar::CoordinateList path = generator.findPath({0, 0}, {2, 30});
    std::vector<AStar::Vec2i> array = {
        {3, 32},
        {0, 30}};
    AStar::CoordinateList path = generator.findbestpath({0, 0}, array);
    generator.print_path(path);
}
void run_Tcpserver()
{
    lyserver::TcpServer::ptr tcps(new lyserver::TcpServer);
    tcps->init();

    std::map<serverType, Address::ptr> addrs, fails;
    Address::ptr addr1 = lyserver::Address::LookupAny("0.0.0.0:8080");
    Address::ptr addr2 = lyserver::Address::LookupAny("0.0.0.0:8999");
    Address::ptr addr3 = lyserver::Address::LookupAny("0.0.0.0:8998");
    Address::ptr addr4 = lyserver::Address::LookupAny("0.0.0.0:8997");
    addrs.insert(pair<serverType, Address::ptr>(serverType::TCP, addr1));
    addrs.insert(pair<serverType, Address::ptr>(serverType::FTP, addr2));
    addrs.insert(pair<serverType, Address::ptr>(serverType::FTPDATA, addr3));
    addrs.insert(pair<serverType, Address::ptr>(serverType::UDP, addr4));
    tcps->bind(addrs, fails, false);

    tcps->start();
}
void run_RF_train()
{
    using namespace Random_Forest;
    RF_train::ptr RFT(new RF_train);

    std::vector<std::string> import_statements;

    // 添加导入语句到vector
    import_statements.push_back("import numpy as np");
    import_statements.push_back("import pandas as pd");
    import_statements.push_back("from sklearn.ensemble import RandomForestClassifier");
    import_statements.push_back("from sklearn.model_selection import train_test_split");
    import_statements.push_back("from sklearn.feature_extraction import DictVectorizer");
    import_statements.push_back("from sklearn.model_selection import cross_val_score, GridSearchCV");
    import_statements.push_back("from sklearn.metrics import accuracy_score, precision_score, f1_score, recall_score");
    import_statements.push_back("from sklearn.multioutput import MultiOutputClassifier");
    import_statements.push_back("from sklearn.pipeline import Pipeline");
    import_statements.push_back("import joblib");
    RFT->import_module(import_statements);
    RFT->load_function("sys.path.append('/home/ly/lyserver_master/other/RF/script')");
    RFT->load_data();
    std::string str;
    RFT->train_model("/home/ly/lyserver_master/other/RF/model", str);
}
// void run_python()
// {
//     using namespace pyApi;
//     auto py = Python();
//     py.run("import sys");
//     py.run("sys.path.append('/home/ly/lyserver_master/lyserver/test')");

//     try
//     {
//         Module module("test2");
//         PyFunction func(module, "add");
//         int result = func.call<int>(1, 2);
//         LY_LOG_INFO(g_logger)<< result;
//     }
//     catch (std::exception & e)
//     {
//         std::cout << e.what() << std::endl;
//     }
// }
void run_http()
{
    using namespace lyserver::http;
    lyserver::http::HttpServer::ptr http(new HttpServer(true));

    std::map<serverType, Address::ptr> addrs, fails;
    Address::ptr addr1 = lyserver::Address::LookupAny("0.0.0.0:8080");
    addrs.insert(pair<serverType, Address::ptr>(serverType::HTTP, addr1));

    http->init(http->make_cb());
    http->bind(addrs, fails, false);
    auto sd = http->getServletDispatch();
    sd->addServlet("/lyserver/xx", [](HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session){
        rsp->setBody("1234567890");
        return 0;
    });
    sd->addGlobServlet("/lyserver/*", [](HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session){
        rsp->setBody(req->toString());
        return 0;
    });
    http->start();
}
int main(int argc, const char *argv[])
{

    // YAML::Node log_yaml = YAML::LoadFile("/home/ly/lyserver_master/configuration/yaml/log.yaml");
    // lyserver::Config::LoadFromYaml(log_yaml);
    // run_RF();
    // run_python();
    // run_AStar();
    // run_RF_train();
    // run_Tcpserver();
    run_http();
    LY_LOG_ERROR(g_logger) << "main";
    return 0;
}