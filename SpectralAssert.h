#pragma once
#include "string"
#ifndef Assert
// Asserts if expr is false
#define Assert(Expr, Msg) \
    SpectralAssert::CallAssert(#Expr, Expr, __FILE__, __LINE__, Msg)


#define AssertAndReturn(Expr, Msg, returnType) \
    SpectralAssert::CallAssert(#Expr, Expr, __FILE__, __LINE__, Msg);\
    if (Expr == false) \
    returnType


class SpectralAssert
{
public:
    static void CallAssert(const char* expr_str, bool expr, const char* file, int line, const std::string& msg);
};

#endif

