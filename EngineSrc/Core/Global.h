#ifndef GLOBAL_HDR
#define GLOBAL_HDR

class Global
{
public:
    static Global& getInstance()
    {
        static Global instance;
        return instance;
    }

    //TODO: Add the global access to GLFWwindow here singleton style.

    bool running;
private:
    Global() : running(true)
    {
    }

public:
    Global(Global const& rhs) = delete;
    Global(Global && rhs) = delete;
    void operator=(Global const& rhs) = delete;
    Global operator=(Global &&rhs) = delete;
};

#endif // !GLOBAL_HDR
