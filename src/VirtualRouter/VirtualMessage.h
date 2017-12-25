//
//  VirtualMessage.h
//  myRouter
//
//  Created by applecz on 2017/12/20.
//  Copyright © 2017年 applecz. All rights reserved.
//

#ifndef VIRTUAL_MESSAGE_H
#define VIRTUAL_MESSAGE_H

const int STR_MSG_LEN = 164;
class VirtualMessage {
private:
    char code[4];
    char src_host[16];
    char dst_host[16];
    char msg[128];
public:
    VirtualMessage() {
        // Padding.
        memset(code, 0, sizeof(code));
        memset(src_host, 0, sizeof(src_host));
        memset(dst_host, 0, sizeof(dst_host));
        memset(msg, 0, sizeof(msg));
    }
    // Get method.
    void getCode(char code[]) {
        strncpy(code, this->code, 4);
    }
    void getSrc(char src[]) {
        strncpy(src, src_host, 32);
    }
    void getDst(char dst[]) {
        strncpy(dst, dst_host, 32);
    }
    void getMsg(char msg[]) {
        strncpy(msg, this->msg, 128);
    }
    char* getStr() {
        char* str = NULL;
        encode(*this, str);
        return str;
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
    
    // Encode.
    static void encode(VirtualMessage msg_package, char* &str_msg) {
        str_msg = new char[STR_MSG_LEN];
        memset(str_msg, 0, sizeof(char)*STR_MSG_LEN);
        strncpy(str_msg, msg_package.code, 4);
        strncpy(str_msg+4, msg_package.src_host, 16);
        strncpy(str_msg+20, msg_package.dst_host, 16);
        strncpy(str_msg+36, msg_package.msg, 128);
        for (int i = 0; i < STR_MSG_LEN; i++)
            if (str_msg[i] == '\0') str_msg[i] = '#';
        str_msg[STR_MSG_LEN-1] = '\0';
        
    }
    // Decode
    static void decode(VirtualMessage &msg_package, const char str_msg[]) {
        // str_msg must be a NULL pointer.
        char str_msg_copy[STR_MSG_LEN];
        strncpy(str_msg_copy, str_msg, STR_MSG_LEN);
        for (int i = 0; i < STR_MSG_LEN; i++)
            if (str_msg_copy[i] == '#') str_msg_copy[i] = '\0';
        str_msg_copy[STR_MSG_LEN-1] = '\0';
        strncpy(msg_package.code, str_msg_copy, 4);
        strncpy(msg_package.src_host, str_msg_copy+4, 16);
        strncpy(msg_package.dst_host, str_msg_copy+20, 16);
        strncpy(msg_package.msg, str_msg_copy+36, 128);
    }
    
    // Print
    void print() {
        printf("\n\n*********** Message Package **********\n");
        printf("  code        : %s\n", code);
        printf("  source      : %s\n", src_host);
        printf("  destination : %s\n", dst_host);
        printf("  message     : %s\n", msg);
        printf("\n************************************\n\n");
    }
};

#endif /* VIRTUAL_MESSAGE_H */
