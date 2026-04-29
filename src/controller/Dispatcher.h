#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "../network/HttpRequest.h"
#include "../network/HttpResponse.h"
#include "ItemController.h"
#include "AuthController.h"

class Dispatcher {
private:
    ItemController* itemController;
    AuthController* authController;

public:
    Dispatcher(ItemController* ic, AuthController* ac);
    
    HttpResponse dispatch(HttpRequest request);
};

#endif // DISPATCHER_H
