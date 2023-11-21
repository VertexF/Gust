#ifndef PIPELINE_HDR
#define PIPELINE_HDR

#include <vector>

namespace Gust
{

class Pipeline
{
public:
    Pipeline(const char* vertexFilePath, const char* fragFilePath);
private:
    static std::vector<char> readFile(const char* filepath);
    void createGraphicsPipeline(const char* vertexFilePath, const char* fragFilePath);
};

}//GUST

#endif // !PIPELINE_HDR