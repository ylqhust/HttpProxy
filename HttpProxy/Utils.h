#pragma once
#include <string>
#include <mutex>

#ifndef DEBUG
#define DEBUG
#endif
namespace ylq
{
    template <const int Label=0>
	struct Utils
	{
        static int log(int label,const char *const t_format,...);
		static std::mutex s_logMutex;
	};
    
    
    template <const int Label>
    std::mutex Utils<Label>::s_logMutex;
    
    template <const int Label>
    int Utils<Label>::log(int label,const char *const t_format,...)
    {
        if(Label!=label)
            return 0;
        std::unique_lock<std::mutex> lock(s_logMutex);
        int result;
        va_list _ArgList;
        va_start(_ArgList, t_format);
        result = vprintf(t_format, _ArgList);
        va_end(_ArgList);
        return result;

    }
};
