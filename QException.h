/****************************************************************************
    QException 本系统中异常基类
****************************************************************************/
#ifndef _QEXCEPTION_H_
#define _QEXCEPTION_H_

#include "Util.h"
enum ExceptionRank{WARNING, ERROR, FITAL_ERROR};

class QException
{
 public:
    explicit QException(const std::string message);
    explicit QException(const std::string source, const std::string operation, const std::string description);
     
    int getErrorCode(){return error_code;}
    std::string getMessage(){return message;}   
  
 private:
	ExceptionRank rank;
 	string source, operation, message;
    int error_code;
};
 
#endif
