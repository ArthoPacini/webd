#include <iostream>
#include <filesystem>
#include <set>
#include <fstream>
#include <string>
#include <vector>
#include <string_view>

#include <array>
#include <string_view>
#include <stdexcept>
#include <algorithm>
#include <map>


#include "web_server/server_http.hpp"
#include "web_server/client_http.hpp"
#include "websocket/server_ws.hpp"
#include "json.hpp"

//using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;


//Return the 'www' folder path
std::filesystem::path get_www_path()
{
    const auto current_path = std::filesystem::current_path(); // current path
    auto www_folder = current_path;
    www_folder /= "www"; //go www folder
    return www_folder;
}

//Return the www_database_index file path
std::filesystem::path www_database_index_path()
{
    const auto current_path = std::filesystem::current_path(); // current path
    auto file_path = current_path;
    file_path /= "www"; //go to cache
    file_path /= "www_database_index.json";
    return file_path;
}

//Load the www_database_index into nlohmann::json object and return it
nlohmann::json load_www_database_index()
{
    const auto file_path = www_database_index_path();
    if(!std::filesystem::is_regular_file(file_path))
        return {};

    nlohmann::json j;
    std::ifstream file(file_path);
    j = nlohmann::json::parse(file, nullptr, false);
    if(j.is_discarded())
        return {};
    return j;
}

//Return the remote_nodes_information file path
std::filesystem::path remote_nodes_information_file_path()
{
    const auto current_path = std::filesystem::current_path(); // current path
    auto file_path = current_path;
    file_path /= "remote_nodes_information.json";
    return file_path;
}

//Load the www_database_index into nlohmann::json object and return it
nlohmann::json load_remote_nodes_information()
{
    const auto file_path = remote_nodes_information_file_path();
    if(!std::filesystem::is_regular_file(file_path))
        return {};

    nlohmann::json j;
    std::ifstream file(file_path);
    file >> j;
    return j;
}

//Split a string into a vector of strings using a character as delimitator, return vector of strings
std::vector<std::string> split_str(const std::string& str, const char delim) 
{
    std::vector<std::string> strings;
    std::size_t start;
    std::size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos) 
    {
        end = str.find(delim, start);
        strings.push_back(str.substr(start, end - start));
    }

    return strings;
}

//Try serving a file from disk
std::string serve_file(const std::string & request_file_path)
{//serve not_found string otherwise
    //const static std::string not_found = "<!DOCTYPE html>\r\n<html lang=\"en\">\r\n<head>\r\n    <meta charset=\"UTF-8\">\r\n    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\r\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n    <title>NOT FOUND<\\/title>\r\n<\\/head>\r\n<body>\r\n    404 not found\r\n<\\/body>\r\n<\\/html>";

    bool serve_index = true;
    for(const auto & c : request_file_path)
        if(c == '/')
        {
            serve_index = false;
            break;
        }


    auto request_path = get_www_path();
    request_path /= request_file_path;
    if(serve_index)
        request_path /= "index.html";
    //std::cout << "REQUEST PATH: " << request_path << std::endl;

    if(!std::filesystem::is_regular_file(request_path))
        return serve_file("notfound");

    std::ifstream input_file(request_path.string());
    std::stringstream buffer;
    buffer << input_file.rdbuf();
    return buffer.str();
}

//Check if file exists in disk
bool does_file_exists(const std::string & request_file_path)
{
    bool serve_index = true;
    for(const auto & c : request_file_path)
        if(c == '/')
        {
            serve_index = false;
            break;
        }


    auto request_path = get_www_path();
    request_path /= request_file_path;
    if(serve_index)
        request_path /= "index.html";
    //std::cout << "REQUEST PATH: " << request_path << std::endl;

    if(!std::filesystem::is_regular_file(request_path))
        return false;

    return true;
}


// ---- MIME LOOKUP TABLE ----
//Constexpr map from jason turner
template <typename Key, typename Value, std::size_t Size>
struct Map {
  std::array<std::pair<Key, Value>, Size> data;

  [[nodiscard]] constexpr Value at(const Key &key) const {
    const auto itr =
        std::find_if(begin(data), end(data),
                     [&key](const auto &v) { return v.first == key; });
    if (itr != end(data)) {
      return itr->second;
    } else {
      throw std::range_error("Not Found");
    }
  }

};

//Constexpr MIME types
using namespace std::literals::string_view_literals;
static constexpr std::array<std::pair<std::string_view, std::string_view>, 40> color_values{
    {{"txt"sv , "text/plain"sv},
{"htm"sv , "text/html"sv},
{"html"sv , "text/html"sv},
{"php"sv , "text/html"sv},
{"css"sv , "text/css"sv},
{"js"sv , "application/javascript"sv},
{"json"sv , "application/json"sv},
{"xml"sv , "application/xml"sv},
{"swf"sv , "application/x-shockwave-flash"sv},
{"flv"sv , "video/x-flv"sv},
{"png"sv , "image/png"sv},
{"jpe"sv , "image/jpeg"sv},
{"jpeg"sv , "image/jpeg"sv},
{"jpg"sv , "image/jpeg"sv},
{"gif"sv , "image/gif"sv},
{"bmp"sv , "image/bmp"sv},
{"ico"sv , "image/vnd.microsoft.icon"sv},
{"tiff"sv , "image/tiff"sv},
{"tif"sv , "image/tiff"sv},
{"svg"sv , "image/svg+xml"sv},
{"svgz"sv , "image/svg+xml"sv},
{"zip"sv , "application/zip"sv},
{"rar"sv , "application/x-rar-compressed"sv},
{"exe"sv , "application/x-msdownload"sv},
{"msi"sv , "application/x-msdownload"sv},
{"cab"sv , "application/vnd.ms-cab-compressed"sv},
{"mp3"sv , "audio/mpeg"sv},
{"qt"sv , "video/quicktime"sv},
{"mov"sv , "video/quicktime"sv},
{"pdf"sv , "application/pdf"sv},
{"psd"sv , "image/vnd.adobe.photoshop"sv},
{"ai"sv , "application/postscript"sv},
{"eps"sv , "application/postscript"sv},
{"ps"sv , "application/postscript"sv},
{"doc"sv , "application/msword"sv},
{"rtf"sv , "application/rtf"sv},
{"xls"sv , "application/vnd.ms-excel"sv},
{"ppt"sv , "application/vnd.ms-powerpoint"sv},
{"odt"sv , "application/vnd.oasis.opendocument.text"sv},
{"ods"sv , "application/vnd.oasis.opendocument.spreadsheet"sv}}};

//Define mime type of file by extension, return MIME TYPE as string_view
std::string_view define_mime_type(const std::string & str)
{
    static constexpr auto map =
      Map<std::string_view, std::string_view, color_values.size()>{{color_values}};

    if(str.rfind('/') == std::string::npos)
        return "text/html";

    if(str.rfind('.') == std::string::npos)
        return "text/html";

    auto dot = str.rfind('.');
    std::string extension = str.substr(dot+1, str.size()-dot-1);
    //std::cout << "extension: " << extension << std::endl;
    std::string_view sv{extension.data(), extension.size()};
    return map.at(sv);
   /*  const auto & find_mime = mimes.find(extension);
    if(find_mime != mimes.end())
        return find_mime->second;

    return "text/html"; */
}

//Parse command line arguments
class InputParser{
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }
        /// @author iain
        const std::string& getCmdOption(const std::string &option) const{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }
        /// @author iain
        bool cmdOptionExists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        std::vector <std::string> tokens;
};