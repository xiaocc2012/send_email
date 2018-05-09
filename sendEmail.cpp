#define SMTP_LOG(n) \
if (recv_len >= 0)\
{\
    LOG_DEBUG << "step " #n " recv data, ret:" << recv_len << ", value:" << std::string(buff, recv_len) << endl; \
}\
else\
{\
    LOG_DEBUG << "step " #n " recv data error, ret:" << recv_len << endl; \
    return -1; \
}

int sendEmail(const std::string& user_account, 
            const std::string& user_passwd, 
            const std::vector<std::string>& dest_account, 
            const std::string& content)
{
    std::string smtp_url;
    int smtp_port = 25;

    //获取smtp服务器地址
    std::string::size_type pos = user_account.find('@');
    if (pos != std::string::npos)
    {
        smtp_url = "smtp." + user_account.substr(pos + 1);
        LOG_DEBUG << "smtp_url:" << smtp_url << ", port:" << smtp_port << endl;
    }
    else
    {
        LOG_ERROR << "get smtp_url error, user_account:" << user_account << endl;
        return -1;
    }

    int sock = ::socket(AF_INET, SOCK_STREAM, 0);
    hostent* he = ::gethostbyname(smtp_url.c_str());
    if (he == NULL)
    {
        LOG_ERROR << "get host error, smtp_url:" << smtp_url << endl;
        return -1;
    }

    struct sockaddr_in addr;
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(smtp_port);
    
    if (::connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        LOG_ERROR << "connect error" << endl;
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
    {
        LOG_ERROR << "set recv timeout error" << endl;
        return 0;
    }

    if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
    {
        LOG_ERROR << "set send timeout error" << endl;
        return 0;
    }

    char buff[1024] = {0};
    size_t len = sizeof(buff);    
    
    //step 1: connect
    int recv_len = ::recv(sock, buff, len, 0);
    SMTP_LOG(1);

    //step 2: EHLO
    std::string send("EHLO ");
    send += smtp_url;
    send += "\r\n";

    ::send(sock, send.data(), send.size(), 0);
    recv_len = ::recv(sock, buff, len, 0);
    SMTP_LOG(2);

    //step 3: AUTH LOGIN
    send = "AUTH LOGIN\r\n";

    ::send(sock, send.data(), send.size(), 0);
    recv_len = ::recv(sock, buff, len, 0);
    SMTP_LOG(3);

    //step 4: account_name
    std::string account_name;
    pos = user_account.find("@");
    if (pos != std::string::npos)
    {
        account_name = user_account.substr(0, pos);
    }

    send = TC_Base64::encode(account_name);
    send += "\r\n";

    ::send(sock, send.data(), send.size(), 0);
    recv_len = ::recv(sock, buff, len, 0);
    SMTP_LOG(4);
        
    //step 5: account_pwd
    send = TC_Base64::encode(user_passwd);
    send += "\r\n";

    ::send(sock, send.data(), send.size(), 0);
    recv_len = ::recv(sock, buff, len, 0);
    SMTP_LOG(5);

    if (std::string(buff, recv_len).find("235") == std::string::npos)
    {
        LOG_ERROR << "account password error" << endl;
        return -1;
    }

    //step 6: MAIL FROM
    send = "MAIL FROM: ";
    send += "<" + user_account + ">";
    send += "\r\n";

    ::send(sock, send.data(), send.size(), 0);
    recv_len = ::recv(sock, buff, len, 0);
    SMTP_LOG(6);

    //step 7: RCPT TO
    for (size_t i = 0; i != dest_account.size(); ++i)
    {
        send = "RCPT TO: ";
        send += "<" + dest_account[i] + ">";
        send += "\r\n";

        ::send(sock, send.data(), send.size(), 0);
        recv_len = ::recv(sock, buff, len, 0);
        SMTP_LOG(7);
    }
    
    //step 8: DATA
    send = "DATA";
    send += "\r\n";

    ::send(sock, send.data(), send.size(), 0);
    recv_len = ::recv(sock, buff, len, 0);
    SMTP_LOG(8);

    //step 9: content
    send = "";
    send += "FROM: " + user_account;
    send += "\r\n";

    send += "TO: ";
    for (size_t i = 0; i != dest_account.size(); ++i)
    {
        if (i != 0)
        {
            send += ",";
        }
        send += dest_account[i];
    }
    send += "\r\n";

    send += "SUBJECT: 测试";
    send += "\r\n\r\n";

    send += content;
    send += "\r\n\r\n";

    send += ".";
    send += "\r\n";

    LOG_DEBUG << "email:" << send << endl;

    ::send(sock, send.data(), send.size(), 0);
    recv_len = ::recv(sock, buff, len, 0);
    SMTP_LOG(9);

    return 0;
}
