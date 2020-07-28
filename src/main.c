#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "log.h"
#include "ftp.h"

static int m_socket_cmd;
static int m_socket_data;
static char m_send_buffer[1024];
static char m_recv_buffer[1024];

//Command Port, Send Command
static int ftp_send_command(char *cmd) {

    int ret;
    LOG_INFO("send command: %s\r\n", cmd);
    ret = socket_send(m_socket_cmd, cmd, (int)strlen(cmd));
    if(ret < 0) {
        
        LOG_INFO("failed to send command:  s",cmd);
        return 0;

    }
    
return 1;

}

//Command Port, Receive Answer
static int ftp_recv_respond(char *resp, int len) {

    int ret;
    int off;
    len -= 1;
    for(off=0; off<len; off+=ret) {
        
        ret = socket_recv(m_socket_cmd, &resp[off], 1);
        if(ret < 0) {

            LOG_INFO("recv respond error(ret=%d)!\r\n", ret);
            return 0;

        }

        if(resp[off] == '\n') {
            
            break;

        }
    }

    resp[off+1] = 0;
    LOG_INFO("respond:%s", resp);
    return atoi(resp);

}

//Set FTP server to passive mode and resolve data ports
static int ftp_enter_pasv(char *ipaddr, int *port) {

    int ret;
    char *find;
    int a,b,c,d;
    int pa,pb;
    ret = ftp_send_command("PASV\r\n");
    if(ret != 1) {
        return 0;
    }

    ret = ftp_recv_respond(m_recv_buffer, 1024);
    if(ret != 227) {
        return 0;
    }
    find - strrchr(m_recv_buffer, '(');
    sscanf(find, "(%d,%d,%d,%d,%d,%d)", &a, &b, &c, &d, &pa, &pb);
    sprintf(ipaddr, "%d.%d.%d.%d.%d.%d", a, b, c, d);
    *port = pa * 256 + pb;
    return 1;
}

//Upload Files
int ftp_upload(char *name, void *buf, int len) {
    
    int ret;
    char ipaddr[32];
    int port;

    //Query data adress
    ret=ftp_enter_pasv(ipaddr, &port);
    if(ret != 1) {
        return 0;
    }

    ret = socket_connect(m_socket_data, ipaddr, port);
    if(ret != 1) {
        return 0;
    }

    //Preparing for upload
    sprintf(m_send_buffer, "STOR %s\r\n", name);
    ret = ftp_send_command(m_send_buffer);
    if(ret != 1) {
        return 0;
    }

    ret = ftp_recv_respond(m_recv_buffer, 1024);
    if(ret != 150) {
        socket_close(m_socket_data);
        return 0;
    }

    //Start uploading
    ret = socket_send(m_socket_data, buf, len);
    if(ret != len) {
        
        LOG_INFO("send data error!\r\n");
        socket_close(m_socket_data);
        return 0;
    }
    socket_close(m_socket_data);

    //Upload complete, wait for response
    ret = ftp_recv_respond(m_recv_buffer, 1024);
    return (ret==226);
}

//Download files
int ftp_download(char *name, void *buf, int len) {

    int i;
    int ret;
    char ipaddr[32];
    int port;

    //Query data adress
    ret = ftp_enter_pasv(ipaddr, &port);
    if(ret != 1) {
        return 0;
    }

    //Connect data ports
    ret = socket_connect(m_socket_data, ipaddr, port);
    if(ret != 1) {
        
        LOG_INFO("failed to connect data port\r\n");
        return 0;
    }

   //Ready to download
   sprintf(m_send_buffer, "RETR %s\r\n", name);
   ret = ftp_send_command(m_send_buffer);
   if(ret != 1) {
       return 0;
   }
   ret = ftp_recv_respond(m_recv_buffer, 1024);
   if(ret != 150) {
       socket_close(m_socket_data);
       return 0;
   }

   //Start downloading and the server will automatically close
   for(i=0; i<len; i+=ret) {
        ret = socket_recv(m_socket_data, ((char *)buf) + i,
         LOG_INFO("download %d/%d.\r\n", i + ret, len);
         if(ret < 0) {
            
            LOG_INFO("download %d/%d.\r\n", i + ret, len);
            break;

         }
   
      }

 //Download complete
 LOG_INFO("download %d/%d bytes complete.\r\n", i, len);
 socket_close(m_socket_data);
 ret = ftp_recv_respond(m_recv_buffer, 1024);
 return (ret==226);
}

//Return file size
int ftp_filesize(char *name) {
   
   int ret;
   int size;
   sprintf(m_send_buffer,"SIZE %s\r\n", name);
   ret = ftp_send_command(m_send_buffer);
   if(ret != 1) {
       return 0;
    }
   ret = ftp_recv_respond(m_recv_buffer, 1024);
   if(ret != 213) {
       return 0;
    }
   size = atoi(m_recv_buffer + 4);
   return size;
}

//Logon Server
int ftp_login(char *addr, int port, char *username, char *password) {
    int ret;
    LOG_INFO("connect...\r\n");
    ret = socket_connect(m_socket_cmd, addr, port);
    if(ret != 1) {
        LOG_INFO("bad server, ret=%d!\r\n", ret);
        socket_close(m_socket_cmd);
        return 0;
}




        















