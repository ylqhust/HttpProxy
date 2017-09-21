//
//  close_gurad.hpp
//  HttpProxy
//
//  Created by yangliqun on 2017/5/1.
//  Copyright © 2017年 yangliqun. All rights reserved.
//

#ifndef close_gurad_hpp
#define close_gurad_hpp

#include <unistd.h>

namespace ylq {
    template <class FD>
    class close_guard
    {
        FD m_fd;
    public:
        close_guard(const FD &t_fd):m_fd(t_fd){}
        
        ~close_guard()
        {
            close(m_fd);
        }
    };
}
#endif /* close_gurad_hpp */
