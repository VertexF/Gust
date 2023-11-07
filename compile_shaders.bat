echo Note: Original shaders are commented out
::D:\Windows\VulkanSDK\1.3.236.0\Bin\glslc.exe EngineSrc\Assets\Shaders\simple_shader.vert -o EngineSrc\Assets\Shaders\simple_shader.vert.spv
::D:\Windows\VulkanSDK\1.3.236.0\Bin\glslc.exe EngineSrc\Assets\Shaders\simple_shader.frag -o EngineSrc\Assets\Shaders\simple_shader.frag.spv

D:\Windows\VulkanSDK\1.3.236.0\Bin\glslc.exe EngineSrc\Assets\Shaders\triangle.vert -o EngineSrc\Assets\Shaders\triangle.vert.spv
D:\Windows\VulkanSDK\1.3.236.0\Bin\glslc.exe EngineSrc\Assets\Shaders\triangle.frag -o EngineSrc\Assets\Shaders\triangle.frag.spv
D:\Windows\VulkanSDK\1.3.236.0\Bin\glslc.exe EngineSrc\Assets\Shaders\redTriangle.vert -o EngineSrc\Assets\Shaders\redTriangle.vert.spv
D:\Windows\VulkanSDK\1.3.236.0\Bin\glslc.exe EngineSrc\Assets\Shaders\redTriangle.frag -o EngineSrc\Assets\Shaders\redTriangle.frag.spv

if EXIST C:\Users\Dan\Documents\Development\git\GameEngine\VS\Assets\Shaders\ (
  copy EngineSrc\Assets\Shaders\triangle.vert.spv C:\Users\Dan\Documents\Development\git\GameEngine\VS\Assets\Shaders\
  copy EngineSrc\Assets\Shaders\triangle.frag.spv C:\Users\Dan\Documents\Development\git\GameEngine\VS\Assets\Shaders\
  copy EngineSrc\Assets\Shaders\redTriangle.vert.spv C:\Users\Dan\Documents\Development\git\GameEngine\VS\Assets\Shaders\
  copy EngineSrc\Assets\Shaders\redTriangle.frag.spv C:\Users\Dan\Documents\Development\git\GameEngine\VS\Assets\Shaders\
) else (
  echo Visual Studio Project has not been created!
)