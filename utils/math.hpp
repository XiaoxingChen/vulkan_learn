// Copyright(c) 2019, NVIDIA CORPORATION. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#  pragma once
#if defined( _MSC_VER )
#  pragma warning( disable : 4201 )  // disable warning C4201: nonstandard extension used: nameless struct/union; needed
                                     // to get glm/detail/type_vec?.hpp without warnings
#elif defined( __GNUC__ )
// don't know how to switch off that warning here
#else
// unknow compiler... just ignore the warnings for yourselves ;)
#endif

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS

#if defined( _MSC_VER )
#  pragma warning( push )
#  pragma warning( disable : 4127 )  // conditional expression is constant (glm)
#endif

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
 #define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#if defined( _MSC_VER )
#  pragma warning( pop )
#endif

namespace vk
{
  namespace su
  {
    glm::mat4x4 createModelViewProjectionClipMatrix(
      vk::Extent2D const & extent ,
      float angle=0.f,
      float scale=1.f,
      const glm::mat4& view=glm::lookAt( glm::vec3( -5.0f, 3.0f, -10.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) ));
    bool isSE3(const glm::mat4& mat, float* error=nullptr, float tol=1e-3);
  }
}  // namespace vk
