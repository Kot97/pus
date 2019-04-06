#ifndef GENERATE_INDEX_HPP
#define GENERATE_INDEX_HPP

#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <string>

namespace fs = boost::filesystem;

void generate_index(const fs::path& dir)
{
    // ścieżka nie istnieje
    if(!fs::exists(dir))
    {
        std::cerr << "No such file or directory " << dir << std::endl;
        return;
    }

    // ścieżka nie określa katalogu
    if(!fs::is_directory(dir))
    {
        std::cerr << dir << " is not a directory" << std::endl;
        return;
    }

    std::ofstream out("index.html");
    if(out.bad())
    {
        std::cerr << "Problem with index.html" << std::endl;
        return;
    }

    std::string begin = "<html><body><center>\n";
    std::string end = "</center></body></html>";

    std::string line_begin = "<img src=\"";
    std::string line_end = "\"></img><br />\n";

    out << begin;
    fs::path temp;
    std::string parent_folder;
    std::string filename;

    for(auto&& i : fs::directory_iterator(dir))
    {
        temp = i.path();
        parent_folder = temp.parent_path().filename().string(); 
        filename = temp.filename().string(); 
        out << line_begin + parent_folder + "/" + filename + line_end;
    }

    out << end;
}

#endif 