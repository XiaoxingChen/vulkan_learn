#include <string>
#include <vector>
#include "utils/resource_manager.h"
#include <map>
#include <fstream>
#include <iostream>
#include <array>

namespace euroc
{
std::string datasetFolder()
{
    return assetsFolder() + "../../../../datasets/";
}

std::map<uint64_t, std::string> parseCamDataCsv(const std::string& dataCsvPath)
{
    std::map<uint64_t, std::string> ret;
    std::fstream fs(dataCsvPath);
    if(!fs.is_open())
    {
        std::cout << "cannot open file: " << dataCsvPath << std::endl;
        return ret;
    }
    // fs.getline(fileName, 1);//abandon first line
    uint64_t timestamp;
    std::string imgBasename;
    char comma;
    std::string dummy;
    fs >> dummy >> dummy;
    // fs >> dummy;

    while(1)
    {
        fs >> timestamp >> comma >> imgBasename;
        if (fs.fail() || fs.eof())
            break;
        ret[timestamp] = imgBasename;
        // std::cout << timestamp << std::endl;
    }
    return ret;
}

std::string getCamFolder(const std::string& eurocDatasetName, size_t cameraIndex)
{
    return std::string(datasetFolder() + eurocDatasetName + "/mav0/cam" + std::to_string(cameraIndex) + "/");
}

std::vector<std::string> imagePaths(const std::string& eurocDatasetName, size_t cameraIndex)
{
    std::string camFolder = getCamFolder(eurocDatasetName, cameraIndex);
    std::string imgFolder(camFolder + "data/");
    std::string dataCsvPath(camFolder + "data.csv");
    auto stampBasenameMap = parseCamDataCsv(dataCsvPath);

    std::vector<std::string> ret;
    for(const auto& stampBasenamePair: stampBasenameMap)
    {
        ret.push_back(imgFolder + stampBasenamePair.second);
    }
    return ret;
}

std::array<size_t, 2> resolution(const std::string& eurocDatasetName, size_t cameraIndex)
{
    std::string camFolder = getCamFolder(eurocDatasetName, cameraIndex);
    return {752, 480};
}

}





