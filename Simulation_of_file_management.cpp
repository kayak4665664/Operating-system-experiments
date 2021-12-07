#include <conio.h>
#include <time.h>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

struct file_pointer {
    int x, y;
    inline file_pointer() {}
};

struct file_detail {
    bool write;       //写权限
    string filename;  //文件名
    file_pointer fp;  //文件指针
    inline file_detail() {}
};

struct file {
    unordered_map<string, file_detail> user_details;  //文件信息
    vector<string> content;                           //文件内容
    string c_time, m_time;  //创建时间 修改时间
    inline file() {}
};

unordered_map<int, file> files;  //所有文件

struct user {
    string password;                       //密码
    unordered_map<string, int> filenames;  //文件名
    inline user(string p) : password(p) {}
    inline user() {}
};

unordered_map<string, user> users;  //所有用户

string USER;  //当前用户

bool log_in = false;  //用户是否登录

inline auto init() {  //初始化，从disk中恢复信息
    ifstream ifs("disk");
    if (ifs) {
        int size;  //用户数量
        ifs >> size;
        for (int i = 0; i < size; ++i) {
            string username, password;  //用户名 密码
            ifs >> username >> password;
            users[username] = user(password);
        }
        ifs >> size;  //文件数量
        for (int i = 0; i < size; ++i) {
            int user_size;  //可访问该文件的用户的数量
            ifs >> user_size;
            files[i] = file();
            for (int j = 0; j < user_size; ++j) {
                string username, filename;  //用户名 文件名
                bool write;                 //写权限
                int x, y;                   //指针
                ifs >> username >> filename >> write >> x >> y;
                files[i].user_details[username].write = write;
                files[i].user_details[username].filename = filename;
                files[i].user_details[username].fp.x = x;
                files[i].user_details[username].fp.y = y;
                users[username].filenames[filename] = i;
            }
            ifs >> files[i].c_time >> files[i].m_time;  //创建时间 修改时间
            int file_size;                              //文件长度
            ifs >> file_size;
            ifs.get();
            for (int j = 0; j < file_size; ++j) {
                string str;
                getline(ifs, str);
                files[i].content.push_back(str);
            }
        }
    }
}

inline auto save() {  //将数据保存至disk
    ofstream ofs("disk", ios::out);
    ofs << users.size() << ' ';  //用户数量
    for (auto user : users) ofs << user.first << ' ' << user.second.password << ' ';  //用户名 密码
    ofs << files.size() << endl;                                  //文件数量
    for (auto file : files) {
        ofs << file.second.user_details.size() << ' ';  //可访问该文件的用户的数量
        for (auto user_detail : file.second.user_details) {
            ofs << user_detail.first << ' ' << user_detail.second.filename
                << ' ' << (user_detail.second.write ? 1 : 0) << ' '
                << user_detail.second.fp.x << ' ' << user_detail.second.fp.y
                << ' ';  //用户文件信息
        }
        ofs << file.second.c_time << ' ' << file.second.m_time << ' ';  //创建时间 修改时间
        ofs << file.second.content.size() << endl;  //文件内容
        for (auto str : file.second.content) ofs << str << endl;
    }
    cout << "\n再见！\n\n";
    Sleep(1000);
}

inline auto ctrlhandler(DWORD fdwctrltype) {  //控制退出程序时执行的动作
    switch (fdwctrltype) {
        case CTRL_C_EVENT:  // CTRL+C
            save();
            return false;
        case CTRL_CLOSE_EVENT:  // 点 X 关闭
            save();
            return false;
        case CTRL_BREAK_EVENT:  // CTRL+BREAK
            save();
            return false;
        case CTRL_LOGOFF_EVENT:  // 用户退出
            save();
            return false;
        case CTRL_SHUTDOWN_EVENT:  // 系统关闭
            save();
            return false;
        default: return false;
    }
}

inline auto inputpsw() {  // 密码输入
    char password[100];
    int index = 0;
    while (true) {
        char ch = getch();
        if (ch == 8) {  // BACKSPACE
            if (index != 0) {
                cout << char(8) << ' ' << char(8);
                --index;
            }
        } else if (ch == '\r') {  // 回车
            password[index] = '\0';
            cout << endl;
            break;
        } else {
            cout << '*';
            password[index++] = ch;
        }
    }
    string res = password;
    return res;
}

inline auto get_time() {  //获取当前时间
    time_t timep;
    time(&timep);
    char tmp[256];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d_%H:%M:%S", localtime(&timep));
    string res = tmp;
    return res;
}

inline auto check() {  //检查指令格式是否正确
    string tmp;
    getline(cin, tmp);
    if (tmp.length() > 0) {
        cout << "指令输入有误，请输入'help'获取帮助！\n";
        return false;
    } else return true;
}

inline auto checkpsw(string para) {  //检查密码是否正确
    string password;
    int cnt = 0;
    while (true) {
        cout << "输入密码: ";
        password = inputpsw();
        if (password != users[para].password) {
            cout << "密码错误，请重新输入！\n";
            ++cnt;
            if (cnt == 3) {
                cout << "尝试次数过多，请稍后再试！\n";
                return false;
            }
        } else return true;
    }
}

inline auto help() {  //帮助
    cout << "NCUT二级文件系统   版本1.0   XXXX  计XX-X  XXXXXXXX\n\n";
    cout << "USERADD [用户名]                             创建用户\n";
    cout << "USERDEL [用户名]                             删除用户\n";
    cout << "SU [用户名]                                  切换用户\n";
    cout << "LOGOUT                                       注销用户\n";
    cout << "FIND [名称]                                  搜索查找\n";
    cout << "DIR                                          目录清单\n";
    cout << "NEW [文件名]                                 新建文件\n";
    cout << "DEL [文件名]                                 删除文件\n";
    cout << "READ [文件名]                                读文件\n";
    cout << "WRITE [文件名]                               写文件\n";
    cout << "REFRESH [文件名]                             截断文件\n";
    cout << "POINTER [文件名]                             文件指针\n";
    cout << "SHARE [文件名] [用户名] [r|w]                共享文件\n";
    cout << "STAT [文件名]                                文件信息\n";
    cout << "RENAME [文件名] [新文件名]                   重命名\n";
    cout << "EXIT                                         退出系统\n";
}

inline auto dir() {  //目录清单
    if (!log_in) cout << "请先登录！\n";
    else {
        for (auto filename : users[USER].filenames) cout << filename.first << endl;
    }
}

inline auto logout() {  //注销用户
    if (!log_in) cout << "当前无用户登录！\n";
    else {
        log_in = false;
        cout << "已注销！\n";
    }
}

inline auto useradd(string para) {  //创建用户
    if (users.size() >= 10) cout << "用户数量已满，无法创建用户！\n";
    else if (users.count(para)) cout << "用户已存在！\n";
    else {
        string password, password1;
        while (true) {
            cout << "输入密码: ";
            password = inputpsw();
            if (password.length() <= 5) cout << "密码太短，请重新输入！\n";
            else break;
        }
        while (true) {
            cout << "确认密码: ";
            password1 = inputpsw();
            if (password1 == password) break;
            else cout << "输入错误，请重新输入！\n";
        }
        users[para] = user(password);
        cout << "用户创建成功，请使用'su'指令登录用户！\n";
    }
}

inline auto del(string para) {  //删除文件
    if (!log_in) cout << "请先登录！\n";
    else if (!users[USER].filenames.count(para)) cout << "文件不存在！\n";
    else {
        files[users[USER].filenames[para]].user_details.erase(USER);
        if (files[users[USER].filenames[para]].user_details.size() == 0)
            files.erase(users[USER].filenames[para]);  //如果文件不属于任何一个用户则彻底删除文件
        users[USER].filenames.erase(para);
        return true;
    }
    return false;
}

inline auto userdel(string para) {  //删除用户
    if (users.size() <= 0) cout << "当前用户数量为0！\n";
    else if (!(users.count(para))) cout << "用户不存在，请重试！\n";
    else if (checkpsw(para)) {
        vector<string> filenames;
        for (auto file : users[para].filenames) filenames.push_back(file.first);
        for (auto filename : filenames) del(filename);  //删除用户所有的文件
        if (log_in && USER == para) log_in = false;
        users.erase(para);  //删除用户
        cout << "用户已删除！\n";
    }
}

inline auto su(string para) {  //切换用户
    if (!(users.count(para))) cout << "用户不存在，请重试！\n";
    else if (log_in && USER == para) cout << "用户已登录！\n";
    else if (checkpsw(para)) {
        USER = para;
        log_in = true;
        cout << "用户登录成功！\n";
    }
}

inline auto find(string para) {  //搜索查找
    cout << "正在查找...\n";
    vector<string> res;
    for (auto username : users) {
        if (username.first.find(para) != string::npos) res.push_back(username.first);
    }
    cout << "查找到 " << res.size() << " 个相关用户" << (res.size() > 0 ? ":\n" : "！\n");
    for (auto str : res) cout << str << endl;
    res.clear();
    if (log_in) {  //查找相关文件
        for (auto filename : users[USER].filenames) {
            if (filename.first.find(para) != string::npos) res.push_back(filename.first);
        }
        cout << "查找到 " << res.size() << " 个相关文件" << (res.size() > 0 ? ":\n" : "！\n");
        for (auto str : res) cout << str << endl;
    } else
        cout << "查找相关文件需要登录用户！\n";
}

inline auto new_f(string para) {  //新建文件
    if (!log_in) cout << "请先登录！\n";
    else if (users[USER].filenames.count(para)) cout << "文件已存在！\n";
    else {
        files[files.size()] = file();
        int pos = files.size() - 1;
        users[USER].filenames[para] = pos;
        files[pos].user_details[USER].write = true;
        files[pos].user_details[USER].filename = para;
        files[pos].user_details[USER].fp.x = 0;
        files[pos].user_details[USER].fp.y = 0;
        files[pos].c_time = files[pos].m_time = get_time();
        cout << "创建文件成功！\n";
    }
}

inline auto read(string para) {  //读文件
    if (!log_in) cout << "请先登录！\n";
    else if (!users[USER].filenames.count(para)) cout << "文件不存在！\n";
    else {
        if(files[users[USER].filenames[para]].content.size() == 0) {
            files[users[USER].filenames[para]].user_details[USER].fp.x = 0;
            files[users[USER].filenames[para]].user_details[USER].fp.y = 0;
            return;
        }  // 重置文件指针
        if (files[users[USER].filenames[para]].user_details[USER].fp.x != 0 &&
                files[users[USER].filenames[para]].user_details[USER].fp.x >= files[users[USER].filenames[para]].content.size() ||
            files[users[USER].filenames[para]].user_details[USER].fp.y >
                files[users[USER].filenames[para]].content[files[users[USER].filenames[para]].user_details[USER].fp.x].length()) {
            files[users[USER].filenames[para]].user_details[USER].fp.x = 0;
            files[users[USER].filenames[para]].user_details[USER].fp.y = 0;
            cout << "文件已被其他用户修改，文件指针已重置！\n" << endl;
        }
        if(files[users[USER].filenames[para]].content[files[users[USER].filenames[para]].user_details[USER].fp.x].length() > 
                files[users[USER].filenames[para]].user_details[USER].fp.y) {
            cout << files[users[USER].filenames[para]].content[files[users[USER].filenames[para]].user_details[USER].fp.x]
                    .substr(files[users[USER].filenames[para]].user_details[USER].fp.y) << endl;
        } //  读取并打印文件内容
        for (int i =
                 files[users[USER].filenames[para]].user_details[USER].fp.x + 1;
             i < files[users[USER].filenames[para]].content.size(); ++i)
            cout << files[users[USER].filenames[para]].content[i] << endl;
    }
}

inline auto write(string para) {  //写文件
    if (!log_in) cout << "请先登录！\n";
    else if (!users[USER].filenames.count(para)) cout << "文件不存在！\n";
    else if (files[users[USER].filenames[para]].user_details[USER].write == false) cout << "没有写入权限！\n";
    else {
        if(! files[users[USER].filenames[para]].content.size()) {
            files[users[USER].filenames[para]].user_details[USER].fp.x = 0;
            files[users[USER].filenames[para]].user_details[USER].fp.y = 0;
        } else if (files[users[USER].filenames[para]].user_details[USER].fp.x != 0 && files[users[USER].filenames[para]].user_details[USER].fp.x >= files[users[USER].filenames[para]].content.size() ||
            files[users[USER].filenames[para]].user_details[USER].fp.y >
                files[users[USER].filenames[para]].content[files[users[USER].filenames[para]].user_details[USER].fp.x].length()) {
            files[users[USER].filenames[para]].user_details[USER].fp.x = 0;
            files[users[USER].filenames[para]].user_details[USER].fp.y = 0;
            cout << "文件已被其他用户修改，文件指针已重置！\n" << endl;
        }  // 重置文件指针
        cout << "请在下方输入文件内容，输入'0'结束写入文件，输入'1'"
                "取消写入文件：\n";
        vector<string> res;
        string tmp;
        while (true) {
            getline(cin, tmp);
            if (tmp == "0" || tmp == "1") break;
            res.push_back(tmp);
        }
        if (tmp == "0") { // 在文件指针的位置开始写入文件
            int pos = files[users[USER].filenames[para]].user_details[USER].fp.x + 1;
            if (!files[users[USER].filenames[para]].content.size())
                    files[users[USER].filenames[para]].content.push_back("");
            if (files[users[USER].filenames[para]].content[files[users[USER].filenames[para]].user_details[USER].fp.x].length() ==
                files[users[USER].filenames[para]].user_details[USER].fp.y) {
                files[users[USER].filenames[para]].content[files[users[USER].filenames[para]].user_details[USER].fp.x] += res[0];
            } else if (files[users[USER].filenames[para]].user_details[USER].fp.y == 0) {
                files[users[USER].filenames[para]].content.insert(files[users[USER].filenames[para]].content.begin() +
                        files[users[USER].filenames[para]].user_details[USER].fp.x,res[0]);
            } else {
                files[users[USER].filenames[para]].content[files[users[USER].filenames[para]].user_details[USER].fp.x] =
                    files[users[USER].filenames[para]].content[files[users[USER].filenames[para]].user_details[USER].fp.x]
                        .substr(0, files[users[USER].filenames[para]].user_details[USER].fp.y) + res[0] +
                    files[users[USER].filenames[para]].content[files[users[USER].filenames[para]].user_details[USER].fp.x]
                        .substr(files[users[USER].filenames[para]].user_details[USER].fp.y);
            }
            for (int i = 1; i < res.size(); ++i, ++pos)
                files[users[USER].filenames[para]].content.insert(
                    files[users[USER].filenames[para]].content.begin() + pos, res[i]);
            files[users[USER].filenames[para]].m_time = get_time();
            cout << "写入文件成功！\n";
        } else cout << "取消写入文件！\n";
    }
}

inline auto stat(string para) {  //文件信息
    if (!log_in) cout << "请先登录！\n";
    else if (!users[USER].filenames.count(para)) cout << "文件不存在！\n";
    else {
        for (auto user : files[users[USER].filenames[para]].user_details) {
            cout << "用户：" << user.first << "    权限：可读" << (user.second.write ? "可写\n" : "\n");
        }
        cout << "创建时间：" << files[users[USER].filenames[para]].c_time
             << "    修改时间：" << files[users[USER].filenames[para]].m_time
             << endl;
        int len = 0;
        for (auto str : files[users[USER].filenames[para]].content) len += str.length();
        cout << "文件长度：" << len << endl;
    }
}

inline auto refresh(string para) {  //截断文件
    if (!log_in) cout << "请先登录！\n";
    else if (!users[USER].filenames.count(para)) cout << "文件不存在！\n";
    else if (files[users[USER].filenames[para]].user_details[USER].write == false) cout << "没有截断权限！\n";
    else {
        files[users[USER].filenames[para]].content.clear();
        files[users[USER].filenames[para]].user_details[USER].fp.x = 0;
        files[users[USER].filenames[para]].user_details[USER].fp.y = 0;
        cout << "文件已截断！\n";
    }
}

inline auto pointer(string para) {  //文件指针
    if (!log_in) cout << "请先登录！\n";
    else if (!users[USER].filenames.count(para)) cout << "文件不存在！\n";
    else if (!files[users[USER].filenames[para]].content.size()) cout << "文件为空，无法设置指针！\n";
    else {
        int x, y, X = files[users[USER].filenames[para]].content.size() - 1, Y;
        while (true) {  // 设置 X 指针
            cout << "输入X(0 -- " << X << ")：";
            cin >> x;
            if (x >= 0 && x <= X) break;
            else cout << "输入错误！\n";
        }
        Y = files[users[USER].filenames[para]].content[x].length();
        while (true) {  // 设置 Y 指针
            cout << "输入Y(0 -- " << Y << ")：";
            cin >> y;
            if (y >= 0 && y <= Y) break;
            else cout << "输入错误！\n";
        }
        files[users[USER].filenames[para]].user_details[USER].fp.x = x;
        files[users[USER].filenames[para]].user_details[USER].fp.y = y;
        cout << "文件指针已设置！\n";
    }
}

inline auto rename(string para1, string para2) {  //重命名
    if (!log_in) cout << "请先登录！\n";
    else if (!users[USER].filenames.count(para1)) cout << "文件不存在！\n";
    else if (para1 == para2) cout << "新文件名与当前文件名重复！\n";
    else if (users[USER].filenames.count(para2)) cout << "新文件名已被占用，请重试！\n";
    else {
        users[USER].filenames[para2] = users[USER].filenames[para1];
        users[USER].filenames.erase(para1);
        files[users[USER].filenames[para2]].user_details[USER].filename = para2;
        cout << "文件名已修改！\n";
    }
}

inline auto share(string para1, string para2, string para3) {  //共享文件
    if (!log_in) cout << "请先登录！\n";
    else if (!users[USER].filenames.count(para1)) cout << "文件不存在！\n";
    else if (!users.count(para2)) cout << "用户不存在！\n";
    else if (para3 != "r" && para3 != "w") cout << "指令输入有误，请输入'help'获取帮助！\n";
    else if (para3 == "w" && !files[users[USER].filenames[para1]].user_details[USER].write) cout << "无共享权限！\n";
    else {
        if (files[users[USER].filenames[para1]].user_details.count(para2)) {  //用户已有该文件
            if (para3 == "w" && files[users[USER].filenames[para1]].user_details[USER].write) {
                files[users[USER].filenames[para1]].user_details[para2].write = true;
            }
        } else {
            string filename = para1;
            if (users[para2].filenames.count(para1))  //文件名已被占用
                filename += char('0' + (rand() % 10));
            users[para2].filenames[filename] = users[USER].filenames[para1];
            files[users[USER].filenames[para1]].user_details[para2].filename = filename;
            files[users[USER].filenames[para1]].user_details[para2].write = (para3 == "w" ? true : false);
            files[users[USER].filenames[para1]].user_details[para2].fp.x = 0;
            files[users[USER].filenames[para1]].user_details[para2].fp.y = 0;
        }
        cout << "文件已共享！\n";
    }
}

inline auto to_lower(string &str) {  //将string中的大写字母转为小写字母
    for (int i = 0; i < str.length(); ++i) {
        if (isupper(str[i])) str[i] = tolower(str[i]);
    }
}

inline auto op() {  //指令处理
    cout << "HELLO! 现在是" << get_time() << "， 输入'help'获取帮助\n\n";
    string op, para1, para2, para3;
    while (true) {
        cout << (log_in ? USER : "NONE") << "@NCUT: ";
        cin >> op;
        to_lower(op);
        if ((op == "help" || op == "exit" || op == "dir" || op == "logout")) {
            if (check()) {
                if (op == "help") help();
                else if (op == "exit") {
                    save();
                    return;
                } else if (op == "dir") dir();
                else if (op == "logout") logout();
            }
        } else if (op == "useradd" || op == "userdel" || op == "su" ||
                   op == "find" || op == "new" || op == "del" || op == "read" ||
                   op == "write" || op == "stat" || op == "refresh" || op == "pointer") {
            cin >> para1;
            if (check()) {
                if (op == "useradd") useradd(para1);
                else if (op == "userdel") userdel(para1);
                else if (op == "su") su(para1);
                else if (op == "find") find(para1);
                else if (op == "new") new_f(para1);
                else if (op == "del") {
                    if (del(para1)) cout << "文件已删除！\n";
                } else if (op == "read") read(para1);
                else if (op == "write") write(para1);
                else if (op == "stat") stat(para1);
                else if (op == "refresh") refresh(para1);
                else if (op == "pointer") pointer(para1);
            }
        } else if (op == "rename") {
            cin >> para1 >> para2;
            if (check()) rename(para1, para2);
        } else if (op == "share") {
            cin >> para1 >> para2 >> para3;
            if (check()) share(para1, para2, para3);
        } else {
            cout << "'" << op << "' 不是内部命令，请重新输入\n";
            getline(cin, op);
        }
        cout << endl;
    }
}

int main() {
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrlhandler, true);  //安装事件钩子 调用Win32 API
    init();                       //初始化
    op();
    system("pause");
    return 0;
}