#include <iostream>

namespace euroc
{
  std::vector<std::string> imagePaths(const std::string& eurocDatasetName="V1_01_easy", size_t cameraIndex=0);
  std::array<size_t, 2> resolution(const std::string& eurocDatasetName="V1_01_easy", size_t cameraIndex=0);
}
