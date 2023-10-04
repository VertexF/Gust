#ifndef TIME_STEP_HDR
#define TIME_STEP_HDR

namespace Gust
{

    class TimeStep
    {
    public:
        TimeStep(float time = 0.f) : _time(time)
        {
        }

        operator float() const { return _time; }

        float getSeconds() const { return _time; }
        float getMilliSecounds() const { return _time; }

    private:
        float _time;
    };

}

#endif // !TIME_STEP_HDR
