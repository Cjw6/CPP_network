//
// Created by cjw on 21-7-3.
//

#ifndef CWORK_NONCOPY_H
#define CWORK_NONCOPY_H

#define NonCopy(x)  \
public:  \
    x(const x&)=delete;  \
    x& operator=(const x&)=delete;

#endif // CWORK_NONCOPY_H
