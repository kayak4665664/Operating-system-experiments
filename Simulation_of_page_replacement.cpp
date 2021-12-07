#include <time.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <vector>
#define FIFO 0
#define LRU 1
using namespace std;

int N;
vector<int> pages;            //页面访问序列
vector<bool> pages_modified;  //页面修改序列
int page_frame[10][110], record[10];
//页框
inline auto initialize() {
    srand((unsigned)time(NULL));  //初始化随机数种子
    for (int i = 0; i < N * 7; ++i) {
        pages.push_back(rand() % (N * 3) + 1);  //随机生成页面访问序列
        pages_modified.push_back(rand() % 2);   //随机生成页面修改序列
    }
    printf("\n页面访问序列如下：\n");
    for (auto i : pages) printf("%-4d", i);
    puts("\n");
}

inline auto print(char *str, int ft) {  //打印页面置换情况
    printf("%s页面置换算法如下：\n\n", str);
    printf(" 序列：");
    for (int i = 0; i < pages.size(); ++i) printf("%-4d", pages[i]);
    printf("\n-----");
    for (int i = 0; i < pages.size(); ++i) printf("----");
    for (int i = 0; i < N; ++i) {
        printf("\n页框%d：", i + 1);
        for (int j = 0; j < pages.size(); ++j) {
            if (page_frame[i][j])
                printf("%-4d", page_frame[i][j]);
            else
                printf("-   ");
        }
    }
    printf("\n缺页率 = %d / %d = %.2f%%\n", ft, pages.size(),
           (float)ft / pages.size() * 100);  //计算缺页率
    puts("");
}

inline auto FIFO_LRU(int mode) {  //先进先出算法
    int fault_rate = 0;           //记录缺页次数
    memset(page_frame, 0, sizeof(page_frame));
    memset(record, 0, sizeof(record));
    for (int i = 0; i < pages.size(); ++i) {
        auto isok = false;
        for (int j = 0; j < N; ++j) {
            if (!page_frame[j][i]) {  //页框为空
                page_frame[j][i] = pages[i];
                record[j] = i;  //记录页面装入/访问时间
                isok = true;
                ++fault_rate;
                break;
            } else if (page_frame[j][i] == pages[i]) {  //页框中已有页面
                isok = true;
                if (mode == LRU) record[j] = i;  // LRU算法，更新访问时间
                break;
            }
        }
        if (!isok) {  //置换页面
            ++fault_rate;
            int min = 1000, pos;
            for (int k = 0; k < N; ++k) {
                if (record[k] < min) {
                    min = record[k];
                    pos = k;  //找出最早装入/近期最少使用的页面
                }
            }
            record[pos] = i;  //更新装入/访问时间
            page_frame[pos][i] = pages[i];
        }
        for (int j = 0; j < N; ++j) page_frame[j][i + 1] = page_frame[j][i];
    }
    print(mode == FIFO ? "先进先出" : "近期最少使用", fault_rate);
}

inline auto OPT() {      //最优淘汰算法
    int fault_rate = 0;  //记录缺页次数
    memset(page_frame, 0, sizeof(page_frame));
    for (int i = 0; i < pages.size(); ++i) {
        auto isok = false;
        for (int j = 0; j < N; ++j) {
            if (!page_frame[j][i]) {  //页框为空
                page_frame[j][i] = pages[i];
                isok = true;
                ++fault_rate;
                break;
            } else if (page_frame[j][i] == pages[i]) {  //页框中已有页面
                isok = true;
                break;
            }
        }
        if (!isok) {  //置换页面
            ++fault_rate;
            unordered_map<int, int> forecast_pages;
            for (int j = 0; j < N; ++j) forecast_pages[page_frame[j][i]] = j;
            for (int pos = i + 1; pos < pages.size();
                 ++pos) {  //找出不被访问或者长时间才被访问的页面
                if (forecast_pages.count(pages[pos]))
                    forecast_pages.erase(pages[pos]);
                if (forecast_pages.size() == 1) break;
            }
            for (auto j : forecast_pages) {
                page_frame[j.second][i] = pages[i];
                break;
            }
        }
        for (int j = 0; j < N; ++j) page_frame[j][i + 1] = page_frame[j][i];
    }
    print("最优淘汰", fault_rate);
}

inline auto CLOCK() {    // CLOCK置换算法
    int fault_rate = 0;  //记录缺页次数
    int pointer = 0;     //记录区指针
    memset(page_frame, 0, sizeof(page_frame));
    memset(record, 0, sizeof(record));
    for (int i = 0; i < pages.size(); ++i) {
        auto isok = false;
        for (int j = 0; j < N; ++j) {  //访问页面
            if (page_frame[j][i] == pages[i]) {
                isok = true;
                record[j] = 1;
                break;
            }
        }
        if (!isok) {  //置换页面
            ++fault_rate;
            int cnt = 0;
            for (int j = 0; j < N; ++j) cnt += record[j];
            if (cnt == N) memset(record, 0, sizeof(record));  //重置访问记录区
            if (record[pointer]) {
                while (true) {
                    ++pointer;
                    if (pointer >= N) pointer = 0;
                    if (!record[pointer]) break;
                }
            }
            page_frame[pointer][i] = pages[i];
            record[pointer] = 1;
            ++pointer;  //修改访问指针
            if (pointer >= N) pointer = 0;
        }
        for (int j = 0; j < N; ++j) page_frame[j][i + 1] = page_frame[j][i];
    }
    print("CLOCK置换", fault_rate);
}

inline auto CLOCK_improved() {  //改进CLOCK置换算法
    int fault_rate = 0;         //记录缺页次数
    int pointer = 0;            //访问区指针
    int modified[10];           //修改区
    memset(page_frame, 0, sizeof(page_frame));
    memset(record, 0, sizeof(record));
    memset(modified, 0, sizeof(modified));
    for (int i = 0; i < pages.size(); ++i) {
        auto isok = false;
        for (int j = 0; j < N; ++j) {  //访问页面
            if (page_frame[j][i] == pages[i]) {
                isok = true;
                record[j] = 1;
                if (pages_modified[i])
                    modified[j] = 1;
                else
                    modified[j] = 0;
                break;
            }
        }
        if (!isok) {  //置换页面
            ++fault_rate;
            int cnt = 0;
            while (true) {  //寻找未被访问未被修改的页面
                ++cnt;
                if (!record[pointer] && !modified[pointer]) {
                    page_frame[pointer][i] = pages[i];
                    record[pointer] = 1;
                    if (pages_modified[i])
                        modified[pointer] = 1;
                    else
                        modified[pointer] = 0;
                    isok = true;
                    ++pointer;
                    if (pointer >= N) pointer = 0;
                    break;
                }
                ++pointer;
                if (pointer >= N) pointer = 0;
                if (cnt == N) break;
            }
            cnt = 0;
            if (!isok) {  //寻找未被访问已被修改的页面
                while (true) {
                    ++cnt;
                    if (!record[pointer] && modified[pointer]) {
                        page_frame[pointer][i] = pages[i];
                        record[pointer] = 1;
                        if (pages_modified[i])
                            modified[pointer] = 1;
                        else
                            modified[pointer] = 0;
                        isok = true;
                        ++pointer;
                        if (pointer >= N) pointer = 0;
                        break;
                    }
                    record[pointer] = 0;  //将所有扫描过的访问位置0
                    ++pointer;
                    if (pointer >= N) pointer = 0;
                    if (cnt == N) break;
                }
            }
            cnt = 0;
            if (!isok) {  //重复寻找未被访问未被修改的页面
                while (true) {
                    ++cnt;
                    if (!record[pointer] && !modified[pointer]) {
                        page_frame[pointer][i] = pages[i];
                        record[pointer] = 1;
                        if (pages_modified[i])
                            modified[pointer] = 1;
                        else
                            modified[pointer] = 0;
                        isok = true;
                        ++pointer;
                        if (pointer >= N) pointer = 0;
                        break;
                    }
                    ++pointer;
                    if (pointer >= N) pointer = 0;
                    if (cnt == N) break;
                }
            }
            if (!isok) {  //重复寻找未被访问已被修改的页面
                while (true) {
                    if (!record[pointer] && modified[pointer]) {
                        page_frame[pointer][i] = pages[i];
                        record[pointer] = 1;
                        if (pages_modified[i])
                            modified[pointer] = 1;
                        else
                            modified[pointer] = 0;
                        isok = true;
                        ++pointer;
                        if (pointer >= N) pointer = 0;
                        break;
                    }
                    ++pointer;
                    if (pointer >= N) pointer = 0;
                }
            }
        }
        for (int j = 0; j < N; ++j) page_frame[j][i + 1] = page_frame[j][i];
    }
    print("改进型CLOCK置换", fault_rate);
}

inline auto RAND() {     //随机置换算法
    int fault_rate = 0;  //记录缺页次数
    memset(page_frame, 0, sizeof(page_frame));
    for (int i = 0; i < pages.size(); ++i) {
        auto isok = false;
        for (int j = 0; j < N; ++j) {
            if (!page_frame[j][i]) {  //页框为空
                page_frame[j][i] = pages[i];
                isok = true;
                ++fault_rate;
                break;
            } else if (page_frame[j][i] == pages[i]) {  //页框中已有页面
                isok = true;
                break;
            }
        }
        if (!isok) {  //置换页面
            ++fault_rate;
            int pos = rand() % N;
            page_frame[pos][i] = pages[i];
        }
        for (int j = 0; j < N; ++j) page_frame[j][i + 1] = page_frame[j][i];
    }
    print("随机置换", fault_rate);
}

auto main() -> int {
    printf("请输入页框数量：");
    scanf("%d", &N);
    initialize();      // 初始化
    FIFO_LRU(FIFO);    // 先进先出算法
    FIFO_LRU(LRU);     // 最近最久未使用页面淘汰算法
    OPT();             // 最优淘汰算法
    CLOCK();           // CLOCK置换算法
    CLOCK_improved();  // 改进型CLOCK置换算法
    RAND();            // 随机置换算法
    system("pause");
    return 0;
}