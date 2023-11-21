#include "Pipeline.h"

#include "Core/Logger.h"

#include <fstream>

namespace Gust
{

Pipeline::Pipeline(const char* vertexFilePath, const char* fragFilePath)
{
    createGraphicsPipeline(vertexFilePath, fragFilePath);
}

std::vector<char> Pipeline::readFile(const char* filepath)
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (file.is_open() == false)
    {
        GUST_CRITICAL("Shader file failed to open {0}", filepath);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> fileBuffer(fileSize);

    file.seekg(0);
    file.read(fileBuffer.data(), fileSize);

    file.close();

    return fileBuffer;
}

void Pipeline::createGraphicsPipeline(const char* vertexFilePath, const char* fragFilePath) 
{
    std::vector<char> vertexFileContents = readFile(vertexFilePath);
    std::vector<char> fragFileContents = readFile(fragFilePath);

    GUST_INFO("Vertex file size {0} ", vertexFileContents.size());
    GUST_INFO("Fragment file size {0} ", fragFileContents.size());
}

} //GUST
