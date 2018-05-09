### c++发送邮件

使用方式:

 `A (账号：gg@sina.com，密码：xxxx)`
 
发送给: 
`B (ww@sina.com)`
`C (yy@qq.com)`

内容为：

`hello world!`

```
std::vector<std::string> dest;
dest.push_back("ww@sina.com");
dest.push_back("yy@qq.com");

std::string content = "hello world!";

sendEmail("gg@sina.com", "xxxx", dest, content);
```
