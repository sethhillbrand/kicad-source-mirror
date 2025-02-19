#ifndef COPILOT_H
#define COPILOT_H

#ifdef _WIN32
#ifdef COPILOT_SHARED_LIBRARY
#define COPILOT_API __declspec( dllexport )
#else
#define COPILOT_API __declspec( dllimport )
#endif
#else
#define COPILOT_API
#endif

class COPILOT_API Copilot
{
public:
    static int launch_copilot();
};

#endif // COPILOT_H