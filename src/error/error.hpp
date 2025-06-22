#pragma once

class Error {
  public:
    const char *message = NULL;
    Error(const char *message){
      this->message = message;
    }
};