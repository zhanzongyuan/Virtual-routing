//
//  MyMessage.h
//  myRouter
//
//  Created by applecz on 2017/12/20.
//  Copyright © 2017年 applecz. All rights reserved.
//

#ifndef MyMessage_h
#define MyMessage_h

class MyMessage {
private:
    char code[4];
    char src_host[32];
    char dst_host[32];
    char msg[128];
public:
    MyMessage() {
        memset(code, 0, sizeof(code));
        memset(src_host, 0, sizeof(src_host));
        memset(dst_host, 0, sizeof(dst_host));
        memset(msg, 0, sizeof(msg));
    }
    // Get method.
    void getCode(char* &code) {
        strncpy(code, this->code, 4);
    }
    void getSrc(char* &src) {
        strncpy(src, src_host, 32);
    }
    void getDst(char* &dst) {
        strncpy(dst, dst_host, 32);
    }
    void getMsg(char* &msg) {
        strncpy(msg, this->msg, 128);
    }
    
    // Set method.
    void setCode(const char* code) {
        strncpy(this->code, code, 4);
    }
    void setSrc(const char* src) {
        strncpy(this->src_host, src, 32);
    }
    void setDst(const char* dst) {
        strncpy(this->dst_host, dst, 32);
    }
    void setMsg(const char* msg) {
        strncpy(this->msg, msg, 128);
    }
    
    // Decode and encode.
    static void encode(MyMessage msg_package, char* &str_msg) {
        // str_msg must be a NULL pointer.
        if (str_msg == NULL) {
            
        }
    }
}

#endif /* MyMessage_h */
