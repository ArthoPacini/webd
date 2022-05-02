#include <iostream>
#include <filesystem>
#include <set>
#include <fstream>
#include <string>
#include <vector>
#include <string_view>
#include <optional>
#include <unordered_map>
#include <algorithm>

#include "web_server/server_http.hpp"
#include "web_server/client_http.hpp"
#include "websocket/server_ws.hpp"
#include "json.hpp"

#include "node_utilities.hpp"

//using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

class Site{
    private:
        std::string m_domain;
        std::string m_description;
        std::vector<std::filesystem::path> m_file_paths;
        std::vector<std::string> m_file_paths_str;
        std::filesystem::path m_path;
        std::uint64_t m_version;
    public:
        Site(const std::string & domain, const std::string & description, const std::vector<std::string> & file_paths, const std::filesystem::path & path, std::uint64_t version) : m_domain(domain), m_description(description), m_path(path), m_version(version) 
        {
            m_file_paths_str = file_paths;
            for(const auto & fp: file_paths)
                if(!fp.empty())
                    m_file_paths.emplace_back(std::filesystem::path(fp));

        }
        Site(const std::string & domain, const std::string & description, std::vector<std::string> file_paths, std::uint64_t version)
        {
            m_domain = domain;
            m_description = description;
            m_version = version;
            auto p = get_www_path();
            p /= domain;
            m_path = p;
            m_file_paths_str = file_paths;
            for(const auto & fp: file_paths)
                if(!fp.empty())
                    m_file_paths.emplace_back(std::filesystem::path(fp));
        }

        auto domain() const { return m_domain; }
        auto description() const { return m_description; }
        auto file_paths() const { return m_file_paths; }
        auto file_paths_str() const { return m_file_paths_str; }
        auto path() const { return m_path; }
        auto version() const { return m_version; }
};

class Database{
    private:
        std::unordered_map<std::string, Site> m_sites;
        nlohmann::json m_www_database_index;
    public:
        Database()
        {
/*             //Load sites info
            std::cout << "[DATABASE]: Loading sites information into RAM..." << std::endl;
            m_www_database_index = load_www_database_index();
            std::cout << "[DATABASE]: Sites information successfully loaded into RAM..." << std::endl;


            //Parse and populate sites
            const auto www_folder = get_www_path();
            if(!std::filesystem::is_directory(www_folder))
            {
                std::cerr << "[DATABASE]: Not able to find 'www' directory, creating folder..." << std::endl;
                std::filesystem::create_directory(www_folder);
                std::cout << "[DATABASE]: 'www' directory folder created..." << std::endl;
                return;
            }

            std::cout << "[DATABASE]: found 'www' directory, iterating over directory entries..." << std::endl;

            for(const auto & entry : std::filesystem::directory_iterator(www_folder))
            {
                const std::string site_name = entry.path().stem().string() + entry.path().extension().string(); 
                std::cout << "[DATABASE]: inserting " << site_name << " into database" << std::endl;
                if(std::filesystem::is_directory(entry))
                    sites[site_name] = entry.path();
            }

            std::cout << "[DATABASE]: added " << sites.size() << " sites into database..." << std::endl;
 */
        }

        /* void get_site_files_structures(const std::filesystem::path & site_path, std::vector<std::string> & files)
        {
            for(const auto & entry : std::filesystem::directory_iterator(site_path))
            {
                if(std::filesystem::is_regular_file(entry))
                    files.push_back(entry.path().string());

                if(std::filesystem::is_directory(entry))
                    get_site_files_structures(entry, files);
            }
        }

        Site load_site_from_disk(const std::filesystem::path & site_path)
        {
            std::vector<std::string> files;
            get_site_files_structures(site_path, files);
            std::string name = site_path.stem().string() + site_path.extension().string();
            std::uint64_t version = 1;
            std::string description
        }

        void load_database_from_disk_structure()
        {
            const auto www_folder = get_www_path();
            if(!std::filesystem::is_directory(www_folder))
            {
                std::cerr << "[DATABASE]: Not able to find 'www' directory, creating folder..." << std::endl;
                std::filesystem::create_directory(www_folder);
                std::cout << "[DATABASE]: 'www' directory folder created..." << std::endl;
                return;
            }

            for(const auto & entry : std::filesystem::directory_iterator(www_folder))
            {
                const std::string site_name = entry.path().stem().string() + entry.path().extension().string(); 
                std::cout << "[DATABASE]: inserting " << site_name << " into database" << std::endl;
                if(std::filesystem::is_directory(entry))
                    sites[site_name] = entry.path();
            }
        } */

        //Try loading the database from www_database_index.json
        void load_database_from_file()
        {
            m_www_database_index = load_www_database_index();
            for(const auto & site : m_www_database_index)
            {
                const std::string & domain = site["domain"].get<std::string>();
                const std::string & description = site["description"].get<std::string>();
                const std::uint64_t & version = site["version"].get<std::uint64_t>();

                std::cout << "[DATABASE LOADING]: Loading site " << domain << std::endl;

                std::vector<std::string> files;
                for(const auto & file : site["files"])
                    files.emplace_back(file);
                

                auto site_path = get_www_path();
                site_path /= domain;

                m_sites.insert({domain, {domain, description, files, site_path, version} });
                std::cout << "[DATABASE LOADING]: Site " << domain << " loaded successfully!" << std::endl;

            }
        }
        
        //Try finding a site by domain
        std::optional<Site> find_site(const std::string & domain)
        {
            const auto & site_find = m_sites.find(domain);
            if(site_find == m_sites.end())
                return {};

            return site_find->second;
        }
        
        //Return the database in nlohmann json object
        const nlohmann::json & www_database_index() { return m_www_database_index; } 

        //Return an resume of the node sites information
        std::string resume() { return m_www_database_index.dump(4); }

        //Update json file
        void www_database_index_update(nlohmann::json & j)
        {
            m_www_database_index = j;
            std::ofstream file_out(www_database_index_path());
            file_out << j.dump(4);
            file_out.flush();
            m_sites.clear(); //reset all
            load_database_from_file(); //load all
        } 

        //Return true if site is newer or we don't have it, returns false if we already have it at current or major version 
        bool is_remote_file_newer(const std::string & domain, std::uint64_t version)
        {
            const auto & site_find = find_site(domain);
            if(site_find)
            {
                const auto & site = *site_find;
                return site.version() < version; 
            }

            return true;
        }

        void update_site(const std::string & domain, std::uint64_t version, const std::string & description, std::vector<std::string> files)
        {
            m_sites.insert_or_assign(domain, Site(domain, description, files, version));
        }

        //Updates json in RAM an saves to disk
        void update_all_sites_json()
        {
            nlohmann::json j;
            for(const auto & site : m_sites)
            {
                nlohmann::json s;
                s["domain"] = site.second.domain();
                s["description"] = site.second.description();
                s["version"] = site.second.version();

                const auto & files = site.second.file_paths_str();
                for(const auto & file : files)
                    s["files"].emplace_back(file);

                j.emplace_back(s);
                //site.second.path();
            }

                m_www_database_index = j;
                www_database_index_update(m_www_database_index);

        }
};

class Node{
    private:
        std::string m_name;
        std::string m_ip;
        std::string m_port;
    public:
        Node(const std::string & ip, const std::string & port, const std::string & name) : m_ip(ip), m_port(port), m_name(name) {}

        auto name() const { return m_name; }
        auto ip() const { return m_ip; }
        auto port() const { return m_port; }
        auto ip_with_port() const 
        {
            if(!m_port.empty()) 
                return m_ip + ":" + m_port;
            return m_ip;    
        }

        friend bool operator< (const Node& a, const Node& b) 
        {
            return a.name() < b.name();
        }
};



class RemoteNodes{
    private:
        Node m_this_node = {"localhost", "", "name"};
        std::set<Node> m_nodes;
    public:
        void insert(const Node & node) { m_nodes.insert(node);}
        void insert(const std::string & ip, const std::string & port, const std::string & name) { insert({ip, port, name}); }

        void erase(const Node & node) { m_nodes.erase(node); }
        void erase(const std::string & ip, const std::string & port, const std::string & name) { erase({ip, port, name}); }

        auto & nodes() const { return m_nodes; }

        void load_nodes()
        {
            const auto remote_nodes_information = load_remote_nodes_information();
            for(const auto & node : remote_nodes_information)
                insert({node["ip"].get<std::string>(), node["port"].get<std::string>(),  node["name"].get<std::string>()});

            std::cout << "\n[REMOTE NODES]: " << m_nodes.size() << " remote nodes loaded" << std::endl;
        }

        nlohmann::json known_nodes() const 
        {
            nlohmann::json nodes_result;
            for(const auto & node : m_nodes)
                nodes_result.push_back({{"name",node.name()},{"ip",node.ip()},{"port",node.port()}});

            return nodes_result;
        }

        void update_on_disk() const 
        {
            const auto known_nodes_to_update = known_nodes();
            std::ofstream remote_nodes_file(remote_nodes_information_file_path());
            remote_nodes_file << known_nodes_to_update.dump(4);
            remote_nodes_file.flush();
        }

        void update_this_node_info(const std::string & name, const std::string & port) 
        {
            m_this_node = Node("localhost", port, name);
        }

        void update_this_node_port_info(const std::string & port) 
        {
            m_this_node = Node(m_this_node.ip(), port, m_this_node.name());
        }

        void load_this_node_info() 
        {
            nlohmann::json this_node_info_json{{"name","WebD_Node"}};
            auto this_node_info_path = get_www_path();
            this_node_info_path /= "this_node_config.json";
            if(!std::filesystem::is_regular_file(this_node_info_path))
            {
                std::ofstream f(this_node_info_path);
                f << this_node_info_json.dump(4);
                m_this_node = Node("localhost", "8080", "WebD_Node");
                return;
            }

            std::ifstream f(this_node_info_path);
            nlohmann::json try_parse = nlohmann::json::parse(f, nullptr, false);
            f.close();
            if(try_parse.is_discarded())
            {
                std::ofstream f1(this_node_info_path);
                f1 << this_node_info_json.dump(4);
                m_this_node = Node("localhost", "8080", "WebD_Node");
                return;
            }

            if(try_parse.contains("name"))
            {
                if(try_parse["name"].is_string())
                    m_this_node = Node("localhost", "8080", try_parse["name"]);
            }
        }

        nlohmann::json this_node() const
        {
            return {{"name", m_this_node.name()}, {"ip","localhost"}, {"port", m_this_node.port()}};
        }
};



bool get_missing_files_from_other_nodes(nlohmann::json info_remote, nlohmann::json info_local, Database & database)
{
    std::cout << "comparing files.." << std::endl;

    if(info_remote == info_local )
    {
        std::cout << "information between nodes are equal... " << std::endl;
        return false;
    }
         
    std::cout << "information between nodes are different... " << std::endl;

    nlohmann::json patch = nlohmann::json::diff(info_local, info_remote);
    nlohmann::json patched_source = info_local.patch(patch);

    //info_local = patched_source;
    //database.update_info(patched_source);
    return true;
    //request_all_missing_files(patched_source);
    //std::cout << "new info: " << patched_source.dump(4) << std::endl;
}

void move_folder(char const *sorc, char const *dst, bool createRoot = true) {
    namespace fs = std::filesystem;

    if(!fs::is_directory(sorc))
        return;

    if (createRoot)
        fs::create_directories(dst);

    for(fs::path p: fs::directory_iterator(sorc)){
        fs::path destFile = dst/p.filename();

        if (fs::is_directory(p)) {
            fs::create_directories(destFile);
            fs::rename(p.string().c_str(), destFile.string().c_str());
        } else {
            //std::cout << "updated " << p.string().c_str() << std::endl;
            fs::rename(p, destFile);
        }
    }
}

void move_all_older_files_to_another_folder(const std::string & domain, std::uint64_t version, Database & database)
{
    const auto & site_find = database.find_site(domain);
    if(!site_find)
        return;

    const auto & site_path = site_find->path();
    auto old_sites_paths = std::filesystem::current_path();
    old_sites_paths /= "www_old";
    old_sites_paths /= (domain + "_" + std::to_string(version));
    move_folder(site_path.string().c_str(), old_sites_paths.string().c_str(), true);
}



void synchronize_with_nodes(RemoteNodes & remote_nodes, Database & database)
{
    const auto www_database = database.www_database_index();
    const auto & nodes = remote_nodes.nodes();
    std::cout << "\n[SYNCHRONIZER]: Initializing node synchronization with remote nodes, found " << nodes.size() << " nodes..." << std::endl;

    for(const auto & node : nodes)
    {
        
        const auto ip_with_port = node.ip_with_port();

        { //get resume
            HttpClient http_client(ip_with_port);

            //get resume from node
            std::cout << "[SYNCHRONIZER]: Connecting into node {" << node.name() << "} @ {" << ip_with_port << "}" << std::endl;
            try
            {
                
                auto request = http_client.request("GET", "/api/v1/resume");
                const auto & content = request->content.string();
                nlohmann::json input_json = nlohmann::json::parse(content, nullptr, false);
                    if(input_json.is_discarded())
                        return;

                for(const auto & site : input_json)
                {
                    std::vector<std::string> files_to_download;
                    const std::string & domain = site["domain"].get<std::string>();
                    const std::string & description = site["description"].get<std::string>();
                    const std::uint64_t & version = site["version"].get<std::uint64_t>();
                    for(const auto & file : site["files"])
                        files_to_download.emplace_back(file.get<std::string>());

                    const auto is_remote_file_newer = database.is_remote_file_newer(domain, version);

                    if(is_remote_file_newer)
                    {
                        std::uint64_t site_version = 0;
                        const auto & site_find = database.find_site(domain);
                        if(site_find)
                            site_version = site_find->version();
                        std::cout << "\n[SYNCHRONIZER]: Node {" << node.name() << "} @ {" << ip_with_port << "} has site " << domain << " in version " << version << ", we have version " << site_find->version() << std::endl;
                        move_all_older_files_to_another_folder(domain, version, database);
                        //download files
                        bool downloaded_all_files = true;
                        {
                            for(const auto & file : files_to_download)
                            {

                                {
                                    HttpClient http_client(ip_with_port);
                                    auto file_url = "/api/v1/site/" + file;
                                    std::cout << "[SYNCHRONIZER]: Downloading {" << file_url << "} from node {" << node.name() << "} @ {" << ip_with_port << "}" << std::endl;
                                    try 
                                    {
                                        auto request = http_client.request("GET", file_url);
                                        //std::cout << request->content.rdbuf() << std::endl; // Alternatively, use the convenience function r1->content.string()

                                        const auto & file_content = request->content.string();

                                        auto file_path = get_www_path();
                                        file_path /= file;
                                        if(!std::filesystem::is_directory(file_path.parent_path()))
                                            std::filesystem::create_directories(file_path.parent_path());
                                        std::ofstream file_output(file_path);
                                        file_output << file_content;
                                        file_output.close();
                                    }
                                    catch(const SimpleWeb::system_error &ec) 
                                    {
                                        downloaded_all_files = false;
                                        std::cout << "[SYNCHRONIZER]: Error downloading file {" << file << "} from node {" << node.name() << "} @ {" << ip_with_port << "}: " << ec.what() << std::endl;
                                    }
                                }

                            }
                        }
                    
                        if(downloaded_all_files)
                        {
                            std::cout << "\n[SYNCHRONIZER]: Success, downloaded all files for site " << domain << " from node {" << node.name() << "} @ {" << ip_with_port << "}" << std::endl;
                            database.update_site(domain, version, description, files_to_download);
                            database.update_all_sites_json();
                            //std::exit(1);
                        }
                        
                    }
                    

                }  
            }
            catch(const std::exception& e)
            {
                std::cout << "[SYNCHRONIZER]: Error connecting into node {" << node.name() << "} @ {" << ip_with_port << "}: " << e.what() << std::endl;
                continue;
            }
            
        }

    }
    std::cout << "[SYNCHRONIZER] Finished synchronizing with remote nodes " << std::endl;
}

void periodically_synchronize_with_nodes(RemoteNodes & remote_nodes, Database & database)
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        std::cout << "[PERIODICALLY SYNCHRONIZER]: Synchronizing with nodes..." << std::endl;
        synchronize_with_nodes(remote_nodes, database);
    }
}

void discover_nodes(RemoteNodes & remote_nodes)
{
    const auto nodes = remote_nodes.nodes();
    for(const auto & node : nodes)
    {
        const auto ip_with_port = node.ip_with_port();

        { //get resume
            HttpClient http_client(ip_with_port);

            //get known nodes from node
            std::cout << "[DISCOVER NODES]: Connecting into node {" << node.name() << "} @ {" << ip_with_port << "}" << std::endl;
            try
            {
                
                auto request = http_client.request("POST", "/api/v1/known-nodes", remote_nodes.this_node().dump().c_str());
                const auto & content = request->content.string();
                //std::cout << "RECEIVING: " << content << std::endl;
                nlohmann::json input_json = nlohmann::json::parse(content, nullptr, false);
                    if(input_json.is_discarded())
                        return;
                
                for(const auto & node_remote : input_json)
                {
                    const std::string & name = node_remote["name"].get<std::string>();
                    const std::string & ip = node_remote["ip"].get<std::string>();
                    const std::string & port = node_remote["port"].get<std::string>();
                    
                    if(!std::any_of(nodes.begin(), nodes.end(), [&](const Node & node){return node.name() == name ;}))
                    {
                        std::cout << "[DISCOVER NODES]: Inserting new discovered remote node {" << node.name() << "} @ {" << ip_with_port << "} into known nodes database..." << std::endl;
                        remote_nodes.insert({ip, port, name});
                        remote_nodes.update_on_disk();
                    }
                }  
            }
            catch(const std::exception& e)
            {
                std::cout << "[DISCOVER NODES]: Error connecting into node {" << node.name() << "} @ {" << ip_with_port << "}: " << e.what() << std::endl;
                continue;
            }
            
        }
    }
}

void discover_nodes_periodically(RemoteNodes & remote_nodes)
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "[PERIODICALLY DISCOVER NODES]: Discovering nodes..." << std::endl;
        discover_nodes(remote_nodes);
    }
}

void reload_sites_periodically(Database & database)
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "[PERIODICALLY RELOAD SITES]: Reloading sites..." << std::endl;
        database.load_database_from_file();
    }
}

void check_folder_and_file_structures()
{
    auto local_path = std::filesystem::current_path();
    auto www_folder_path = get_www_path();
    auto www_database_path = www_database_index_path();
    auto remote_nodes_database = remote_nodes_information_file_path(); 

    if(!std::filesystem::is_directory(www_folder_path))
        std::filesystem::create_directories(www_folder_path);
    
    if(!std::filesystem::is_regular_file(www_database_path))
    {
        std::ofstream file{www_database_path};
        file << R"([{"description":"NOT FOUND PAGE","domain":"notfound","files":["notfound/index.html"],"version":1}])";
    }

    if(!std::filesystem::is_regular_file(remote_nodes_database))
    {
        std::ofstream file{remote_nodes_database};
        file << nlohmann::json::array().dump(4);
    }

    auto not_found_www_folder = www_folder_path;
    not_found_www_folder /= "notfound";

    if(!std::filesystem::is_directory(not_found_www_folder))
        std::filesystem::create_directories(not_found_www_folder);

    auto not_found_index_html = not_found_www_folder;
    not_found_index_html /= "index.html";

    if(!std::filesystem::is_regular_file(not_found_index_html))
    {
        std::ofstream file{not_found_index_html};
        file << R"(<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta http-equiv="X-UA-Compatible" content="IE=edge"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>NOT FOUND</title></head><body>404 not found</body></html>)";
    }
}

void update_this_node_name(const std::string & name)
{
    nlohmann::json this_node_info_json{{"name",name}};
    auto this_node_info_path = get_www_path();
    this_node_info_path /= "this_node_config.json";
    std::ofstream f(this_node_info_path);
    f << this_node_info_json.dump(4);
}

int main(int argc, char **argv)
{
    check_folder_and_file_structures();

    InputParser input(argc, argv);
    const std::string &node_name = input.getCmdOption("-n");

    if(!node_name.empty())
        update_this_node_name(node_name);

    
    RemoteNodes remote_nodes;
    remote_nodes.load_nodes();
    Database database;
    database.load_database_from_file();
    //database.load_database_from_disk_structure();

    //synchronize_with_nodes(remote_nodes, database);
    std::cout << "[SYNCHRONIZED]: FINISHED" << std::endl;

    //start periodically sync
    std::thread(periodically_synchronize_with_nodes, std::ref(remote_nodes), std::ref(database)).detach();
    std::thread(reload_sites_periodically, std::ref(database)).detach();
    

    
    const std::string &port = input.getCmdOption("-p");
    
    HttpServer server;
    server.config.port = 8080;
    if (!port.empty()){
        server.config.port = std::stoi(port);
    }
    remote_nodes.load_this_node_info();
    remote_nodes.update_this_node_port_info(std::to_string(server.config.port));

    std::thread(discover_nodes_periodically, std::ref(remote_nodes)).detach();

    server.config.thread_pool_size = 8;// std::thread::hardware_concurrency();

    //std::thread(sync_thread, std::ref(database)).detach();

    server.resource["^/api/v1/ping$"]["GET"] = [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        std::cout << "GET ^/api/v1/ping$" << std::endl;
        
        return response->write(SimpleWeb::StatusCode::success_ok, "{\"pong\":true}", {{"Content-Type","application/json"}});
    };

    server.resource["^/api/v1/site/([A-Za-z0-9./_]+)$"]["GET"] = [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        const std::string & path_match_1st = request->path_match[1].str();
        std::cout << "GET ^/api/v1/site/" << path_match_1st << std::endl;

        return response->write(SimpleWeb::StatusCode::success_ok, serve_file(path_match_1st), {{"Content-Type",define_mime_type(path_match_1st).data()}});
    };

    server.resource["^/api/v1/resume$"]["GET"] = [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        std::cout << "GET ^/api/v1/resume" << std::endl;

        return response->write(SimpleWeb::StatusCode::success_ok, database.resume(), {{"Content-Type","application/json"}});
    };

    server.resource["^/api/v1/known-nodes$"]["GET"] = [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        std::cout << "GET ^/api/v1/known-nodes" << std::endl;

        return response->write(SimpleWeb::StatusCode::success_ok, remote_nodes.known_nodes().dump(4), {{"Content-Type","application/json"}});
    };

    server.resource["^/api/v1/known-nodes$"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        std::cout << "POST ^/api/v1/known-nodes" << std::endl;
        
        nlohmann::json input = nlohmann::json::parse(request->content.string(), nullptr, false);
        if(!input.is_discarded())
        {
            //std::cout << "POST RECEBIDO: " << input.dump(4) << std::endl;
            //return response->write(SimpleWeb::StatusCode::success_ok, remote_nodes.known_nodes().dump(4), {{"Content-Type","application/json"}});

            if(input.contains("name") /* && input.contains("ip") */ && input.contains("port"))
            {
                if(input["name"].is_string() /* && input["ip"].is_string() */ && input["port"].is_string())
                {
                               // std::cout << "POST RECEBIDO: " << input.dump(4) << std::endl;

                    //request->remote_endpoint().address().to_v4().to_string()
                    //std::cout << "[REMOTE_NODE]: Node {" << input["name"] << "} @ {" << request->remote_endpoint().address().to_v4().to_string()<< ":" << request->remote_endpoint().port() << "} asking to be added to remote nodes database..." << std::endl;
                    if(request->remote_endpoint().address().to_string().find("127.0.0.1") != std::string::npos)
                        remote_nodes.insert("localhost", input["port"], input["name"]);
                    else
                        remote_nodes.insert(request->remote_endpoint().address().to_string(), input["port"], input["name"]);

                    remote_nodes.update_on_disk();
                }
            }
        }
        std::cout << "RETURNING: " << remote_nodes.known_nodes().dump(4) << std::endl;
        return response->write(SimpleWeb::StatusCode::success_ok, remote_nodes.known_nodes().dump(4), {{"Content-Type","application/json"}});
    };

    std::cout << "Rodando servidor na porta " << server.config.port << std::endl;
    std::thread http_server_thread([&]{server.start();});
    http_server_thread.join();
  
    return 0;
}
