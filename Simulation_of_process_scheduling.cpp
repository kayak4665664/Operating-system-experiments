#include <time.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace std;

struct PCB {
    int id;                           //作业编号
    int submission_time;              //提交时间
    int run_time;                     //运行时间
    int start_time;                   //开始时间
    int complete_time;                //完成时间
    int cycling_time;                 //周转时间
    int progress_rate;                //运行进度
    int priority;                     //优先级
    double weighted_turnaround_time;  //带权周转时间
    double response_ratio;            //响应比
    inline PCB(int s_t, int r_t, int p)
        : submission_time(s_t), run_time(r_t), priority(p) {
        id = start_time = complete_time = cycling_time =
            weighted_turnaround_time = response_ratio = progress_rate = 0;
    }  // PCB构造函数
    PCB() {}
};

inline auto cmp(const PCB a, const PCB b) {
    return a.submission_time < b.submission_time;
}  // 自定义比较函数 按提交时间从早到晚排序

int N, current_time;  // 记录当前的时间
vector<PCB> jobs;     // N个进程

inline auto initialize() {        //初始化进程
    srand((unsigned)time(NULL));  //初始化随机数种子
    printf("作业  提交时间  运行时间\n");
    for (int i = 1; i <= N; ++i) {  //随机生成N个进程
        jobs.push_back(PCB(rand() % 15, rand() % 15 + 1, rand() % N + 1));
    }
    stable_sort(jobs.begin(), jobs.end(),
                cmp);  //将N个进程按照提交时间从早到晚排序
    for (int i = 1; i <= N; ++i) {
        jobs[i - 1].id = i;
        printf("%-4d  %-8d  %-8d\n", jobs[i - 1].id,
               jobs[i - 1].submission_time, jobs[i - 1].run_time);
    }
}

inline auto run(int &current_time, PCB &current_job) {  // 模拟进程的执行
    current_job.start_time = current_time;              //确定开始时间
    current_job.complete_time =
        current_time + current_job.run_time;  //确定完成时间
    current_time += current_job.run_time;     //修改当前时间
}

inline auto run(int &current_time, PCB &current_job, double &ave_ct,
                double &ave_wtt) {  // 模拟进程的执行
    current_job.cycling_time =
        current_time - current_job.submission_time;  //确定周转时间
    current_job.weighted_turnaround_time =
        (double)current_job.cycling_time /
        current_job.run_time;  //计算带权周转时间
    printf("%-4d  %-8d  %-8d  %-8d  %-8d  %-8d  %.2lf\n", current_job.id,
           current_job.submission_time, current_job.run_time,
           current_job.start_time, current_job.complete_time,
           current_job.cycling_time,
           current_job.weighted_turnaround_time);  //打印进程调度情况
    ave_ct += current_job.cycling_time;            //记录平均周转时间
    ave_wtt += current_job.weighted_turnaround_time;  //记录平均带权周转时间
}

inline auto print(double &ave_ct, double &ave_wtt) {  //格式化打印
    printf("平均周转时间：%.2lf\n", ave_ct / N);
    printf("平均带权周转时间：%.2lf\n\n", ave_wtt / N);
}

inline auto FCFS() {  //先来先服务算法
    auto tmp_jobs = jobs;
    double ave_ct = 0.0, ave_wtt = 0.0;
    puts("\n先来先服务算法调度过程如下：\n");
    printf(
        "作业  提交时间  运行时间  开始时间  完成时间  周转时间  "
        "带权周转时间\n");
    for (int i = 0; i < N; ++i) {
        if (current_time < tmp_jobs[i].submission_time)
            current_time = tmp_jobs[i].submission_time;
        run(current_time, tmp_jobs[i]);                   //执行进程
        run(current_time, tmp_jobs[i], ave_ct, ave_wtt);  //执行进程
    }
    print(ave_ct, ave_wtt);  //格式化打印
}

inline auto SJF() {  //短作业优先算法
    current_time = 0;
    auto tmp_jobs = jobs;
    PCB current_job;
    double ave_ct = 0.0, ave_wtt = 0.0;
    puts("\n短作业优先算法调度过程如下：\n");
    printf(
        "作业  提交时间  运行时间  开始时间  完成时间  周转时间  "
        "带权周转时间\n");
    while (!tmp_jobs.empty()) {
        int min_run_time = 10000, pos = 10000;
        for (auto i = 0; i < tmp_jobs.size(); ++i) {
            if (tmp_jobs[i].submission_time <= current_time &&
                tmp_jobs[i].run_time < min_run_time) {  //找出当前最短作业
                current_job = tmp_jobs[i];
                pos = i;
                min_run_time = tmp_jobs[i].run_time;
            }
        }
        if (pos != 10000) {
            tmp_jobs.erase(tmp_jobs.begin() + pos);
            run(current_time, current_job);  //执行当前最短作业
            run(current_time, current_job, ave_ct, ave_wtt);
        } else
            ++current_time;  //如果未找出，则将时间+1
    }
    print(ave_ct, ave_wtt);
}

inline auto HRN() {  //最高相应比优先算法
    current_time = 0;
    auto tmp_jobs = jobs;
    PCB current_job;
    double ave_ct = 0.0, ave_wtt = 0.0;
    puts("\n最高相应比优先算法调度过程如下：\n");
    printf(
        "作业  提交时间  运行时间  开始时间  完成时间  周转时间  "
        "带权周转时间\n");
    while (!tmp_jobs.empty()) {
        int pos = 10000;
        double max_response_ratio = 0.0;
        for (auto i = 0; i < tmp_jobs.size(); ++i) {
            if (tmp_jobs[i].submission_time <=
                current_time) {  //找出当前响应比最高的进程
                tmp_jobs[i].response_ratio =
                    1 + (double)(current_time - tmp_jobs[i].submission_time) /
                            tmp_jobs[i].run_time;  //计算响应比
                if (tmp_jobs[i].response_ratio > max_response_ratio) {
                    pos = i;
                    max_response_ratio = tmp_jobs[i].response_ratio;
                    current_job = tmp_jobs[i];
                }
            }
        }
        if (pos != 10000) {
            tmp_jobs.erase(tmp_jobs.begin() + pos);
            run(current_time, current_job);  //执行当前响应比最高的进程
            run(current_time, current_job, ave_ct, ave_wtt);
        } else
            ++current_time;  //如果未找出，则将时间+1
    }
    print(ave_ct, ave_wtt);
}

inline auto RR() {  //轮转法
    current_time = 0;
    auto tmp_jobs = jobs;
    double ave_ct = 0.0, ave_wtt = 0.0;
    int time_slice;  //时间片
    int pos = 0;     //记录位置
    printf("请输入轮转法时间片大小：");
    scanf("%d", &time_slice);
    puts("\n轮转法调度过程如下：\n");
    printf(
        "作业  提交时间  运行时间  开始时间  完成时间  周转时间  "
        "带权周转时间\n");
    while (!tmp_jobs.empty()) {
        bool isok = false;
        for (auto i = pos; i < tmp_jobs.size();) {
            if (tmp_jobs[i].submission_time <= current_time) {
                isok = true;
                if (!tmp_jobs[i].progress_rate)
                    tmp_jobs[i].start_time = current_time;  //确定开始时间
                if (time_slice <
                    (tmp_jobs[i].run_time -
                     tmp_jobs[i].progress_rate)) {  //执行一个时间片
                    tmp_jobs[i].progress_rate += time_slice;
                    current_time += time_slice;
                    ++i;
                    pos = 0;
                } else {  //剩余时间不足一个时间片
                    current_time += tmp_jobs[i].run_time -
                                    tmp_jobs[i].progress_rate;  //确定当前时间
                    tmp_jobs[i].complete_time = current_time;  //确定完成时间
                    run(current_time, tmp_jobs[i], ave_ct,
                        ave_wtt);                  //当前进程结束
                    if (i == tmp_jobs.size() - 1)  //记录下一个进程的位置
                        pos = 0;
                    else
                        pos = i;
                    tmp_jobs.erase(tmp_jobs.begin() + i);
                    break;
                }
            } else
                ++i;
        }
        if (!isok) ++current_time;  //如果未找出，则将时间+1
    }
    print(ave_ct, ave_wtt);
}

inline auto HPF_static() {  //静态优先级法
    current_time = 0;
    auto tmp_jobs = jobs;
    PCB current_job;
    double ave_ct = 0.0, ave_wtt = 0.0;
    int time_slice;  //时间片
    puts("\n静态优先级法调度过程如下：\n");
    printf(
        "作业  优先级  提交时间  运行时间  开始时间  完成时间  周转时间  "
        "带权周转时间\n");
    while (!tmp_jobs.empty()) {
        int max_priority = -10000, pos = 10000;
        for (auto i = 0; i < tmp_jobs.size(); ++i) {
            if (tmp_jobs[i].submission_time <= current_time &&
                tmp_jobs[i].priority > max_priority) {  //找出当前最高优先级作业
                current_job = tmp_jobs[i];
                pos = i;
                max_priority = tmp_jobs[i].priority;
            }
        }
        if (pos != 10000) {
            tmp_jobs.erase(tmp_jobs.begin() + pos);
            run(current_time, current_job);  //执行当前最高优先级作业
            current_job.cycling_time =
                current_time - current_job.submission_time;  //确定周转时间
            current_job.weighted_turnaround_time =
                (double)current_job.cycling_time /
                current_job.run_time;  //计算带权周转时间
            printf("%-4d  %-6d  %-8d  %-8d  %-8d  %-8d  %-8d  %.2lf\n",
                   current_job.id, current_job.priority,
                   current_job.submission_time, current_job.run_time,
                   current_job.start_time, current_job.complete_time,
                   current_job.cycling_time,
                   current_job.weighted_turnaround_time);  //打印进程调度情况
            ave_ct += current_job.cycling_time;  //记录平均周转时间
            ave_wtt +=
                current_job.weighted_turnaround_time;  //记录平均带权周转时间
        } else
            ++current_time;  //如果未找出，则将时间+1
    }
    print(ave_ct, ave_wtt);
}

inline auto HPF_dynamic() {  //动态优先级法
    current_time = 0;
    auto tmp_jobs = jobs;
    double ave_ct = 0.0, ave_wtt = 0.0;
    int time_slice;  //时间片
    printf("请输入时间片大小：");
    scanf("%d", &time_slice);
    puts("\n动态优先级法调度过程如下：\n");
    printf(
        "作业  优先级  提交时间  运行时间  开始时间  完成时间  周转时间  "
        "带权周转时间\n");
    while (!tmp_jobs.empty()) {
        bool isok = false;
        int max_priority = -10000, pos = 10000;
        for (auto i = 0; i < tmp_jobs.size(); ++i) {
            if (tmp_jobs[i].submission_time <= current_time &&
                tmp_jobs[i].priority > max_priority) {  //找出当前最高优先级作业
                pos = i;
                max_priority = tmp_jobs[i].priority;
            }
        }
        if (pos != 10000) {
            if (!tmp_jobs[pos].progress_rate)
                tmp_jobs[pos].start_time = current_time;  //确定开始时间
            if (time_slice < (tmp_jobs[pos].run_time -
                              tmp_jobs[pos].progress_rate)) {  //执行一个时间片
                tmp_jobs[pos].progress_rate += time_slice;
                current_time += time_slice;
            } else {  //剩余时间不足一个时间片
                current_time += tmp_jobs[pos].run_time -
                                tmp_jobs[pos].progress_rate;  //修改当前时间
                tmp_jobs[pos].complete_time = current_time;  //确定完成时间
                tmp_jobs[pos].cycling_time =
                    current_time -
                    tmp_jobs[pos].submission_time;  //确定周转时间
                tmp_jobs[pos].weighted_turnaround_time =
                    (double)tmp_jobs[pos].cycling_time /
                    tmp_jobs[pos].run_time;  //计算带权周转时间
                printf(
                    "%-4d  %-6d  %-8d  %-8d  %-8d  %-8d  %-8d  %.2lf\n",
                    tmp_jobs[pos].id, tmp_jobs[pos].priority,
                    tmp_jobs[pos].submission_time, tmp_jobs[pos].run_time,
                    tmp_jobs[pos].start_time, tmp_jobs[pos].complete_time,
                    tmp_jobs[pos].cycling_time,
                    tmp_jobs[pos].weighted_turnaround_time);  //打印进程调度情况
                ave_ct += tmp_jobs[pos].cycling_time;  //记录平均周转时间
                ave_wtt +=
                    tmp_jobs[pos]
                        .weighted_turnaround_time;  //记录平均带权周转时间
                tmp_jobs.erase(tmp_jobs.begin() + pos);
            }
            for (auto i = 0; i < tmp_jobs.size(); ++i) {
                if (tmp_jobs[i].submission_time <= current_time) {  //修改优先级
                    if (i == pos)
                        --tmp_jobs[i].priority;  //占有CPU时间越长，优先级越低
                    else
                        ++tmp_jobs[i].priority;  //等待时间越长，优先级越高
                }
            }
        } else
            ++current_time;  //如果未找出，则将时间+1
    }
    print(ave_ct, ave_wtt);
}

inline auto Multi_level_feedback_RR() {  //多级反馈轮转法
    current_time = 0;
    auto tmp_jobs = jobs;
    double ave_ct = 0.0, ave_wtt = 0.0;
    int time_slice;   //时间片
    int ready_queue;  //就绪队列的数量
    int job_cnt = 0;  //进程计数器
    printf("请输入时间片大小：");
    scanf("%d", &time_slice);
    printf("请输入就绪队列数量：");
    scanf("%d", &ready_queue);
    vector<PCB> ready_queues[100];  //就绪队列
    puts("\n多级反馈轮转法调度过程如下：\n");
    printf(
        "作业  提交时间  运行时间  开始时间  完成时间  周转时间  "
        "带权周转时间\n");
    while (job_cnt != N) {
        for (int i = 0; i < tmp_jobs.size();) {  //将新进程放入第一个就绪队列
            if (tmp_jobs[i].submission_time <= current_time) {
                ready_queues[0].push_back(tmp_jobs[i]);
                tmp_jobs.erase(tmp_jobs.begin() + i);
            } else
                ++i;
        }
        bool isok = false;
        int current_time_slice = time_slice;
        for (int i = 0; i < ready_queue; ++i) {
            if (ready_queues[i].size()) {
                isok = true;
                if (!ready_queues[i][0].progress_rate)
                    ready_queues[i][0].start_time =
                        current_time;  //确定开始时间
                if (current_time_slice <
                    ready_queues[i][0].run_time -
                        ready_queues[i][0]
                            .progress_rate) {  //在一个时间片内不能执行完
                    ready_queues[i][0].progress_rate += current_time_slice;
                    current_time += current_time_slice;  //修改当前时间
                    if (i != ready_queue - 1) {  //将进程转入下一个进程末尾
                        ready_queues[i + 1].push_back(ready_queues[i][0]);
                        ready_queues[i].erase(ready_queues[i].begin());
                    } else {  //进入最后一个队列末尾
                        ready_queues[i].push_back(ready_queues[i][0]);
                        ready_queues[i].erase(ready_queues[i].begin());
                    }
                } else {
                    current_time +=
                        ready_queues[i][0].run_time -
                        ready_queues[i][0].progress_rate;  //修改当前时间
                    ready_queues[i][0].complete_time =
                        current_time;  //确定完成时间
                    run(current_time, ready_queues[i][0], ave_ct,
                        ave_wtt);  //当前进程结束
                    ready_queues[i].erase(ready_queues[i].begin());  //进程撤离
                    ++job_cnt;  //增加进程计数器
                }
                break;
            }
            current_time_slice *= 2;  //增大时间片
        }
        if (!isok) ++current_time;  //如果未找出，则将时间+1
    }
    print(ave_ct, ave_wtt);
}

auto main() -> int {
    printf("请输入进程的数量：");
    scanf("%d", &N);
    puts("");
    initialize();               //初始化进程
    FCFS();                     //先来先服务算法
    SJF();                      //短作业优先算法
    HRN();                      //最高相应比优先算法
    RR();                       //轮转法
    HPF_static();               //静态优先级法
    HPF_dynamic();              //动态优先级法
    Multi_level_feedback_RR();  //多级反馈轮转法
    system("pause");
    return 0;
}